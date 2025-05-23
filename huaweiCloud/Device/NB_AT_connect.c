/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "decode.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_uart.h"
#include "hi_uart.h"
#include "iot_watchdog.h"
#include "iot_errno.h"
#include "Uart_Function.h"
#include "MessUp.h"

#define UART_BUFF_SIZE 1024
#define U_SLEEP_TIME   100000
extern SER;
extern NB_buffer[UART_BUFF_SIZE];
char observe[100] = {0};
char discover[100] = {0};

void Uart2GpioInit(void)
{
    IoTGpioInit(IOT_IO_NAME_GPIO_11);
    // 设置GPIO0的管脚复用关系为UART1_TX Set the pin reuse relationship of GPIO0 to UART1_ TX
    IoSetFunc(IOT_IO_NAME_GPIO_11, IOT_IO_FUNC_GPIO_11_UART2_TXD);
    IoTGpioInit(IOT_IO_NAME_GPIO_12);
    // 设置GPIO1的管脚复用关系为UART1_RX Set the pin reuse relationship of GPIO1 to UART1_ RX
    IoSetFunc(IOT_IO_NAME_GPIO_12, IOT_IO_FUNC_GPIO_12_UART2_RXD);
}

void Uart2Config(void)
{
    uint32_t ret;
    /* 初始化UART配置，波特率 9600，数据bit为8,停止位1，奇偶校验为NONE */
    /* Initialize UART configuration, baud rate is 9600, data bit is 8, stop bit is 1, parity is NONE */
    IotUartAttribute uart_attr = {
        .baudRate = 9600,
        .dataBits = 8,
        .stopBits = 1,
        .parity = 0,
    };
    ret = IoTUartInit(HI_UART_IDX_2, &uart_attr);
    if (ret != IOT_SUCCESS) {
        printf("Init Uart2 Falied Error No : %d\n", ret);
        return;
    }else{
        printf("Init Uart2 Success : %d\n", ret);
    }
}

// 先补空格，再把字符串转化为16进制ascll码
void StringToHexASCII(const char *input, int inputLength, char *output, int outputLength) {
	// 清空输出字符串
	memset(output, 0, outputLength);

	// 用于存储当前写入位置
	int pos = 0;

	int leadingZeros = outputLength - inputLength * 2; // 需要补充的零的数量
	for (int i = 0; i < leadingZeros; i += 2) {
		output[pos++] = '2';
		output[pos++] = '0'; // 添加一个前导20
	}

	// 转换每个字符为 16 进制 ASCII 并写入输出
	for (int i = 0; i < inputLength; i++) {
		// 将字符转换为 16 进制字符串
		pos += snprintf(output + pos, outputLength - pos + 1, "%02X", (unsigned char)input[i]);
		//printf("pos:%d ,input[%d]:%02x,output:%s\n", pos, i, input[i], output);
		// 如果已经填满了输出缓冲区，就返回
		if (pos >= outputLength) {
			break;
		}
	}

}

// 消息上报函数Char类型
void SendMess_Up_Char(Mess_Adder_t MessAdder, char *MessContent, int MessLength, Mess_Length_t MessFixedLength) 
{
	// 上报消息长度包括地址域长度，要加1
	int length_int = MessFixedLength + 1;
    unsigned char uartReadBuff[UART_BUFF_SIZE] = {0};   
	// 创建用于发送的字符串
	char ATUP[100] = "AT+NMGS=";
	char DouHao = ',';
	char Enter[] = "\r\n";

	//处理字符串为长度2*MessFixedLength的16进制ascll
	char MessContentHexASCII[MessFixedLength * 2];
	StringToHexASCII(MessContent, MessLength, MessContentHexASCII, MessFixedLength * 2);
	
	// 计算最终字符串的长度，并判断是否超出缓冲区
	int remaining_space = sizeof(ATUP) - strlen(ATUP) - 1; // -1 for the null terminator

	// 使用 snprintf 安全地格式化字符串，避免缓冲区溢出
	snprintf(ATUP + strlen(ATUP), remaining_space, "%d%c%02x%s", length_int, DouHao, MessAdder, MessContentHexASCII);
	strcat(ATUP, Enter); // 加入换行符

	// 发送最终字符串
	printf("AT Send Finish:%s", ATUP);
	IoTUartWrite(HI_UART_IDX_2, (unsigned char*)ATUP, strlen(ATUP));
    usleep(1000000);
    IoTUartRead(HI_UART_IDX_2, (unsigned char*)uartReadBuff, strlen(uartReadBuff));
    while(strstr(uartReadBuff, "OK") != NULL)
    {
        printf("Send_huawei_fail\r\n");
        IoTUartWrite(HI_UART_IDX_2, (unsigned char*)ATUP, strlen(ATUP));
        usleep(500000);
    }
    memset(uartReadBuff, 0, UART_BUFF_SIZE);
	
}

//解析上方发过来的数据请求
void Uart2_Read(char* uartReadBuff)
{
    int len;
    char InBuf[100];
    unsigned char uartReadBuff1[UART_BUFF_SIZE] = {0};        //接收的字符串
    len = IoTUartRead(HI_UART_IDX_2, uartReadBuff, UART_BUFF_SIZE);
    int cloudCode = 99;
    if (len > 0) {
        // 把接收到的数据打印出来 Print the received data
        printf("Uart Read Data is:%.*s.len: %d \r\n", len, uartReadBuff,len);
        if (strstr(uartReadBuff, "close") != NULL) {
            printf("关闭阀门\n");
            SetOffNo(InBuf,0);
            IoTUartWrite(HI_UART_IDX_1, (unsigned char *)InBuf, 23);  //通过串口1发送数据
            SER++;
            printf("uart1 send data:");
            for (size_t i = 0; i < 20; i++)
            {
                printf("%02X ", (unsigned char)InBuf[i]);
            }
            printf("\n");
            usleep(1000000);
            // 监听UART1 接收数据 Receive data through UART1
            len = IoTUartRead(HI_UART_IDX_1, uartReadBuff, UART_BUFF_SIZE);
            if (len > 0)
            {
                // 把接收到的数据打印出来 Print the received data
            memcpy(uartReadBuff1, uartReadBuff, len-9);
            printf("Uart1 Read Data1 is:\n");
                for (int i = 0; i < len-9; i ++) {
                    printf("%02X ", uartReadBuff1[i]);
                }
            printf("\r\n");
            memset(uartReadBuff, 0, UART_BUFF_SIZE); //清空缓冲区
            ParseMessage(uartReadBuff1,len-9);
            }
        } else if (strstr(uartReadBuff, "open") != NULL){
            printf("打开阀门\n");
            SetOffNo(InBuf,0);
            usleep(1000000);
            // 监听UART1 接收数据 Receive data through UART1
            len = IoTUartRead(HI_UART_IDX_1, uartReadBuff, UART_BUFF_SIZE);
            
            if (len > 0)
            {
                // 把接收到的数据打印出来 Print the received data
            memcpy(uartReadBuff1, uartReadBuff, len-9);
            printf("Uart1 Read Data1 is:\n");
                for (int i = 0; i < len-9; i ++) {
                    printf("%02X ", uartReadBuff1[i]);
                }
            printf("\r\n");
            memset(uartReadBuff, 0, UART_BUFF_SIZE); //清空缓冲区
            ParseMessage(uartReadBuff1,len-9);
            }

        }
        memset(uartReadBuff, 0, len); //清空缓冲区
        }
}
void Uart2_Read(char* uartReadBuff)
{
    int len;
    char InBuf[100];
    unsigned char uartReadBuff1[UART_BUFF_SIZE] = {0};        //接收的字符串
    int cloudCode = 99;
   
    
        // 把接收到的数据打印出来 Print the received data
        printf("Uart Read Data is:%.*s\r\n", len, uartReadBuff);
        if (!strcmp(uartReadBuff, "close")) {
            printf("关闭阀门\n");
            cloudCode = 9;
            UartGetCurrentData(cloudCode);
            printf("NB_Buffer:{%s}\n",NB_buffer);
        }
        else if (!strcmp(uartReadBuff, "open")){
            printf("打开阀门\n");
            cloudCode = 8;
            UartGetCurrentData(cloudCode);
            printf("NB_Buffer:{%s}\n",NB_buffer);
        }
        else if(!strcmp(uartReadBuff, "inquire"))
        {
            printf("查询当前流量\n");
            UartGetCurrentData(1);
            usleep(U_SLEEP_TIME);
            printf("NB_Buffer:{%s}\n",NB_buffer);

            UartGetCurrentData(2);
            usleep(U_SLEEP_TIME);
            printf("NB_Buffer:{%s}\n",NB_buffer);
        memset(uartReadBuff, 0, sizeof(uartReadBuff)); //清空缓冲区

    }
}


void AT_Order(char* send_buffer)
{
    unsigned char uartReadBuff2[UART_BUFF_SIZE] = {0};
    IotUartWrite(HI_UART_IDX_2, send_buffer, strlen(send_buffer));   
    printf("Uart2 Send:%s\n",send_buffer);  osDelay(50);

    IoTUartRead(HI_UART_IDX_2, uartReadBuff2, UART_BUFF_SIZE);
    printf("Uart2 Recver:%s\n",uartReadBuff2);

    if(!strstr(send_buffer, "MIPLOPEN")){
        osDelay(500);
        extractObserveValue(uartReadBuff2);
    }
    else if (!strstr(send_buffer, "MIPLOBSERVERSP")){
        osDelay(500);
        extractDiscoverValue(uartReadBuff2);
    }
    if(strstr(uartReadBuff2, "OK")){
        memset(uartReadBuff2, 0, UART_BUFF_SIZE);
        NB_OneNet();
    }
    else
        memset(uartReadBuff2, 0, UART_BUFF_SIZE);
}



void NB_huawei(void)
{
    uint32_t len = 0;
    unsigned char uartReadBuff2[UART_BUFF_SIZE] = {0};

    // 对UART1的一些初始化 Some initialization of UART1
    Uart2GpioInit();
    // 对UART1参数的一些配置 Some configurations of UART1 parameters
    Uart2Config();
    const char *NCDP = "AT+NCDP=124.70.30.197,5683\r\n";
    const char *NRB = "AT+NRB\r\n";
    const char *NMGS = "AT+NMGS=2,1600\r\n";
    const char *NNMI = "AT+NNMI=1\r\n";
    const char *CIMI = "AT+CIMI\r\n";
    const char *CGSN = "AT+CGSN=1\r\n";
    const char *CGATT = "AT+CGATT=1\r\n";

    char send_nb[]="ATE0\r";                                                 //取消回显
    IoTUartWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    Uart2_Read(uartReadBuff2);
    usleep(U_SLEEP_TIME);
    Uart2_Read(uartReadBuff2);

    IoTUartWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    usleep(U_SLEEP_TIME);
    Uart2_Read(uartReadBuff2);

    IoTUartWrite(HI_UART_IDX_2, (unsigned char*)NCDP, strlen(NCDP)); 
    printf("内容：%s,长度：%lu。已经发送\n", NCDP, strlen(NCDP)); //通过串口1发送数据
    usleep(U_SLEEP_TIME);
    len = IoTUartRead(HI_UART_IDX_2, uartReadBuff2, UART_BUFF_SIZE);
    if (len > 0){
        printf("Uart2 Read data is:%s \n",uartReadBuff2);
    }

    IoTUartWrite(HI_UART_IDX_2, (unsigned char*)NMGS, strlen(NMGS)); 
    printf("内容：%s,长度：%lu。已经发送\n", NMGS, strlen(NMGS)); //通过串口1发送数据
    usleep(U_SLEEP_TIME);
    len = IoTUartRead(HI_UART_IDX_2, uartReadBuff2, UART_BUFF_SIZE);
    if (len > 0){
        printf("Uart2 Read data is:%s \n",uartReadBuff2);
    }
    UartGetCurrentData(9);
    sleep(1);
    UartGetCurrentData(8);
    sleep(1);
    while (1)
    {
        printf("查询当前流量\n");
        UartGetCurrentData(1);
        usleep(U_SLEEP_TIME);
        printf("NB_Buffer:{%s}\n",NB_buffer);
        //Uart2_Read(uartReadBuff2);

        UartGetCurrentData(2);
        usleep(U_SLEEP_TIME);
        printf("NB_Buffer:{%s}\n",NB_buffer);
        //Uart2_Read(uartReadBuff2);
        usleep(20*1000000);
    }
    

}
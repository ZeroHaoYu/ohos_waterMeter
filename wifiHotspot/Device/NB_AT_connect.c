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

#define UART_BUFF_SIZE 1024
#define U_SLEEP_TIME   100000
extern SER;

void Uart2GpioInit(void)
{
    // IoTGpioInit(IOT_IO_NAME_GPIO_11);
    // 设置GPIO0的管脚复用关系为UART1_TX Set the pin reuse relationship of GPIO0 to UART1_ TX
    IoSetFunc(IOT_IO_NAME_GPIO_11, IOT_IO_FUNC_GPIO_11_UART2_TXD);
    // IoTGpioInit(IOT_IO_NAME_GPIO_12);
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

//解析上方发过来的数据请求
void Uart2_Read(char* uartReadBuff)
{
    int len;
    char InBuf[100];
    unsigned char uartReadBuff1[UART_BUFF_SIZE] = {0};        //接收的字符串
    len = IoTUartRead(HI_UART_IDX_2, uartReadBuff, UART_BUFF_SIZE);
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


void NB_Connect(void)
{
    uint32_t len = 0;
    unsigned char uartReadBuff[UART_BUFF_SIZE] = {0};

    // 对UART1的一些初始化 Some initialization of UART1
    Uart2GpioInit();
    // 对UART1参数的一些配置 Some configurations of UART1 parameters
    Uart2Config();
    
    // char send_nb[]="AT+QLWSERV=\"221.229.214.202\",5683";
    char send_nb[]="ATE0\r";                                                 //取消回显
    IoTUartWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    Uart2_Read(uartReadBuff);
    usleep(1000000);
    Uart2_Read(uartReadBuff);

    IoTUrtWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    usleep(1000000);
    Uart2_Read(uartReadBuff);

    strcpy(send_nb, "AT+CGSN=1\r");                                          //查询查询模块版本
    IoTUartWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    usleep(1000000);
    Uart2_Read(uartReadBuff);

    strcpy(send_nb, "AT+QLWSERV=\"221.229.214.202\",5683\r");                //配置电信 loT 平台地址和端口
    IoTUartWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    usleep(1000000);
    Uart2_Read(uartReadBuff);

    strcpy(send_nb, "AT+QLWCONF=\"123476589012345\"\r");                    //配置模块将连接的电信 IoT 平台的IMEI参数。
    IoTUartWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    usleep(1000000);
    Uart2_Read(uartReadBuff);

    strcpy(send_nb, "AT+QLWADDOBJ=19,0,1,\"0\"\r");                          //添加 LwM2M 对象
    IoTUartWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    usleep(1000000);
    Uart2_Read(uartReadBuff);


    strcpy(send_nb, "AT+QLWOPEN=0\r");
    IoTUartWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    printf("NB connect down!\n");
    usleep(8000000);
    Uart2_Read(uartReadBuff);

    strcpy(send_nb, "AT+QLWCFG=\"dataformat\",0,0\r");
    IoTUartWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    usleep(500000);
    Uart2_Read(uartReadBuff);

    strcpy(send_nb, "AT+QLWDATASEND=19,0,0,2,zy,0x0100\r");
    IoTUartWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    usleep(1000000);
    Uart2_Read(uartReadBuff);
    Uart2_Read(uartReadBuff);
    int time=120;
    while (time--)
    {
        printf("after%dsecond close.\n",time);
        
        usleep(1000000);
    }
   
    strcpy(send_nb, "AT+QLWCLOSE\r");
    IoTUartWrite(HI_UART_IDX_2, (unsigned char *)send_nb, strlen(send_nb));  //通过串口1发送数据
    printf("内容：%s,长度：%lu。已经发送\n", send_nb, strlen(send_nb));
    usleep(4000000);
}

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_uart.h"
#include "hi_uart.h"
#include "iot_watchdog.h"
#include "iot_errno.h"
#include "decode.h"
#include "Uart_Function.h"
#include "MessUp.h"

#define UART_BUFF_SIZE 1024
#define U_SLEEP_TIME 5000000
#define NB_BUFFER_SIZE 1024

#include <stdlib.h>
char InBuf[125];
int SER=0;
int CTR= 0x03; //CTR码
char NB_buffer[NB_BUFFER_SIZE];
int NB_buffer_index=0;


// 通用函数，将格式化的字符串写入 NB_buffer
/**
 * 向缓冲区中添加格式化的字符串。
 * 
 * 此函数使用可变参数列表，允许动态地添加格式化的字符串到一个固定大小的缓冲区中。
 * 如果缓冲区空间不足，将打印出界错误并终止添加。
 * 
 * @param format 格式字符串，用于指定输出格式。
 * @param ... 可变参数列表，与格式字符串指定的参数类型匹配。
 */
void addToBuffer(const char *format, ...) {
    if (NB_buffer_index >= NB_BUFFER_SIZE) {
        // 防止越界
        printf("越界\n");
        return;
    }

    // 初始化可变参数列表
    va_list args;
    va_start(args, format);

    // 使用 vsnprintf 获取格式化的字符串的长度
    int remainingSpace = NB_BUFFER_SIZE - NB_buffer_index;
    int written = vsnprintf(NB_buffer + NB_buffer_index, remainingSpace, format, args);

    if (written >= 0 && written < remainingSpace) {
        // printf("Written to NB_buffer: %.*s\n", written, NB_buffer + NB_buffer_index);
        // printf("written_Length :%d\n",written);
        // 更新 NB_buffer_index
        // 更新缓冲区索引以记录已写入的字符数
        NB_buffer_index += written;
        
    } else {
        // 处理长度不足或者错误的情况
        NB_buffer_index = NB_BUFFER_SIZE - 1;
    }

    // 结束可变参数列表
    va_end(args);
}


// 反转字符串
void reverseString(char *str) {
    size_t length = strlen(str);
    for (size_t i = 0; i < length / 2; ++i) {
        char temp = str[i];
        str[i] = str[length - 1 - i];
        str[length - 1 - i] = temp;
    }
}

// 将两位十六进制字符串转换为16位二进制数
void hexToBinary(const char *hex, char *binary) {
    int value = 0;

    sscanf(hex, "%x", &value);

    for (int i = 15; i >= 0; --i) {
        binary[i] = (char)((value >> i) & 1) + '0';
       
    }

    binary[16] = '\0';
}

// 检测每一位比特是否为1
void checkExceptions(char binaryStr[]) {
    int i;
    // 检测比特0和比特1组合的状态
    char combinedBits[3] = {binaryStr[1], binaryStr[0], '\0'};
    if (combinedBits[0] == '0' && combinedBits[1] == '0') {
        addToBuffer("devicestatus:0'");                              //阀门状态: 关
    } 
    else if (combinedBits[0] == '1' && combinedBits[1] == '0') {
        addToBuffer("阀门状态: 无阀\n");
    } 
    else if (combinedBits[0] == '0' && combinedBits[1] == '1') {
        addToBuffer("devicestatus:1'");                             //阀门状态: 开
    } 
    else if (combinedBits[0] == '1' && combinedBits[1] == '1') {
        addToBuffer("阀门状态: 异常\n");
    }

    // 检测其他比特是否为1，表示异常
    for (i = 2; i < 16; i++) {
        if (binaryStr[i] == '1') {
            switch (i)
            {
            case 2:
                printf("电池电压欠压\r\n");
                addToBuffer("电池电压欠压\r\n");
                break;
            case 3:
                addToBuffer("计量异常\r\n");
                break;
            case 4:
                addToBuffer("电池门打开\r\n");
                break;
            case 5:
                addToBuffer("余量报警\r\n");
                break;
            case 6:
                addToBuffer("反向计量\r\n");
                break;
            case 7:
                addToBuffer("flash异常\r\n");
                break;
            case 8:
                addToBuffer("过零告警\r\n");
                break;
            case 9:
                addToBuffer("触摸按键异常\r\n");
                break;
            case 10:
                addToBuffer("红外异常\r\n");
                break;
            default:
                break;
            }
        }else{
            switch (i)
            {
            case 2:
                printf("电池电压正常\r\n");
                addToBuffer("Battery_voltage:1'");   
                SendMess_Up_Char(Mess_Adder_voltage, "1", 1, Mess_Length_voltage);
                break;
            case 3:
                printf("计量正常\r\n");
                addToBuffer("Metering:1'");
                SendMess_Up_Char(Mess_Adder_Metering, "1", 2, Mess_Length_Metering);
                break;
                break;
            case 4:
                printf("电池门关闭\r\n");
                addToBuffer("battery_door:0'");
                SendMess_Up_Char(Mess_Adder_battery_door, "0", 2, Mess_Length_battery_door);
                break;
            case 5:
                printf("余量无报警\r\n");
                addToBuffer("alarm:1'");
                break;
            case 6:
                printf("正向计量\r\n");
                addToBuffer("Forward_meteringrn'");
                break;
            case 7:
                printf("flash正常\r\n");
                addToBuffer("flash:1'");
                SendMess_Up_Char(Mess_Adder_flash, "1", 2, Mess_Length_flash);
                break;
            case 8:
                printf("过零正常\r\n");
                addToBuffer("Zero_crossing:1'");
                break;
            case 9:
                printf("触摸按键正常\r\n");
                addToBuffer("button:1'");
                SendMess_Up_Char(Mess_Adder_button, "1", 2, Mess_Length_button);
                break;
            case 10:
                printf("红外正常\r\n");
                addToBuffer("Infrared:1'");
                SendMess_Up_Char(Mess_Adder_Infrared, "1", 2, Mess_Length_Infrared);
                break;
            default:
                break;
            }
        }
    }
}


//校验码 
char CheckCode(char* buf,int leng)
{
	unsigned char checksum = 0;
    for (int i = 0; i < leng; i++) {
        checksum += buf[i];
    }
    return checksum;
}

void UartInit(void)
{
     //UART1：读取水表的数据
    IoTGpioInit(IOT_IO_NAME_GPIO_0);                                //初始化引脚0
    IoSetFunc(IOT_IO_NAME_GPIO_0, IOT_IO_FUNC_GPIO_0_UART1_TXD);    //引脚复用为UART1_TX
    IoTGpioInit(IOT_IO_NAME_GPIO_1);                                //初始化引脚1
    IoSetFunc(IOT_IO_NAME_GPIO_1, IOT_IO_FUNC_GPIO_1_UART1_RXD);    //引脚复用为UART1_RX

    uint32_t ret;
    /* 初始化UART配置，波特率 4800，数据bit为8,停止位1，奇偶校验为EVEN */
    IotUartAttribute uart_attr = {
        .baudRate = 9600,
        .dataBits = 8,
        .stopBits = 1,
        .parity = IOT_UART_PARITY_EVEN,
    };
    ret = IoTUartInit(HI_UART_IDX_1, &uart_attr);
    if (ret != IOT_SUCCESS)
    {
        printf("Init Uart1 Falied Error No. : %d\n", ret);
        return;
    }else{
        printf("Init Uart1 success No. : %d\n", ret);
    }
}

// 解析报文函数
void ParseMessage(char* receivedBuf, int length) {
    int index=0;
    int address_index=0;
    struct Meter meter;
    unsigned char CurrentFlow[11]={0};
    unsigned char SettledFlow[11]={0};
    unsigned char Date[20] = {0};
    // 检查报文是否足够长
    if (length < 13 || receivedBuf[2] != 0x68 ||  receivedBuf[length-1] != 0x16) {
        addToBuffer("Invalid message length.\n");
        return;
    }

    // 检查帧开始标志
    for (int i = 0; i < 2; i++) {
        if ((unsigned char)receivedBuf[index] != 0xFE) {
            addToBuffer("Invalid frame start.\n");
            return;
        }
        index++;
    }
    index++;
    CTR=(unsigned char)receivedBuf[11];
    meter.controlCode=CTR;
    // 输出设备地址
    addToBuffer("CTR:%02X'",CTR);
    switch (CTR)
    {
    case 0x81:
        printf("查询正常\r\n");

        addToBuffer("ret:1'");//当前累积流量，结算日累积流量，实时时间，状态ST,10 08 00 00 29  00 00 00 00 29     57 04 16 24 11 23 20     01 00   EE 16 
        for (size_t i = 0; i < 5; i++)
        {
            meter.currentFlow[i]=(unsigned char)receivedBuf[20-i];
            meter.settledFlow[i]=(unsigned char)receivedBuf[25-i];
        }
        addToBuffer("settledFlow:");             //结算日累积流量:
        for (size_t i = 0; i < 5; i++)
        {
            addToBuffer("%02X",meter.settledFlow[i]);
        }
        addToBuffer("'");
        sprintf(SettledFlow,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",meter.settledFlow[0],meter.settledFlow[1],meter.settledFlow[2],meter.settledFlow[3],meter.settledFlow[4],meter.settledFlow[5],meter.settledFlow[6],meter.settledFlow[7],meter.settledFlow[8],meter.settledFlow[9]);
        SettledFlow[11]='\0';
        SendMess_Up_Char(Mess_Adder_settledFlow, SettledFlow, 11,Mess_Length_settledFlow);
        printf("日累积流量上传成功\r\n");

        addToBuffer("currentFlow:");          //当前累积流量:
        for (size_t i = 0; i < 5; i++)
        {
            addToBuffer("%02X", meter.currentFlow[i]);
        }
        addToBuffer("'");
        sprintf(CurrentFlow,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",meter.currentFlow[0],meter.currentFlow[1],meter.currentFlow[2],meter.currentFlow[3],meter.currentFlow[4],meter.currentFlow[5],meter.currentFlow[6],meter.currentFlow[7],meter.currentFlow[8],meter.currentFlow[9]);
        CurrentFlow[11]='\0';
        SendMess_Up_Char(Mess_Adder_currentFlow, CurrentFlow, 11,Mess_Length_currentFlow);
        printf("当前累积流量上传成功\r\n");
        for (size_t i = 6,j=length-11; i != SIZE_MAX; i--,j++)
        {
            // printf("receivedBuf[%d]:%02x",j,receivedBuf[j]);
            meter.date[i]=(unsigned char)receivedBuf[j];
        }
        meter.date[7]='\0';
        addToBuffer("Date:%02X%02X/%02X/%02X/%02X/%02X/%02X'",meter.date[0],meter.date[1],meter.date[2],meter.date[3],meter.date[4],meter.date[5],meter.date[6]);
        
        // sprintf(Date, "%02X%02X/%02X/%02X/%02X/%02X/%02X",meter.date[0],meter.date[1],meter.date[2],meter.date[3],meter.date[4],meter.date[5],meter.date[6]);
        // Date[20] = '\0';
        // SendMess_Up_Char(Mess_Adder_Date, &Date, 20,Mess_Length_Date);
        // printf("日期上传成功\r\n");

        char sum[5];  // 最多占用5个字符，包括终止符 '\0'
        memset(sum, 0, sizeof(sum));
        snprintf(sum, sizeof(sum), "%02X%02X", receivedBuf[length - 4], receivedBuf[length - 3]);
        // 分配存储空间
        char binary[17];  // 16位二进制数 + 1位终止符 '\0'

        // 转换十六进制为二进制
        hexToBinary(sum, binary);
        
        for (size_t i = 8,j=0; i < 16; i++,j++)
        {
            meter.data_binaryStr[j]=binary[i];
        }
        for (size_t i = 0,j=0; i < 8; i++,j++)
        {
            meter.data_binaryStr[j+8]=binary[i];
        }
        // 添加字符串终止符
        meter.data_binaryStr[16] = '\0';

         // 打印结果
        checkExceptions(meter.data_binaryStr);
        break;
    case 0x83:         //查询地址
        addToBuffer("DeviceAddress: ");
        for (int i = 3 + ADDRESS_length; i >= 4; i--) {
            meter.address[address_index]=(unsigned char)receivedBuf[i];
            address_index++;
        }
        addToBuffer("meter.address:");
        for (size_t i = 0; i < 7; i++)
        {
            addToBuffer("%02X ", (unsigned char)meter.address[i]);
        }
        addToBuffer("\n");
        break;
    case 0x84:         //水阀开关应答
        addToBuffer("开关阀正常应答\r\n");
        
        // 将两个 unsigned char 转换为字符串
          // 最多占用5个字符，包括终止符 '\0'
        memset(sum, 0, sizeof(sum));
        snprintf(sum, sizeof(sum), "%02X%02X", receivedBuf[length - 4], receivedBuf[length - 3]);
        // 分配存储空间
        memset(binary, 0, sizeof(binary));
        // 转换十六进制为二进制
        hexToBinary(sum, binary);
        
        for (size_t i = 8,j=0; i < 16; i++,j++)
        {
            meter.data_binaryStr[j]=binary[i];
        }
        for (size_t i = 0,j=0; i < 8; i++,j++)
        {
            meter.data_binaryStr[j+8]=binary[i];
        }
        // 添加字符串终止符
        meter.data_binaryStr[16] = '\0';

         // 打印结果
        checkExceptions(meter.data_binaryStr);
    case 0xB1:     //查询NB消息成功应答                   
        addToBuffer("ret:1'");
        printf("功能:查询NB信息成功'\n");
        for (int i = 3 + ADDRESS_length; i >= 4; i--) {
            meter.address[address_index]=(unsigned char)receivedBuf[i];
            address_index++;
        }
        addToBuffer("No:");             //水表电子号
        for (size_t i = 0; i < 7; i++)
        {
            addToBuffer("%02X", (unsigned char)meter.address[i]);
        }
        addToBuffer("'");

        length=length-3;
        meter.report_result=(unsigned char)receivedBuf[length];
        if (meter.report_result==0)
        {
            // addToBuffer("上报结果:成功\n");
        }else{
            // addToBuffer("上报结果:失败\n");
        }
        
        length-=7;
        for (size_t i = 0,j=length; i <7; i++,j++)
        {
            // printf("receivedBuf[%d]:%02x",j,receivedBuf[j]);
            meter.date[i]=(unsigned char)receivedBuf[j];
        }
        meter.date[7]='\0';
        addToBuffer("Date:%02X%02X/%02X/%02X/%02X/%02X/%02X'",meter.date[0],meter.date[1],meter.date[2],meter.date[3],meter.date[4],meter.date[5],meter.date[6]);

         // 提取数组中的两个字节
        unsigned char byte2 = receivedBuf[--length];  // 
        unsigned char byte1 = receivedBuf[--length];  // 
        // 合并成一个整数
        meter.TXPower = (byte2 << 8) | byte1;
        addToBuffer("TXPower:%d'", meter.TXPower);    //发射功率
        char TXPower[2];
        sprintf(TXPower, "%d", meter.TXPower);
        SendMess_Up_Char(Mess_Adder_TXPower, &TXPower, sizeof(TXPower),Mess_Length_TXPower);
        printf("TXPower上报成功\r\n");

        meter.Battery_voltage=(unsigned char)receivedBuf[--length];
        addToBuffer("voltage(20mv):%d'",meter.Battery_voltage);//电池电压
        char voltage[3];
        sprintf(voltage, "%d", meter.Battery_voltage);
        SendMess_Up_Char(Mess_Adder_voltage, &voltage, sizeof(voltage),Mess_Length_voltage);
        printf("voltage上报成功\r\n");

        char byte3=receivedBuf[--length];
        char byte4=receivedBuf[--length];
        meter.RSRP = (byte3 << 8) | byte4;
        addToBuffer("RSRP:%d'", meter.RSRP ); //信号强度
        char RSRP[3];
        sprintf(RSRP, "%d", meter.RSRP);
        SendMess_Up_Char(Mess_Adder_RSRP, &RSRP, sizeof(RSRP),Mess_Length_RSRP);
        printf("RSRP上报成功\r\n");

        meter.ECL=(unsigned char)receivedBuf[--length];
        addToBuffer("ECR:%d'",meter.ECL);
        char ECL[1];
        sprintf(ECL, "%d", meter.ECL);
        SendMess_Up_Char(Mess_Adder_ECR, &ECL, sizeof(ECL),Mess_Length_ECR);
        printf("ECR上报成功\r\n");

        byte2 = receivedBuf[--length];  // 
        byte1 = receivedBuf[--length];  // 
        meter.report_failed = (byte2 << 8) | byte1;
        addToBuffer("report_failed:%d'", meter.report_failed );  //上报失败次数
        char failed[2];
        sprintf(failed, "%d", meter.report_failed);
        SendMess_Up_Char(Mess_Adder_report_failed, &failed, sizeof(failed),Mess_Length_report_failed);
        printf("report_failed上报成功\r\n");

        byte2 = receivedBuf[--length];  // 
        byte1 = receivedBuf[--length];  // 
        meter.report_success = (byte2 << 8) | byte1;
        addToBuffer("report_success:%d'", meter.report_success );    //上报成功次数
        char success[4];
        sprintf(success, "%d", meter.report_success);
        SendMess_Up_Char(Mess_Adder_report_success, &success, sizeof(success),Mess_Length_report_success);
        printf("report_success上报成功\r\n");

        byte3=receivedBuf[--length];
        byte4=receivedBuf[--length];
        meter.SNR = (byte3 << 8) | byte4;
        addToBuffer("SNR:%d'",meter.SNR);                 //信噪比

        byte1 = receivedBuf[--length];  // 
        byte2 = receivedBuf[--length];  // 
        char byte5 = receivedBuf[--length];  // 
        char byte6 = receivedBuf[--length];  // 
        meter.CellID = (byte1 << 8*3) | (byte2 << 8*2)|(byte5 << 8)|byte6;
        addToBuffer("CellID:%d'", meter.CellID );          //小区ID

        addToBuffer("Valve Off Daily Reports:%d'",(int)(unsigned char)receivedBuf[--length]);//关阀一天上报次数
        addToBuffer("Valve Off Consecutive Reporting Days:%d'",(int)(unsigned char)receivedBuf[--length]);  //关阀连续上报天数

        meter.Escalation_cycle=(unsigned char)receivedBuf[--length];
        addToBuffer("Escalation_cycle(h):%d'",meter.Escalation_cycle);    //上报周期(小时)

        meter.enableEncryption=(unsigned char)receivedBuf[--length];
        if(meter.enableEncryption==1)
        {
            addToBuffer("Encrypt:Enabled'");        //加密使能
        }else{
            addToBuffer("Encrypt:disabled'");
        }
    
        length-=15;
        for (size_t i = 0; i < 15; i++)
        {
            meter.IMSI[i]=receivedBuf[length++];
        }
        length-=15;
         meter.IMSI[15]='\0';
        addToBuffer("IMSI:%s'",meter.IMSI);
       
        meter.CSQ=(unsigned char)receivedBuf[--length];
        addToBuffer("CSQ:%d'",meter.CSQ);

        length-=20;
        for (size_t i = 0; i < 20; i++)
        {
            meter.NCCID[i]=receivedBuf[length++];
        }
        meter.NCCID[20]='\0';
        length-=20;
        addToBuffer("NCCID:%s'",meter.NCCID);

        length-=15;
        for (size_t i = 0; i < 15; i++)
        {
            meter.IMEI[i]=receivedBuf[length++];
        }
        meter.IMEI[15]='\0';
        length-=15;
        addToBuffer("IMEI:%s'",meter.IMEI);

        break;
    default:
        addToBuffer("error!'");
        break;

    }
}

void Send_to_onenet(char* InBuf){
    int count=2;
    while (count--)
    {
        IoTUartWrite(HI_UART_IDX_2, (unsigned char *)InBuf, strlen(InBuf));
        printf("Send_to_onenet:%s\n",InBuf);
        usleep(1000000);
        char uartReadBuff[UART_BUFF_SIZE] = {0};
        IoTUartRead(HI_UART_IDX_2, uartReadBuff, UART_BUFF_SIZE);
        if(!strstr(uartReadBuff, "OK\n\nMIPLEVENT:0,26") && count == 0) {
            NB_OneNet();
        }
    }
}

int i=123;
int UartGetCurrentData(int code)
{
    char send_NB_buffer[1024];                           //上传上云的消息
    unsigned char uartReadBuff1[UART_BUFF_SIZE] = {0};        //接收的字符串
    uint32_t len = 0;
    unsigned char uartReadBuff[UART_BUFF_SIZE] = {0};
    // 对UART1的一些初始化 Some initialization of UART1
    int count=2;
   
    // SetOffNo(InBuf,1);
    int cnt=0;

    while (count--)
    {
        memset(InBuf, 0, 125); //清空缓冲区
        switch (code)
        {
        case 1:
            GetCurrentData(InBuf);
            // 通过UART1 发送数据 Send data through UART1
            IoTUartWrite(HI_UART_IDX_1, (unsigned char *)InBuf, 20);  //通过串口1发送数据
            break;
        case 2:
            GetCurrent_NB(InBuf);
            IoTUartWrite(HI_UART_IDX_1, (unsigned char *)InBuf, 20);  //通过串口1发送数据
            break;
        case 3:
            GetAddr(InBuf);
            IoTUartWrite(HI_UART_IDX_1, (unsigned char *)InBuf, 20);  //通过串口1发送数据
            break;
        case 8:
            SetOffNo(InBuf,1); //开
            IoTUartWrite(HI_UART_IDX_1, (unsigned char *)InBuf, 21);  //通过串口1发送数据
            break;
        case 9:
            SetOffNo(InBuf,0);//关
            IoTUartWrite(HI_UART_IDX_1, (unsigned char *)InBuf, 21);  //通过串口1发送数据
            break;
        default:
            printf("queryCode error!\n");
            return -1;
        }
       
        SER++;

        printf("uart1 send data:");
        for (size_t i = 0; i < 20; i++)
        {
            printf("%02X ", (unsigned char)InBuf[i]);
        }
        printf("\n");
        
        usleep(500000);
        // 监听UART1 接收数据 Receive data through UART1
        len = IoTUartRead(HI_UART_IDX_1, uartReadBuff, UART_BUFF_SIZE);
        printf("Uart1DateLen:%d\n",len);
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

        usleep(500000);
        printf("__________________________\n");
        printf("NB_buffer:\n %s\n", NB_buffer);
        sprintf(send_NB_buffer, "AT+MIPLNOTIFY=0,%s,3200,0,5750,1,%d,%s,0,0,%d",observe,sizeof(NB_buffer),NB_buffer,i);
        Send_to_onenet(send_NB_buffer);
        
        memset(send_NB_buffer, 0, NB_BUFFER_SIZE); //清空缓冲区
        NB_buffer_index=0;
       
        usleep(500000);
        printf("count:%d\n",count);
    }
    return 0;
}


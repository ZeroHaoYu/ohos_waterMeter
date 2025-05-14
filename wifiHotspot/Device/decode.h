#ifndef WATERMETERPROTOCOL_H
#define WATERMETERPROTOCOL_H

// 定义帧起始符
#define wakeup_begin 0    //唤醒
#define head_length 4


#define FRAME_START 0x68

// 定义水表类型
#define WATER_METER_TYPE 0x10

// 定义地址域起始位
#define ADDRESS_begin 6 
#define ADDRESS_length 7 

// 定义数据域长度
#define L 0x03

// 定义数据域
#define DATA "\x81\x0A"  // 请替换为实际的数据

// 定义结束符
#define FRAME_END 0x16
extern int SER;
struct Meter {
    
    char address[7];  // 地址域
    char controlCode;  // 控制码
    char dataLength;  // 数据长度
    char dataIdentifier[2];  // 数据标识
    char ser;  // SER序列号
    char checksum;  // 校验码
    char *data;  //水表状态
    char data_binaryStr[17];
    char date[7];
    char currentFlow[5];    // 当前累积流量
    char settledFlow[5];    // 结算日累积流量
    int TXPower;
    int Battery_voltage;
    int report_result;
    int RSRP;            //信号强度
    int ECL;             //覆盖等级
    int report_failed;   //上报失败次数
    int report_success;   //上报失败次数
    int SNR;              //信噪比
    int CellID;           //小区ID
    int Escalation_cycle; //上报周期
    int enableEncryption;  //加密使能
    char IMSI[15];
    int CSQ;
    char NCCID[20];
    char IMEI[15];
};

void SetOffNo(char* InBuf,int flag);    //开关阀门
void GetCurrent_NB(char* InBuf);        //查询表具当前NB消息
void GetCurrentData(char* InBuf);       //查询表具当前数据 标准188 包含当当前时间、当前累计流量、当前冻结日流量、表状态
void GetAddr(char* InBuf);              //查看水表地址
void NB_Connect(void);           //NB连接上云
void Uart2_Read(char* uartReadBuff);
#endif // WATERMETERPROTOCOL_H

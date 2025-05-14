#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "decode.h"


//查询表具当前NB消息
void GetCurrent_NB(char* InBuf)
{
    int index;
    index=wakeup_begin;
    for (int i = 0; i < head_length; i++)
    {
        InBuf[index]=0xFE;
        index++;
    }
    InBuf[index++] = 0x68; //帧开始标志
	InBuf[index++] = 0x10; //冷水表
    index=ADDRESS_begin;

    for (int i = 0; i < ADDRESS_length; i++)//地址位
    {
        InBuf[index]=0xAA;
        index++;
    }
    InBuf[index++] = 0x31; //控制码CTR
    InBuf[index++] = 0x03; //长度
    InBuf[index++] = 0x31;//数据标识
    InBuf[index++] = 0x08;
    InBuf[index++] = SER; //SER序列号
    InBuf[index++]=CheckCode((char*)InBuf+4,index-4);
    InBuf[index++] = 0x16; //结束符
}

//开关阀门
void SetOffNo(char* InBuf,int flag)
{
    printf("test\r\n");
    // flag //1:0x55开阀 0:0x99关阀
    int index;
    switch (flag)
    {
    case 1:
        flag=0x55;
        break;
    case 0:
        flag=0x99;
        break;
    default:
        printf("SetOffNo flag error\r\n");
        return;
        break;
    }
    index=wakeup_begin;
    for (int i = 0; i < head_length; i++)
    {
        InBuf[index]=0xFE;
        index++;
    }
    InBuf[index++] = 0x68; //帧开始标志
	InBuf[index++] = 0x10; //冷水表
    index=ADDRESS_begin;

    for (int i = 0; i < ADDRESS_length; i++)//地址位
    {
        InBuf[index]=0xAA;
        index++;
    }
    InBuf[index++] = 0x04; //控制码CTR
    InBuf[index++] = 0x04; //长度
    InBuf[index++] = 0xA0;//数据标识
    InBuf[index++] = 0x17;
    InBuf[index++] = SER; //SER序列号
    InBuf[index++] = flag; //水阀控制码
    InBuf[index++]=CheckCode((char*)InBuf+4,index-4);
    InBuf[index++] = 0x16; //结束符
    InBuf[index++] = '\0'; //结束符
}

//查询表具当前数据 标准188 包含当当前时间、当前累计流量、当前冻结日流量、表状态
void GetCurrentData(char* InBuf)
{
    int index;
    index=wakeup_begin;
    for (int i = 0; i < head_length; i++)
    {
        InBuf[index]=0xFE;
        index++;
    }
    InBuf[index++] = 0x68; //帧开始标志
	InBuf[index++] = 0x10; //冷水表
    index=ADDRESS_begin;

    for (int i = 0; i < ADDRESS_length; i++)//地址位
    {
        InBuf[index]=0xAA;
        index++;
    }
    InBuf[index++] = 0x01; //控制码CTR
    InBuf[index++] = 0x03; //长度
    InBuf[index++] = 0x90;//数据标识
    InBuf[index++] = 0x1F;
    InBuf[index++] = SER; //SER序列号
    InBuf[index++]=CheckCode((char*)InBuf+4,index-4);
    InBuf[index++] = 0x16; //结束符
}

void GetAddr(char* InBuf)
{
    int index;
    index=wakeup_begin;
    for (int i = 0; i < head_length; i++)
    {
        InBuf[index]=0xFE;
        index++;

    }
    InBuf[index++] = 0x68; //帧开始标志
	InBuf[index++] = 0x10; //冷水表
    index=ADDRESS_begin;

    for (int i = 0; i < ADDRESS_length; i++)//地址位
    {
        InBuf[index]=0xAA;
        index++;
    }
    InBuf[index++] = 0x03; //控制码CTR
    InBuf[index++] = 0x03; //长度
    //DI0=0x81 DI1=0x0A
    InBuf[index++] = 0x81;//数据标识
    InBuf[index++] = 0x0A;
    InBuf[index++] = SER; //SER序列号
    InBuf[index++]=CheckCode((char*)InBuf+4,index-4);
    InBuf[index++] = 0x16; //结束符
}
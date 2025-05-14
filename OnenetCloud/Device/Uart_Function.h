#include<stdio.h> 


void Uart1GpioInit(void);

void Uart1Config(void);

int UartGetCurrentData(int code);

void PrintHello(void);

void UartInit(void);

void NB_huawei(void);

void NB_Connect(void);

void Uart2GpioInit(void);

void Uart2Config(void);

void AT_Order(char* send_buffer);

void Cloud_command(void *Inbuf);

extern char execute[16];
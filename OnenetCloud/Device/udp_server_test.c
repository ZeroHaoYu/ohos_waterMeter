/*
 * Copyright (c) 2020, HiHope Community.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// #include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "net_demo.h"
#include "net_common.h"
#include "lwip/netifapi.h"
#include "net_common.h"
#include "Uart_Function.h"

#define IDX_0          0
#define IDX_1          1
#define IDX_2          2
#define IDX_3          3
#define IDX_4          4
#define IDX_5          5
#define DELAY_TICKS_10     (10)
#define DELAY_TICKS_500    (500)
#define COUNTER            (360)
#define CH_NUM             (7)

static volatile int g_hotspotStarted = 0;
char message[128] = "";
extern char NB_buffer[];

char code_1[2] = "1";
char code_2[2] = "2";
char code_3[2] = "3";
char code_8[2] = "8";
char code_9[2] = "9";


void UdpServerTest(unsigned short port)
{

    ssize_t retval = 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    int queryCode =99;
    struct sockaddr_in clientAddr = {0};
    socklen_t clientAddrLen = sizeof(clientAddr);
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (retval < 0) {
        printf("bind failed, %ld!\r\n", retval);
        close(sockfd);
    }
    printf("bind to port %hu success!\r\n", port);
    while (1)
    {
        memset(message,0,sizeof(message));
        retval = recvfrom(sockfd, message, sizeof(message), 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (retval < 0) {
            printf("recvfrom failed, %ld!\r\n", retval);
            printf("do_cleanup...\r\n");
            close(sockfd);
        }
        printf("recv message {%s} done!\r\n", message);
        printf("peer info: ipaddr = %s, port = %hu\r\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        if (strcmp(message,code_1) == 0 )
        {
            queryCode=1;
        }else if (strcmp(message,code_2) == 0)
        {
            queryCode=2;
        }else if (strcmp(message,code_3) == 0)
        {
            queryCode=3;
        }else if (strcmp(message,code_8) == 0)
        {
            queryCode=8;
        }else if (strcmp(message,code_9) == 0)
        {
            queryCode=9;
        }
        printf("queryCode:%d\n",queryCode);
        UartGetCurrentData(queryCode);
        
        printf("NB_Buffer:{%s}\n",NB_buffer);
        retval = sendto(sockfd, NB_buffer, strlen(NB_buffer), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
        if (retval <= 0) {
            printf("send failed, %ld!\r\n", retval);
        }
        printf("send message {%s} %ld done!\r\n", NB_buffer, retval);
        memset(NB_buffer, 0, 1024); //清空缓冲区
    }
}
SERVER_TEST_DEMO(UdpServerTest);

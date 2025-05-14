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

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_hotspot.h"
#include "lwip/netifapi.h"
#include "net_common.h"
#include "Uart_Function.h"
#define STACK_SIZE     10240
#define AP_SSID        "LmbuaaMateX5典藏版"
#define AP_SKEY        "1234qwer"
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


static void OnHotspotStateChanged(int state)
{
    printf("OnHotspotStateChanged: %d.\r\n", state);
    if (state == WIFI_HOTSPOT_ACTIVE) {
        g_hotspotStarted = 1;
    } else {
        g_hotspotStarted = 0;
    }
}

static volatile int g_joinedStations = 0;

static void PrintStationInfo(StationInfo* info)
{
    if (!info) return;
    static char macAddress[32] = {0};
    unsigned char* mac = info->macAddress;
    // int ret = snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X",
    int ret = snprintf_s(macAddress, sizeof(macAddress), sizeof(macAddress) - 1, "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[IDX_0], mac[IDX_1], mac[IDX_2], mac[IDX_3], mac[IDX_4], mac[IDX_5]);
    if (ret < 0) {
        return;
    }
    printf(" PrintStationInfo: mac=%s, reason=%d.\r\n", macAddress, info->disconnectedReason);
}

static void OnHotspotStaJoin(StationInfo* info)
{
    g_joinedStations++;
    PrintStationInfo(info);
    printf("+OnHotspotStaJoin: active stations = %d.\r\n", g_joinedStations);
}

static void OnHotspotStaLeave(StationInfo* info)
{
    g_joinedStations--;
    PrintStationInfo(info);
    printf("-OnHotspotStaLeave: active stations = %d.\r\n", g_joinedStations);
}

WifiEvent g_defaultWifiEventListener = {
    .OnHotspotStaJoin = OnHotspotStaJoin,
    .OnHotspotStaLeave = OnHotspotStaLeave,
    .OnHotspotStateChanged = OnHotspotStateChanged,
};

static struct netif* g_iface = NULL;

int StartHotspot(const HotspotConfig* config)
{
    WifiErrorCode errCode = WIFI_SUCCESS;

    errCode = RegisterWifiEvent(&g_defaultWifiEventListener);
    printf("RegisterWifiEvent: %d\r\n", errCode);

    errCode = SetHotspotConfig(config);
    printf("SetHotspotConfig: %d\r\n", errCode);

    g_hotspotStarted = 0;
    errCode = EnableHotspot();
    printf("EnableHotspot: %d\r\n", errCode);

    while (!g_hotspotStarted) {
        osDelay(DELAY_TICKS_10);
    }
    printf("g_hotspotStarted = %d.\r\n", g_hotspotStarted);

    g_iface = netifapi_netif_find("ap0");
    if (g_iface) {
        ip4_addr_t ipaddr;
        ip4_addr_t gateway;
        ip4_addr_t netmask;

        IP4_ADDR(&ipaddr,  192, 168, 1, 1);     /* input your IP for example: 192 168 1 1 */
        IP4_ADDR(&gateway, 192, 168, 1, 1);     /* input your gateway for example: 192 168 1 1 */
        IP4_ADDR(&netmask, 255, 255, 255, 0);   /* input your netmask for example: 255 255 255 0 */
        err_t ret = netifapi_netif_set_addr(g_iface, &ipaddr, &netmask, &gateway);
        printf("netifapi_netif_set_addr: %d\r\n", ret);

        ret = netifapi_dhcps_stop(g_iface); // 海思扩展的HDCP服务接口
        printf("netifapi_dhcps_stop: %d\r\n", ret);

        ret = netifapi_dhcps_start(g_iface, 0, 0); // 海思扩展的HDCP服务接口
        printf("netifapi_dhcp_start: %d\r\n", ret);
    }
    return errCode;
}

void StopHotspot(void)
{
    if (g_iface) {
        err_t ret = netifapi_dhcps_stop(g_iface);  // 海思扩展的HDCP服务接口
        printf("netifapi_dhcps_stop: %d\r\n", ret);
    }

    WifiErrorCode errCode = UnRegisterWifiEvent(&g_defaultWifiEventListener);
    printf("UnRegisterWifiEvent: %d\r\n", errCode);

    errCode = DisableHotspot();
    printf("EnableHotspot: %d\r\n", errCode);
}

void UartTask2(void)
{
    //NB_Connect();
     NB_huawei();
}
static void WifiHotspotTask(void)
{
    WifiErrorCode errCode;
    HotspotConfig config = {0};

    // strcpy(config.ssid, "ABCD");
    // strcpy(config.preSharedKey, "12345678");
    strcpy_s(config.ssid, WIFI_MAX_SSID_LEN, AP_SSID);
    strcpy_s(config.preSharedKey, WIFI_MAX_KEY_LEN, AP_SKEY);
    config.securityType = WIFI_SEC_TYPE_PSK;
    config.band = HOTSPOT_BAND_TYPE_2G;
    config.channelNum = CH_NUM;

    osDelay(DELAY_TICKS_10);
//热点将开启1分钟
    printf("starting AP ...\r\n");
    errCode = StartHotspot(&config);
    printf("StartHotspot: %d\r\n", errCode);
//热点将开启1分钟
    int timeout = COUNTER;
    UdpServerTest(5678);
    while (1) {
        printf("AP ON！\n");
        osDelay(DELAY_TICKS_500);
    }
  // 可以通过串口工具发送 AT+PING=192.168.xxx.xxx(如手机连接到该热点后的IP) 去ping连接到该热点的设备的IP地址 
    printf("stop AP ...\r\n");
    //StopHotspot();
    printf("stop AP ...\r\n");
}

void UdpServerTest(unsigned short port)
{
    UartInit();
    ssize_t retval = 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
    int queryCode=99;                            //北向发过来的查询码，初始化为异常值
    struct sockaddr_in clientAddr = {0};
    socklen_t clientAddrLen = sizeof(clientAddr);
    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (retval < 0) {
        printf("bind failed, %ld!\r\n", retval);
         printf("do_cleanup...\r\n");
        close(sockfd);
    }
    printf("retval:%d\n",retval);
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


static void WifiHotspotDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "WifiHotspotTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = STACK_SIZE*2;
    attr.priority = osPriorityNormal;

    if (osThreadNew(WifiHotspotTask, NULL, &attr) == NULL) {
        printf("[WifiHotspotDemo] Falied to create WifiHotspotTask!\n");
    }
    
    attr.name = "UartTask2";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 8 * 1024; // 任务栈大小*1024 stack size 5*1024
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)UartTask2, NULL, &attr) == NULL)
    {
        printf("[UartTask2] Failed to create UartTask!\n");
    }

}

APP_FEATURE_INIT(WifiHotspotDemo);

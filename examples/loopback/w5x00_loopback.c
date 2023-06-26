/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "w5x00_spi.h"

#include "loopback.h"

#include "w5100s.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_LOOPBACK 0

/* Port */
#define PORT_LOOPBACK 5000

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 56, 177},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 56, 2},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
        .dhcp = NETINFO_STATIC                       // DHCP enable/disable
};

/* Loopback */
static uint8_t g_loopback_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    /* Initialize */
    int retval = 0;

    set_clock_khz();

    stdio_init_all();

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    // 1. Version 확인 ----------------------------------------
    // uint8_t version = getVER();
    // printf("Ver: 0x%02X\n", version);

    // 2. 한 바이트 레지스터 설정 ------------------------------
    // // RMSR 레지스터에 값을 설정하는 예시
    // uint8_t rmsr_value = 0x54; // default = 0x55
    // setRMSR(rmsr_value);

    // // RMSR 레지스터 값을 읽어오는 예시
    // uint8_t read_rmsr = getRMSR();

    // // 값 출력
    // printf("RMSR value: 0x%02X\n", read_rmsr);

    // 3. 여러 바이트 레지스터 설정 ----------------------------
    // // 4바이트로 이루어진 배열 선언
    // uint8_t subr_array[4] = {0xFF, 0xFF, 0xFF, 0x01};

    // // SUBR 레지스터에 값을 설정하기 위해 setSUBR 함수 호출
    // setSUBR(subr_array);

    // // SUBR 레지스터 값을 읽어오기 위해 getSUBR 함수 호출
    // uint8_t read_subr_array[4];
    // getSUBR(read_subr_array);

    // // 값 출력
    // printf("SUBR value: 0x%02X 0x%02X 0x%02X 0x%02X\n", 
    //        read_subr_array[0], read_subr_array[1], read_subr_array[2], read_subr_array[3]);

    network_initialize(g_net_info);

    /* Get network information */
    print_network_information(g_net_info);

    /* Infinite loop */
    while (1)
    {
        /* TCP server loopback test */
        if ((retval = loopback_tcps(SOCKET_LOOPBACK, g_loopback_buf, PORT_LOOPBACK)) < 0)
        {
            printf(" Loopback error : %d\n", retval);

            while (1)
                ;
        }
        if (getSn_SR(SOCKET_LOOPBACK) == SOCK_ESTABLISHED)
        {
            if (getSn_RX_RSR(SOCKET_LOOPBACK) > 0)
            {
                retval = loopback_tcps(SOCKET_LOOPBACK, g_loopback_buf, PORT_LOOPBACK);

                if (retval > 0)
                {
                    // 클라이언트로부터 메시지를 받은 경우에만 처리
                    // 클라이언트가 보낸 메시지의 첫 번째 바이트를 IR 값으로 사용 (클라이언트가 보낸 메시지의 첫 번째 바이트로 가정)
                    uint8_t rmsr_value = g_loopback_buf[0]; // default = 0x55
                    setRMSR(rmsr_value);

                    // RMSR 레지스터 값을 읽어오는 예시
                    uint8_t read_rmsr = getRMSR();

                    // 값 출력
                    printf("Set RMSR value: 0x%02X\n", rmsr_value);
                    printf("RMSR value: 0x%02X\n", read_rmsr);
                }
            }
        }
    }
}

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}

/*
 * timer.c
 *
 *  Created on: 2020Äê1ÔÂ22ÈÕ
 *      Author: wrangler
 */
#include <SI_C8051F310_Register_Enums.h>
#include "timer.h"

extern unsigned long Sys_Clk;
extern unsigned int Uart_Baud;

unsigned char Timer_GetUartCounter()
{
	//baud = T1_CLK_SRC / (256 - T1H) / 2
	unsigned long count;
	count = Sys_Clk / 12;
	count = count / 2;
	count = count / Uart_Baud;
	count = 256 - count;
	return count & 0xff;
}

void Timer_Init()
{
    //Mode2:8-bit with auto-reload
    TMOD = 0x20;
    //sysclk/12 as source clock,
    CKCON &= ~0x03;
    TH1 = Timer_GetUartCounter();
    //enable timer1
    TCON = 0x40;
}




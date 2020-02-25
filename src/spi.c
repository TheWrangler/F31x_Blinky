/*
 * spi.c
 *
 *  Created on: 2020Äê1ÔÂ22ÈÕ
 *      Author: wrangler
 */
#include <SI_C8051F310_Register_Enums.h>
#include "spi.h"

extern unsigned long Sys_Clk;

extern idata unsigned char cmd_msg[42];
unsigned char sta_cmd_send_count = 0;
unsigned char sta_cmd_send_complete = 1;

unsigned char SPI_GetClockRate()
{
	//F_SCK = F_SYSCLK / 2 / (SPI0CKR + 1)
	unsigned long count = Sys_Clk / 2;
	count = count / 2500000;//SCK = 2.5Mbps
	count = count - 1;
	return count & 0xff;
}

void SPI_Init()
{
	//rising edge output data,falling edge acquire data

	//enable master mode,SCK=0 when idle
    SPI0CFG   |= 0x40;
    //3-wire Single-Master Mode
    SPI0CN    = 0x01;
    //SCK = 2.5Mbps
    SPI0CKR = SPI_GetClockRate();
}

SI_INTERRUPT(SPI0_Interrupt, 6)
{
	if(SPI0CN & 0x80 == 0x80)
	{
		SPI0CN &= ~0x80;

		if(sta_cmd_send_count == 42)
		{
			sta_cmd_send_complete = 1;
			return;
		}

		SPI0DAT = cmd_msg[sta_cmd_send_count];
		sta_cmd_send_count++;
	}

	SPI0CN &= ~0x70;
}

void SPI_Send(/*unsigned char* pBuf,unsigned char len*/)
{
	sta_cmd_send_count = 0;
	sta_cmd_send_complete = 0;
	SPI0CN |= 0x80;

//	unsigned char i;
//	for(i=0;i<len;i++)
//	{
//		while(SPI0CN & 0x02 != 0x02){;}
//		SPI0DAT = pBuf[i];
//	}
}

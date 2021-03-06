/*
 * uart.c
 *
 *  Created on: 2020��1��22��
 *      Author: wrangler
 */
#include <SI_C8051F310_Register_Enums.h>
#include "uart.h"
#include "../sta.h"
#include "../cmd.h"

unsigned int Uart_Baud = 9600;
unsigned char Uart_Recv_buff[UART_RECV_BUF_NUM];
unsigned char Uart_Recv_buff_size = 0;
extern unsigned char packet_buff[ASCII_FRAME_LEN];
extern unsigned char packet_buff_size;

extern idata unsigned char sta_msg[STA_MSG_LEN];
extern unsigned char sta_msg_len;
unsigned char sta_msg_send_count = 0;
unsigned char sta_msg_send_complete = 1;

void UART_Init()
{
	//8-bit Uart Mode
	//receive enable
    SCON0     = 0x10;
}

SI_INTERRUPT(UART0_Interrupt, 4)
{
	if((SCON0 & 0x01) == 0x01)//receive
	{
		SCON0 &= ~0x01;//clear Recv. INT Flag

		if(Uart_Recv_buff_size == UART_RECV_BUF_NUM)
			Uart_Recv_buff_size = 0;

		Uart_Recv_buff[Uart_Recv_buff_size] = SBUF0;
		Uart_Recv_buff_size++;
//      if(packet_buff_size == ASCII_FRAME_LEN)
//        packet_buff_size = 0;
//
//      packet_buff[packet_buff_size] = SBUF0;
//      packet_buff_size++;
	}

	if((SCON0 & 0x02) == 0x02)
	{
		SCON0 &= ~0x02;
		if(sta_msg_send_count == sta_msg_len)
		{
			sta_msg_send_complete = 1;
			return;
		}

		SBUF0 = sta_msg[sta_msg_send_count];
		sta_msg_send_count++;
	}
}

void UART_Send()
{
	sta_msg_send_count = 0;
	sta_msg_send_complete = 0;
	SCON0 |= 0x02;
}

void UART_GetBuff(/*unsigned char* pBuf,unsigned char len*/)
{
	unsigned char i=0,len;
	unsigned char temp = Uart_Recv_buff_size;

	if(Uart_Recv_buff_size == 0)
		return;

	len = ASCII_FRAME_LEN - packet_buff_size;
	if(len > temp)
		len = temp;

	for(i=0;i<len;i++)
	  packet_buff[packet_buff_size + i] = Uart_Recv_buff[i];

	//atom process
	temp = Uart_Recv_buff_size - len;
	Uart_Recv_buff_size = temp;
	for(i=0;i<temp;i++)
		Uart_Recv_buff[i] = Uart_Recv_buff[len + i];


	packet_buff_size += len;
}

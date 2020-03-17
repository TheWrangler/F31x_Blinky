/*
 * cmd.c
 *
 *  Created on: 2020年1月27日
 *      Author: wrangler
 */
#include "./src/uart.h"
#include "cmd.h"

//cmd format
//0-1 eb 90
//2-5 ftw_lower_1
//6-9 ftw_upper_1
//10-13 ftw_lower_2
//14-17 ftw_upper_2
//18-21 step
//22-23 rate
//24-27 re-sweep period
//28 tx衰减
//29 系统模式， 01-tv,11-tr/tv切换,bit2-校准使能，bit3-射频开关
//30 rx三个接收通道供电控制，bit0-通道1，bit1-通道2，bit2-通道3
//31-33 rx三个接收通道衰减
//34-36 rx三个接收通道移相控制
//37~40 校准时长
//41 crc

idata unsigned char cmd_msg[42] = {0xeb,0x90};

unsigned char Uart_buff[15];
unsigned char Uart_buff_size = 0;

float step_scale = 4294967296.0 / 3400.0;//2^32/fs
float clock_scale = 24.0 / 3400.0;  //us

unsigned long freq_modulate = 20;//MHz/us
unsigned long freq_sweep_bw = 250;//us
unsigned long freq_mid = 17500;//MHz

//求最大公约数
unsigned long gcd(unsigned long x, unsigned long y)
{
	unsigned long gcdNum = 1;
	while (x != y)
	{
		if (x % 2 == 0 && y % 2 == 0)
		{
			x /= 2;
			y /= 2;
			gcdNum *= 2;
		}
		else
		{
			if (x > y)
				x -= y;
			else
				y -= x;
		}
	}
	return gcdNum * x;
}

unsigned long String2Long(unsigned char* buf,unsigned char dec)
{
	unsigned char i;
	unsigned long res=0,var,temp = 1;
	for(i=0;i<dec;i++)
	{
		var = buf[dec - i - 1] - 0x30;
		var *= temp;
		res += var;

		temp *= 10;
	}

	return res;
}

void Format2Cmd(unsigned char* cmd,unsigned long var)
{
	cmd[0] = ((var & 0xff000000) >> 24);
	cmd[1] = ((var & 0xff0000) >> 16);
	cmd[2] = ((var & 0xff00) >> 8);
	cmd[3] = var;
}

void FTWGen(unsigned long freq)
{
	unsigned long ftw=0,div;
	float temp,m,n;

	//整数频率
	ftw = freq_sweep_bw >> 1;
	ftw = freq - ftw;
	div = gcd(ftw,3400);
	m = ftw / div;
	n = 3400 / div;
	temp = 65536.0 / n;
	temp *= 65536.0;
	temp *= m;
	ftw = temp;
	cmd_msg[2] = ((ftw & 0xff000000) >> 24);
	cmd_msg[3] = ((ftw & 0xff0000) >> 16);
	cmd_msg[4] = ((ftw & 0xff00) >> 8);
	cmd_msg[5] = ftw;

	ftw = freq_sweep_bw >> 1;
	ftw = freq + ftw;
	div = gcd(ftw,3400);
	m = ftw / div;
	n = 3400 / div;
	temp = 65536.0 / n;
	temp *= 65536.0;
	temp *= m;
	ftw = temp;
	cmd_msg[6] = ((ftw & 0xff000000) >> 24);
	cmd_msg[7] = ((ftw & 0xff0000) >> 16);
	cmd_msg[8] = ((ftw & 0xff00) >> 8);
	cmd_msg[9] = ftw;

	//小数频率
	ftw = freq_sweep_bw >> 1;
	m = freq + 2.5 - ftw;
	ftw = m *10.0;
	div = gcd(ftw,34000);
	m = ftw / div;
	n = 34000 / div;
	temp = 65536.0 / n;
	temp *= 65536.0;
	temp *= m;
	ftw = temp;
	cmd_msg[10] = ((ftw & 0xff000000) >> 24);
	cmd_msg[11] = ((ftw & 0xff0000) >> 16);
	cmd_msg[12] = ((ftw & 0xff00) >> 8);
	cmd_msg[13] = ftw;

	ftw = freq_sweep_bw >> 1;
	m = freq + 2.5 + ftw;
	ftw = m * 10.0;
	div = gcd(ftw,34000);
	m = ftw / div;
	n = 34000 / div;
	temp = 65536.0 / n;
	temp *= 65536.0;
	temp *= m;
	ftw = temp;
	cmd_msg[14] = ((ftw & 0xff000000) >> 24);
	cmd_msg[15] = ((ftw & 0xff0000) >> 16);
	cmd_msg[16] = ((ftw & 0xff00) >> 8);
	cmd_msg[17] = ftw;
}

void BuffDel(unsigned char len)
{
	unsigned char i;
	Uart_buff_size -= len;
	for(i=0;i<Uart_buff_size;i++)
		Uart_buff[i] = Uart_buff[i+len];
}

void CmdCrcCalc()
{
	unsigned char i;
	cmd_msg[41] = 0;
	for(i=0;i<41;i++)
		cmd_msg[41] += cmd_msg[i];
}

unsigned char Frame2CmdConvert()
{
	unsigned char len;
	unsigned long var;
	float fvar;

	len = UART_GetBuff(Uart_buff + Uart_buff_size,15-Uart_buff_size);
	Uart_buff_size += len;

	if(Uart_buff_size < 4)
		return 0;

	if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'K')
			&& (Uart_buff[2] == 'R'))
	{
		if(Uart_buff_size < 10)
			return 0;

		var = String2Long(Uart_buff+4,6);
		var /= 32768;
		freq_modulate = var;

		len = 10;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'P')
			&& (Uart_buff[2] == 'W'))
	{
		if(Uart_buff_size < 11)
			return 0;

		var = String2Long(Uart_buff+4,7);
		freq_sweep_bw = freq_modulate * var;
		freq_sweep_bw /= 4;

		fvar = var / clock_scale;
		fvar = freq_sweep_bw / fvar;
		var = fvar * step_scale;
		Format2Cmd(cmd_msg+18,var);

		cmd_msg[22] = 0x00;
		cmd_msg[23] = 0x01;

		len = 11;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'G')
			&& (Uart_buff[2] == 'N'))
	{
		if(Uart_buff_size < 6)
			return 0;

		len = 6;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'R')
			&& (Uart_buff[2] == 'F'))
	{
		if(Uart_buff_size < 5)
			return 0;
		if(Uart_buff[4] == '1')
			cmd_msg[29] |= 0x08;
		else cmd_msg[29] &= ~0x08;
		len = 5;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'C')
			&& (Uart_buff[2] == 'T'))
	{
		if(Uart_buff_size < 10)
			return 0;

		var = String2Long(Uart_buff+4,6);
		if(var != 0)
			cmd_msg[29] |= 0x04;
		else cmd_msg[29] &= ~0x04;

		fvar = var;
		fvar /= 10.0;
		fvar /= clock_scale;
		fvar *= 1000000.0;
		var = fvar;
		Format2Cmd(cmd_msg+37,var);

		len = 10;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'F')
			&& (Uart_buff[2] == 'R')
			&& (Uart_buff[3] == 'Q'))
	{
		if(Uart_buff_size < 10)
			return 0;

		var = String2Long(Uart_buff+5,5);
		var -= 13500;
		var /= 4;

		freq_mid = var;

		len = 10;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'P')
			&& (Uart_buff[2] == 'R')
			&& (Uart_buff[3] == 'T'))
	{
		if(Uart_buff_size < 12)
			return 0;

		var = String2Long(Uart_buff+5,7);
		var /= clock_scale;
		Format2Cmd(cmd_msg+24,var);
		len = 12;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'M')
			&& (Uart_buff[2] == 'O')
			&& (Uart_buff[3] == 'D'))
	{
		if(Uart_buff_size < 6)
			return 0;

		cmd_msg[29] &= ~0x03;
		cmd_msg[30] &= ~0x07;
		if(Uart_buff[5] == '1')
		{
			cmd_msg[29] |= 0x01;
			cmd_msg[30] |= 0x01;
		}
		else if(Uart_buff[5] == '2')
		{
			cmd_msg[29] |= 0x03;
			cmd_msg[30] |= 0x03;
		}
		else if(Uart_buff[5] == '3')
		{
			cmd_msg[29] |= 0x01;
			cmd_msg[30] |= 0x03;
		}
		else if(Uart_buff[5] == '4')
		{
			cmd_msg[29] |= 0x01;
			cmd_msg[30] |= 0x07;
		}
		len = 6;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'A')
			&& (Uart_buff[2] == 'T')
			&& (Uart_buff[3] == 'T'))
	{
		if(Uart_buff_size < 7)
			return 0;
		var = String2Long(Uart_buff+5,2);
		var = (var << 1);
		cmd_msg[28] = var;
		cmd_msg[28] = ~cmd_msg[28];

		len = 7;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'A')
			&& (Uart_buff[2] == 'A')
			&& (Uart_buff[3] == 'A'))
	{
		if(Uart_buff_size < 7)
			return 0;
		var = String2Long(Uart_buff+5,2);
		var = ~var;
		cmd_msg[31] = 0;
		cmd_msg[31] |= (var & 0x01)<<5;
		cmd_msg[31] |= (var & 0x02)<<3;
		cmd_msg[31] |= (var & 0x04)<<1;
		cmd_msg[31] |= (var & 0x08)>>1;
		cmd_msg[31] |= (var & 0x10)>>3;
		cmd_msg[31] |= (var & 0x20)>>5;
		len = 7;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'P')
			&& (Uart_buff[2] == 'S')
			&& (Uart_buff[3] == 'A'))
	{
		if(Uart_buff_size < 7)
			return 0;
		var = String2Long(Uart_buff+5,2);
		cmd_msg[34] = var;
		len = 7;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'A')
			&& (Uart_buff[2] == 'A')
			&& (Uart_buff[3] == 'B'))
	{
		if(Uart_buff_size < 7)
			return 0;
		var = String2Long(Uart_buff+5,2);
		var = ~var;
		cmd_msg[32] = 0;
		cmd_msg[32] |= (var & 0x01)<<5;
		cmd_msg[32] |= (var & 0x02)<<3;
		cmd_msg[32] |= (var & 0x04)<<1;
		cmd_msg[32] |= (var & 0x08)>>1;
		cmd_msg[32] |= (var & 0x10)>>3;
		cmd_msg[32] |= (var & 0x20)>>5;
		len = 7;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'P')
			&& (Uart_buff[2] == 'S')
			&& (Uart_buff[3] == 'B'))
	{
		if(Uart_buff_size < 7)
			return 0;
		var = String2Long(Uart_buff+5,2);
		cmd_msg[35] = var;
		len = 7;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'A')
			&& (Uart_buff[2] == 'A')
			&& (Uart_buff[3] == 'C'))
	{
		if(Uart_buff_size < 7)
			return 0;
		var = String2Long(Uart_buff+5,2);
		var = ~var;
		cmd_msg[33] = 0;
		cmd_msg[33] |= (var & 0x01)<<5;
		cmd_msg[33] |= (var & 0x02)<<3;
		cmd_msg[33] |= (var & 0x04)<<1;
		cmd_msg[33] |= (var & 0x08)>>1;
		cmd_msg[33] |= (var & 0x10)>>3;
		cmd_msg[33] |= (var & 0x20)>>5;
		len = 7;
	}
	else if((Uart_buff[0] == '#')
			&& (Uart_buff[1] == 'P')
			&& (Uart_buff[2] == 'S')
			&& (Uart_buff[3] == 'C'))
	{
		if(Uart_buff_size < 7)
			return 0;
		var = String2Long(Uart_buff+5,2);
		cmd_msg[36] = var;
		len = 7;

		FTWGen(freq_mid);
		CmdCrcCalc();
		BuffDel(len);
		return 1;
	}
	else len = 1;

	BuffDel(len);

	return 0;
}




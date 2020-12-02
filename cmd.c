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
//24-25 pulse_period,us
//26-28 re-sweep period,us
//29 tx衰减
//30 bit0~bit2系统模式 001-tv,010-th/tv切换,011-th/tv固定,100-一发三收,bit3-射频开关
//31-33 rx三个接收通道衰减
//34-36 rx三个接收通道移相控制
//37~40 校准时长,ms
//41~44 触发延迟,us
//45 crc

idata unsigned char cmd_msg[CMD_MSG_LEN] = {0xeb,0x90};
unsigned char cmd_msg_valid = 0;

unsigned char packet_buff[ASCII_FRAME_LEN];
unsigned char packet_buff_size = 0;

double step_scale = 4294967296.0 / 3400.0;//2^32/fs
double clock_scale = 24.0 / 3400.0;  //us

unsigned long freq_modulate = 20;//MHz/us
unsigned long freq_sweep_bw = 250;//us
unsigned long freq_mid = 17500;//MHz

extern unsigned char spi_cmd_send_complete;

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

//字符串转数值
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

//32位转换位数组
void Format2Cmd(unsigned char* cmd,unsigned long var,unsigned char byte_width)
{
  if(byte_width == 4)
  {
      cmd[0] = ((var & 0xff000000) >> 24);
      cmd[1] = ((var & 0xff0000) >> 16);
      cmd[2] = ((var & 0xff00) >> 8);
      cmd[3] = var;
  }
  else if(byte_width == 3)
  {
      cmd[0] = ((var & 0xff0000) >> 16);
      cmd[1] = ((var & 0xff00) >> 8);
      cmd[2] = var;
  }
  else
  {
      cmd[0] = ((var & 0xff00) >> 8);
      cmd[1] = var;
  }
}

//FTW计算
void FTWGen(unsigned long freq,unsigned char* cmd_msg)
{
	unsigned long ftw=0,div;
	double temp,m,n;

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
	cmd_msg[0] = ((ftw & 0xff000000) >> 24);
	cmd_msg[1] = ((ftw & 0xff0000) >> 16);
	cmd_msg[2] = ((ftw & 0xff00) >> 8);
	cmd_msg[3] = ftw;

	ftw = freq_sweep_bw >> 1;
	ftw = freq + ftw;
	div = gcd(ftw,3400);
	m = ftw / div;
	n = 3400 / div;
	temp = 65536.0 / n;
	temp *= 65536.0;
	temp *= m;
	ftw = temp;
	cmd_msg[4] = ((ftw & 0xff000000) >> 24);
	cmd_msg[5] = ((ftw & 0xff0000) >> 16);
	cmd_msg[6] = ((ftw & 0xff00) >> 8);
	cmd_msg[7] = ftw;
}

void BuffDel(unsigned char len)
{
	unsigned char i;
	packet_buff_size -= len;
	for(i=0;i<packet_buff_size;i++)
	  packet_buff[i] = packet_buff[i+len];
}

//计算累加和校验
void CmdCrcCalc()
{
	unsigned char i;
	cmd_msg[CMD_MSG_LEN-1] = 0;
	for(i=0;i<CMD_MSG_LEN-1;i++)
		cmd_msg[CMD_MSG_LEN-1] += cmd_msg[i];
}

unsigned char FrameHeaderSearch()
{
  unsigned char i=0;
  while(i+3<packet_buff_size)
  {
      if((packet_buff[i] != '#')
          || (packet_buff[i+1] != 'K')
          || (packet_buff[i+2] != 'R')
          || (packet_buff[i+3] != ':'))
      {
        i++;
        continue;
      }

      break;
  }

  return i;
}

//ASCII命令转二进制命令
unsigned char Frame2CmdConvert()
{
	unsigned char len;

//	len = UART_GetBuff(packet_buff + packet_buff_size,ASCII_FRAME_LEN-packet_buff_size);
//	packet_buff_size += len;

	if(packet_buff_size < ASCII_FRAME_LEN)
		return 0;

	if(spi_cmd_send_complete != 1)
	  return 0;

	len = FrameHeaderSearch();
	if(len != 0)
	{
	  BuffDel(packet_buff_size - len);
	  return 0;
	}

	if(KRParser(packet_buff) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(FreqParser(packet_buff+11) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(PWParser(packet_buff+22) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(PRTParser(packet_buff+34) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(ATTParser(packet_buff+47) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(RFParser(packet_buff+62) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(MODParser(packet_buff+68) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(CTParser(packet_buff+75) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(AAParser(packet_buff+86,31) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(PSParser(packet_buff+94,34) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(AAParser(packet_buff+102,32) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(PSParser(packet_buff+110,35) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(AAParser(packet_buff+118,33) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(PSParser(packet_buff+126,36) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	if(YSParser(packet_buff+140) == 0)
	{
		packet_buff_size = 0;
		return 0;
	}

	packet_buff_size = 0;

  FTWGen(freq_mid,cmd_msg+2);
  FTWGen(freq_mid+2.5,cmd_msg+10);
  CmdCrcCalc();

  cmd_msg_valid = 1;
	return 1;
}

unsigned char KRParser(unsigned char* buf)
{
  unsigned long var;

  if((buf[0] != '#')
        || (buf[1] != 'K')
        || (buf[2] != 'R')
        || (buf[3] != ':')
        || (buf[10] != ';'))
    return 0;

  var = String2Long(buf+4,6);
  var /= 32768;
  freq_modulate = var;

  return 1;
}

unsigned char FreqParser(unsigned char* buf)
{
  unsigned long var;

  if((buf[0] != '#')
          || (buf[1] != 'F')
          || (buf[2] != 'R')
          || (buf[3] != 'Q')
          || (buf[4] != ':')
          || (buf[10] != ';'))
    return 0;

  var = String2Long(buf+5,5);
  var -= 13500;
  var /= 4;

  freq_mid = var;
  return 1;
}

unsigned char PWParser(unsigned char* buf)
{
	unsigned long var;
	double fvar;

	if((buf[0] != '#')
		|| (buf[1] != 'P')
		|| (buf[2] != 'W')
		|| (buf[3] != ':')
		|| (buf[11] != ';'))
		return 0;

	var = String2Long(buf+4,7);
	Format2Cmd(cmd_msg+24,var,2);

	freq_sweep_bw = freq_modulate * var;
	freq_sweep_bw /= 4;

	fvar = var / clock_scale;
	fvar = freq_sweep_bw / fvar;
	var = fvar * step_scale;
	Format2Cmd(cmd_msg+18,var,4);

	cmd_msg[22] = 0x00;
	cmd_msg[23] = 0x01;

	return 1;
}

unsigned char PRTParser(unsigned char * buf)
{
	unsigned long var;

	if((buf[0] != '#')
			|| (buf[1] != 'P')
			|| (buf[2] != 'R')
			|| (buf[3] != 'T')
			|| (buf[4] != ':')
			|| (buf[12] != ';'))
		return 0;

  var = String2Long(buf+5,7);
  //var /= clock_scale;
  Format2Cmd(cmd_msg+26,var,3);

  return 1;
}

unsigned char ATTParser(unsigned char* buf)
{
	unsigned long var;

	if((buf[0] != '#')
	      || (buf[1] != 'A')
	      || (buf[2] != 'T')
	      || (buf[3] != 'T')
	      || (buf[4] != ':')
	      || (buf[7] != ';'))
		return 0;


  var = String2Long(buf+5,2);
  if(var > 18)
    var = 18;
  if(var != 0)
    var += 13;

  var = (var << 1);

  cmd_msg[29] = var;
  cmd_msg[29] = ~cmd_msg[29];
  return 1;
}

unsigned char RFParser(unsigned char* buf)
{
	if((buf[0] != '#')
			|| (buf[1] != 'R')
			|| (buf[2] != 'F')
			|| (buf[3] != ':')
			|| (buf[5] != ';'))
		return 0;

	if(buf[4] == '1')
		cmd_msg[30] |= 0x08;
	else cmd_msg[30] &= ~0x08;

	return 1;
}

unsigned char MODParser(unsigned char* buf)
{
	if((buf[0] != '#')
	      || (buf[1] != 'M')
	      || (buf[2] != 'O')
	      || (buf[3] != 'D')
	      || (buf[4] != ':')
	      || (buf[6] != ';'))
		  return 0;

  cmd_msg[30] &= ~0x07;
  if(buf[5] == '1')
  {
    cmd_msg[30] |= 0x01;//一发一收tv
  }
  else if(buf[5] == '2')
  {
    cmd_msg[30] |= 0x02;//一发两收tr/tv
  }
  else if(buf[5] == '3')
  {
    cmd_msg[30] |= 0x03;//一发两收tr+tv
  }
  else if(buf[5] == '4')
  {
    cmd_msg[30] |= 0x04;//一发三收
  }

  return 1;
}

unsigned char CTParser(unsigned char* buf)
{
	unsigned long var;
	//double fvar;

	if((buf[0] != '#')
			|| (buf[1] != 'C')
			|| (buf[2] != 'T')
			|| (buf[3] != ':')
			|| (buf[10] != ';'))
		return 0;

	var = String2Long(buf+4,6);
	var *= 100;
	Format2Cmd(cmd_msg+37,var,4);

	return 1;
}

unsigned char AAParser(unsigned char* buf,unsigned char index)
{
	unsigned long var;

	if((buf[0] != '#')
			|| (buf[1] != 'A')
			|| (buf[2] != 'A')
			/*|| (buf[3] != 'A')*/
			|| (buf[4] != ':')
			|| (buf[7] != ';'))
		return 0;

	var = String2Long(buf+5,2);
//	var = ~var;
//	cmd_msg[index] = 0;
//	cmd_msg[index] |= (var & 0x01)<<5;
//	cmd_msg[index] |= (var & 0x02)<<3;
//	cmd_msg[index] |= (var & 0x04)<<1;
//	cmd_msg[index] |= (var & 0x08)>>1;
//	cmd_msg[index] |= (var & 0x10)>>3;
//	cmd_msg[index] |= (var & 0x20)>>5;
	cmd_msg[index] = var;

	return 1;
}

unsigned char PSParser(unsigned char* buf,unsigned char index)
{
	unsigned long var;

	if((buf[0] != '#')
			|| (buf[1] != 'P')
			|| (buf[2] != 'S')
			/*|| (buf[3] != 'A')*/
			|| (buf[4] != ':')
			|| (buf[7] != ';'))
		return 0;

	var = String2Long(buf+5,2);
	cmd_msg[index] = var;

	return 1;
}

unsigned char YSParser(unsigned char* buf)
{
	unsigned long var;
	//double fvar;

	if((buf[0] != '#')
	        || (buf[1] != 'Y')
	        || (buf[2] != 'S')
	        || (buf[3] != ':')
	        || (buf[6] != ';'))
		return 0;

  var = String2Long(buf+4,2);
//  fvar = var;
//  fvar /= clock_scale;
//  var = fvar;
  Format2Cmd(cmd_msg+41,var,4);

  return 1;
}



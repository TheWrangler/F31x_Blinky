/*
 * sta.c
 *
 *  Created on: 2020年1月27日
 *      Author: wrangler
 */
#include "sta.h"
#include "cmd.h"

idata unsigned char sta_msg[STA_MSG_LEN];
unsigned char sta_msg_len = 0;

float temperature = 0.0;
float bite = 0.0;

extern unsigned char packet_buff[ASCII_FRAME_LEN];

unsigned char Float_Format(float var,unsigned char* pBuf)
{
	unsigned int byte;
	unsigned char index=0;

	//100/symbol
	if(var < 0)
	{
		pBuf[index] = '-';
		index++;
	}
	else
	{
		byte = var / 100.0;
		if(byte != 0)
		{
			pBuf[index] = byte + 0x30;
			index++;
		}
	}

	//10
	var -= (byte * 100);
	byte = var / 10.0;
	if(byte != 0)
	{
		pBuf[index] = byte + 0x30;
		index++;
	}

	//1
	var -= (byte * 10);
	byte = var;
	pBuf[index] = byte + 0x30;
	index++;

	//.
	pBuf[index] = '.';
	index++;

	//.1
	var -= byte;
	var *= 10;
	byte = var;
	pBuf[index] = byte + 0x30;
	index++;

	return index;
}

void Teamperature_Convert(unsigned int adc_var)
{
	temperature = adc_var;
	temperature /= 1024.0;
	temperature *= 3300;
	temperature -= 897;
	temperature /= 3.35;
}

void Bite_Convert(unsigned int adc_var)
{
	bite = adc_var;
	bite *= 3.3;
	bite /= 4096.0;
//	bite *= 33.7985;
//	bite += 16.9617;
}

void UpdateFrame()
{
  unsigned char i;
  for(i=0;i<ASCII_FRAME_LEN;i++)
    sta_msg[i] = packet_buff[i];
}

void UpdateSta()
{
  unsigned char len;

  sta_msg_len = ASCII_FRAME_LEN;

  sta_msg[sta_msg_len++] = '#';
  sta_msg[sta_msg_len++] = 'T';
  sta_msg[sta_msg_len++] = 'P';
  sta_msg[sta_msg_len++] = ':';

  len = Float_Format(temperature,sta_msg+sta_msg_len);
  sta_msg_len += len;

  sta_msg[sta_msg_len++] = ';';

  sta_msg[sta_msg_len++] = '#';
  sta_msg[sta_msg_len++] = 'B';
  sta_msg[sta_msg_len++] = 'T';
  sta_msg[sta_msg_len++] = ':';

  len = Float_Format(bite,sta_msg+sta_msg_len);
  sta_msg_len += len;

  sta_msg[sta_msg_len++] = ';';
  sta_msg[sta_msg_len++] = '*';
  sta_msg[sta_msg_len++] = 0x0d;
  sta_msg[sta_msg_len++] = 0x0a;
}

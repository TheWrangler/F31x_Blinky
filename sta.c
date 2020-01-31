/*
 * sta.c
 *
 *  Created on: 2020Äê1ÔÂ27ÈÕ
 *      Author: wrangler
 */
#include "sta.h"

extern idata unsigned char sta_msg[18];
extern unsigned char sta_msg_len;

float temperature = 0.0;
float bite = 0.0;

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

}

void State_Msg()
{
	sta_msg_len = 5;
	sta_msg_len += Float_Format(temperature,sta_msg+sta_msg_len);
	sta_msg[sta_msg_len] = ',';
	sta_msg_len++;
	sta_msg_len += Float_Format(bite,sta_msg+sta_msg_len);

	sta_msg[sta_msg_len] = '*';
	sta_msg_len++;
	sta_msg[sta_msg_len] = 0x0d;
	sta_msg_len++;

	sta_msg[sta_msg_len] = 0x0a;
	sta_msg_len++;
}

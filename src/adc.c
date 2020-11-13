/*
 * adc.c
 *
 *  Created on: 2020��1��22��
 *      Author: wrangler
 */
#include <SI_C8051F310_Register_Enums.h>
#include "adc.h"

char adc_covert_completed = 1;

void ADC_Init()
{
	//positive input p3.2
    AMX0P     = 0x12;
    //negitive input gnd
    AMX0N     = 0x1F;
    //SAR clock 2.5MHz
    ADC0CF    = 0x48;
    //VDD as reference,enable internal temperature sensor
    REF0CN = 0x0f;
    //start convert when write 1 to AD0BUSY,enable ADC0
    ADC0CN    = 0x80;
}


char ADC_Get(unsigned int *pVar)
{
	unsigned int adc_var;

	if((ADC0CN & 0x10) != 0x10/*ADC_ConvertFinish == 1*/)
	{
		adc_var = ADC0H;
		adc_var = (adc_var << 8);
		adc_var += ADC0L;

		(*pVar) = adc_var;
		//ADC_ConvertFinish = 0;
		//ADC0CN &= ~0x20;
		//ADC0CN &= ~0x80;
		adc_covert_completed = 1;
		return 1;
	}
	else return 0;
}

void ADC_Start(char target)
{
  if(adc_covert_completed != 1)
    return;

	if(target == 0)
		AMX0P = 0x1e;
	else AMX0P = 0x12;

	//ADC0CN |= 0x80;
	ADC0CN |= 0x10;

	adc_covert_completed = 0;
}

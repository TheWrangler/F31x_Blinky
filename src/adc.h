/*
 * adc.h
 *
 *  Created on: 2020��1��22��
 *      Author: wrangler
 */

#ifndef ADC_H_
#define ADC_H_

void ADC_Init();
void ADC_ISR();
char ADC_Get(unsigned int *pVar);
void ADC_Start(/*char target*/);

#endif /* ADC_H_ */

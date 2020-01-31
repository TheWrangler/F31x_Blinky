/*
 * sta.h
 *
 *  Created on: 2020Äê1ÔÂ27ÈÕ
 *      Author: wrangler
 */

#ifndef STA_H_
#define STA_H_

unsigned char Float_Format(float var,unsigned char* pBuf);
void Teamperature_Convert(unsigned int adc_var);
void Bite_Convert(unsigned int adc_var);
void State_Msg();

#endif /* STA_H_ */

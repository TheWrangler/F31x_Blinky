/*
 * sta.h
 *
 *  Created on: 2020��1��27��
 *      Author: wrangler
 */

#ifndef STA_H_
#define STA_H_

#define STA_MSG_LEN 175

unsigned char Float_Format(float var,unsigned char* pBuf);
void Teamperature_Convert(unsigned int adc_var);
void Bite_Convert(unsigned int adc_var);
void UpdateFrame();
void UpdateSta();

#endif /* STA_H_ */

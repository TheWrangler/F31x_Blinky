/*
 * uart.h
 *
 *  Created on: 2020��1��22��
 *      Author: wrangler
 */

#ifndef UART_H_
#define UART_H_

void UART_Init();
void UART_Send();
unsigned char UART_GetBuff(unsigned char* pBuf,unsigned char len);

#endif /* UART_H_ */

/*
 * spi.h
 *
 *  Created on: 2020��1��23��
 *      Author: wrangler
 */

#ifndef SPI_H_
#define SPI_H_

unsigned char SPI_GetClockRate();
void SPI_Init();
void SPI_Send(/*unsigned char* pBuf,unsigned char len*/);

#endif /* SRC_SPI_H_ */

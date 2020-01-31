/*
 * cmd.h
 *
 *  Created on: 2020Äê1ÔÂ27ÈÕ
 *      Author: wrangler
 */

#ifndef SRC_CMD_H_
#define SRC_CMD_H_

unsigned long gcd(unsigned long x, unsigned long y);
unsigned long String2Long(unsigned char* buf,unsigned char dec);
void Format2Cmd(unsigned char* cmd,unsigned long var);
void FTWGen(unsigned long freq);
void BuffDel(unsigned char len);
void CmdCrcCalc();
unsigned char Frame2CmdConvert();

#endif /* SRC_CMD_H_ */

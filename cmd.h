/*
 * cmd.h
 *
 *  Created on: 2020��1��27��
 *      Author: wrangler
 */

#ifndef SRC_CMD_H_
#define SRC_CMD_H_

#define CMD_MSG_LEN 46
#define ASCII_FRAME_LEN 147

unsigned long gcd(unsigned long x, unsigned long y);
unsigned long String2Long(unsigned char* buf,unsigned char dec);
void Format2Cmd(unsigned char* cmd,unsigned long var,unsigned char byte_width);
void FTWGen(double freq,unsigned char* cmd_msg);
void BuffDel(unsigned char len);
void CmdCrcCalc();
unsigned char Frame2CmdConvert();

unsigned char KRParser(unsigned char* buf);
unsigned char FreqParser(unsigned char* buf);
unsigned char PWParser(unsigned char* buf);
unsigned char PRTParser(unsigned char * buf);
unsigned char ATTParser(unsigned char* buf);
unsigned char RFParser(unsigned char* buf);
unsigned char MODParser(unsigned char* buf);
unsigned char CTParser(unsigned char* buf);
unsigned char AAParser(unsigned char* buf,unsigned char index);
unsigned char PSParser(unsigned char* buf,unsigned char index);
unsigned char YSParser(unsigned char* buf);

#endif /* SRC_CMD_H_ */

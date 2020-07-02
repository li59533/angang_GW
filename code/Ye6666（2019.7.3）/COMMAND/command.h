#ifndef __COMMAND_H
#define __COMMAND_H			 
#include "sys.h"
#include "app.h"

//#define COMMAND_STOP 0x00
//#define COMMAND_START 0x01
#define COMMAND_SAMPLE_RATE_1 0x02
#define COMMAND_IP 0x03

#define MAX_COMMAND COMMAND_IP

#define DATA_AD 0x40
#define DATA_ROT 0x41

#define ID_START 0X100


u8 analy_cmd(unsigned char *pcmd,int len);



#endif

#ifndef __MODULE_1_H
#define __MODULE_1_H
#include "stm32f4xx.h"
#include "sys.h"


	
extern void initMODULE_1(void);
extern void setComUART3(uint8_t baudrate,uint8_t parity);
extern void MODULE1_ModeSet(uint8_t mode) ;
extern void module_1_receiveloop_debug(void);
extern void WriteConfigdatatoUSART3(uint8_t *sourdata,uint32_t length);
#endif

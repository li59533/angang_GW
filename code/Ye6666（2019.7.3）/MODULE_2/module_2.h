#ifndef __MODULE_2_H
#define __MODULE_2_H
#include "stm32f4xx.h"
#include "sys.h"


	
extern void initMODULE_2(void);
extern void setComUART6(uint8_t baudrate,uint8_t parity);
extern void MODULE2_ModeSet(uint8_t mode) ;
extern void WriteConfigdatatoUSART6(uint8_t *sourdata,uint32_t length);

#endif

#ifndef __GPIO_H
#define __GPIO_H
#include "stm32f4xx.h"
#include "sys.h"


	#define MCU1_NSS PBout(12)
	#define MCU2_NSS PHout(6)
	#define MCU3_NSS PHout(11)
	#define MCU4_NSS PHout(7)
	
	extern u8 spi_data_come[SLAVE_MACHINE_NUM];


void gpio_init(void);
extern u8 machine_num;

#endif

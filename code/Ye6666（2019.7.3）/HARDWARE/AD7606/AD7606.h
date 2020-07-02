#ifndef _AD7606_H
#define _AD7606_H
#include "stm32f4xx.h"




typedef enum
{
	AD_OS_X1 = 0,
	AD_OS_X2,
	AD_OS_X4,
	AD_OS_X8,
	AD_OS_X16,
	AD_OS_X32,
	AD_OS_X64
}AD7606_OS_E;

typedef enum
{
	AD_RANGE_5V = 0,
	AD_RANGE_10V
}AD7606_RANGE_SEL;

extern s16 *ps_current_tx;

void AD7606_Init(void);
void AD7606_Reset(void);
void AD7606_SetOS(uint8_t _AD_OS);
void AD7606_SetInputRange(uint8_t _AD_Range);
void AD7606_FSMCConfig(void);
void AD7606_CtrlLinesConfig(void);
void AD7606_SetSampleFreq(uint32_t _ulFreq);
void AD7606_Start_AD(void);
void AD7606_Stop_AD(void);



#endif

#ifndef __SPI_H
#define __SPI_H
#include "sys.h"
#include "ad7606.h"


#define SPI_SPEED_SET SPI_BaudRatePrescaler_8
#define SPI_BLOCK_SIZE  ( MAX_SPI_TX_POINTS*AD_CHANNELS*sizeof(int16_t) )

void SPI2_Init(u16 Speed);			 //³õÊ¼»¯SPI2¿Ú
void SPI2_DMA_Configuration(void);
void SPI2_SendRecive(int machine_id);
u8 SPI2_ReadWriteByte(u8 TxData);
extern int current_rx_machine;


#endif


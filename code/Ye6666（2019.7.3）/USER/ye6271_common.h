#ifndef __YE6271_COMMON_H__
#define __YE6271_COMMON_H__

#include "sys.h"

#define MAIN_BOARD 0
#define SLAVE_MACHINE_NUM 4
#define AD_CHANNELS 4
#define SPI_UDP_RATE 4
#define MAX_UDP_TX_POINTS 16  //128
#define MAX_SPI_TX_POINTS (MAX_UDP_TX_POINTS*SPI_UDP_RATE)
#define BUFFER_PER_MACHINE 2 

#endif


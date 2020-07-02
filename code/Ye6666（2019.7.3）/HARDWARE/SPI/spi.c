#include "ye6271_common.h"
#include "spi.h"
#include "string.h"
#include "lwip/opt.h"
#include "lwip_comm.h"
#include "lwip/lwip_sys.h"
#include "lwip/api.h"
#include "gpio.h"
#include "malloc.h"
#include "udp_server.h"

#define SPI2_DR_Addr  0x4000380c

u8 *BUFFER_DUMP;
int current_rx_machine;

u8 SPI2_ReadWriteByte(u8 TxData)
{		 			 
 
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET);
    	SPI_SendData(SPI2,TxData);
		
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)==RESET);
   	return SPI_ReceiveData(SPI2);
}


void SPI_GPIOConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);  //开启时钟
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;  //引脚初始化
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);  //打开引脚的复用功能
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2);
}

void SPI_Config(u16 Speed)
{
	SPI_InitTypeDef  SPI_InitStructure;
    SPI_GPIOConfig();
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);  //时钟
    
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //全双工模式
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;   //作为主机使用
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;   //数据长度8
    SPI_InitStructure.SPI_CPOL  = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;   //软件设置NSS功能
    SPI_InitStructure.SPI_BaudRatePrescaler = Speed;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2,&SPI_InitStructure);
    SPI_Cmd(SPI2,ENABLE);
}

void SPI2_Init(u16 Speed)
{	 
	BUFFER_DUMP = (u8 *)mymalloc(SRAMIN,SPI_BLOCK_SIZE);
	SPI_GPIOConfig();
	SPI_Config(Speed);
}


void SPI2_DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 , ENABLE);
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 

	DMA_DeInit(DMA1_Stream3);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = SPI2_DR_Addr;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BUFFER_DUMP;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = SPI_BLOCK_SIZE/2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);
 	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);

	DMA_DeInit(DMA1_Stream4);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0;
	DMA_InitStructure.DMA_PeripheralBaseAddr = SPI2_DR_Addr; 
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BUFFER_DUMP;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = SPI_BLOCK_SIZE/2;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream4, &DMA_InitStructure);
 	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

	current_rx_machine  = -1;
	DMA_ITConfig(DMA1_Stream3,DMA_IT_TC,ENABLE);

}

void SPI2_SendRecive(int machine_id)
{
	DMA1_Stream3->M0AR = (uint32_t)thread_contex[machine_id].bank[thread_contex[machine_id].current_write_bank];
	//DMA1_Stream3->M0AR = (uint32_t)thread_contex[machine_id].bank[0];
	DMA_SetCurrDataCounter(DMA1_Stream3, 0);//读
	DMA_SetCurrDataCounter(DMA1_Stream4, 0);//写
	DMA_SetCurrDataCounter(DMA1_Stream3, SPI_BLOCK_SIZE/2);//读
	DMA_SetCurrDataCounter(DMA1_Stream4, SPI_BLOCK_SIZE/2);//写
	DMA_ClearITPendingBit(DMA1_Stream3,DMA_IT_TCIF3|DMA_IT_FEIF3|DMA_IT_DMEIF3|DMA_IT_TEIF3|DMA_IT_HTIF3);
	DMA_ClearITPendingBit(DMA1_Stream4,DMA_IT_TCIF4|DMA_IT_FEIF4|DMA_IT_DMEIF4|DMA_IT_TEIF4|DMA_IT_HTIF4);
	DMA_Cmd(DMA1_Stream3, ENABLE);
	DMA_Cmd(DMA1_Stream4, ENABLE);
}

void DMA1_Stream3_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream3,DMA_IT_TCIF3))
	{
#if 1
		MCU1_NSS = 1;
		MCU2_NSS = 1;
		MCU3_NSS = 1;
		MCU4_NSS = 1;
#endif		
		if(current_rx_machine>=0&&current_rx_machine<SLAVE_MACHINE_NUM){			
			thread_contex[current_rx_machine].current_write_bank = (thread_contex[current_rx_machine].current_write_bank+1)&1;
			OSSemPost(thread_contex[current_rx_machine].sem);
			current_rx_machine = -1;
		}
		DMA_ClearITPendingBit(DMA1_Stream3,DMA_IT_TCIF3);
	}
}





void DMA1_Stream4_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream4,DMA_IT_TCIF4))
	{
		DMA_ClearITPendingBit(DMA1_Stream4,DMA_IT_TCIF4);
	}
}






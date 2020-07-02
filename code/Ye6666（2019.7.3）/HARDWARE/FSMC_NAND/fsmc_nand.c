/*
*********************************************************************************************************
*	                                  
*	模块名称 : NAND Flash驱动模块    
*	文件名称 : fsmc_nand.c
*	版    本 : V1.0
*	说    明 : 提供NAND Flash (MT29F64G08CBAA)的底层接口函数。
*	修改记录 :
*		版本号  日期       作者    说明
*		v1.0    2011-05-25 armfly  ST固件库 V3.5.0版本。
*   v2.0    2014-12-19 wunihaoo 
*	Copyright (C), 2010-2014
*
*********************************************************************************************************
*/
#include "string.h"
#include "stdio.h"
#include "fsmc_nand.h"
#include <stm32f4xx.h>

/* 定义NAND Flash的物理地址。这个是有硬件决定的 */
#define Bank2_NAND_ADDR    ((uint32_t)0x70000000)
#define Bank_NAND_ADDR     Bank2_NAND_ADDR 

/* 定义操作NAND Flash用到3个宏 */
#define NAND_CMD_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA)
#define NAND_ADDR_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA)
#define NAND_DATA_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA)

static uint8_t FSMC_NAND_GetStatus(void);

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_Init
*	功能说明: 配置FSMC和GPIO用于NAND Flash接口。这个函数必须在读写nand flash前被调用一次。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void NAND_Init(void)
{
	FSMC_NANDInitTypeDef FSMC_NANDInitStructure;
	FSMC_NAND_PCCARDTimingInitTypeDef  p;	
	
	/* FSMC 配置 */
	p.FSMC_SetupTime = 0x01;//0x1;
	p.FSMC_WaitSetupTime = 0x03;//0x3;
	p.FSMC_HoldSetupTime = 0x02;//0x2;
	p.FSMC_HiZSetupTime = 0x01;//0x1;
	
	FSMC_NANDInitStructure.FSMC_Bank = FSMC_Bank2_NAND;							/* 定义FSMC BANK 号 */
	FSMC_NANDInitStructure.FSMC_Waitfeature = FSMC_Waitfeature_Enable;			/* 插入等待时序使能 */
	FSMC_NANDInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;		/* 数据宽度 8bit */
	FSMC_NANDInitStructure.FSMC_ECC = FSMC_ECC_Disable;							/* ECC错误检查和纠正功能使能 FSMC_ECC_Enable*/
	FSMC_NANDInitStructure.FSMC_ECCPageSize = FSMC_ECCPageSize_8192Bytes;		/* ECC 页面大小 */
	FSMC_NANDInitStructure.FSMC_TCLRSetupTime = 0x00;
	FSMC_NANDInitStructure.FSMC_TARSetupTime = 0x00;
	FSMC_NANDInitStructure.FSMC_CommonSpaceTimingStruct = &p;
	FSMC_NANDInitStructure.FSMC_AttributeSpaceTimingStruct = &p;
	
	FSMC_NANDInit(&FSMC_NANDInitStructure);
	
	/* FSMC NAND Bank 使能 */
	FSMC_NANDCmd(FSMC_Bank2_NAND, ENABLE);
		
	FSMC_NAND_Reset();
}

/*
*********************************************************************************************************
*	函 数 名: NAND_ReadID
*	功能说明: 读NAND Flash的ID。ID存储到形参指定的结构体变量中。
*	形    参：无
*	返 回 值: 32bit的NAND Flash ID
*********************************************************************************************************
*/
uint32_t NAND_ReadID(void)
{
	uint32_t data = 0;	
	
	/* 发送命令 Command to the command area */ 	
	NAND_CMD_AREA = 0x90;
	NAND_ADDR_AREA = 0x00;
	
	/* 顺序读取NAND Flash的ID */	
	data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);
	data =  ((data << 24) & 0xFF000000) | 
			((data << 8 ) & 0x00FF0000) | 
			((data >> 8 ) & 0x0000FF00) | 
			((data >> 24) & 0x000000FF) ;
	return data;
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_WritePage
*	功能说明: 写一组数据至NandFlash指定页面的指定位置，写入的数据长度不大于一页的大小。
*	形    参：- _pBuffer: 指向包含待写数据的缓冲区 
*          	- _ulPageNo: 页号，所有的页统一编码，范围为：0 - 65535
*			  - _usAddrInPage : 页内地址，范围为：0-8639
*             - _usByteCount: 写入的字节个数
*	返 回 值: 执行结果：
*				- NAND_FAIL 表示失败
*				- NAND_OK 表示成功
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_WritePage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount,uint32_t _ulBlockNo) //数据缓冲区、页号、页内地址（0~2111（2048+64））、写入字节数
{
	uint16_t i;
	uint32_t row_address=_ulPageNo+_ulBlockNo*NAND_BLOCK_SIZE;
	
	//nand_lock();
	
	/* 发送页写命令 */
	NAND_CMD_AREA = NAND_CMD_WRITE0;
	
	/* 发送页内地址 ， 对于 HY27UF081G2A
				      Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		第1字节： CA7  CA6  CA5  CA4  CA3  CA2  CA1  CA0		(_usPageAddr 的bit7 - bit0)
		第2字节： 0    0    0    CA12 CA11 CA10 CA9  CA8		(_usPageAddr 的bit12 - bit8, 高3bit必须是0)
		第3字节： PA7  PA6  PA5  PA4  PA3  PA2  PA1  PA0
		第4字节： BA15 BA14 BA13 BA12 BA11 BA10 BA9  BA8
		第5字节：	0    0    0    0    BA19 BA18 BA17 BA16
	//CAX，column(列地址) address; PAX=page address;BAX=block address
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = row_address;
	NAND_ADDR_AREA = (row_address & 0xFF00) >> 8;
	NAND_ADDR_AREA = (row_address & 0xF0000) >> 16;
	/* 写数据 */
	for(i = 0; i < _usByteCount; i++)
	{
		NAND_DATA_AREA = _pBuffer[i];
	}
	NAND_CMD_AREA = NAND_CMD_WRITE_TRUE1;
	
	/* 检查操作状态 */	
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		//nand_unlock();
		return NAND_OK;
	}
	
	//nand_unlock();
	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_ReadPage
*	功能说明: 从NandFlash指定页面的指定位置读一组数据，读出的数据长度不大于一页的大小。
*	形    参：- _pBuffer: 指向包含待写数据的缓冲区 
*             - _ulPageNo: 页号，所有的页统一编码，范围为：0 - 65535
*			  - _usAddrInPage : 页内地址，范围为：0-8639
*             - _usByteCount: 写入的字节个数
*	返 回 值: 执行结果：
*				- NAND_FAIL 表示失败
*				- NAND_OK 表示成功
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadPage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount,uint32_t _ulBlockNo)
{
	uint16_t i;
	uint32_t row_address=_ulPageNo+_ulBlockNo*NAND_BLOCK_SIZE;
	uint32_t ulTimeout = 0x100;
	
	//nand_lock();

    /* 发送页面读命令 */
    NAND_CMD_AREA = NAND_CMD_AREA_A;

	/* 发送页内地址 ， 对于 HY27UF081G2A
				      Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		第1字节： CA7  CA6  CA5  CA4  CA3  CA2  CA1  CA0		(_usPageAddr 的bit7 - bit0)
		第2字节： 0    0    0    CA12 CA11 CA10 CA9  CA8		(_usPageAddr 的bit12 - bit8, 高3bit必须是0)
		第3字节： PA7  PA6  PA5  PA4  PA3  PA2  PA1  PA0
		第4字节： BA15 BA14 BA13 BA12 BA11 BA10 BA9  BA8
		第5字节：	0    0    0    0    BA19 BA18 BA17 BA16
	//CAX，column(列地址) address; PAX=page address;BAX=block address
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = row_address;
	NAND_ADDR_AREA = (row_address & 0xFF00) >> 8;
	NAND_ADDR_AREA = (row_address & 0xF0000) >> 16;
	
	NAND_CMD_AREA = NAND_CMD_AREA_TRUE1;
	    
	 /* 必须等待，否则读出数据异常, 此处应该判断超时 */
	while(( GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_6) == 0 ) &&( ulTimeout != 0x00));		
	{
		ulTimeout--; 
	}
	if(ulTimeout == 0x00)
	{         
    //nand_unlock();		
		return NAND_FAIL;      
	}
	else
	{
		/* 读数据到缓冲区pBuffer */    
		for(i = 0; i < _usByteCount; i++)
		{
			_pBuffer[i] = NAND_DATA_AREA;
		}	
		
		//nand_unlock();
		return NAND_OK;
	}
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_WriteSpare
*	功能说明: 向1个PAGE的Spare区写入数据
*	形    参：- _pBuffer: 指向包含待写数据的缓冲区 
*             - _ulPageNo: 页号，所有的页统一编码，范围为：0 - 65535
*			  - _usAddrInSpare : 页内备用区的偏移地址，范围为：0-447
*             - _usByteCount: 写入的字节个数
*	返 回 值: 执行结果：
*				- NAND_FAIL 表示失败
*				- NAND_OK 表示成功
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_WriteSpare(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInSpare, uint16_t _usByteCount,uint32_t _ulBlockNo)
{
	if (_usByteCount > NAND_SPARE_AREA_SIZE)
	{
		return NAND_FAIL;
	}
	
	return FSMC_NAND_WritePage(_pBuffer, _ulPageNo, NAND_PAGE_SIZE + _usAddrInSpare, _usByteCount, _ulBlockNo);
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_ReadSpare
*	功能说明: 读1个PAGE的Spare区的数据
*	形    参：- _pBuffer: 指向包含待写数据的缓冲区 
*             - _ulPageNo: 页号，所有的页统一编码，范围为：0 - 65535
*			  - _usAddrInSpare : 页内备用区的偏移地址，范围为：0-447
*             - _usByteCount: 写入的字节个数
*	返 回 值: 执行结果：
*				- NAND_FAIL 表示失败
*				- NAND_OK 表示成功
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadSpare(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInSpare, uint16_t _usByteCount,uint32_t _ulBlockNo)
{
	if (_usByteCount > NAND_SPARE_AREA_SIZE)
	{
		return NAND_FAIL;
	}
		
	return FSMC_NAND_ReadPage(_pBuffer, _ulPageNo, NAND_PAGE_SIZE + _usAddrInSpare, _usByteCount, _ulBlockNo);
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_WriteData
*	功能说明: 向1个PAGE的主数据区写入数据
*	形    参：- _pBuffer: 指向包含待写数据的缓冲区 
*             - _ulPageNo: 页号，所有的页统一编码，范围为：0 - 65535
*			  - _usAddrInPage : 页内数据区的偏移地址，范围为：0-8191
*             - _usByteCount: 写入的字节个数
*	返 回 值: 执行结果：
*				- NAND_FAIL 表示失败
*				- NAND_OK 表示成功
*********************************************************************************************************
*/
uint8_t FSMC_NAND_WriteData(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount,uint32_t _ulBlockNo)
{
	if (_usByteCount > NAND_PAGE_SIZE || _usByteCount == 0)
	{
		return NAND_FAIL;
	}
	
	return FSMC_NAND_WritePage(_pBuffer, _ulPageNo, _usAddrInPage, _usByteCount, _ulBlockNo);
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_ReadData
*	功能说明: 读1个PAGE的主数据的数据
*	形    参：- _pBuffer: 指向包含待写数据的缓冲区 
*             - _ulPageNo: 页号，所有的页统一编码，范围为：0 - 65535
*			  - _usAddrInPage : 页内数据区的偏移地址，范围为：0-8192
*             - _usByteCount: 写入的字节个数
*	返 回 值: 执行结果：
*				- NAND_FAIL 表示失败
*				- NAND_OK 表示成功
*********************************************************************************************************
*/
uint8_t FSMC_NAND_ReadData(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount,uint32_t _ulBlockNo)
{
	if (_usByteCount > NAND_PAGE_SIZE)
	{
		return NAND_FAIL;
	}
	
	return FSMC_NAND_ReadPage(_pBuffer, _ulPageNo, _usAddrInPage, _usByteCount, _ulBlockNo);
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_EraseBlock
*	功能说明: 擦除NAND Flash一个块（block）
*	形    参：- _ulBlockNo: 块号，范围为：0 - 4096
*	返 回 值: NAND操作状态，有如下几种值：
*             - NAND_TIMEOUT_ERROR  : 超时错误
*             - NAND_READY          : 操作成功
*********************************************************************************************************
*/
uint8_t FSMC_NAND_EraseBlock(uint32_t _ulBlockNo)
{
	uint8_t ucStatus;
	
	//nand_lock();
	
	/* 发送擦除命令 */
	NAND_CMD_AREA = NAND_CMD_ERASE0;
	
	_ulBlockNo <<= 8;	/* 块号转换为页编号 */
	NAND_ADDR_AREA = _ulBlockNo;
	NAND_ADDR_AREA = _ulBlockNo >> 8;
	NAND_ADDR_AREA = _ulBlockNo >> 16;
	NAND_ADDR_AREA = (_ulBlockNo >> 24)&0x0F;
	NAND_CMD_AREA = NAND_CMD_ERASE1;
	
	ucStatus = FSMC_NAND_GetStatus();
	
	//nand_unlock();
	
	return ucStatus;
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_Reset
*	功能说明: 复位NAND Flash
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
uint8_t FSMC_NAND_Reset(void)
{
	NAND_CMD_AREA = NAND_CMD_RESET;
	
		/* 检查操作状态 */	
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}
	
	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_ReadStatus
*	功能说明: 使用Read statuc 命令读NAND Flash内部状态
*	形    参：- Address: 被擦除的快内任意地址
*	返 回 值: NAND操作状态，有如下几种值：
*             - NAND_BUSY: 内部正忙
*             - NAND_READY: 内部空闲，可以进行下步操作
*             - NAND_ERROR: 先前的命令执行失败
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadStatus(void)
{
	uint8_t ucData;
	uint8_t ucStatus = NAND_BUSY;
	
	/* 读状态操作 */
	NAND_CMD_AREA = NAND_CMD_STATUS;
	ucData = *(__IO uint8_t *)(Bank_NAND_ADDR);
	
	if((ucData & NAND_ERROR) == NAND_ERROR)
	{
		ucStatus = NAND_ERROR;
	} 
	else if((ucData & NAND_READY) == NAND_READY)
	{
		ucStatus = NAND_READY;
	}
	else
	{
		ucStatus = NAND_BUSY; 
	}
	
	return (ucStatus);
}

/*
*********************************************************************************************************
*	函 数 名: FSMC_NAND_GetStatus
*	功能说明: 获取NAND Flash操作状态
*	形    参：- Address: 被擦除的快内任意地址
*	返 回 值: NAND操作状态，有如下几种值：
*             - NAND_TIMEOUT_ERROR  : 超时错误
*             - NAND_READY          : 操作成功
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_GetStatus(void)
{
	uint32_t ulTimeout = 0x100;
	uint8_t ucStatus = NAND_READY;
	
	ucStatus = FSMC_NAND_ReadStatus(); 
	
	/* 等待NAND操作结束，超时后会退出 */
	while ((ucStatus != NAND_READY) &&( ulTimeout != 0x00))
	{
		ucStatus = FSMC_NAND_ReadStatus();
		ulTimeout--;      
	}
	
	if(ulTimeout == 0x00)
	{          
		ucStatus =  NAND_TIMEOUT_ERROR;      
	} 
	
	/* 返回操作状态 */
	return (ucStatus);      
}

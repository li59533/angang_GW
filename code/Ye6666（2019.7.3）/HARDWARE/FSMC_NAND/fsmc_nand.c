/*
*********************************************************************************************************
*	                                  
*	ģ������ : NAND Flash����ģ��    
*	�ļ����� : fsmc_nand.c
*	��    �� : V1.0
*	˵    �� : �ṩNAND Flash (MT29F64G08CBAA)�ĵײ�ӿں�����
*	�޸ļ�¼ :
*		�汾��  ����       ����    ˵��
*		v1.0    2011-05-25 armfly  ST�̼��� V3.5.0�汾��
*   v2.0    2014-12-19 wunihaoo 
*	Copyright (C), 2010-2014
*
*********************************************************************************************************
*/
#include "string.h"
#include "stdio.h"
#include "fsmc_nand.h"
#include <stm32f4xx.h>

/* ����NAND Flash�������ַ���������Ӳ�������� */
#define Bank2_NAND_ADDR    ((uint32_t)0x70000000)
#define Bank_NAND_ADDR     Bank2_NAND_ADDR 

/* �������NAND Flash�õ�3���� */
#define NAND_CMD_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA)
#define NAND_ADDR_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA)
#define NAND_DATA_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA)

static uint8_t FSMC_NAND_GetStatus(void);

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_Init
*	����˵��: ����FSMC��GPIO����NAND Flash�ӿڡ�������������ڶ�дnand flashǰ������һ�Ρ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void NAND_Init(void)
{
	FSMC_NANDInitTypeDef FSMC_NANDInitStructure;
	FSMC_NAND_PCCARDTimingInitTypeDef  p;	
	
	/* FSMC ���� */
	p.FSMC_SetupTime = 0x01;//0x1;
	p.FSMC_WaitSetupTime = 0x03;//0x3;
	p.FSMC_HoldSetupTime = 0x02;//0x2;
	p.FSMC_HiZSetupTime = 0x01;//0x1;
	
	FSMC_NANDInitStructure.FSMC_Bank = FSMC_Bank2_NAND;							/* ����FSMC BANK �� */
	FSMC_NANDInitStructure.FSMC_Waitfeature = FSMC_Waitfeature_Enable;			/* ����ȴ�ʱ��ʹ�� */
	FSMC_NANDInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;		/* ���ݿ�� 8bit */
	FSMC_NANDInitStructure.FSMC_ECC = FSMC_ECC_Disable;							/* ECC������;�������ʹ�� FSMC_ECC_Enable*/
	FSMC_NANDInitStructure.FSMC_ECCPageSize = FSMC_ECCPageSize_8192Bytes;		/* ECC ҳ���С */
	FSMC_NANDInitStructure.FSMC_TCLRSetupTime = 0x00;
	FSMC_NANDInitStructure.FSMC_TARSetupTime = 0x00;
	FSMC_NANDInitStructure.FSMC_CommonSpaceTimingStruct = &p;
	FSMC_NANDInitStructure.FSMC_AttributeSpaceTimingStruct = &p;
	
	FSMC_NANDInit(&FSMC_NANDInitStructure);
	
	/* FSMC NAND Bank ʹ�� */
	FSMC_NANDCmd(FSMC_Bank2_NAND, ENABLE);
		
	FSMC_NAND_Reset();
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_ReadID
*	����˵��: ��NAND Flash��ID��ID�洢���β�ָ���Ľṹ������С�
*	��    �Σ���
*	�� �� ֵ: 32bit��NAND Flash ID
*********************************************************************************************************
*/
uint32_t NAND_ReadID(void)
{
	uint32_t data = 0;	
	
	/* �������� Command to the command area */ 	
	NAND_CMD_AREA = 0x90;
	NAND_ADDR_AREA = 0x00;
	
	/* ˳���ȡNAND Flash��ID */	
	data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);
	data =  ((data << 24) & 0xFF000000) | 
			((data << 8 ) & 0x00FF0000) | 
			((data >> 8 ) & 0x0000FF00) | 
			((data >> 24) & 0x000000FF) ;
	return data;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_WritePage
*	����˵��: дһ��������NandFlashָ��ҳ���ָ��λ�ã�д������ݳ��Ȳ�����һҳ�Ĵ�С��
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*          	- _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ�ڵ�ַ����ΧΪ��0-8639
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_WritePage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount,uint32_t _ulBlockNo) //���ݻ�������ҳ�š�ҳ�ڵ�ַ��0~2111��2048+64������д���ֽ���
{
	uint16_t i;
	uint32_t row_address=_ulPageNo+_ulBlockNo*NAND_BLOCK_SIZE;
	
	//nand_lock();
	
	/* ����ҳд���� */
	NAND_CMD_AREA = NAND_CMD_WRITE0;
	
	/* ����ҳ�ڵ�ַ �� ���� HY27UF081G2A
				      Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� CA7  CA6  CA5  CA4  CA3  CA2  CA1  CA0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    CA12 CA11 CA10 CA9  CA8		(_usPageAddr ��bit12 - bit8, ��3bit������0)
		��3�ֽڣ� PA7  PA6  PA5  PA4  PA3  PA2  PA1  PA0
		��4�ֽڣ� BA15 BA14 BA13 BA12 BA11 BA10 BA9  BA8
		��5�ֽڣ�	0    0    0    0    BA19 BA18 BA17 BA16
	//CAX��column(�е�ַ) address; PAX=page address;BAX=block address
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = row_address;
	NAND_ADDR_AREA = (row_address & 0xFF00) >> 8;
	NAND_ADDR_AREA = (row_address & 0xF0000) >> 16;
	/* д���� */
	for(i = 0; i < _usByteCount; i++)
	{
		NAND_DATA_AREA = _pBuffer[i];
	}
	NAND_CMD_AREA = NAND_CMD_WRITE_TRUE1;
	
	/* ������״̬ */	
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
*	�� �� ��: FSMC_NAND_ReadPage
*	����˵��: ��NandFlashָ��ҳ���ָ��λ�ö�һ�����ݣ����������ݳ��Ȳ�����һҳ�Ĵ�С��
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ�ڵ�ַ����ΧΪ��0-8639
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadPage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount,uint32_t _ulBlockNo)
{
	uint16_t i;
	uint32_t row_address=_ulPageNo+_ulBlockNo*NAND_BLOCK_SIZE;
	uint32_t ulTimeout = 0x100;
	
	//nand_lock();

    /* ����ҳ������� */
    NAND_CMD_AREA = NAND_CMD_AREA_A;

	/* ����ҳ�ڵ�ַ �� ���� HY27UF081G2A
				      Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� CA7  CA6  CA5  CA4  CA3  CA2  CA1  CA0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    CA12 CA11 CA10 CA9  CA8		(_usPageAddr ��bit12 - bit8, ��3bit������0)
		��3�ֽڣ� PA7  PA6  PA5  PA4  PA3  PA2  PA1  PA0
		��4�ֽڣ� BA15 BA14 BA13 BA12 BA11 BA10 BA9  BA8
		��5�ֽڣ�	0    0    0    0    BA19 BA18 BA17 BA16
	//CAX��column(�е�ַ) address; PAX=page address;BAX=block address
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = row_address;
	NAND_ADDR_AREA = (row_address & 0xFF00) >> 8;
	NAND_ADDR_AREA = (row_address & 0xF0000) >> 16;
	
	NAND_CMD_AREA = NAND_CMD_AREA_TRUE1;
	    
	 /* ����ȴ���������������쳣, �˴�Ӧ���жϳ�ʱ */
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
		/* �����ݵ�������pBuffer */    
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
*	�� �� ��: FSMC_NAND_WriteSpare
*	����˵��: ��1��PAGE��Spare��д������
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInSpare : ҳ�ڱ�������ƫ�Ƶ�ַ����ΧΪ��0-447
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
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
*	�� �� ��: FSMC_NAND_ReadSpare
*	����˵��: ��1��PAGE��Spare��������
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInSpare : ҳ�ڱ�������ƫ�Ƶ�ַ����ΧΪ��0-447
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
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
*	�� �� ��: FSMC_NAND_WriteData
*	����˵��: ��1��PAGE����������д������
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ����������ƫ�Ƶ�ַ����ΧΪ��0-8191
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
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
*	�� �� ��: FSMC_NAND_ReadData
*	����˵��: ��1��PAGE�������ݵ�����
*	��    �Σ�- _pBuffer: ָ�������д���ݵĻ����� 
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ����������ƫ�Ƶ�ַ����ΧΪ��0-8192
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*				- NAND_FAIL ��ʾʧ��
*				- NAND_OK ��ʾ�ɹ�
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
*	�� �� ��: FSMC_NAND_EraseBlock
*	����˵��: ����NAND Flashһ���飨block��
*	��    �Σ�- _ulBlockNo: ��ţ���ΧΪ��0 - 4096
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_TIMEOUT_ERROR  : ��ʱ����
*             - NAND_READY          : �����ɹ�
*********************************************************************************************************
*/
uint8_t FSMC_NAND_EraseBlock(uint32_t _ulBlockNo)
{
	uint8_t ucStatus;
	
	//nand_lock();
	
	/* ���Ͳ������� */
	NAND_CMD_AREA = NAND_CMD_ERASE0;
	
	_ulBlockNo <<= 8;	/* ���ת��Ϊҳ��� */
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
*	�� �� ��: FSMC_NAND_Reset
*	����˵��: ��λNAND Flash
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
uint8_t FSMC_NAND_Reset(void)
{
	NAND_CMD_AREA = NAND_CMD_RESET;
	
		/* ������״̬ */	
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}
	
	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadStatus
*	����˵��: ʹ��Read statuc �����NAND Flash�ڲ�״̬
*	��    �Σ�- Address: �������Ŀ��������ַ
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_BUSY: �ڲ���æ
*             - NAND_READY: �ڲ����У����Խ����²�����
*             - NAND_ERROR: ��ǰ������ִ��ʧ��
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadStatus(void)
{
	uint8_t ucData;
	uint8_t ucStatus = NAND_BUSY;
	
	/* ��״̬���� */
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
*	�� �� ��: FSMC_NAND_GetStatus
*	����˵��: ��ȡNAND Flash����״̬
*	��    �Σ�- Address: �������Ŀ��������ַ
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_TIMEOUT_ERROR  : ��ʱ����
*             - NAND_READY          : �����ɹ�
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_GetStatus(void)
{
	uint32_t ulTimeout = 0x100;
	uint8_t ucStatus = NAND_READY;
	
	ucStatus = FSMC_NAND_ReadStatus(); 
	
	/* �ȴ�NAND������������ʱ����˳� */
	while ((ucStatus != NAND_READY) &&( ulTimeout != 0x00))
	{
		ucStatus = FSMC_NAND_ReadStatus();
		ulTimeout--;      
	}
	
	if(ulTimeout == 0x00)
	{          
		ucStatus =  NAND_TIMEOUT_ERROR;      
	} 
	
	/* ���ز���״̬ */
	return (ucStatus);      
}

/******************** (C) COPYRIGHT 2011 STMicroelectronics ********************
* File Name          : fsmc_nand.h
* Author             : MCD Application Team
* Version            : V3.3.0
* Date               : 21-March-2011
* Description        : Header for fsmc_nand.c file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FSMC_NAND_H
#define __FSMC_NAND_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
/* 	
	定义有效的 NAND ID
	HY27UF081G2A  	= 0xAD 0xF1 0x80 0x1D
	K9F1G08U0A		= 0xEC 0xF1 0x80 0x15
	K9F1G08U0B		= 0xEC 0xF1 0x00 0x95
*/
#define NAND_MAKER_ID	0xAD
#define NAND_DEVICE_ID	0xF1
#define NAND_THIRD_ID 	0x80
#define NAND_FOURTH_ID	0x1D

#define HY27UF081G2A	0xADF1801D
#define K9F1G08U0A		0xECF18015
#define K9F1G08U0B		0xECF10095

/* Exported constants --------------------------------------------------------*/
/* NAND Area definition  for STM3210E-EVAL Board RevD */
#define CMD_AREA                   (uint32_t)(1<<16)  /* A16 = CLE  high */
#define ADDR_AREA                  (uint32_t)(1<<17)  /* A17 = ALE high */
#define DATA_AREA                  ((uint32_t)0x00000000) 

/* FSMC NAND memory command */
#define	NAND_CMD_AREA_A            ((uint8_t)0x00)
#define	NAND_CMD_AREA_B            ((uint8_t)0x01)
#define NAND_CMD_AREA_C            ((uint8_t)0x50)
#define NAND_CMD_AREA_TRUE1        ((uint8_t)0x30)

#define NAND_CMD_WRITE0            ((uint8_t)0x80)
#define NAND_CMD_WRITE_TRUE1       ((uint8_t)0x10)
	
#define NAND_CMD_ERASE0            ((uint8_t)0x60)
#define NAND_CMD_ERASE1            ((uint8_t)0xD0)  

#define NAND_CMD_READID            ((uint8_t)0x90)	

#define NAND_CMD_LOCK_STATUS       ((uint8_t)0x7A)
#define NAND_CMD_RESET             ((uint8_t)0xFF)

/* NAND memory status */
#define NAND_BUSY                  ((uint8_t)0x00)
#define NAND_ERROR                 ((uint8_t)0x01)
#define NAND_READY                 ((uint8_t)0x40)
#define NAND_TIMEOUT_ERROR         ((uint8_t)0x80)

/* FSMC NAND memory parameters */
/* 用于MT29F64G08CBABAWP-IT */
	#define NAND_PAGE_SIZE             ((uint16_t)0x2000) /* 8192 bytes per page w/o Spare Area */
	#define NAND_BLOCK_SIZE            ((uint16_t)0x0100) /* 256 pages per block */
	#define NAND_ZONE_SIZE             ((uint16_t)0x1000) /* 4096 Block per zone */
	#define NAND_SPARE_AREA_SIZE       ((uint16_t)0x01C0) /* last 448 bytes as spare area */
	#define NAND_MAX_ZONE              ((uint16_t)0x0001) /* 1 zones of 4096 block */

	/* 命令代码定义 */
	#define NAND_CMD_COPYBACK_A			((uint8_t)0x00)		/* PAGE COPY-BACK 命令序列 */
	#define NAND_CMD_COPYBACK_B			((uint8_t)0x35)	
	#define NAND_CMD_COPYBACK_C			((uint8_t)0x85)	
	#define NAND_CMD_COPYBACK_D			((uint8_t)0x10)	
	
	#define NAND_CMD_STATUS				((uint8_t)0x70)		/* 读NAND Flash的状态字 */

	#define MAX_PHY_BLOCKS_PER_ZONE  4096	/* 每个区最大物理块号 */
	#define MAX_LOG_BLOCKS_PER_ZONE  4000	/* 每个区最大逻辑块号 */
	
	#define NAND_BLOCK_COUNT			16384 /* 块个数 */
	#define NAND_PAGE_TOTAL_SIZE		(NAND_PAGE_SIZE + NAND_SPARE_AREA_SIZE)	/* 页面总大小 */

/* Exported functions ------------------------------------------------------- */
#define NAND_OK   0
#define NAND_FAIL 1
#define SWAP_Block 0
#define SWAP_Block_1 1
/* Private variables ----------------------------------------------------------*/
/* Private function prototypes ------------------------------------------------*/
/* exported functions ---------------------------------------------------------*/
void NAND_Init(void);
uint32_t NAND_ReadID(void);
uint8_t FSMC_NAND_WriteData(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount,uint32_t _ulBlockNo);
uint8_t FSMC_NAND_ReadData(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount,uint32_t _ulBlockNo);
uint8_t FSMC_NAND_EraseBlock(uint32_t _ulBlockNo);
uint8_t FSMC_NAND_Reset(void);
uint8_t NAND_WriteToNew(uint32_t _ulPage,uint16_t ul_Addrnum,uint32_t Block);
uint8_t NAND_WriteToNewBlock(uint32_t block);
uint8_t FSMC_NAND_PageCopyBack(uint32_t _ulSrcPageNo,uint32_t _ulSrcBlockNo, uint32_t _ulTarPageNo,uint32_t _ulTarBlockNo);

#endif /* __FSMC_NAND_H */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

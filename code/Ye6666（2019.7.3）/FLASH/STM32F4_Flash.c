
#include "STM32F4_Flash.h"
#include "app.h"


#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH修改部分
 

//FLASH ???????
#define ADDR_FLASH_SECTOR_0     ((u32)0x08000000) 	//??0????, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((u32)0x08004000) 	//??1????, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((u32)0x08008000) 	//??2????, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((u32)0x0800C000) 	//??3????, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((u32)0x08010000) 	//??4????, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5     ((u32)0x08020000) 	//??5????, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6     ((u32)0x08040000) 	//??6????, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7     ((u32)0x08060000) 	//??7????, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8     ((u32)0x08080000) 	//??8????, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9     ((u32)0x080A0000) 	//??9????, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10    ((u32)0x080C0000) 	//??10????,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((u32)0x080E0000) 	//??11????,128 Kbytes  //修改部分

uint16_t STMFLASH_GetFlashSector_1(u32 addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}

void initEEPROM()
{
	loadConfig();
}


void EarsePage(uint32_t address)  //  
{
	FLASH_Unlock();
	FLASH_DataCacheCmd(DISABLE);
	FLASH_EraseSector(STMFLASH_GetFlashSector_1(address),VoltageRange_3);
//  FLASH_ErasePage(address);
	FLASH_Lock();
}


void RDBYTES(uint32_t address,uint16_t num, uint8_t *Data)
{
	uint16_t count;
	for(count=0;count<num/2;count++)
	{
		*((uint16_t *)Data+count)=*((uint16_t *)address+count);
	}
}

void WRBYTES(uint32_t address,uint16_t num, unsigned char *Data)
{
	uint16_t count;
	FLASH_Unlock();
//	FLASH_ErasePage(address);
	for(count=0;count<(num/2+1);count++)
	{
		FLASH_ProgramHalfWord((address+2*count), *((uint16_t*)(Data+2*count)));
	}
	FLASH_Lock();
}		

#define  FlashDataAddress        0x08040000    //原0x080E0000    往片内FLASH写数据的地址
#define  FlashParameterAddress   0x08060000    //原0x080C0000    往片内FLASH写参数的地址
void earseParameter(void)
{
	EarsePage(FlashParameterAddress);		
}
void saveParameter(void)
{
	EarsePage(FlashParameterAddress);
	WRBYTES(FlashParameterAddress,sizeof(struct PARAMETER),(uint8_t*)&Parameter);
}

void savePower(void)
{
	WRBYTES(FlashParameterAddress,sizeof(struct PARAMETER),(uint8_t*)&Parameter);
}
void loadParameter(void)
{
	RDBYTES(FlashParameterAddress,sizeof(Parameter.vaildsign),(unsigned char*)&Parameter);
	if(Parameter.vaildsign!=0x55AA)
	{	
		Parameter.vaildsign=0x55AA;
		return;
	}
	RDBYTES(FlashParameterAddress,sizeof(struct PARAMETER),((unsigned char*)&Parameter));

}
void saveConfig()
{
	EarsePage(FlashDataAddress);
	WRBYTES(FlashDataAddress,sizeof(struct CONFIG),(uint8_t*)&config);
//	WRBYTES(FlashDataAddress+0xA0,sizeof(struct ADJUSTDATA),(uint8_t*)&AdjustData);
	
}
void loadConfig()
{
	RDBYTES(FlashDataAddress,sizeof(config.vaildsign),(unsigned char*)&config);
	if(config.vaildsign!=0x45AA)
	{	
		config.vaildsign=0x45AA;
		saveConfig();
		saveParameter();
		return;
	}
	RDBYTES(FlashDataAddress,sizeof(struct CONFIG)+1,((unsigned char*)&config));
//	loadParameter();
}





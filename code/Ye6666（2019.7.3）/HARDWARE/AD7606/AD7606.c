#include"ye6271_common.h"
#include"AD7606.h"
#include"timer.h"
#include"delay.h"
#include"gpio.h"
#include"spi.h"
#include"malloc.h"
#include"paramaters.h"


/* 设置过采样的GPIO:PH12 PH9 PH10 */
#define OS0_1()		GPIOH->BSRRL = GPIO_Pin_12
#define OS0_0()		GPIOH->BSRRH = GPIO_Pin_12
#define OS1_1()		GPIOH->BSRRL = GPIO_Pin_9
#define OS1_0()		GPIOH->BSRRH = GPIO_Pin_9
#define OS2_1()		GPIOH->BSRRL = GPIO_Pin_10
#define OS2_0()		GPIOH->BSRRH = GPIO_Pin_10

/* 启动AD转换的GPIO : PH7*/
#define CONVST_1()	GPIOH->BSRRL = GPIO_Pin_7
#define CONVST_0()	GPIOH->BSRRH = GPIO_Pin_7

/* 设置输入量程的GPIO :  */ //低电平正负5V；高电平正负10V
#define RANGE_0()	GPIOH->BSRRH = GPIO_Pin_8
#define RANGE_1()	GPIOH->BSRRL = GPIO_Pin_8

/* AD7606复位口线 : PH6  */
#define RESET_1()	GPIOH->BSRRL = GPIO_Pin_6
#define RESET_0()	GPIOH->BSRRH = GPIO_Pin_6

/* AD7606 FSMC总线地址，只能读，无需写 */
#define AD7606_U15_RESULT()	*(__IO int16_t *)0x6C000000

s16 *ps_current_tx;

int current_point;
int current_buffer;

int16_t *AD_RESULT_U15[2];  //当前B组AD采样值

void AD7606_Init(void)
{
	AD_RESULT_U15[0] = (int16_t *)mymalloc(SRAMIN,SPI_BLOCK_SIZE);
	AD_RESULT_U15[1] = (int16_t *)mymalloc(SRAMIN,SPI_BLOCK_SIZE);
	AD7606_CtrlLinesConfig();
	AD7606_FSMCConfig();
	AD7606_SetOS(AD_OS_X1);
	AD7606_SetInputRange(AD_RANGE_10V);	/* 0表示输入量程为正负5V, 1表示正负10V */	
	AD7606_Reset();
	CONVST_1();					/* 启动转换的GPIO平时设置为高 */
	AD7606_SetSampleFreq(sys_para.sample_rate);
}

/*
*********************************************************************************************************
*	函 数 名: AD7606_CtrlLinesConfig
*	功能说明: 配置LCD控制口线，FSMC管脚设置为复用功能
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
/*
	PD0/FSMC_D2
	PD1/FSMC_D3
	PD4/FSMC_NOE		--- 读控制信号，OE = Output Enable ， N 表示低有效
	PD5/FSMC_NWE		--- 写控制信号，AD7606 只有读，无写信号
	PD8/FSMC_D13
	PD9/FSMC_D14
	PD10/FSMC_D15

	PD14/FSMC_D0
	PD15/FSMC_D1

	PE4/FSMC_A20		
	PE5/FSMC_A21		
	PE7/FSMC_D4
	PE8/FSMC_D5
	PE9/FSMC_D6
	PE10/FSMC_D7
	PE11/FSMC_D8
	PE12/FSMC_D9
	PE13/FSMC_D10
	PE14/FSMC_D11
	PE15/FSMC_D12

	//PG9/FSMC_NE2		--- 主片选
	PG12/FSMC_NE4		--- 主片选
	以上已经在sram初始化中做了。

	其他的控制IO:

	PH12	---> AD7606_OS0		OS2:OS0 选择数字滤波参数
	PH9		---> AD7606_OS1
	PH10	---> AD7606_OS2
	PH7		---> AD7606_CONVST	启动ADC转换 (CONVSTA 和 CONVSTB 已经并联)
	PH8		---> AD7606_RANGE	输入模拟电压量程，正负5V或正负10V
	PH6		---> AD7606_RESET	复位
	PH11       ---> AD7606_BUSY	AD7606 U15忙信号
*/
 void AD7606_CtrlLinesConfig(void)
{
	//fsmc 的GPIO 已经在sram中初始化了。
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
		
	/*	配置几个控制用的GPIO
	PH12	---> AD7606_OS0		OS2:OS0 选择数字滤波参数
	PH9		---> AD7606_OS1
	PH10	---> AD7606_OS2
	PH7		---> AD7606_CONVST	启动ADC转换 (CONVSTA 和 CONVSTB 已经并联)
	PH8		---> AD7606_RANGE	输入模拟电压量程，正负5V或正负10V
	PH6		---> AD7606_RESET	复位
	PH11	---> AD7606_BUSY	AD7606 U15忙信号
	*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 |GPIO_Pin_7 |GPIO_Pin_8 |GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_12;
	GPIO_Init(GPIOH, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOH, &GPIO_InitStructure);//初始化GPIOE2,3,4
	
	GPIO_ResetBits(GPIOH,GPIO_Pin_6);	//复位默认低电平
	GPIO_SetBits(GPIOH,GPIO_Pin_8);	//输入模拟电压量程正负10V
}
/*
*********************************************************************************************************
*	函 数 名: AD7606_FSMCConfig
*	功能说明: 配置FSMC并口访问时序
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_FSMCConfig(void)
{
	FSMC_NORSRAMInitTypeDef  init;
	FSMC_NORSRAMTimingInitTypeDef  timing;

	//init PG12-NCE4
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOG, &GPIO_InitStructure);


	/*
		AD7606规格书要求(3.3V时)：RD读信号低电平脉冲宽度最短21ns，高电平脉冲最短宽度15ns。

		按照如下配置 读数均正常。为了和同BANK的LCD配置相同，选择3-0-6-1-0-0
		3-0-5-1-0-0  : RD高持续75ns， 低电平持续50ns.  1us以内可读取8路样本数据到内存。
		1-0-1-1-0-0  : RD高75ns，低电平执行12ns左右，下降沿差不多也12ns.  数据读取正确。
	*/
	/* FSMC_Bank1_NORSRAM4 configuration */
	timing.FSMC_AddressSetupTime = 3;
	timing.FSMC_AddressHoldTime = 0;
	timing.FSMC_DataSetupTime = 6;
	timing.FSMC_BusTurnAroundDuration = 1;
	timing.FSMC_CLKDivision = 0;
	timing.FSMC_DataLatency = 0;
	timing.FSMC_AccessMode = FSMC_AccessMode_A;

	/*
	 	configured as follow:
	    - Data/Address MUX = Disable
	    - Memory Type = SRAM
	    - Data Width = 16bit
	    - Write Operation = Enable
	    - Extended Mode = Enable
	    - Asynchronous Wait = Disable
	*/
	init.FSMC_Bank = FSMC_Bank1_NORSRAM4;
	init.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	init.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	init.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	init.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	init.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	init.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	init.FSMC_WrapMode = FSMC_WrapMode_Disable;
	init.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	init.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	init.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	init.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	init.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	init.FSMC_ReadWriteTimingStruct = &timing;
	init.FSMC_WriteTimingStruct = &timing;
	FSMC_NORSRAMInit(&init);
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);

}
/*
*********************************************************************************************************
*	函 数 名: AD7606_SetOS
*	功能说明: 配置AD7606数字滤波器，也就设置过采样倍率。
*			 通过设置 AD7606_OS0、OS1、OS2口线的电平组合状态决定过采样倍率。
*			 启动AD转换之后，AD7606内部自动实现剩余样本的采集，然后求平均值输出。
*
*			 过采样倍率越高，转换时间越长。
*			 无过采样时，AD转换时间 4us;
*				2倍过采样时 = 8.7us;
*				4倍过采样时 = 16us
*			 	64倍过采样时 = 286us
*
*	形    参: _ucOS : 过采样倍率
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_SetOS(uint8_t _AD_OS)
{
	switch (_AD_OS)
	{
		case 0:	
			OS2_0();
			OS1_0();
			OS0_0();
			break;
			
		case 1:
			OS2_0();
			OS1_0();
			OS0_1();
			break;

		case 2:
			OS2_0();
			OS1_1();
			OS0_0();
			break;

		case 3:
			OS2_0();
			OS1_1();
			OS0_1();
			break;

		case 4:
			OS2_1();
			OS1_0();
			OS0_0();
			break;

		case 5:
			OS2_1();
			OS1_0();
			OS0_1();
			break;

		case 6:
			OS2_1();
			OS1_1();
			OS0_0();
			break;
			
		default:
			OS2_0();
			OS1_0();
			OS0_0();
			break;
	}
}
/*
*********************************************************************************************************
*	函 数 名: AD7606_SetInputRange
*	功能说明: 配置AD7606模拟信号输入量程。
*	形    参: _ucRange : 0 表示正负5V   1表示正负10V
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_SetInputRange(uint8_t _AD_Range)
{
	if (_AD_Range == 0)
	{
		RANGE_0();	/* 设置为正负5V */
	}
	else
	{
		RANGE_1();	/* 设置为正负10V */
	}
}
/*
*********************************************************************************************************
*	函 数 名: AD7606_Reset
*	功能说明: 硬件复位AD7606。复位之后恢复到正常工作状态。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_Reset(void)
{
	RESET_0();	/* 退出复位状态 */
	RESET_1();	/* 进入复位状态 */
	RESET_1();	/* 仅用于延迟。 RESET复位高电平脉冲宽度最小50ns。 */
  	delay_us(100);
	RESET_0();	/* 退出复位状态 */
}
/*
*********************************************************************************************************
*		下面的函数用于定时采集模式。 TIM5硬件定时中断中读取ADC结果，存在全局FIFO
*
*
*********************************************************************************************************
*/

/*
		CONVST 引脚，PH7使用TIM3输出脉冲，触发AD7606启动ADC转换。
		设置BUSY口线为下降沿中断。在中断服务程序保存ADC结果。
*/

/*
*********************************************************************************************************
*	函 数 名: AD7606_SetSampleFreq
*	功能说明: 配置硬件工作在自动采集模式
*	形    参：_ulFreq : 采样频率，单位Hz，	1k，2k，5k，10k，20K，50k，100k，200k
*	返 回 值: 无
*********************************************************************************************************
*/
void AD7606_SetSampleFreq(uint32_t _ulFreq)
{
	// 配置PH11, BUSY 作为中断输入口，下降沿触发 
	{
		EXTI_InitTypeDef   EXTI_InitStructure;
		GPIO_InitTypeDef   GPIO_InitStructure;
		NVIC_InitTypeDef   NVIC_InitStructure;

		/* Enable GPIOI clock */
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
		/* Enable SYSCFG clock */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

		/* Configure PH11 pin as input floating */
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_Init(GPIOH, &GPIO_InitStructure);

		/* Connect EXTI Line11 to PH11 pin */
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOH, EXTI_PinSource11);

		/* Configure EXTI Line11 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line11;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
		
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);

		NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}

	//设置7606转换定时器
	{
		uint32_t uiTIMxCLK;
		uint16_t usPrescaler;
		uint16_t usPeriod;

	    /*-----------------------------------------------------------------------
			system_stm32f4xx.c 文件中 void SetSysClock(void) 函数对时钟的配置如下：

			HCLK = SYSCLK / 1     (AHB1Periph)
			PCLK2 = HCLK / 2      (APB2Periph)
			PCLK1 = HCLK / 4      (APB1Periph)

			因为APB1 prescaler != 1, 所以 APB1上的TIMxCLK = PCLK1 x 2 = SystemCoreClock / 2;
			因为APB2 prescaler != 1, 所以 APB2上的TIMxCLK = PCLK2 x 2 = SystemCoreClock;

			APB1 定时器有 TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM6, TIM12, TIM13,TIM14
			APB2 定时器有 TIM1, TIM8 ,TIM9, TIM10, TIM11
		*/

		uiTIMxCLK = SystemCoreClock / 2;
		_ulFreq*=2;
		if (_ulFreq < 3000)
		{
			usPrescaler = 100 - 1;					/* 分频比 = 10 */
			usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;		/* 自动重装的值 */
		}
		else	/* 大于4K的频率，无需分频 */
		{
			usPrescaler = 0;					/* 分频比 = 1 */
			usPeriod = uiTIMxCLK / _ulFreq - 1;	/* 自动重装的值 */
		}

		TIM3_Int_Init(usPeriod,usPrescaler);
	}	
}

void AD7606_Start_AD(void)
{
	current_buffer = 0;
	current_point = 0;
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
}

void AD7606_Stop_AD(void)
{
	TIM_Cmd(TIM3,DISABLE); //停止定时器3
}

/*
*********************************************************************************************************
*	函 数 名: EXTI15_10_IRQHandler
*	功能说明: 外部中断服务程序入口。PH11/AD7606_BUSY 下降沿中断触发
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/


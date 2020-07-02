#include"ye6271_common.h"
#include"AD7606.h"
#include"timer.h"
#include"delay.h"
#include"gpio.h"
#include"spi.h"
#include"malloc.h"
#include"paramaters.h"


/* ���ù�������GPIO:PH12 PH9 PH10 */
#define OS0_1()		GPIOH->BSRRL = GPIO_Pin_12
#define OS0_0()		GPIOH->BSRRH = GPIO_Pin_12
#define OS1_1()		GPIOH->BSRRL = GPIO_Pin_9
#define OS1_0()		GPIOH->BSRRH = GPIO_Pin_9
#define OS2_1()		GPIOH->BSRRL = GPIO_Pin_10
#define OS2_0()		GPIOH->BSRRH = GPIO_Pin_10

/* ����ADת����GPIO : PH7*/
#define CONVST_1()	GPIOH->BSRRL = GPIO_Pin_7
#define CONVST_0()	GPIOH->BSRRH = GPIO_Pin_7

/* �����������̵�GPIO :  */ //�͵�ƽ����5V���ߵ�ƽ����10V
#define RANGE_0()	GPIOH->BSRRH = GPIO_Pin_8
#define RANGE_1()	GPIOH->BSRRL = GPIO_Pin_8

/* AD7606��λ���� : PH6  */
#define RESET_1()	GPIOH->BSRRL = GPIO_Pin_6
#define RESET_0()	GPIOH->BSRRH = GPIO_Pin_6

/* AD7606 FSMC���ߵ�ַ��ֻ�ܶ�������д */
#define AD7606_U15_RESULT()	*(__IO int16_t *)0x6C000000

s16 *ps_current_tx;

int current_point;
int current_buffer;

int16_t *AD_RESULT_U15[2];  //��ǰB��AD����ֵ

void AD7606_Init(void)
{
	AD_RESULT_U15[0] = (int16_t *)mymalloc(SRAMIN,SPI_BLOCK_SIZE);
	AD_RESULT_U15[1] = (int16_t *)mymalloc(SRAMIN,SPI_BLOCK_SIZE);
	AD7606_CtrlLinesConfig();
	AD7606_FSMCConfig();
	AD7606_SetOS(AD_OS_X1);
	AD7606_SetInputRange(AD_RANGE_10V);	/* 0��ʾ��������Ϊ����5V, 1��ʾ����10V */	
	AD7606_Reset();
	CONVST_1();					/* ����ת����GPIOƽʱ����Ϊ�� */
	AD7606_SetSampleFreq(sys_para.sample_rate);
}

/*
*********************************************************************************************************
*	�� �� ��: AD7606_CtrlLinesConfig
*	����˵��: ����LCD���ƿ��ߣ�FSMC�ܽ�����Ϊ���ù���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*
	PD0/FSMC_D2
	PD1/FSMC_D3
	PD4/FSMC_NOE		--- �������źţ�OE = Output Enable �� N ��ʾ����Ч
	PD5/FSMC_NWE		--- д�����źţ�AD7606 ֻ�ж�����д�ź�
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

	//PG9/FSMC_NE2		--- ��Ƭѡ
	PG12/FSMC_NE4		--- ��Ƭѡ
	�����Ѿ���sram��ʼ�������ˡ�

	�����Ŀ���IO:

	PH12	---> AD7606_OS0		OS2:OS0 ѡ�������˲�����
	PH9		---> AD7606_OS1
	PH10	---> AD7606_OS2
	PH7		---> AD7606_CONVST	����ADCת�� (CONVSTA �� CONVSTB �Ѿ�����)
	PH8		---> AD7606_RANGE	����ģ���ѹ���̣�����5V������10V
	PH6		---> AD7606_RESET	��λ
	PH11       ---> AD7606_BUSY	AD7606 U15æ�ź�
*/
 void AD7606_CtrlLinesConfig(void)
{
	//fsmc ��GPIO �Ѿ���sram�г�ʼ���ˡ�
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
		
	/*	���ü��������õ�GPIO
	PH12	---> AD7606_OS0		OS2:OS0 ѡ�������˲�����
	PH9		---> AD7606_OS1
	PH10	---> AD7606_OS2
	PH7		---> AD7606_CONVST	����ADCת�� (CONVSTA �� CONVSTB �Ѿ�����)
	PH8		---> AD7606_RANGE	����ģ���ѹ���̣�����5V������10V
	PH6		---> AD7606_RESET	��λ
	PH11	---> AD7606_BUSY	AD7606 U15æ�ź�
	*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 |GPIO_Pin_7 |GPIO_Pin_8 |GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_12;
	GPIO_Init(GPIOH, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
	GPIO_Init(GPIOH, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4
	
	GPIO_ResetBits(GPIOH,GPIO_Pin_6);	//��λĬ�ϵ͵�ƽ
	GPIO_SetBits(GPIOH,GPIO_Pin_8);	//����ģ���ѹ��������10V
}
/*
*********************************************************************************************************
*	�� �� ��: AD7606_FSMCConfig
*	����˵��: ����FSMC���ڷ���ʱ��
*	��    �Σ���
*	�� �� ֵ: ��
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
		AD7606�����Ҫ��(3.3Vʱ)��RD���źŵ͵�ƽ���������21ns���ߵ�ƽ������̿��15ns��

		������������ ������������Ϊ�˺�ͬBANK��LCD������ͬ��ѡ��3-0-6-1-0-0
		3-0-5-1-0-0  : RD�߳���75ns�� �͵�ƽ����50ns.  1us���ڿɶ�ȡ8·�������ݵ��ڴ档
		1-0-1-1-0-0  : RD��75ns���͵�ƽִ��12ns���ң��½��ز��Ҳ12ns.  ���ݶ�ȡ��ȷ��
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
*	�� �� ��: AD7606_SetOS
*	����˵��: ����AD7606�����˲�����Ҳ�����ù��������ʡ�
*			 ͨ������ AD7606_OS0��OS1��OS2���ߵĵ�ƽ���״̬�������������ʡ�
*			 ����ADת��֮��AD7606�ڲ��Զ�ʵ��ʣ�������Ĳɼ���Ȼ����ƽ��ֵ�����
*
*			 ����������Խ�ߣ�ת��ʱ��Խ����
*			 �޹�����ʱ��ADת��ʱ�� 4us;
*				2��������ʱ = 8.7us;
*				4��������ʱ = 16us
*			 	64��������ʱ = 286us
*
*	��    ��: _ucOS : ����������
*	�� �� ֵ: ��
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
*	�� �� ��: AD7606_SetInputRange
*	����˵��: ����AD7606ģ���ź��������̡�
*	��    ��: _ucRange : 0 ��ʾ����5V   1��ʾ����10V
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_SetInputRange(uint8_t _AD_Range)
{
	if (_AD_Range == 0)
	{
		RANGE_0();	/* ����Ϊ����5V */
	}
	else
	{
		RANGE_1();	/* ����Ϊ����10V */
	}
}
/*
*********************************************************************************************************
*	�� �� ��: AD7606_Reset
*	����˵��: Ӳ����λAD7606����λ֮��ָ�����������״̬��
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_Reset(void)
{
	RESET_0();	/* �˳���λ״̬ */
	RESET_1();	/* ���븴λ״̬ */
	RESET_1();	/* �������ӳ١� RESET��λ�ߵ�ƽ��������С50ns�� */
  	delay_us(100);
	RESET_0();	/* �˳���λ״̬ */
}
/*
*********************************************************************************************************
*		����ĺ������ڶ�ʱ�ɼ�ģʽ�� TIM5Ӳ����ʱ�ж��ж�ȡADC���������ȫ��FIFO
*
*
*********************************************************************************************************
*/

/*
		CONVST ���ţ�PH7ʹ��TIM3������壬����AD7606����ADCת����
		����BUSY����Ϊ�½����жϡ����жϷ�����򱣴�ADC�����
*/

/*
*********************************************************************************************************
*	�� �� ��: AD7606_SetSampleFreq
*	����˵��: ����Ӳ���������Զ��ɼ�ģʽ
*	��    �Σ�_ulFreq : ����Ƶ�ʣ���λHz��	1k��2k��5k��10k��20K��50k��100k��200k
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void AD7606_SetSampleFreq(uint32_t _ulFreq)
{
	// ����PH11, BUSY ��Ϊ�ж�����ڣ��½��ش��� 
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

	//����7606ת����ʱ��
	{
		uint32_t uiTIMxCLK;
		uint16_t usPrescaler;
		uint16_t usPeriod;

	    /*-----------------------------------------------------------------------
			system_stm32f4xx.c �ļ��� void SetSysClock(void) ������ʱ�ӵ��������£�

			HCLK = SYSCLK / 1     (AHB1Periph)
			PCLK2 = HCLK / 2      (APB2Periph)
			PCLK1 = HCLK / 4      (APB1Periph)

			��ΪAPB1 prescaler != 1, ���� APB1�ϵ�TIMxCLK = PCLK1 x 2 = SystemCoreClock / 2;
			��ΪAPB2 prescaler != 1, ���� APB2�ϵ�TIMxCLK = PCLK2 x 2 = SystemCoreClock;

			APB1 ��ʱ���� TIM2, TIM3 ,TIM4, TIM5, TIM6, TIM6, TIM12, TIM13,TIM14
			APB2 ��ʱ���� TIM1, TIM8 ,TIM9, TIM10, TIM11
		*/

		uiTIMxCLK = SystemCoreClock / 2;
		_ulFreq*=2;
		if (_ulFreq < 3000)
		{
			usPrescaler = 100 - 1;					/* ��Ƶ�� = 10 */
			usPeriod =  (uiTIMxCLK / 100) / _ulFreq  - 1;		/* �Զ���װ��ֵ */
		}
		else	/* ����4K��Ƶ�ʣ������Ƶ */
		{
			usPrescaler = 0;					/* ��Ƶ�� = 1 */
			usPeriod = uiTIMxCLK / _ulFreq - 1;	/* �Զ���װ��ֵ */
		}

		TIM3_Int_Init(usPeriod,usPrescaler);
	}	
}

void AD7606_Start_AD(void)
{
	current_buffer = 0;
	current_point = 0;
	TIM_Cmd(TIM3,ENABLE); //ʹ�ܶ�ʱ��3
}

void AD7606_Stop_AD(void)
{
	TIM_Cmd(TIM3,DISABLE); //ֹͣ��ʱ��3
}

/*
*********************************************************************************************************
*	�� �� ��: EXTI15_10_IRQHandler
*	����˵��: �ⲿ�жϷ��������ڡ�PH11/AD7606_BUSY �½����жϴ���
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/


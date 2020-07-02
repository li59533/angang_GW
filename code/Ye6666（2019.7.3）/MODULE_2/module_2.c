#include "app.h"

static const uint32_t  BAUNDRATE_INT_TAB[]={1200,2400,4800,9600,19200,38400,57600,115200};


static uint8_t  RxdBuf_debug[RX_BUFFER_SIZE];  //�������õ���
static uint8_t  TxdBuf_debug[TX_BUFFER_SIZE];
static uint8_t  CompareData[TX_BUFFER_SIZE];


static uint8_t  TxdBytes_debug=0;
static uint8_t  TxdTelBytes_debug=0;
uint8_t  module_2_ReceivedTel_debug=0;
uint8_t  RxdBytes_debug_module2=0;


static uint8_t  CmdBuf_debug[2*TX_BUFFER_SIZE];
static uint32_t CmdBuf_debugLength=0;


uint16_t  current_receive_module_2_node=0;

void module_2_receiveloop_debug(void);


void  setComUART6(uint8_t baudrate,uint8_t parity)
{
	uint32_t br;
	USART_InitTypeDef  USART_InitStructure;

	USART_ITConfig(USART6,USART_IT_RXNE,DISABLE);
	USART_ITConfig(USART6,USART_IT_TC,DISABLE);
	USART_Cmd(USART6, DISABLE);	 
	/*-------------------------USART6------------------------*/
	br=BAUNDRATE_INT_TAB[baudrate];
		
	USART_InitStructure.USART_BaudRate = br;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	if(parity==0) {USART_InitStructure.USART_WordLength = USART_WordLength_8b;USART_InitStructure.USART_Parity = USART_Parity_No;}
	if(parity==1) {USART_InitStructure.USART_WordLength = USART_WordLength_9b;USART_InitStructure.USART_Parity = USART_Parity_Odd;}
	if(parity==2) {USART_InitStructure.USART_WordLength = USART_WordLength_9b;USART_InitStructure.USART_Parity = USART_Parity_Even;}		
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	
	USART_Init(USART6, &USART_InitStructure); 
	USART_ITConfig(USART6,USART_IT_TC,ENABLE);
	USART_ITConfig(USART6,USART_IT_RXNE,ENABLE);
	USART_Cmd(USART6, ENABLE);
}


void  initMODULE_2(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE); 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6,ENABLE);
	
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_USART6); 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_USART6); 
	
	//USART3   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(GPIOC,&GPIO_InitStructure); 
	
	//M0 M1   PG5 PG4
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(GPIOG,&GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(GPIOG,&GPIO_InitStructure); 

	//Usart6 NVIC 
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);

	//RS485_TX_EN=0;				

		/* uart init */
	setComUART6(config.module_2_baudrate,config.parity);
	
	/* enable interrupt */
	//USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
}

void  MODULE2_ModeSet(uint8_t mode) 
{
	if(Normal_Mode==mode)
	{
		GPIO_ResetBits(GPIOG,GPIO_Pin_5);
		GPIO_ResetBits(GPIOG,GPIO_Pin_4);
	}
	else if(Coordinator_Mode==mode)
	{
		GPIO_SetBits(GPIOG,GPIO_Pin_4);
		GPIO_ResetBits(GPIOG,GPIO_Pin_5);
	}
	else if(LowPower_Mode==mode)
	{
		GPIO_ResetBits(GPIOG,GPIO_Pin_4);
		GPIO_SetBits(GPIOG,GPIO_Pin_5);
	}
	else if(Sleep_Mode==mode)
	{
		GPIO_SetBits(GPIOG,GPIO_Pin_4);
		GPIO_SetBits(GPIOG,GPIO_Pin_5);
	}
}


static uint8_t  getTelLength_debug(void)
{					
	return(((uint16_t)RxdBuf_debug[3]<<8)+RxdBuf_debug[2]);				
}


static uint8_t  isVaildTel_debug(void)
{
	if(RxdBytes_debug_module2>=1)if(RxdBuf_debug[0]!=0x7e)return(0);
//	if(RxdBytes_debug_module2>=2)if(RxdBuf_debug[1]>255)return(0);
	if(RxdBytes_debug_module2>=2)if((RxdBuf_debug[1]!=0x42)&&(RxdBuf_debug[1]!=0x45)&&(RxdBuf_debug[1]!=0x48)&&(RxdBuf_debug[1]!=0x50))return(0);   //��������	
	if(RxdBytes_debug_module2>=4) 
	{ 
		uint16_t length=getTelLength_debug();
		if((length>40)) return(0);  //��������Ϊ40
	}	
	return(1);				 // �Ϸ���
}


static uint8_t  sumofRxdBuf_debug(uint16_t l)  //��͵ı��ģ���������ʼ��ʶ,	l	�ĳ��Ȱ�����ʼ��ʶ
{ 
	uint8_t sum=0;
	uint16_t i=0;
	if(l<2) return (0);
	{
		for(i=1;i<l-2;i++)
		sum=sum+RxdBuf_debug[i];
		return (sum);
	}
}



static uint8_t  isTelComplete_debug(void)	   // =0 ������    =1 CRC Error   =2 ��ȷ
{
	uint16_t  temp16;
	uint8_t   dat_len;

	if(RxdBytes_debug_module2<4)return(0);
  ////////////////
	dat_len=getTelLength_debug()+6;	//�����ݳ���
	if(dat_len==0)return(0);
	if(RxdBytes_debug_module2<dat_len)return(0);

	temp16=sumofRxdBuf_debug(dat_len);
//	if (RxdBuf_debug[dat_len-1]==0x7e)
//		return(2); 
	if ((RxdBuf_debug[dat_len-2]==temp16)&&(RxdBuf_debug[dat_len-1]==0x7e))
		return(2); 
	else
	{
		return(1);
	}
	
}						 

static uint8_t  leftRxdTel_debug(void)		//��������һλ
{
	uint8_t i;
	if(RxdBytes_debug_module2<1)return(0);     // �޷�����
	for	(i=1;i<RxdBytes_debug_module2;i++)
	{
		RxdBuf_debug[i-1]=RxdBuf_debug[i];		
	}
	RxdBytes_debug_module2--;
	return(1);					 // ����һ���ֽڳɹ�
}


static void  RxdByte_debug(uint8_t c)
{	
	uint8_t 	i;
	RxdBuf_debug[RxdBytes_debug_module2]=c;
	RxdBytes_debug_module2++;

	switch(RxdBytes_debug_module2)
	{
		case 0:	break;
		case 3:	break;
		case 1:
		case 2:
		case 4:
				while(!isVaildTel_debug())	//������Ϸ�			 
				{
					if(!leftRxdTel_debug())break;	  // �������ֽ�
				}
				break;
		default:		
				i=isTelComplete_debug();
				if(i==2)
				{
					//do some thing
					for(i=0;i<RxdBytes_debug_module2;i++)	CmdBuf_debug[i]=RxdBuf_debug[i];
					CmdBuf_debugLength=RxdBytes_debug_module2;
					//module_2_receiveloop_debug();
					//WritedatatoLAN(CmdBuf_debug,CmdBuf_debugLength);//��Ҫ���ж���ӣ��ڱ��Ĵ����м�
					//OSSemPost(sem_uart3_buf);  //�ж������ͷ��ź���   //ԭ����û�����ã��ص�����ʽ�����ʱ����ģ�鴦��û�ж�Ӧ����Ч
					OSSemPost(sem_uart6_buf);  //�ж������ͷ��ź���
					module_2_ReceivedTel_debug=1;
					RxdBytes_debug_module2=0;
				}
				else if(i==1)	 // CRC error
				{
					leftRxdTel_debug();
					while(!isVaildTel_debug())	//������Ϸ�			 
					{
						if(!leftRxdTel_debug())break;
					}	
				}
				else if(i==0) //û���������
				{
				
				}
				else
				{
				}
				break;			
		}
}


static uint8_t  hasByteToTxd_debug()		 // =1 ���ֽڴ���
{
	if(TxdBytes_debug<TxdTelBytes_debug)return(1);
	TxdTelBytes_debug=0;
	TxdBytes_debug=0;
	return(0);	
}


static uint8_t  getTxdByte_debug()   
{
	uint8_t re;
	if(TxdBytes_debug <TxdTelBytes_debug)
	{
		re=TxdBuf_debug[TxdBytes_debug];
		TxdBytes_debug++;
		return(re);
	}
	else
	{
		TxdTelBytes_debug=0;
		TxdBytes_debug=0;
		return(0);
	}
}


static void  startTxd()
{	
	USART_SendData(USART6,getTxdByte_debug()); 
}


//void  WritedatatoUSART6(uint8_t *sourdata,uint32_t length)
//{
//	uint32_t i=0;
//	u8 err;
//	LED0_SET();
//	OSSemPend(uart6_write,0,&err);   //��������ֹ�в�ͬ�̶߳�Ҫд��
//	LED1_SET();
//	TxdBuf_debug[0]=config.module_2_destination_addr>>8;  
//	TxdBuf_debug[1]=config.module_2_destination_addr;  
//	TxdBuf_debug[2]=config.module_2_channel; 
//	for(i=0;i<length;i++)
//		TxdBuf_debug[3+i]=sourdata[i];
//	TxdBytes_debug=0;
//	TxdTelBytes_debug=3+length;
//	startTxd();
//	OSSemPost(uart6_write);
//}


void  WritedatatoUSART6(uint8_t *sourdata,uint32_t length)
{
	uint32_t i=0,j;
	u8 err;
	LED0_SET();
	OSSemPend(uart6_write,0,&err);   //��������ֹ�в�ͬ�̶߳�Ҫд��
	LED1_SET();
	
	memset(TxdBuf_debug,0,RX_BUFFER_SIZE);
	memset(CompareData,0,RX_BUFFER_SIZE);
	
	TxdBuf_debug[0]=config.module_2_destination_addr>>8;  
	TxdBuf_debug[1]=config.module_2_destination_addr;  
	TxdBuf_debug[2]=config.module_2_channel; 
	TxdBuf_debug[3]=sourdata[0];        				//0x7e
	TxdBuf_debug[4]=sourdata[1];        				//0x45
	TxdBuf_debug[5]=sourdata[2];       		 			//0x11
	TxdBuf_debug[6]=sourdata[3];        				//0x00
	TxdBuf_debug[7]=sourdata[4];        				//0x01
	TxdBuf_debug[8]=sourdata[5];        				//�¶ȵ�
	TxdBuf_debug[9]=sourdata[6];        				//�¶�
	TxdBuf_debug[10]=sourdata[7];       				//�¶�
	TxdBuf_debug[11]=sourdata[8];       				//�¶ȸ�
	TxdBuf_debug[12]=sourdata[9];       				//��ص���
	TxdBuf_debug[13]=sourdata[10];      				//SN��ߣ�����λ�����ڵ��ַ��
	TxdBuf_debug[14]=sourdata[11];						//SN���
	TxdBuf_debug[15]=config.module_2_source_addr>>8;	//ԭ���Ľڵ��ַ��   ��Ϊ�м̷��͵�ַ
	TxdBuf_debug[16]=config.module_2_source_addr;		//ԭ���Ľڵ��ַ��
	TxdBuf_debug[17]=sourdata[14];     					//���λ
	TxdBuf_debug[18]=sourdata[15];						//���λ
	TxdBuf_debug[19]=sourdata[16];      				//��
	TxdBuf_debug[20]=sourdata[17]; 						//��
	TxdBuf_debug[21]=sourdata[18];      				//ʱ
	TxdBuf_debug[22]=sourdata[19];      				//��
	TxdBuf_debug[23]=sourdata[20];      				//��
	TxdBuf_debug[24]=0;
	for(i=4;i<24;i++)
		TxdBuf_debug[24]+=TxdBuf_debug[i];				//��У��     				
	TxdBuf_debug[25]=sourdata[22];	    				//0x7e
	TxdBytes_debug=0;
	TxdTelBytes_debug=3+length;
	

	CompareData[0]=config.module_2_destination_addr>>8;  
	CompareData[1]=config.module_2_destination_addr;  
	CompareData[2]=config.module_2_channel; 
	CompareData[3]=sourdata[0];        				//0x7e
	CompareData[4]=sourdata[1];        				//0x45
	CompareData[5]=sourdata[2];       		 		//0x11
	CompareData[6]=sourdata[3];        				//0x00
	CompareData[7]=sourdata[4];        				//0x01
	CompareData[8]=sourdata[5];        				//�¶ȵ�
	CompareData[9]=sourdata[6];        				//�¶�
	CompareData[10]=sourdata[7];       				//�¶�
	CompareData[11]=sourdata[8];       				//�¶ȸ�
	CompareData[12]=sourdata[9];       				//��ص���
	CompareData[13]=sourdata[10];      				//SN��ߣ�����λ�����ڵ��ַ��
	CompareData[14]=sourdata[11];					//SN���
	CompareData[15]=config.module_2_source_addr>>8;	//ԭ���Ľڵ��ַ��   ��Ϊ�м̷��͵�ַ
	CompareData[16]=config.module_2_source_addr;	//ԭ���Ľڵ��ַ��
	CompareData[17]=sourdata[14];     				//���λ
	CompareData[18]=sourdata[15];					//���λ
	CompareData[19]=sourdata[16];      				//��
	CompareData[20]=sourdata[17]; 					//��
	CompareData[21]=sourdata[18];      				//ʱ
	CompareData[22]=sourdata[19];      				//��
	CompareData[23]=sourdata[20];      				//��
	CompareData[24]=0;
	for(i=4;i<24;i++)
		CompareData[24]+=CompareData[i];				//��У��     				
	CompareData[25]=sourdata[22];	    				//0x7e

	for(j=0;j<26;j++)
	{
		if(TxdBuf_debug[j]!=CompareData[j])return;
	}
	
	startTxd();
	OSSemPost(uart6_write);
}


void  WritedatatoUSART6_duishi(uint8_t *sourdata,uint32_t length)
{
	uint32_t i=0;
	u8 err;
	LED0_SET();
	OSSemPend(uart6_write,0,&err);   //��������ֹ�в�ͬ�̶߳�Ҫд��
	LED1_SET();
	TxdBuf_debug[0]=current_receive_module_2_node>>8;  //����������ȥ
	TxdBuf_debug[1]=current_receive_module_2_node;  
	TxdBuf_debug[2]=config.module_2_channel; 
	for(i=0;i<length;i++)
		TxdBuf_debug[3+i]=sourdata[i];
	TxdBytes_debug=0;
	TxdTelBytes_debug=3+length;
	startTxd();
	OSSemPost(uart6_write);
}


void  WriteConfigdatatoUSART6(uint8_t *sourdata,uint32_t length)
{
	uint32_t i=0;
	u8 err;
	LED0_SET();
	OSSemPend(uart6_write,0,&err);   //��������ֹ�в�ͬ�̶߳�Ҫд��
	for(i=0;i<length;i++)
		TxdBuf_debug[i]=sourdata[i];
	TxdBytes_debug=0;
	TxdTelBytes_debug=length;
	startTxd();
	OSSemPost(uart6_write);
}


void  bridge_command_require_time(void)
{
	if(config.DataToBoardMode==BRIDGEMODE)  //�м�ģʽ
	{
		uint32_t i;
		u8 err;
		OSSemPend(uart6_write,0,&err);   //��������ֹ�в�ͬ�̶߳�Ҫд��
		TxdBuf_debug[0]=config.module_2_destination_addr>>8;  
		TxdBuf_debug[1]=config.module_2_destination_addr;  
		TxdBuf_debug[2]=config.module_2_channel;  
		TxdBuf_debug[3]=0x7e;
		TxdBuf_debug[4]=COMMAND_REQUIRE_TIME;  //0X48
		TxdBuf_debug[5]=0x03;
		TxdBuf_debug[6]=0x00;
		TxdBuf_debug[7]=config.module_2_source_addr>>8;//ǰ�����ֽڲ�����
		TxdBuf_debug[8]=config.module_2_source_addr;
		TxdBuf_debug[9]=0X00;
		TxdBuf_debug[10]=0;
		for(i=4;i<10;i++)
			TxdBuf_debug[10]+=TxdBuf_debug[i];
		TxdBuf_debug[11]=0x7e;
		TxdBytes_debug=0;
		TxdTelBytes_debug=12;
		startTxd();	
		OSSemPost(uart6_write);
	}
}


void  USART6_IRQHandler(void)
{	
	uint8_t c;
	if(USART_GetITStatus(USART6,USART_IT_RXNE)!= RESET) 
	{ 
		RxdByte_debug((uint8_t)(USART_ReceiveData(USART6)));	
	}
	else if(USART_GetITStatus(USART6,USART_IT_TC)!= RESET) 
	{	
		USART_ClearITPendingBit(USART6,USART_IT_TC);
		if(hasByteToTxd_debug())		
		{
			c=getTxdByte_debug();
			USART_SendData(USART6,c);
		}		
		else
		{	
		}
	}
	ResetBit(USART6->SR,3);  // ??ORE
}




//static  uint8_t command_adjust_time1(void)   //���ػط�ʱ��
//{  
//	uint32_t i;
//	RTC_TimeTypeDef time;
//	RTC_DateTypeDef date;
//	current_receive_module_2_node=CmdBuf_debug[5]+(((uint16_t)CmdBuf_debug[4])<<8);
//	if(CmdBuf_debug[2]==0x02)                  //�ط��ڵ�
//	{
//		RTC_GetTime(RTC_Format_BIN,&time);
//		RTC_GetDate(RTC_Format_BIN,&date);
//		CmdBuf_debug[0]=0x7e;
//		CmdBuf_debug[1]=COMMAND_SET_TIME;
//		CmdBuf_debug[2]=0x07;     //����
//		CmdBuf_debug[3]=0x00;
//		CmdBuf_debug[4]=(date.RTC_Year+2000);
//		CmdBuf_debug[5]=(date.RTC_Year+2000)>>8;
//		CmdBuf_debug[6]=date.RTC_Month;	
//		CmdBuf_debug[7]=date.RTC_Date;
//		CmdBuf_debug[8]=time.RTC_Hours;
//		CmdBuf_debug[9]=time.RTC_Minutes;
//		CmdBuf_debug[10]=time.RTC_Seconds;
//		CmdBuf_debug[11]=0;
//		for(i=1;i<11;i++)
//			CmdBuf_debug[11]+=CmdBuf_debug[i];
//		CmdBuf_debug[12]=0x7e;
//		CmdBuf_debugLength=13;
//		WritedatatoUSART6_duishi(CmdBuf_debug,CmdBuf_debugLength);
//	}
//	else                                    //�ط��м�
//	{
//		RTC_GetTime(RTC_Format_BIN,&time);
//		RTC_GetDate(RTC_Format_BIN,&date);
//		CmdBuf_debug[0]=0x7e;
//		CmdBuf_debug[1]=COMMAND_SET_TIME;
//		CmdBuf_debug[2]=0x08;     //����
//		CmdBuf_debug[3]=0x00;
//		CmdBuf_debug[4]=(date.RTC_Year);
//		CmdBuf_debug[5]=(date.RTC_Year)>>8;
//		CmdBuf_debug[6]=date.RTC_Month;	
//		CmdBuf_debug[7]=date.RTC_Date;
//		CmdBuf_debug[8]=time.RTC_Hours;
//		CmdBuf_debug[9]=time.RTC_Minutes;
//		CmdBuf_debug[10]=time.RTC_Seconds;
//		CmdBuf_debug[11]=date.RTC_WeekDay;
//		CmdBuf_debug[12]=0x00;
//		for(i=1;i<12;i++)
//			CmdBuf_debug[12]+=CmdBuf_debug[i];
//		CmdBuf_debug[13]=0x7e;
//		CmdBuf_debugLength=14;
//		WritedatatoUSART6_duishi(CmdBuf_debug,CmdBuf_debugLength);		
//	}
//	return (1);
//}


static uint8_t  command_adjust_time2(void)    //�����м̶��ԣ������������ʱ��  //�м����Ƚϣ���һ��ÿ�ζ�����
{ 
	if(config.DataToBoardMode==BRIDGEMODE)
	{
		uint32_t year,month,day,weekday,hour,minute,second;
//		RTC_TimeTypeDef time;
//		RTC_GetTime(RTC_Format_BIN,&time);
//		if((CmdBuf_debug[10]-time.RTC_Seconds)>3||(time.RTC_Seconds-CmdBuf_debug[10])>3)
//		{
			year = ((uint16_t)CmdBuf_debug[5]<<8)+CmdBuf_debug[4];     //�м̼�2000
			if(year>2000)
			{
				year=year-2000;
			}
			month = CmdBuf_debug[6];
			day = CmdBuf_debug[7];
			hour = CmdBuf_debug[8];
			minute = CmdBuf_debug[9];
			second = CmdBuf_debug[10];
			weekday = CmdBuf_debug[11];
			RTC_Set_Time(hour,minute,second,1);
			RTC_Set_Date(year,month,day,weekday);
			RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//����Ѿ���ʼ������
//		}
	Parameter.battery_reset_flag=CmdBuf_debug[12];
	Parameter.addr_node_high=CmdBuf_debug[13];
	Parameter.addr_node_lower=CmdBuf_debug[14];
	}
	return (1);
}


static uint8_t Reback(void)   //ACK
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	uint32_t i;
	current_receive_module_2_node=CmdBuf_debug[13]+(((uint16_t)CmdBuf_debug[12])<<8);  //��10��11���ֽ��ǽڵ��ַ
	RTC_GetTime(RTC_Format_BIN,&time);
	RTC_GetDate(RTC_Format_BIN,&date);
	
	CmdBuf_debug[0]=0x7e;
	CmdBuf_debug[1]=COMMAND_SET_TIME;    //0x50
	CmdBuf_debug[2]=0x0b;     //����
	CmdBuf_debug[3]=0x00;
	CmdBuf_debug[4]=(date.RTC_Year+2000);   //ʱ������ã�ȫ����2000Ϊ��׼��ʽ��ͨѶ���շ�
	CmdBuf_debug[5]=(date.RTC_Year+2000)>>8;
	CmdBuf_debug[6]=date.RTC_Month;	
	CmdBuf_debug[7]=date.RTC_Date;
	CmdBuf_debug[8]=time.RTC_Hours;
	CmdBuf_debug[9]=time.RTC_Minutes;
	CmdBuf_debug[10]=time.RTC_Seconds;
	CmdBuf_debug[11]=date.RTC_WeekDay;
	CmdBuf_debug[12]=Parameter.battery_reset_flag;
	CmdBuf_debug[13]=Parameter.addr_node_high;
	CmdBuf_debug[14]=Parameter.addr_node_lower;
	CmdBuf_debug[15]=0;
	for(i=1;i<15;i++)
		CmdBuf_debug[15]+=CmdBuf_debug[i];
	CmdBuf_debug[16]=0x7e;
	CmdBuf_debugLength=17;
	WritedatatoUSART6_duishi(CmdBuf_debug,CmdBuf_debugLength);

	return (1);
}



extern  uint8_t WriteDataToTXDBUF(uint8_t * source,uint16_t length);
static uint8_t  command_reply_temp(void)
{
	if(config.DataToBoardMode==GATAWAYMODE)          //����ģʽ
	{
		if((CmdBuf_debug[9]>=99)&&(CmdBuf_debug[10]==Parameter.addr_node_high)&&(CmdBuf_debug[11]==Parameter.addr_node_lower))
		{
			Parameter.battery_reset_flag=0;
		}
		WriteDataToTXDBUF(CmdBuf_debug,CmdBuf_debugLength);
		Reback();
	}
	else if(config.DataToBoardMode==BRIDGEMODE)      //�м�ģʽ
	{
//		WritedatatoLAN(CmdBuf_debug,CmdBuf_debugLength);  �м�ģʽ�����յ�0x45,�յ�Ҳ���ظ�
	}
	return 1;
}



uint32_t receive_echo=0;

void  module_2_receiveloop_debug()
{
	module_1_ReceivedTel_debug=0;
	module_2_ReceivedTel_debug=0;//�¼�
		//OpenAllInt();
 		
	switch( CmdBuf_debug[1])
	{     //��������
//		case COMMAND_ID:     //0x02
//			   command_id();
//			break;
//		case COMMAND_CHANNELKIND:     //0x03 ͨ����������
//			   command_channelkind();
//			break;
//		case COMMAND_REPLYCONFIG:     //0x03 ͨ����������
//			   command_replyconfig();
//			break;
//		case COMMAND_STOP:
//			   command_stop();
//			break;
//		case COMMAND_SETCONFIG:   //����IP��ַ
//			   command_setconfig();
//			break;
//		case COMMAND_SET_RUNMODE:   //����IP��ַ
//			   command_setrunmode();
//			break;
//		case COMMAND_START:
//			   command_start();
// 			break;
//		case COMMAND_REQUIRE_PERIODWAVE:
//			   command_require_periodwave();
//			break;
//		case COMMAND_SET_AP:
//			   command_set_ap();
//			break;
//		case COMMAND_SET_CHANNEL_CONDITION:
//			   command_set_channel_condition();
//			break;
//		case COMMAND_REPLY_CHANNEL_CONDITION:
//			   command_reply_channel_condition();
//			break;
//		case COMMAND_REPLY_RATE:    //���ò�������
//			   command_reply_SampleParameter();
//		  break;
//		case COMMAND_SAMPLE_RATE:    //���ò�������
//			   command_set_SampleParameter();
//			break;
//		case COMMAND_SET_TCPSERVE:   //����tcp server��Ŀ���ַ����
//			   command_set_tcpserve();
//		  break;
//		case COMMAND_REPLYTCPSERVE:   //����tcp server��Ŀ���ַ����
//			   command_replytcpserve();
//			break; 
//		case COMMAND_REQUIRE_TIME:           //0x48   ��Ϊ����ʱ
// 			   command_adjust_time1();
//        break;
		case COMMAND_SET_TIME:               //0x50   ��Ϊ�м�ʱ
				command_adjust_time2();
				receive_echo=1;
		break;		
//		case COMMAND_REPLYAP:
//			   command_replyap();  //����apֵ
//			break;
//      case COMMAND_APPLYNETSET:
//			   command_applynetset();  //Ӧ����������
//			break;
//		case 0x40:
//			   command_counter();  //Ӧ����������
//			break;
//		case COMMAND_REPLY_RUNMODE:
//			   command_reply_runmode();
//		  break;
//		case COMMAND_ADJUST_TEMP:
//			   command_adjust_temp();
//		  break;
//		case COMMAND_ADJUST_ADC:
//			   command_adjust_adc();
//		  break;
//		case COMMAND_SET_SCALE:
//			   command_set_scale();
//			break;
//		case COMMAND_REPLY_SCALE:
//			   command_reply_scale();
//			break;
//      case COMMAND_SET_SNNUMBER:
//			   command_set_snnumber();
//			break;
		case COMMAND_REPLYTEMP:       //0x45
			command_reply_temp();
			break;
		default:
			break;
	}
	
}


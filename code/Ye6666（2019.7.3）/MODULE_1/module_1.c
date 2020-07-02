#include "app.h"
#include "ucos_ii.h"



static uint8_t 	 TxdBuf_debug[TX_BUFFER_SIZE];
static uint8_t   RxdBuf_debug[RX_BUFFER_SIZE];  //�������õ���


static uint8_t   CmdBuf_debug[2*TX_BUFFER_SIZE];
static uint32_t  CmdBuf_debugLength=0;


static uint8_t  TxdBytes_debug=0;
static uint8_t  TxdTelBytes_debug=0;
uint8_t   module_1_ReceivedTel_debug=0;
uint8_t   RxdBytes_debug_module1=0;

volatile uint8_t  reply_flag=0;


static const uint32_t  BAUNDRATE_INT_TAB[]={1200,2400,4800,9600,19200,38400,57600,115200};


void  setComUART3(uint8_t baudrate,uint8_t parity)    //baudrateΪ������ѡ��parityΪУ��ѡ��
{
	uint32_t  br;
	USART_InitTypeDef  USART_InitStructure;
	
	USART_ITConfig(USART3,USART_IT_RXNE,DISABLE);
	USART_ITConfig(USART3,USART_IT_TC,DISABLE);
	USART_Cmd(USART3, DISABLE);	 
	/*-------------------------USART3------------------------*/
	br=BAUNDRATE_INT_TAB[baudrate];
		
	USART_InitStructure.USART_BaudRate = br;                //���ڲ�����
	USART_InitStructure.USART_StopBits = USART_StopBits_1;  //ֹͣλ
	if(parity==0) {USART_InitStructure.USART_WordLength = USART_WordLength_8b;USART_InitStructure.USART_Parity = USART_Parity_No;}  //����żУ��
	if(parity==1) {USART_InitStructure.USART_WordLength = USART_WordLength_9b;USART_InitStructure.USART_Parity = USART_Parity_Odd;} // ��У��
	if(parity==2) {USART_InitStructure.USART_WordLength = USART_WordLength_9b;USART_InitStructure.USART_Parity = USART_Parity_Even;}// żУ��		
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�ģʽ
	
	USART_Init(USART3, &USART_InitStructure);     //��ʼ��
	USART_ITConfig(USART3,USART_IT_TC,ENABLE);    //ʹ�ܽ����ж�
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);  //ʹ�ܷ����ж�
	USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���3
}


void  initMODULE_1(void)
{

	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//ʹ��USART3ʱ��
	

	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); //GPIOB10����ΪUSART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); //GPIOB11����ΪUSART3
	
	//USART3 ���ų�ʼ��  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10; //GPIOB10  GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;          //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	  //�ٶ�
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;        //�������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          //����
	GPIO_Init(GPIOB,&GPIO_InitStructure);   //��ʼ��


	//M0 M1   PG1   PF15   ���ų�ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOG,&GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(GPIOF,&GPIO_InitStructure); 
	 

	//Usart3 NVIC �жϳ�ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		
	NVIC_Init(&NVIC_InitStructure);	


	//RS485_TX_EN=0;			

	/* uart init */
	setComUART3(config.module_1_baudrate,config.parity);

	/* enable interrupt */
	//USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
}


void  MODULE1_ModeSet(uint8_t mode) 
{
	if(Normal_Mode==mode)                   //00
	{
		GPIO_ResetBits(GPIOG,GPIO_Pin_1);  
		GPIO_ResetBits(GPIOF,GPIO_Pin_15);
	}
	else if(Coordinator_Mode==mode)        //10  
	{
		GPIO_SetBits(GPIOG,GPIO_Pin_1);
		GPIO_ResetBits(GPIOF,GPIO_Pin_15);
	}
	else if(LowPower_Mode==mode)           //01 
	{
		GPIO_ResetBits(GPIOG,GPIO_Pin_1);
		GPIO_SetBits(GPIOF,GPIO_Pin_15);
	}
	else if(Sleep_Mode==mode)              //11 
	{
		GPIO_SetBits(GPIOG,GPIO_Pin_1);
		GPIO_SetBits(GPIOF,GPIO_Pin_15);
	}
}


// PC12
#define Enable485TXD() {}
#define Disable485TXD() {}

	
static  uint8_t  getTelLength_debug(void)
{					
	return(((uint16_t)RxdBuf_debug[3]<<8)+RxdBuf_debug[2]);				
}


static uint8_t  isVaildTel_debug(void)
{
	if(RxdBytes_debug_module1>=1)if(RxdBuf_debug[0]!=0x7e)return(0);
	if(RxdBytes_debug_module1>=2)if((RxdBuf_debug[1]!=0x42)&&(RxdBuf_debug[1]!=0x45)&&(RxdBuf_debug[1]!=0x48)&&(RxdBuf_debug[1]!=0x50))return(0);   //��������
		
	if(RxdBytes_debug_module1>=4) 
	{ 
		uint16_t length=getTelLength_debug();
		if((length>40)) return(0);  //��������Ϊ40
	}
	return(1);				 // �Ϸ���
}


static  uint8_t sumofRxdBuf_debug(uint16_t l)  //��͵ı��ģ���������ʼ��ʶ,	l	�ĳ��Ȱ�����ʼ��ʶ
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



static  uint8_t isTelComplete_debug(void)	   // =0 ������  =1 CRC Error =2 ��ȷ
{
	uint16_t  temp16;
	uint8_t   dat_len;

	if(RxdBytes_debug_module1<4)return(0);
  
	dat_len=getTelLength_debug()+6;	//�����ݳ���
	if(dat_len==0)return(0);
	if(RxdBytes_debug_module1<dat_len)return(0);

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
	if(RxdBytes_debug_module1<1)return(0);     // �޷�����
	for	(i=1;i<RxdBytes_debug_module1;i++)
	{
		RxdBuf_debug[i-1]=RxdBuf_debug[i];		
	}
	RxdBytes_debug_module1--;
	return(1);					 // ����һ���ֽڳɹ�
}


static void  RxdByte_debug(uint8_t c)
{	
	uint8_t 	i;
	RxdBuf_debug[RxdBytes_debug_module1]=c;
	RxdBytes_debug_module1++;

	switch(RxdBytes_debug_module1)
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
					for(i=0;i<RxdBytes_debug_module1;i++)	CmdBuf_debug[i]=RxdBuf_debug[i];
					CmdBuf_debugLength=RxdBytes_debug_module1;
					module_1_ReceivedTel_debug=1;
					RxdBytes_debug_module1=0;
					OSSemPost(sem_uart3_buf);  //�ж������ͷ��ź���
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


static  uint8_t hasByteToTxd_debug()		 // =1 ���ֽڴ���
{
	if(TxdBytes_debug<TxdTelBytes_debug)return(1);
	TxdTelBytes_debug=0;
	TxdBytes_debug=0;
	return(0);	
}


static  uint8_t  getTxdByte_debug()   //
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
	Enable485TXD();
	USART_SendData(USART3,getTxdByte_debug()); 
}



void  USART3_IRQHandler(void)
{	
	uint8_t c;
	if(USART_GetITStatus(USART3,USART_IT_RXNE)!= RESET) 
	{ 
		RxdByte_debug((uint8_t)(USART_ReceiveData(USART3)));	
	}
	else if(USART_GetITStatus(USART3,USART_IT_TC)!= RESET) 	
	{	  
		USART_ClearITPendingBit(USART3,USART_IT_TC);
		if(hasByteToTxd_debug())	
		{
			Enable485TXD();
			c=getTxdByte_debug();
			USART_SendData(USART3,c);
		}		
		else		
		{
			Disable485TXD();
		}
	}
	ResetBit(USART3->SR,3);  // ORE��0
}


//uint16_t command_id(void)
//{ 
//	uint16_t i=0;
//	 {
//		CmdBuf_debug[0]=0x7e;
//		CmdBuf_debug[1]=0x02;
//		CmdBuf_debug[2]=0x08;
//		CmdBuf_debug[3]=0x00;
//		CmdBuf_debug[11]=config.SNnumber;
//		CmdBuf_debug[10]=config.SNnumber>>8;
//		CmdBuf_debug[9]=config.SNnumber>>16;
//		CmdBuf_debug[8]=config.SNnumber>>24;
//		CmdBuf_debug[7]=config.SNnumber>>32;
//		CmdBuf_debug[6]=config.SNnumber>>40;
//		CmdBuf_debug[5]=config.SNnumber>>48;
//		CmdBuf_debug[4]=config.SNnumber>>56;
//		CmdBuf_debug[12]=0;
//		for(i=1;i<12;i++)
//		CmdBuf_debug[12]+=CmdBuf_debug[i];
//		CmdBuf_debug[13]=0x7e;
//	  CmdBuf_debugLength=14;
//    WriteDataToTXDBUF_uart2(CmdBuf_debug,CmdBuf_debugLength);
//	 }
//	 return 1;
//}

//uint8_t command_stop(void)
//{
//	WriteDataToTXDBUF_uart2(CmdBuf_debug,CmdBuf_debugLength);
//	setConfiguration();//��������״̬����λ�����ӵ�ʱ�����ȷ���ֹͣ����������뵽����״̬
//	E32_ModeSet(Sleep_Mode);  //��ģ���������ģʽ���Ա�����
//	E32PowerSelect(1); //��ģ���Դ
//	ADBEGIN();//��ʼADC
//	return 1;
//}

//uint8_t command_start(void)
//{
//		
//  WriteDataToTXDBUF_uart2(CmdBuf_debug,CmdBuf_debugLength);
//	clrConfiguration();//�������״̬
//	ADOVER();//����
//	return 1;
//}

//uint8_t command_setconfig(void)
//{
//	uint32_t i;
//	WriteDataToTXDBUF_uart2(CmdBuf_debug,CmdBuf_debugLength);
//	
//	CmdBuf_debug[9]=CmdBuf_debug[9]&0x3F;//�������ݷ�ʽΪ8N1;
//	//CmdBuf_debug[9]=;
//	
//	config.module_1_destination_addr=CmdBuf_debug[4]+((uint16_t)CmdBuf_debug[5]<<8);
//	config.module_1_source_addr=CmdBuf_debug[7]+((uint16_t)CmdBuf_debug[8]<<8);
//	config.module_1_datacheck=CmdBuf_debug[9]&0x7;
//	config.module_1_datacheck=(CmdBuf_debug[9]>>3)&0x6;
//	config.module_1_datacheck=(CmdBuf_debug[9]>>6)&0x3;
//	
//	config.module_1_wakeup_time=CmdBuf_debug[10];
//	
//	config.module_1_datacheck=CmdBuf_debug[11]&3;
//  config.module_1_FEC=(CmdBuf_debug[11]>>2)&1;
//  config.module_1_wakeup_time=(CmdBuf_debug[11]>>3)&7;
//	config.module_1_IO_workstyle=(CmdBuf_debug[11]>>6)&1;
//	config.module_1_transmission_mode=(CmdBuf_debug[11]>>7)&1;
//	
//	uart0Init(config.module_1_datacheck,0,0,8,1);  //���³�ʼ��һ�´���
//	saveConfig();
//	for(i=0;i<6;i++)   //���͸�����ģ��
//		CmdBuf[i]=CmdBuf_debug[i+6];
//	CmdBuf_Length=6;
//	WriteDataToTXDBUF(CmdBuf,CmdBuf_Length);
//	return 1;
//	
//}

//uint8_t command_reply_config(void)
//{
//	uint32_t i=0;
//	CmdBuf_debug[0]=0x7e;
//	CmdBuf_debug[1]=COMMAND_REPLYCONFIG;
//	CmdBuf_debug[2]=0x07;
//	CmdBuf_debug[3]=0x00;
//	CmdBuf_debug[4]=config.module_1_destination_addr;
//	CmdBuf_debug[5]=config.module_1_destination_addr>>8;
//	//************5���ֽڵ�ģ�����ò���**************//
//	CmdBuf_debug[6]=0xc2;  //���籣��
//	CmdBuf_debug[7]=config.module_1_source_addr;
//	CmdBuf_debug[8]=config.module_1_source_addr>>8;
//	CmdBuf_debug[9]=config.module_1_datacheck|(config.module_1_datacheck<<3)|(config.module_1_datacheck<<6);
//	CmdBuf_debug[10]=config.module_1_wakeup_time;
//	CmdBuf_debug[11]=config.module_1_datacheck|(config.module_1_FEC<<2)|(config.module_1_wakeup_time<<3)|(config.module_1_IO_workstyle<<6)|(config.module_1_transmission_mode<<7);
//	CmdBuf_debug[12]=0;
//	for(i=1;i<12;i++)
//		CmdBuf_debug[12]+=CmdBuf_debug[i];
//	CmdBuf_debug[13]=0x7e;
//	CmdBuf_debugLength=14;
//	WriteDataToTXDBUF_uart2(CmdBuf_debug,CmdBuf_debugLength);
//	return 1;
//	
//}

//uint8_t command_set_snnumber(void)   //����TCP_SERVER��Ŀ����λ����ַ
//{
//	{
//    WriteDataToTXDBUF_uart2(CmdBuf_debug,CmdBuf_debugLength);
//	 }

//	 config.SNnumber=(uint64_t)CmdBuf_debug[4]+((uint64_t)CmdBuf_debug[5]<<8)+((uint64_t)CmdBuf_debug[6]<<16)+((uint64_t)CmdBuf_debug[7]<<24)+((uint64_t)CmdBuf_debug[8]<<32)+((uint64_t)CmdBuf_debug[9]<<40)+((uint64_t)CmdBuf_debug[10]<<48)+((uint64_t)CmdBuf_debug[11]<<56);//��AP�ַ�����ֵ��config
//	 saveConfig();
//	 return (1);
//}


uint16_t  current_receive_module_1_node=0;
//static  uint8_t command_adjust_time(void)   //���ػ��м̻ط�ʱ��
//{  
//	uint32_t i;
//	RTC_TimeTypeDef time;
//	RTC_DateTypeDef date;
//	current_receive_module_1_node=CmdBuf_debug[5]+(((uint16_t)CmdBuf_debug[4])<<8);
//	if(CmdBuf_debug[2]==0x02)                  //�ط��ڵ�
//	{
//		RTC_GetTime(RTC_Format_BIN,&time);
//		RTC_GetDate(RTC_Format_BIN,&date);
//		CmdBuf_debug[0]=0x7e;
//		CmdBuf_debug[1]=COMMAND_SET_TIME;    //0x50
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
//		WritedatatoUSART3(CmdBuf_debug,CmdBuf_debugLength);
//	}
//	else                                    //�ط��м�
//	{
//		RTC_GetTime(RTC_Format_BIN,&time);
//		RTC_GetDate(RTC_Format_BIN,&date);
//		CmdBuf_debug[0]=0x7e;
//		CmdBuf_debug[1]=COMMAND_SET_TIME;   //0x50
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
//		CmdBuf_debug[12]=0;
//		for(i=1;i<12;i++)
//			CmdBuf_debug[12]+=CmdBuf_debug[i];
//		CmdBuf_debug[13]=0x7e;
//		CmdBuf_debugLength=14;
//		WritedatatoUSART3(CmdBuf_debug,CmdBuf_debugLength);		
//	}
//	return (1);
//}





static uint8_t Reback(void)       //ACK
{
	uint32_t i;
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	current_receive_module_1_node=CmdBuf_debug[13]+(((uint16_t)CmdBuf_debug[12])<<8);
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
	WritedatatoUSART3(CmdBuf_debug,CmdBuf_debugLength);
	
	return (1);
}


extern  uint8_t WriteDataToTXDBUF(uint8_t * source,uint16_t length);
uint8_t  command_reply_temp1(void)
{
//	if(config.DataToBoardMode==GATAWAYMODE)     //����ģʽ
//	{
//		//current_receive_module_1_node=CmdBuf_debug[5]+(((uint16_t)CmdBuf_debug[4])<<8);//
//		Reback();
//		WritedatatoTxdbuf(CmdBuf_debug,CmdBuf_debugLength);
//		
//	}
//	else if(config.DataToBoardMode==BRIDGEMODE)  //�м�ģʽ
//	{
		//current_receive_module_1_node=CmdBuf_debug[5]+(((uint16_t)CmdBuf_debug[4])<<8);//
		
	if((CmdBuf_debug[9]>=99)&&(CmdBuf_debug[10]==Parameter.addr_node_high)&&(CmdBuf_debug[11]==Parameter.addr_node_lower))
	{
		Parameter.battery_reset_flag=0;
	}
	WriteDataToTXDBUF(CmdBuf_debug,CmdBuf_debugLength);
	Reback();
//	}
	return 1;
}




void  WritedatatoUSART3(uint8_t *sourdata,uint32_t length)
{
	uint32_t i=0;
	u8 err;
	LED0_SET();
	OSSemPend(uart3_write,0,&err);
	TxdBuf_debug[0]=current_receive_module_1_node>>8;  //����������ȥ
	TxdBuf_debug[1]=current_receive_module_1_node;  
	TxdBuf_debug[2]=config.module_1_channel; 
	for(i=0;i<length;i++)
		TxdBuf_debug[3+i]=sourdata[i];
	TxdBytes_debug=0;
	TxdTelBytes_debug=3+length;
	startTxd();
	OSSemPost(uart3_write);
}


void  WriteConfigdatatoUSART3(uint8_t *sourdata,uint32_t length)
{
	uint32_t i=0;
	u8 err;
	LED0_SET();
	OSSemPend(uart3_write,0,&err);   //��������ֹ�в�ͬ�߳Ƕ�Ҫд��
	for(i=0;i<length;i++)
		TxdBuf_debug[i]=sourdata[i];
	TxdBytes_debug=0;
	TxdTelBytes_debug=length;
	startTxd();
	OSSemPost(uart3_write);
}


void  AskForModule_1_Config(void)
{
	TxdBuf_debug[0]=0XC1;  //
	TxdBuf_debug[1]=0XC1;  //
	TxdBuf_debug[2]=0XC1;
	TxdBytes_debug=0;
	TxdTelBytes_debug=3;
	startTxd();
}


void  module_1_receiveloop_debug()
{

	module_1_ReceivedTel_debug=0;
		//OpenAllInt();
 		
	switch( CmdBuf_debug[1])       //��������
	{     
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
//		case COMMAND_REQUIRE_TIME:           //0x48
// 			   command_adjust_time();
//        break;		
//		case COMMAND_REPLYAP:
//			   command_replyap();  //����apֵ
//			break;
//    case COMMAND_APPLYNETSET:
//			   command_applynetset();  //Ӧ����������
//			break;
//		 case 0x40:
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
//    case COMMAND_SET_SNNUMBER:
//			   command_set_snnumber();
//			break;
    case COMMAND_REPLYTEMP:               //0x45
			reply_flag=1;
//			command_reply_temp();
			break;
		default:
			break;
	}
	
}

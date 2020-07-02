#include "app.h"

static const uint32_t  BAUNDRATE_INT_TAB[]={1200,2400,4800,9600,19200,38400,57600,115200};


static uint8_t  RxdBuf_debug[RX_BUFFER_SIZE];  //用来设置调试
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
	if(RxdBytes_debug_module2>=2)if((RxdBuf_debug[1]!=0x42)&&(RxdBuf_debug[1]!=0x45)&&(RxdBuf_debug[1]!=0x48)&&(RxdBuf_debug[1]!=0x50))return(0);   //命令限制	
	if(RxdBytes_debug_module2>=4) 
	{ 
		uint16_t length=getTelLength_debug();
		if((length>40)) return(0);  //限制最大的为40
	}	
	return(1);				 // 合法的
}


static uint8_t  sumofRxdBuf_debug(uint16_t l)  //求和的报文，不包含起始标识,	l	的长度包括起始标识
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



static uint8_t  isTelComplete_debug(void)	   // =0 不完整    =1 CRC Error   =2 正确
{
	uint16_t  temp16;
	uint8_t   dat_len;

	if(RxdBytes_debug_module2<4)return(0);
  ////////////////
	dat_len=getTelLength_debug()+6;	//给数据长度
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

static uint8_t  leftRxdTel_debug(void)		//数组左移一位
{
	uint8_t i;
	if(RxdBytes_debug_module2<1)return(0);     // 无法左移
	for	(i=1;i<RxdBytes_debug_module2;i++)
	{
		RxdBuf_debug[i-1]=RxdBuf_debug[i];		
	}
	RxdBytes_debug_module2--;
	return(1);					 // 丢弃一个字节成功
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
				while(!isVaildTel_debug())	//如果不合法			 
				{
					if(!leftRxdTel_debug())break;	  // 丢弃首字节
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
					//WritedatatoLAN(CmdBuf_debug,CmdBuf_debugLength);//不要在中断里加，在报文处理中加
					//OSSemPost(sem_uart3_buf);  //中断里面释放信号量   //原来是没有作用，回的命令式网络对时，但模块处理没有对应，无效
					OSSemPost(sem_uart6_buf);  //中断里面释放信号量
					module_2_ReceivedTel_debug=1;
					RxdBytes_debug_module2=0;
				}
				else if(i==1)	 // CRC error
				{
					leftRxdTel_debug();
					while(!isVaildTel_debug())	//如果不合法			 
					{
						if(!leftRxdTel_debug())break;
					}	
				}
				else if(i==0) //没收完继续收
				{
				
				}
				else
				{
				}
				break;			
		}
}


static uint8_t  hasByteToTxd_debug()		 // =1 有字节待发
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
//	OSSemPend(uart6_write,0,&err);   //阻塞，防止有不同线程都要写入
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
	OSSemPend(uart6_write,0,&err);   //阻塞，防止有不同线程都要写入
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
	TxdBuf_debug[8]=sourdata[5];        				//温度低
	TxdBuf_debug[9]=sourdata[6];        				//温度
	TxdBuf_debug[10]=sourdata[7];       				//温度
	TxdBuf_debug[11]=sourdata[8];       				//温度高
	TxdBuf_debug[12]=sourdata[9];       				//电池电量
	TxdBuf_debug[13]=sourdata[10];      				//SN码高，给上位机做节点地址用
	TxdBuf_debug[14]=sourdata[11];						//SN码低
	TxdBuf_debug[15]=config.module_2_source_addr>>8;	//原来的节点地址高   改为中继发送地址
	TxdBuf_debug[16]=config.module_2_source_addr;		//原来的节点地址低
	TxdBuf_debug[17]=sourdata[14];     					//年高位
	TxdBuf_debug[18]=sourdata[15];						//年低位
	TxdBuf_debug[19]=sourdata[16];      				//月
	TxdBuf_debug[20]=sourdata[17]; 						//日
	TxdBuf_debug[21]=sourdata[18];      				//时
	TxdBuf_debug[22]=sourdata[19];      				//分
	TxdBuf_debug[23]=sourdata[20];      				//秒
	TxdBuf_debug[24]=0;
	for(i=4;i<24;i++)
		TxdBuf_debug[24]+=TxdBuf_debug[i];				//和校验     				
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
	CompareData[8]=sourdata[5];        				//温度低
	CompareData[9]=sourdata[6];        				//温度
	CompareData[10]=sourdata[7];       				//温度
	CompareData[11]=sourdata[8];       				//温度高
	CompareData[12]=sourdata[9];       				//电池电量
	CompareData[13]=sourdata[10];      				//SN码高，给上位机做节点地址用
	CompareData[14]=sourdata[11];					//SN码低
	CompareData[15]=config.module_2_source_addr>>8;	//原来的节点地址高   改为中继发送地址
	CompareData[16]=config.module_2_source_addr;	//原来的节点地址低
	CompareData[17]=sourdata[14];     				//年高位
	CompareData[18]=sourdata[15];					//年低位
	CompareData[19]=sourdata[16];      				//月
	CompareData[20]=sourdata[17]; 					//日
	CompareData[21]=sourdata[18];      				//时
	CompareData[22]=sourdata[19];      				//分
	CompareData[23]=sourdata[20];      				//秒
	CompareData[24]=0;
	for(i=4;i<24;i++)
		CompareData[24]+=CompareData[i];				//和校验     				
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
	OSSemPend(uart6_write,0,&err);   //阻塞，防止有不同线程都要写入
	LED1_SET();
	TxdBuf_debug[0]=current_receive_module_2_node>>8;  //从哪来往哪去
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
	OSSemPend(uart6_write,0,&err);   //阻塞，防止有不同线程都要写入
	for(i=0;i<length;i++)
		TxdBuf_debug[i]=sourdata[i];
	TxdBytes_debug=0;
	TxdTelBytes_debug=length;
	startTxd();
	OSSemPost(uart6_write);
}


void  bridge_command_require_time(void)
{
	if(config.DataToBoardMode==BRIDGEMODE)  //中继模式
	{
		uint32_t i;
		u8 err;
		OSSemPend(uart6_write,0,&err);   //阻塞，防止有不同线程都要写入
		TxdBuf_debug[0]=config.module_2_destination_addr>>8;  
		TxdBuf_debug[1]=config.module_2_destination_addr;  
		TxdBuf_debug[2]=config.module_2_channel;  
		TxdBuf_debug[3]=0x7e;
		TxdBuf_debug[4]=COMMAND_REQUIRE_TIME;  //0X48
		TxdBuf_debug[5]=0x03;
		TxdBuf_debug[6]=0x00;
		TxdBuf_debug[7]=config.module_2_source_addr>>8;//前三个字节不包括
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




//static  uint8_t command_adjust_time1(void)   //网关回发时间
//{  
//	uint32_t i;
//	RTC_TimeTypeDef time;
//	RTC_DateTypeDef date;
//	current_receive_module_2_node=CmdBuf_debug[5]+(((uint16_t)CmdBuf_debug[4])<<8);
//	if(CmdBuf_debug[2]==0x02)                  //回发节点
//	{
//		RTC_GetTime(RTC_Format_BIN,&time);
//		RTC_GetDate(RTC_Format_BIN,&date);
//		CmdBuf_debug[0]=0x7e;
//		CmdBuf_debug[1]=COMMAND_SET_TIME;
//		CmdBuf_debug[2]=0x07;     //长度
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
//	else                                    //回发中继
//	{
//		RTC_GetTime(RTC_Format_BIN,&time);
//		RTC_GetDate(RTC_Format_BIN,&date);
//		CmdBuf_debug[0]=0x7e;
//		CmdBuf_debug[1]=COMMAND_SET_TIME;
//		CmdBuf_debug[2]=0x08;     //长度
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


static uint8_t  command_adjust_time2(void)    //对于中继而言，这个就是设置时间  //中继做比较，不一定每次都设置
{ 
	if(config.DataToBoardMode==BRIDGEMODE)
	{
		uint32_t year,month,day,weekday,hour,minute,second;
//		RTC_TimeTypeDef time;
//		RTC_GetTime(RTC_Format_BIN,&time);
//		if((CmdBuf_debug[10]-time.RTC_Seconds)>3||(time.RTC_Seconds-CmdBuf_debug[10])>3)
//		{
			year = ((uint16_t)CmdBuf_debug[5]<<8)+CmdBuf_debug[4];     //中继减2000
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
			RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//标记已经初始化过了
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
	current_receive_module_2_node=CmdBuf_debug[13]+(((uint16_t)CmdBuf_debug[12])<<8);  //第10跟11个字节是节点地址
	RTC_GetTime(RTC_Format_BIN,&time);
	RTC_GetDate(RTC_Format_BIN,&date);
	
	CmdBuf_debug[0]=0x7e;
	CmdBuf_debug[1]=COMMAND_SET_TIME;    //0x50
	CmdBuf_debug[2]=0x0b;     //长度
	CmdBuf_debug[3]=0x00;
	CmdBuf_debug[4]=(date.RTC_Year+2000);   //时间的设置，全部以2000为标准格式在通讯间收发
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
	if(config.DataToBoardMode==GATAWAYMODE)          //网关模式
	{
		if((CmdBuf_debug[9]>=99)&&(CmdBuf_debug[10]==Parameter.addr_node_high)&&(CmdBuf_debug[11]==Parameter.addr_node_lower))
		{
			Parameter.battery_reset_flag=0;
		}
		WriteDataToTXDBUF(CmdBuf_debug,CmdBuf_debugLength);
		Reback();
	}
	else if(config.DataToBoardMode==BRIDGEMODE)      //中继模式
	{
//		WritedatatoLAN(CmdBuf_debug,CmdBuf_debugLength);  中继模式不会收到0x45,收到也不回复
	}
	return 1;
}



uint32_t receive_echo=0;

void  module_2_receiveloop_debug()
{
	module_1_ReceivedTel_debug=0;
	module_2_ReceivedTel_debug=0;//新加
		//OpenAllInt();
 		
	switch( CmdBuf_debug[1])
	{     //看命令行
//		case COMMAND_ID:     //0x02
//			   command_id();
//			break;
//		case COMMAND_CHANNELKIND:     //0x03 通道类型设置
//			   command_channelkind();
//			break;
//		case COMMAND_REPLYCONFIG:     //0x03 通道类型设置
//			   command_replyconfig();
//			break;
//		case COMMAND_STOP:
//			   command_stop();
//			break;
//		case COMMAND_SETCONFIG:   //设置IP地址
//			   command_setconfig();
//			break;
//		case COMMAND_SET_RUNMODE:   //设置IP地址
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
//		case COMMAND_REPLY_RATE:    //设置采样参数
//			   command_reply_SampleParameter();
//		  break;
//		case COMMAND_SAMPLE_RATE:    //设置采样参数
//			   command_set_SampleParameter();
//			break;
//		case COMMAND_SET_TCPSERVE:   //设置tcp server的目标地址类似
//			   command_set_tcpserve();
//		  break;
//		case COMMAND_REPLYTCPSERVE:   //返回tcp server的目标地址类似
//			   command_replytcpserve();
//			break; 
//		case COMMAND_REQUIRE_TIME:           //0x48   当为网关时
// 			   command_adjust_time1();
//        break;
		case COMMAND_SET_TIME:               //0x50   当为中继时
				command_adjust_time2();
				receive_echo=1;
		break;		
//		case COMMAND_REPLYAP:
//			   command_replyap();  //返回ap值
//			break;
//      case COMMAND_APPLYNETSET:
//			   command_applynetset();  //应用网络设置
//			break;
//		case 0x40:
//			   command_counter();  //应用网络设置
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


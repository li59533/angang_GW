#include "app.h"
#include "ucos_ii.h"



static uint8_t 	 TxdBuf_debug[TX_BUFFER_SIZE];
static uint8_t   RxdBuf_debug[RX_BUFFER_SIZE];  //用来设置调试


static uint8_t   CmdBuf_debug[2*TX_BUFFER_SIZE];
static uint32_t  CmdBuf_debugLength=0;


static uint8_t  TxdBytes_debug=0;
static uint8_t  TxdTelBytes_debug=0;
uint8_t   module_1_ReceivedTel_debug=0;
uint8_t   RxdBytes_debug_module1=0;

volatile uint8_t  reply_flag=0;


static const uint32_t  BAUNDRATE_INT_TAB[]={1200,2400,4800,9600,19200,38400,57600,115200};


void  setComUART3(uint8_t baudrate,uint8_t parity)    //baudrate为波特率选择，parity为校验选择
{
	uint32_t  br;
	USART_InitTypeDef  USART_InitStructure;
	
	USART_ITConfig(USART3,USART_IT_RXNE,DISABLE);
	USART_ITConfig(USART3,USART_IT_TC,DISABLE);
	USART_Cmd(USART3, DISABLE);	 
	/*-------------------------USART3------------------------*/
	br=BAUNDRATE_INT_TAB[baudrate];
		
	USART_InitStructure.USART_BaudRate = br;                //串口波特率
	USART_InitStructure.USART_StopBits = USART_StopBits_1;  //停止位
	if(parity==0) {USART_InitStructure.USART_WordLength = USART_WordLength_8b;USART_InitStructure.USART_Parity = USART_Parity_No;}  //无奇偶校验
	if(parity==1) {USART_InitStructure.USART_WordLength = USART_WordLength_9b;USART_InitStructure.USART_Parity = USART_Parity_Odd;} // 奇校验
	if(parity==2) {USART_InitStructure.USART_WordLength = USART_WordLength_9b;USART_InitStructure.USART_Parity = USART_Parity_Even;}// 偶校验		
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
	
	USART_Init(USART3, &USART_InitStructure);     //初始化
	USART_ITConfig(USART3,USART_IT_TC,ENABLE);    //使能接收中断
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);  //使能发送中断
	USART_Cmd(USART3, ENABLE);                    //使能串口3
}


void  initMODULE_1(void)
{

	GPIO_InitTypeDef  GPIO_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//使能USART3时钟
	

	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); //GPIOB10复用为USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); //GPIOB11复用为USART3
	
	//USART3 引脚初始化  
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10; //GPIOB10  GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;          //服用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	  //速度
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;        //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          //上拉
	GPIO_Init(GPIOB,&GPIO_InitStructure);   //初始化


	//M0 M1   PG1   PF15   引脚初始化
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
	 

	//Usart3 NVIC 中断初始化
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
	if(RxdBytes_debug_module1>=2)if((RxdBuf_debug[1]!=0x42)&&(RxdBuf_debug[1]!=0x45)&&(RxdBuf_debug[1]!=0x48)&&(RxdBuf_debug[1]!=0x50))return(0);   //命令限制
		
	if(RxdBytes_debug_module1>=4) 
	{ 
		uint16_t length=getTelLength_debug();
		if((length>40)) return(0);  //限制最大的为40
	}
	return(1);				 // 合法的
}


static  uint8_t sumofRxdBuf_debug(uint16_t l)  //求和的报文，不包含起始标识,	l	的长度包括起始标识
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



static  uint8_t isTelComplete_debug(void)	   // =0 不完整  =1 CRC Error =2 正确
{
	uint16_t  temp16;
	uint8_t   dat_len;

	if(RxdBytes_debug_module1<4)return(0);
  
	dat_len=getTelLength_debug()+6;	//给数据长度
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


static uint8_t  leftRxdTel_debug(void)		//数组左移一位
{
	uint8_t i;
	if(RxdBytes_debug_module1<1)return(0);     // 无法左移
	for	(i=1;i<RxdBytes_debug_module1;i++)
	{
		RxdBuf_debug[i-1]=RxdBuf_debug[i];		
	}
	RxdBytes_debug_module1--;
	return(1);					 // 丢弃一个字节成功
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
					for(i=0;i<RxdBytes_debug_module1;i++)	CmdBuf_debug[i]=RxdBuf_debug[i];
					CmdBuf_debugLength=RxdBytes_debug_module1;
					module_1_ReceivedTel_debug=1;
					RxdBytes_debug_module1=0;
					OSSemPost(sem_uart3_buf);  //中断里面释放信号量
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


static  uint8_t hasByteToTxd_debug()		 // =1 有字节待发
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
	ResetBit(USART3->SR,3);  // ORE清0
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
//	setConfiguration();//进入设置状态，上位机连接的时候，首先发送停止命令让其进入到设置状态
//	E32_ModeSet(Sleep_Mode);  //让模块进入休眠模式，以便设置
//	E32PowerSelect(1); //打开模块电源
//	ADBEGIN();//开始ADC
//	return 1;
//}

//uint8_t command_start(void)
//{
//		
//  WriteDataToTXDBUF_uart2(CmdBuf_debug,CmdBuf_debugLength);
//	clrConfiguration();//清楚设置状态
//	ADOVER();//结束
//	return 1;
//}

//uint8_t command_setconfig(void)
//{
//	uint32_t i;
//	WriteDataToTXDBUF_uart2(CmdBuf_debug,CmdBuf_debugLength);
//	
//	CmdBuf_debug[9]=CmdBuf_debug[9]&0x3F;//限制数据方式为8N1;
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
//	uart0Init(config.module_1_datacheck,0,0,8,1);  //重新初始化一下串口
//	saveConfig();
//	for(i=0;i<6;i++)   //发送给无线模块
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
//	//************5个字节的模块设置参数**************//
//	CmdBuf_debug[6]=0xc2;  //掉电保存
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

//uint8_t command_set_snnumber(void)   //设置TCP_SERVER的目标上位机地址
//{
//	{
//    WriteDataToTXDBUF_uart2(CmdBuf_debug,CmdBuf_debugLength);
//	 }

//	 config.SNnumber=(uint64_t)CmdBuf_debug[4]+((uint64_t)CmdBuf_debug[5]<<8)+((uint64_t)CmdBuf_debug[6]<<16)+((uint64_t)CmdBuf_debug[7]<<24)+((uint64_t)CmdBuf_debug[8]<<32)+((uint64_t)CmdBuf_debug[9]<<40)+((uint64_t)CmdBuf_debug[10]<<48)+((uint64_t)CmdBuf_debug[11]<<56);//把AP字符串赋值给config
//	 saveConfig();
//	 return (1);
//}


uint16_t  current_receive_module_1_node=0;
//static  uint8_t command_adjust_time(void)   //网关或中继回发时间
//{  
//	uint32_t i;
//	RTC_TimeTypeDef time;
//	RTC_DateTypeDef date;
//	current_receive_module_1_node=CmdBuf_debug[5]+(((uint16_t)CmdBuf_debug[4])<<8);
//	if(CmdBuf_debug[2]==0x02)                  //回发节点
//	{
//		RTC_GetTime(RTC_Format_BIN,&time);
//		RTC_GetDate(RTC_Format_BIN,&date);
//		CmdBuf_debug[0]=0x7e;
//		CmdBuf_debug[1]=COMMAND_SET_TIME;    //0x50
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
//		WritedatatoUSART3(CmdBuf_debug,CmdBuf_debugLength);
//	}
//	else                                    //回发中继
//	{
//		RTC_GetTime(RTC_Format_BIN,&time);
//		RTC_GetDate(RTC_Format_BIN,&date);
//		CmdBuf_debug[0]=0x7e;
//		CmdBuf_debug[1]=COMMAND_SET_TIME;   //0x50
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
	WritedatatoUSART3(CmdBuf_debug,CmdBuf_debugLength);
	
	return (1);
}


extern  uint8_t WriteDataToTXDBUF(uint8_t * source,uint16_t length);
uint8_t  command_reply_temp1(void)
{
//	if(config.DataToBoardMode==GATAWAYMODE)     //网关模式
//	{
//		//current_receive_module_1_node=CmdBuf_debug[5]+(((uint16_t)CmdBuf_debug[4])<<8);//
//		Reback();
//		WritedatatoTxdbuf(CmdBuf_debug,CmdBuf_debugLength);
//		
//	}
//	else if(config.DataToBoardMode==BRIDGEMODE)  //中继模式
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
	TxdBuf_debug[0]=current_receive_module_1_node>>8;  //从哪来往哪去
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
	OSSemPend(uart3_write,0,&err);   //阻塞，防止有不同线城都要写入
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
 		
	switch( CmdBuf_debug[1])       //看命令行
	{     
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
//		case COMMAND_REQUIRE_TIME:           //0x48
// 			   command_adjust_time();
//        break;		
//		case COMMAND_REPLYAP:
//			   command_replyap();  //返回ap值
//			break;
//    case COMMAND_APPLYNETSET:
//			   command_applynetset();  //应用网络设置
//			break;
//		 case 0x40:
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

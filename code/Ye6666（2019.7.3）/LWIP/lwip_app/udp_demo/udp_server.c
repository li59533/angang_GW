#include "ye6271_common.h"
#include "udp_server.h"
#include "lwip_comm.h"
#include "includes.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/lwip_sys.h"
#include "lwip/sockets.h"
#include "string.h"
#include "malloc.h"
#include "spi.h"
#include "app.h"
#include "command.h"
#include "delay.h"





#define UDP_BLOCK_SIZE  ( MAX_UDP_TX_POINTS*AD_CHANNELS*sizeof(int16_t) )
	
uint8_t  NetReceveBuf[NetReceBufLine];

uint8_t    TXDBUF[TxdBufLine][50];
uint16_t   TXDBUFLength[TxdBufLine];

static uint8_t   RxdTelBuf[512];
static uint8_t   CmdBuf[512];
static uint8_t   RXDCmdBuf[512];
static uint32_t  RxdBytes=0;

uint32_t   NetReceBufHeadIndex=0;
uint32_t   NetReceBufTailIndex=0;

extern uint8_t  RxdBytes_debug_module1;
extern uint8_t  RxdBytes_debug_module2;

uint32_t  CmdBufLength=0;

extern uint8_t  reply_flag;
extern uint8_t  command_reply_temp1(void);

//TCP�ͻ�������
#define  TCP_CLIENT_PRIO		21
#define  TCP_CLIENT_STK_SIZE	2048
OS_STK   TCP_CLIENT_TASK_STK[TCP_CLIENT_STK_SIZE];

#define  ANALYSIS_NETBUF_PRIO		20
#define  ANALYSIS_NETBUF_STK_SIZE	300
OS_STK   ANALYSIS_NETBUF_TASK_STK[ANALYSIS_NETBUF_STK_SIZE];

#define  ANALYSIS_UART_3_BUF_PRIO		18
#define  ANALYSIS_UART_3_BUF_STK_SIZE	300
OS_STK   ANALYSIS_UART_3_BUF_TASK_STK[ANALYSIS_UART_3_BUF_STK_SIZE];

#define  ANALYSIS_UART_6_BUF_PRIO		19
#define  ANALYSIS_UART_6_BUF_STK_SIZE	300
OS_STK   ANALYSIS_UART_6_BUF_TASK_STK[ANALYSIS_UART_6_BUF_STK_SIZE];

#define  LED_TIME_BUF_PRIO		16
#define  LED_TIME_STK_SIZE	    300
OS_STK   LED_TIME_TASK_STK[LED_TIME_STK_SIZE];


#define  SEND_TXDBUF_BUF_PRIO		17
#define  SEND_TXDBUF_STK_SIZE	    500
OS_STK   SEND_TXDBUF_TASK_STK[SEND_TXDBUF_STK_SIZE];

struct netconn  *tcp_clientconn;		//TCP CLIENT�������ӽṹ��


THREAD_CONTEX  thread_contex[SLAVE_MACHINE_NUM];


u16  dest_port;


void  init_thread_data(void)
{
	int i;	
	for(i=0;i<SLAVE_MACHINE_NUM;i++)
	{
		thread_contex[i].port = UDP_SERVER_PORT;
		thread_contex[i].sem = OSSemCreate(0);
		//thread_contex[i].sem2 = OSSemCreate(0);
		thread_contex[i].current_write_bank = 0;
		thread_contex[i].bank[0] = (u8 *)mymalloc(SRAMIN,SPI_BLOCK_SIZE);
		thread_contex[i].bank[1] = (u8 *)mymalloc(SRAMIN,SPI_BLOCK_SIZE);
	}
}


//udp������
static uint16_t  getTelLength(void)  //�����TLV��V�ĳ���
{
	return(((uint16_t)RxdTelBuf[3]<<8)+RxdTelBuf[2]);//RxdTelBuf
}


static uint8_t  isVaildTel(void)
{
	if(RxdBytes>=1)if(RxdTelBuf[0]!=0x7e)return(0);//RxdTelBuf

	if(RxdBytes>=4) 
	{
		uint16_t length=getTelLength();
		if((length>1000)) return(0);  //
	}
	return(1);				 //
}


static uint8_t  sumofRxdBuf(uint16_t l)  //
{
	uint16_t i;
	uint8_t sum=0;
	if(l<2) return (0);
	for(i=1;i<l-2;i++)
		sum=sum+RxdTelBuf[i];//RxdTelBuf
	return (sum);
}


static uint8_t  isTelComplete(void)	 
{
	uint32_t  temp8;
	uint32_t   dat_len;

	if(RxdBytes<4)return(0);

	dat_len=getTelLength()+6;	
	if(dat_len==0)return(0);
	if(RxdBytes<(dat_len))return(0);

	temp8=sumofRxdBuf(dat_len);

    if (RxdTelBuf[dat_len-1]==0x7e) //RxdTelBuf
		return(2);
	if ((RxdTelBuf[dat_len-1]==0x7e)&&(RxdTelBuf[dat_len-2]==temp8))//RxdTelBuf
		return(2);
	else
	{
		return(1);
	}
}


static uint8_t  leftRxdTel(void)		
{
	uint32_t i;
	if(RxdBytes<1)return(0);     
	for	(i=1;i<RxdBytes;i++)
	{
		RxdTelBuf[i-1]=RxdTelBuf[i];//RxdTelBuf
	}
	RxdBytes--;
	return(1);					

}


uint32_t  leftRxdTel_counter=0;
volatile uint32_t  delaycounter=0;


static uint8_t  command_reply_runmode(void)   //��ѯ����ģʽ
{
	uint16_t i;
	{
		CmdBuf[0]=0x7e;
		CmdBuf[1]=COMMAND_REPLY_RUNMODE;
		CmdBuf[2]=0x01;
		CmdBuf[3]=0x00;
		CmdBuf[4]=config.DataToBoardMode; 		
		CmdBuf[5]=0;
		for(i=1;i<5;i++)
		CmdBuf[5]+=CmdBuf[i];		 
		CmdBuf[6]=0x7e;
		CmdBufLength=7;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	return 1;
}


static uint8_t  command_setrunmode(void)   //���ù���ģʽ
{
	uint8_t i;
	{
		for(i=0;i<CmdBufLength;i++)
			CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
	}
	config.DataToBoardMode=RXDCmdBuf[4];
	saveConfig();
	return 1;
}


static uint8_t  command_set_tcpserve(void)   //����TCP_SERVER��Ŀ����λ����ַ
{
	uint8_t i;
	{
		for(i=0;i<CmdBufLength;i++)
			CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	for(i=0;i<CmdBuf[5];i++)
		config.TcpServer_IP[i]=CmdBuf[6+i];//��AP�ַ�����ֵ��config
	config.TcpServer_IP[CmdBuf[5]]=0;
	for(i=0;i<CmdBuf[7+CmdBuf[5]];i++)
		config.TcpServer_Port[i]=CmdBuf[8+CmdBuf[5]+i];//��AP�ַ�����ֵ��config
	config.TcpServer_Port[CmdBuf[7+CmdBuf[5]]]=0;
	saveConfig();
	return (1);
}


static uint8_t  command_replytcpserve()  //��ȡTCP_SERVER��Ŀ����λ����ַ
{
	uint8_t i;
	uint8_t TcpServer_IPlength=strlen(config.TcpServer_IP);
	uint8_t TcpServer_Portlength=strlen(config.TcpServer_Port);
	CmdBufLength=10+TcpServer_Portlength+TcpServer_IPlength;
	CmdBuf[0]=0X7E;
	CmdBuf[1]=0X54;   //T
	CmdBuf[2]=CmdBufLength-6;
	CmdBuf[3]=(CmdBufLength-6)>>8;   //L
	CmdBuf[4]=0X00;   //V_IP
	CmdBuf[5]=TcpServer_IPlength;
	
	{
		for(i=0;i<TcpServer_IPlength;i++)
		CmdBuf[6+i]=config.TcpServer_IP[i]; 
	}
	
	CmdBuf[6+TcpServer_IPlength]=0X01;   //V_MASK
	CmdBuf[7+TcpServer_IPlength]=TcpServer_Portlength;
	
	{
		for(i=0;i<TcpServer_Portlength;i++)
		CmdBuf[8+TcpServer_IPlength+i]=config.TcpServer_Port[i];	 
	}
	
	CmdBuf[8+TcpServer_IPlength+TcpServer_Portlength]=0X00;   //У���
	 
	for(i=1;i<(8+TcpServer_IPlength+TcpServer_Portlength);i++)
		CmdBuf[8+TcpServer_IPlength+TcpServer_Portlength]+=CmdBuf[i];//��AP�ַ�����ֵ��config
	CmdBuf[9+TcpServer_IPlength+TcpServer_Portlength]=0x7e;
	CmdBufLength=10+TcpServer_IPlength+TcpServer_Portlength;
	WritedatatoLAN(CmdBuf,CmdBufLength);
	return 1;
}


uint8_t  command_setip()  //����IP��ַ  //����DHCP
{
	uint8_t i;
	{
		for(i=0;i<CmdBufLength;i++)
			CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
	}
	WritedatatoLAN(CmdBuf,CmdBufLength);
	
	for(i=0;i<CmdBuf[5];i++)
		config.LocalIP[i]=CmdBuf[6+i];//��AP�ַ�����ֵ��config
	config.LocalIP[CmdBuf[5]]=0;
	for(i=0;i<CmdBuf[7+CmdBuf[5]];i++)
		config.LocalMASK[i]=CmdBuf[8+CmdBuf[5]+i];//��AP�ַ�����ֵ��config
	config.LocalMASK[CmdBuf[7+CmdBuf[5]]]=0;
	for(i=0;i<CmdBuf[9+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]];i++)
		config.LocalGATEWAY[i]=CmdBuf[10+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]+i];//��AP�ַ�����ֵ��config
	config.LocalGATEWAY[CmdBuf[9+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]]]=0;
	 //for(uint8_t i=0;i<CmdBuf[11+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]];i++)
	config.DHCP=CmdBuf[12+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]+CmdBuf[9+CmdBuf[5]+CmdBuf[7+CmdBuf[5]]]];//��AP�ַ�����ֵ��config
	saveConfig();
	return 1;
}


static uint8_t  command_replyip()  //��ȡIP��ַ
{
	uint8_t i;
	uint8_t localIPlength=strlen(config.LocalIP);
	uint8_t LocalMASKlength=strlen(config.LocalMASK);
	uint8_t LocalGATEWAYlength=strlen(config.LocalGATEWAY);
	CmdBufLength=15+localIPlength+LocalMASKlength+LocalGATEWAYlength;
	CmdBuf[0]=0X7E;
	CmdBuf[1]=0X04;   //T
	CmdBuf[2]=CmdBufLength-6;
	CmdBuf[3]=(CmdBufLength-6)>>8;   //L
	CmdBuf[4]=0X00;   //V_IP
	CmdBuf[5]=localIPlength;
	{
		for(i=0;i<localIPlength;i++)
			CmdBuf[6+i]=config.LocalIP[i]; 
	}
	CmdBuf[6+localIPlength]=0X01;   //V_MASK
	CmdBuf[7+localIPlength]=LocalMASKlength;
	{
		for(i=0;i<LocalMASKlength;i++)
			CmdBuf[8+localIPlength+i]=config.LocalMASK[i]; 
	}
	CmdBuf[8+localIPlength+LocalMASKlength]=0X02;   //V_GATEWAY
	CmdBuf[9+localIPlength+LocalMASKlength]=LocalGATEWAYlength;
	{
		for(i=0;i<LocalGATEWAYlength;i++)
			CmdBuf[10+localIPlength+LocalMASKlength+i]=config.LocalGATEWAY[i]; 
	}
	CmdBuf[10+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0X03;
	CmdBuf[11+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0X01;
	CmdBuf[12+localIPlength+LocalMASKlength+LocalGATEWAYlength]=config.DHCP;
	CmdBuf[13+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0;
	for(i=1;i<(13+localIPlength+LocalMASKlength+LocalGATEWAYlength);i++)
		CmdBuf[13+localIPlength+LocalMASKlength+LocalGATEWAYlength]+=CmdBuf[i];//��AP�ַ�����ֵ��config
	CmdBuf[14+localIPlength+LocalMASKlength+LocalGATEWAYlength]=0x7e;
	CmdBufLength=15+localIPlength+LocalMASKlength+LocalGATEWAYlength;
	WritedatatoLAN(CmdBuf,CmdBufLength);
	return 1;
}


static uint16_t  command_id(void)
{ 
	uint16_t i;
	{
		CmdBuf[0]=0x7e;
		CmdBuf[1]=0x02;
		CmdBuf[2]=0x08;
		CmdBuf[3]=0x00;
		CmdBuf[11]=config.SNnumber;
		CmdBuf[10]=config.SNnumber>>8;
		CmdBuf[9]=config.SNnumber>>16;
		CmdBuf[8]=config.SNnumber>>24;
		CmdBuf[7]=config.SNnumber>>32;
		CmdBuf[6]=config.SNnumber>>40;
		CmdBuf[5]=config.SNnumber>>48;
		CmdBuf[4]=config.SNnumber>>56;
		CmdBuf[12]=0;
		for(i=1;i<12;i++)
		CmdBuf[12]+=CmdBuf[i];
		CmdBuf[13]=0x7e;
		CmdBufLength=14;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	return 1;
}


static uint16_t  command_version(void)
{ 
	uint16_t i;
	{
		CmdBuf[0]=0x7e;
		CmdBuf[1]=0x19;
		CmdBuf[2]=0x04;
		CmdBuf[3]=0x00;

		CmdBuf[7]=config.interface_type[3];
		CmdBuf[6]=config.interface_type[2];
		CmdBuf[5]=config.interface_type[1];
		CmdBuf[4]=config.interface_type[0];
		CmdBuf[8]=0;
		for(i=1;i<8;i++)
		CmdBuf[8]+=CmdBuf[i];
		CmdBuf[9]=0x7e;
		CmdBufLength=10;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	return 1;
}


static uint8_t  command_set_snnumber(void)   //����TCP_SERVER��Ŀ����λ����ַ
{
	uint8_t i;
	{
		for(i=0;i<CmdBufLength;i++)
		CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	config.SNnumber=(uint64_t)CmdBuf[4]+((uint64_t)CmdBuf[5]<<8)+((uint64_t)CmdBuf[6]<<16)+((uint64_t)CmdBuf[7]<<24)+((uint64_t)CmdBuf[8]<<32)+((uint64_t)CmdBuf[9]<<40)+((uint64_t)CmdBuf[10]<<48)+((uint64_t)CmdBuf[11]<<56);//��AP�ַ�����ֵ��config
	saveConfig();
	return (1);
}
 

//static uint8_t  command_applynetset(void)  //����
//{
//	__disable_fault_irq();
//	NVIC_SystemReset();
//	return 1;
//}


static uint8_t  command_set_time(void)   //����λ�������ʱ
{ 
	uint8_t i=0;
	uint32_t year,month,day,weekday,hour,minute,second;
	{
		for(i=0;i<CmdBufLength;i++)
			CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	year = ((uint16_t)CmdBuf[5]<<8)+CmdBuf[4]-2000;
	month = CmdBuf[6];
	day = CmdBuf[7];
	hour = CmdBuf[8];
	minute = CmdBuf[9];
	second = CmdBuf[10];
	weekday = CmdBuf[11];
	RTC_Set_Time(hour,minute,second,1);
	RTC_Set_Date(year,month,day,weekday);
	RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//����Ѿ���ʼ������	 
	return (1);
}


static uint8_t  command_select_module(void)  
{
	uint32_t i;
	{
		for(i=0;i<CmdBufLength;i++)
			CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	Parameter.current_module=CmdBuf[5];
	return 1;
}


static uint8_t command_receive_beacon()
{ 
	uint32_t i;
	{
		for(i=0;i<CmdBufLength;i++)
			CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	ReceiveBeaconMessage();
	return 1;
}


static uint8_t command_battery_reset()      	//�������
{ 
	uint32_t i;
	{
		for(i=0;i<CmdBufLength;i++)
			CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	Parameter.battery_reset_flag=CmdBuf[4];
	Parameter.addr_node_high=CmdBuf[5];
	Parameter.addr_node_lower=CmdBuf[6];
	return 1;
}


static uint8_t  command_delay_set()      	//�м���ʱ�趨
{ 
	uint32_t i;
	{
		for(i=0;i<CmdBufLength;i++)
			CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	config.send_delay_ms=((uint16_t)CmdBuf[4]<<8)+CmdBuf[5];

	return 1;
}


static uint8_t  command_set_config(void)
{
	uint32_t i;
	{
		for(i=0;i<CmdBufLength;i++)
			CmdBuf[i]=RXDCmdBuf[i];
		CmdBufLength=CmdBufLength;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	if(Parameter.current_module==0)
	{
		RXDCmdBuf[9]=RXDCmdBuf[9]&0x3F;//�������ݷ�ʽΪ8N1;
		//CmdBuf_debug[9]=;
		
		config.module_1_destination_addr=RXDCmdBuf[5]+((uint16_t)RXDCmdBuf[4]<<8);
		config.module_1_source_addr=RXDCmdBuf[8]+((uint16_t)RXDCmdBuf[7]<<8);
		config.module_1_airspeed=RXDCmdBuf[9]&0x7;
		config.module_1_baudrate=(RXDCmdBuf[9]>>3)&0x7;
		config.module_1_datacheck=(RXDCmdBuf[9]>>6)&0x3;
		
		config.module_1_channel=RXDCmdBuf[10];
		
		config.module_1_power=RXDCmdBuf[11]&3;
		config.module_1_FEC=(RXDCmdBuf[11]>>2)&1;
		config.module_1_wakeup_time=(RXDCmdBuf[11]>>3)&7;
		config.module_1_IO_workstyle=(RXDCmdBuf[11]>>6)&1;
		config.module_1_transmission_mode=(RXDCmdBuf[11]>>7)&1;
		OSTimeDlyHMSM(0,0,0,10);
		setComUART3(3,0);  //���³�ʼ��һ�´��ڣ�9600
		OSTimeDlyHMSM(0,0,0,100);
		saveConfig();
		for(i=0;i<6;i++)   //���͸�����ģ��
			CmdBuf[i]=RXDCmdBuf[i+6];
		CmdBufLength=6;
		WriteConfigdatatoUSART3(CmdBuf,CmdBufLength);
		OSTimeDlyHMSM(0,0,0,100);
		setComUART3(config.module_1_baudrate,0);  //���³�ʼ��һ�´���	
	}
	else
	{
		RXDCmdBuf[9]=RXDCmdBuf[9]&0x3F;//�������ݷ�ʽΪ8N1;
		//CmdBuf_debug[9]=;
	
		config.module_2_destination_addr=RXDCmdBuf[5]+((uint16_t)RXDCmdBuf[4]<<8);
		config.module_2_source_addr=RXDCmdBuf[8]+((uint16_t)RXDCmdBuf[7]<<8);
		config.module_2_airspeed=RXDCmdBuf[9]&0x7;
		config.module_2_baudrate=(RXDCmdBuf[9]>>3)&0x7;
		config.module_2_datacheck=(RXDCmdBuf[9]>>6)&0x3;
	
		config.module_2_channel=RXDCmdBuf[10];
	
		config.module_2_power=RXDCmdBuf[11]&3;
		config.module_2_FEC=(RXDCmdBuf[11]>>2)&1;
		config.module_2_wakeup_time=(RXDCmdBuf[11]>>3)&7;
		config.module_2_IO_workstyle=(RXDCmdBuf[11]>>6)&1;
		config.module_2_transmission_mode=(RXDCmdBuf[11]>>7)&1;
		OSTimeDlyHMSM(0,0,0,10);
		setComUART6(3,0);  //���³�ʼ��һ�´���
		OSTimeDlyHMSM(0,0,0,100);
		saveConfig();
		for(i=0;i<6;i++)   //���͸�����ģ��
			CmdBuf[i]=RXDCmdBuf[i+6];
		CmdBufLength=6;
		WriteConfigdatatoUSART6(CmdBuf,CmdBufLength);	
		OSTimeDlyHMSM(0,0,0,100);
		setComUART6(config.module_2_baudrate,0);  //���³�ʼ��һ�´���	
	}
	return 1;	
}


uint8_t  command_reply_config(void)
{
	uint32_t i=0;
	if(Parameter.current_module==0)
	{
		CmdBuf[0]=0x7e;
		CmdBuf[1]=COMMAND_REPLYCONFIG;
		CmdBuf[2]=0x08;
		CmdBuf[3]=0x00;
		CmdBuf[4]=config.module_1_destination_addr>>8;
		CmdBuf[5]=config.module_1_destination_addr;
		//************5���ֽڵ�ģ�����ò���**************//
		CmdBuf[6]=0xc0;  //���籣��
		CmdBuf[7]=config.module_1_source_addr>>8;
		CmdBuf[8]=config.module_1_source_addr;
		CmdBuf[9]=config.module_1_airspeed|(config.module_1_baudrate<<3)|(config.module_1_datacheck<<6);
		CmdBuf[10]=config.module_1_channel;
		CmdBuf[11]=config.module_1_power|(config.module_1_FEC<<2)|(config.module_1_wakeup_time<<3)|(config.module_1_IO_workstyle<<6)|(config.module_1_transmission_mode<<7);
		CmdBuf[12]=0;
		for(i=1;i<12;i++)
			CmdBuf[12]+=CmdBuf[i];
		CmdBuf[13]=0x7e;
		CmdBufLength=14;
		WritedatatoLAN(CmdBuf,CmdBufLength);
	}
	else
	{
		CmdBuf[0]=0x7e;
		CmdBuf[1]=COMMAND_REPLYCONFIG;
		CmdBuf[2]=0x08;
		CmdBuf[3]=0x00;
		CmdBuf[4]=config.module_2_destination_addr>>8;
		CmdBuf[5]=config.module_2_destination_addr;
		//************5���ֽڵ�ģ�����ò���**************//
		CmdBuf[6]=0xc0;  //���籣��
		CmdBuf[7]=config.module_2_source_addr>>8;
		CmdBuf[8]=config.module_2_source_addr;
		CmdBuf[9]=config.module_2_airspeed|(config.module_2_baudrate<<3)|(config.module_2_datacheck<<6);
		CmdBuf[10]=config.module_2_channel;
		CmdBuf[11]=config.module_2_power|(config.module_2_FEC<<2)|(config.module_2_wakeup_time<<3)|(config.module_2_IO_workstyle<<6)|(config.module_2_transmission_mode<<7);
		CmdBuf[12]=0;
		for(i=1;i<12;i++)
			CmdBuf[12]+=CmdBuf[i];
		CmdBuf[13]=0x7e;
		CmdBufLength=14;
		WritedatatoLAN(CmdBuf,CmdBufLength);	
	}
	return 1;	
}


void  set_module_2_config(void)
{
	setComUART6(3,0);  //����ģ�������ֻ����9600�Ĳ�������
	OSTimeDlyHMSM(0,0,0,1);
	CmdBuf[0]=0xc0;  //���籣��
	CmdBuf[1]=config.module_2_source_addr>>8;
	CmdBuf[2]=config.module_2_source_addr;
	CmdBuf[3]=config.module_2_airspeed|(config.module_2_baudrate<<3)|(config.module_2_datacheck<<6);
	CmdBuf[4]=config.module_2_channel;
	CmdBuf[5]=config.module_2_power|(config.module_2_FEC<<2)|(config.module_2_wakeup_time<<3)|(config.module_2_IO_workstyle<<6)|(config.module_2_transmission_mode<<7);
	CmdBufLength=6;
	WriteConfigdatatoUSART6(CmdBuf,CmdBufLength);
	OSTimeDlyHMSM(0,0,0,100);
	setComUART6(config.module_2_baudrate,0);  //����ģ�������ֻ����9600�Ĳ�������
	OSTimeDlyHMSM(0,0,0,100);
}


void  Reset_module_2_config(void)
{
	CmdBuf[0]=0xc3; //���籣��
	CmdBuf[1]=0xc3;
	CmdBuf[2]=0xc3;
	CmdBufLength=3;
	WriteConfigdatatoUSART6(CmdBuf,CmdBufLength);
}


void  Reset_module_1_config(void)
{
	CmdBuf[0]=0xc3;  //���籣��
	CmdBuf[1]=0xc3;
	CmdBuf[2]=0xc3;
	CmdBufLength=3;
	WriteConfigdatatoUSART3(CmdBuf,CmdBufLength);
}


void  Ask_module_1_config(void)
{
	CmdBuf[0]=0xc1;  //���籣��
	CmdBuf[1]=0xc1;
	CmdBuf[2]=0xc1;
	CmdBufLength=3;
	WriteConfigdatatoUSART3(CmdBuf,CmdBufLength);
}


void  set_module_1_config(void)
{
	setComUART3(3,0);  //����ģ�������ֻ����9600�Ĳ�������
	OSTimeDlyHMSM(0,0,0,1);
	CmdBuf[0]=0xc0;  //���籣��
	CmdBuf[1]=config.module_1_source_addr>>8;
	CmdBuf[2]=config.module_1_source_addr;
	CmdBuf[3]=config.module_1_airspeed|(config.module_1_baudrate<<3)|(config.module_1_datacheck<<6);
	CmdBuf[4]=config.module_1_channel;
	CmdBuf[5]=config.module_1_power|(config.module_1_FEC<<2)|(config.module_1_wakeup_time<<3)|(config.module_1_IO_workstyle<<6)|(config.module_1_transmission_mode<<7);
	CmdBufLength=6;
	WriteConfigdatatoUSART3(CmdBuf,CmdBufLength);
	OSTimeDlyHMSM(0,0,0,100);
	setComUART3(config.module_1_baudrate,0);  //����ģ�������ֻ����9600�Ĳ�������
	OSTimeDlyHMSM(0,0,0,100);
}


static uint8_t  AnalyCmd(uint16_t length)
{
  	uint8_t i;	
	switch( RXDCmdBuf[1] )
	{     
		case COMMAND_STOP:             //ֹͣ
			{
				for(i=0;i<CmdBufLength;i++)
					CmdBuf[i]=RXDCmdBuf[i];
				CmdBufLength=CmdBufLength;
				WritedatatoLAN(CmdBuf,CmdBufLength);
			}
			MODULE1_ModeSet(Sleep_Mode);
			MODULE2_ModeSet(Sleep_Mode);
			break;
			
		case COMMAND_START:            //����  
			{
				for(i=0;i<CmdBufLength;i++)
					CmdBuf[i]=RXDCmdBuf[i];
				CmdBufLength=CmdBufLength;
				WritedatatoLAN(CmdBuf,CmdBufLength);
			}
			MODULE1_ModeSet(Normal_Mode);
			MODULE2_ModeSet(Normal_Mode);
			break;
			
		case COMMAND_SET_RUNMODE:     //���ù���ģʽ
			command_setrunmode();
			break;
		
		case COMMAND_REPLY_RUNMODE:   //��ȡ����ģʽ
			command_reply_runmode();
			break;
		
		case COMMAND_SET_TCPSERVE:    //����tcp server��Ŀ���ַ
			command_set_tcpserve();
			break;
		
		case COMMAND_REPLYTCPSERVE:   //��ȡtcp server��Ŀ���ַ
			command_replytcpserve();
			break; 
		
		case COMMAND_SETIP:           //����IP��ַ
			command_setip();
			break;
		
		case COMMAND_REPLYIP:         //��ȡIP��ַ
			command_replyip();
			break;
		
		case COMMAND_SET_SNNUMBER:    //����SN��
			command_set_snnumber();
			break;
				
		case COMMAND_ID:              //��ȡSN��
			command_id();
		    command_version();
			break;		
		
		case COMMAND_SET_TIME:        //�����ʱ
			command_set_time();
			break;
		
		case COMMAND_SETCONFIG:       //����ģ�����
			command_set_config();
			break;
	  
		case COMMAND_REPLYCONFIG:     //��ȡģ�����
			command_reply_config();
			break;
		
//		case COMMAND_APPLYNETSET:     //����
//			command_applynetset();  
//			break;
		
		case COMMAND_SELECT_MODULE:   //ѡ��ģ��
			command_select_module();
			break;
				
		case COMMAND_RECEIVE_BEACON:  //����
			command_receive_beacon();
			break;

		case COMMAND_BATTERY_RESET:   //�������
			command_battery_reset();
			break;
		
		case COMMAND_DELAY_SET:       //�м���ʱ�趨
			command_delay_set();
			break;
		default:;break;		  
	}
	return 1;
}


static void  RxdByte(uint8_t c)
{
	uint32_t 	i,j;
	RxdTelBuf[RxdBytes]=c;//RxdTelBuf
	RxdBytes++;

	switch(RxdBytes)
	{
		case 0:	break;
		case 3:	break;
		case 1:
		case 2:
		case 4:while(!isVaildTel())	//
				{
					if(!leftRxdTel())break;	  //
				}
				break;
		default:
				i=isTelComplete();
				if(i==2)
				{
					for(j=0;j<RxdBytes;j++)
						RXDCmdBuf[j]=RxdTelBuf[j];
					CmdBufLength=RxdBytes;
					AnalyCmd(CmdBufLength);				
					RxdBytes=0;
				}
				else if(i==1)	 // CRC error
				{
					leftRxdTel_counter++;
					//if()
					leftRxdTel();
					while(!isVaildTel())	//
					{
						leftRxdTel_counter++;
						if(!leftRxdTel())break;
					}
				}
				else if(i==0) //
				{
				}
				else
				{
				}
				break;
	}
}


static uint8_t  ReadRxdBuf(void)
{
	uint8_t  c;
	c=NetReceveBuf[NetReceBufTailIndex];
	IncreaseNetBuf(NetReceBufTailIndex);
	return (c);
}


void  WritedatatoLAN(uint8_t *data,uint32_t length)
{
	u8 err;
	LED0_SET();
//	OSSemPend(lan_write,0,&err);   //��������ֹ�в�ͬ�߳Ƕ�Ҫд��
	LED1_SET();
	if(isInConnectNet())
	{
		err=netconn_write(tcp_clientconn,data,length,NETCONN_COPY);
		if(err!=ERR_OK)
			LostConnectNet();
	}
//	OSSemPost(lan_write);
}


void  transimit_data(uint8_t source,uint8_t dest,uint8_t *data,uint32_t Length)
{
	switch(dest)
	{
		case U3:
			WritedatatoUSART3(data,Length);
		break;
		case U6:
			WritedatatoUSART6(data,Length);
		break;
		case LAN:
			WritedatatoLAN(data,Length);
		break;
		default:			
		break;
	}
}


void  analysis_netbuf(void *arg)
{
	for(;;)
	{
		if(!isNetReceBufEmpty()) 
		{
			RxdByte(ReadRxdBuf());
		} 
		else
		{
		//	OSTimeDly(1);
//			delay_ms(1);
			OSTimeDlyHMSM(0,0,0,1);
		}	
	}		
}


static void  tcp_client_thread(void *arg)
{
	OS_CPU_SR cpu_sr;

	uint32_t i=0;
	uint8_t *qq;
	struct pbuf *q;
	err_t err,recv_err;
	static ip_addr_t server_ipaddr,loca_ipaddr,server_port;
	static u16_t  loca_port;
	
	LWIP_UNUSED_ARG(arg);
	while(lwip_comm_init()) 	//lwip��ʼ��
	{
		OSTimeDly(1000);
	}
	
	if(config.DHCP==1)
	{
		lwip_comm_dhcp_creat();
	}	
	
	inet_aton(config.TcpServer_IP,&server_ipaddr);
	server_port.addr=atoi(config.TcpServer_Port);
	while (1) 
	{
		tcp_clientconn = netconn_new(NETCONN_TCP);  //����һ��TCP����
		err = netconn_connect(tcp_clientconn,&server_ipaddr,server_port.addr);//���ӷ�����
		if(err != ERR_OK) 
		{
			netconn_delete(tcp_clientconn); //����ֵ������ERR_OK,ɾ��tcp_clientconn����
			OSTimeDly(2);	
		}
		else if (err == ERR_OK)    //���������ӵ�����
		{ 
			struct netbuf *recvbuf;
			BuildConnectNet();
			tcp_clientconn->recv_timeout = 10;
			netconn_getaddr(tcp_clientconn,&loca_ipaddr,&loca_port,1); //��ȡ����IP����IP��ַ�Ͷ˿ں�
			//printf("�����Ϸ�����%d.%d.%d.%d,�����˿ں�Ϊ:%d\r\n",lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3],loca_port);
			while(1)
			{				
				if((recv_err = netconn_recv(tcp_clientconn,&recvbuf)) == ERR_OK)  //���յ�����
				{	
					OS_ENTER_CRITICAL(); //���ж�
				//	memset(tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //���ݽ��ջ���������
					for(q=recvbuf->p;q!=NULL;q=q->next)  //����������pbuf����
					{
						//�ж�Ҫ������TCP_CLIENT_RX_BUFSIZE�е������Ƿ����TCP_CLIENT_RX_BUFSIZE��ʣ��ռ䣬�������
						//�Ļ���ֻ����TCP_CLIENT_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
//						if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//��������
//						else memcpy(tcp_client_recvbuf+data_len,q->payload,q->len);
//						data_len += q->len;  	
//						if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
						qq=q->payload;
						for(i=0;i<q->len;i++)    //���������Ĳ�����ô�氡
						{
							NetReceveBuf[NetReceBufHeadIndex]=*(qq+i);
							IncreaseNetBuf(NetReceBufHeadIndex);
						}							
					}
					OS_EXIT_CRITICAL();  //���ж�					
					netbuf_delete(recvbuf);
//					temp = analy_cmd(tcp_client_recvbuf,data_len);
//					data_len=0;  //������ɺ�data_lenҪ���㡣					
//					if(temp==0){
//						err =netconn_write(tcp_clientconn ,tcp_client_recvbuf,data_len,NETCONN_COPY); //����tcp_server_sentbuf�е�����
//					}
				}
				else if(recv_err==ERR_CLSD)
				{
					LostConnectNet(); //�رշ���
					printf("Close server,error=%d\r\n",recv_err);
					netconn_close(tcp_clientconn);
					netconn_delete(tcp_clientconn);
					break;				 
				}
			}
		}
	}
}


//����UDP�߳�
//����ֵ:0 UDP�����ɹ�
//���� UDP����ʧ��

RTC_TimeTypeDef  time;
RTC_DateTypeDef  date;

OS_EVENT  *sem_uart3_buf;
OS_EVENT  *sem_uart6_buf;

uint8_t  module_ready=0;


void  analysis_uart_3_buf(void *arg)
{
	u8 err;
	
	MODULE1_ModeSet(Sleep_Mode);  //�ϵ��������
	OSTimeDly(1500);
	set_module_1_config();
	OSTimeDly(500);	
	RxdBytes_debug_module1=0;
	MODULE1_ModeSet(Normal_Mode);
	OSTimeDly(1000);
	
	module_ready=1;
	sem_uart3_buf = OSSemCreate(0);
	
	while(1)
	{
		OSSemPend(sem_uart3_buf,0,&err);
		if(module_1_ReceivedTel_debug)
			module_1_receiveloop_debug();
		LED1_SET();
//		LED0_CLR();
//    	OSTimeDly(500);
	}
}


extern void     module_2_receiveloop_debug(void);
extern uint8_t  module_2_ReceivedTel_debug;
uint32_t  HeadIndex=0;       //д���ݽ�����ָ��
uint32_t  TailIndex=0;       //���淢����ָ��


uint8_t  WriteDataToTXDBUF(uint8_t * source,uint16_t length)
{ 	
	uint16_t i,j;
	u8 err;

	OSSemPend(txdbuff_write,0,&err);
//	__disable_irq();	   //��Ӧ�����������жϣ�������
	if(isTxdBufFull())     //�������˾Ͳ�д
	{
//		__enable_irq();
		OSSemPost(txdbuff_write);
		return 0;
	}
	else                   //���в�����д    
	{ 
		uint16_t SN_addr;
		SN_addr=(uint16_t)(source[10]<<8)+source[11];//��Ҫ����ǿ��ת��������λ
		if((SN_addr==0)||(SN_addr>0xFFFF))    //�����ϴ�Ϊ1-60�������˳�
		{
			OSSemPost(txdbuff_write);//����Ҫ�ӣ���Ȼ�ź�������Ӧ
			return 0;
		}
		else
		{
			for(i=0;i<TxdBufLine;i++)
			{						
				if(   TXDBUF[i][0]==source[0]&&TXDBUF[i][1]==source[1]&&TXDBUF[i][2]==source[2]
					&&TXDBUF[i][3]==source[3]&&TXDBUF[i][4]==source[4]&&TXDBUF[i][5]==source[5]
					&&TXDBUF[i][6]==source[6]&&TXDBUF[i][7]==source[7]&&TXDBUF[i][8]==source[8]
					&&TXDBUF[i][9]==source[9]&&TXDBUF[i][10]==source[10]&&TXDBUF[i][11]==source[11]
					&&TXDBUF[i][12]==source[12]&&TXDBUF[i][13]==source[13]&&TXDBUF[i][14]==source[14]
					&&TXDBUF[i][15]==source[15]&&TXDBUF[i][16]==source[16]&&TXDBUF[i][17]==source[17]
					&&TXDBUF[i][18]==source[18]&&TXDBUF[i][19]==source[19]&&TXDBUF[i][20]==source[20]
					&&TXDBUF[i][21]==source[21]&&TXDBUF[i][22]==source[22])
				{
					OSSemPost(txdbuff_write);//����Ҫ�ӣ���Ȼ�ź�������Ӧ
					return 0;
				}	
			}
		
			for(j=0;j<length;j++)
			TXDBUF[HeadIndex][j]=source[j];		
			TXDBUFLength[HeadIndex]=length;	
			Increase(HeadIndex);
	//		__enable_irq();
			OSSemPost(txdbuff_write);
//			SN_addr=0;
			return 1;
		}
	}
}


//void  analysis_uart_6_buf(void *arg)  //Ϊʲô����ϵ��ˣ�����ģ��Ҫͬʱ���ⲿ����
//{
//	u8 err;
//		sem_uart6_buf = OSSemCreate(0);
//	if(module_ready)
//	{	
//		while(1)
//		{
//			OSSemPend(sem_uart6_buf,0,&err);
//			if(module_2_ReceivedTel_debug)
//				module_2_receiveloop_debug();
//			LED1_SET();
//		}
//	}else
//	{
//			OSTimeDlyHMSM(0,0,0,500);
//	}
//}


void  analysis_uart_6_buf(void *arg)
{
	u8 err;

	MODULE2_ModeSet(Sleep_Mode);  //�ϵ��������
	OSTimeDly(1500);
	set_module_2_config();
	OSTimeDly(500);
	RxdBytes_debug_module2=0;
	MODULE2_ModeSet(Normal_Mode);  
	OSTimeDly(1000);
	
	module_ready=1;
	sem_uart6_buf = OSSemCreate(0);
	
	while(1)
	{
		OSSemPend(sem_uart6_buf,0,&err);
		if(module_2_ReceivedTel_debug)
			module_2_receiveloop_debug();
		LED1_SET();
//		LED0_CLR();
//    	OSTimeDly(500);
	}
}


extern void  bridge_command_require_time(void);


uint32_t  lasthour;
uint32_t  lastday;



static uint8_t  currentinputIO=1;
static uint8_t  lastinputIO;
static uint8_t  InputIOstatue;
uint16_t  InputIOtimer=0;
uint8_t   Resetflag=0;
uint8_t   time_counter_beacon=0;

void  update_reset_statue(void)
{
//	uint16_t i;
	lastinputIO=currentinputIO;
	currentinputIO=GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_6);
	InputIOstatue=currentinputIO^lastinputIO;
	if(InputIOstatue) 
	{
		if(currentinputIO==0)
		InputIOtimer=3000;//3�����ͷ���
		else 
		{
			Resetflag=0;
			InputIOtimer=0;
		}	
	}
	if(Resetflag)
	{
		strcpy((char *)config.TcpServer_IP,(char *)macUser_Esp32_TcpServer_IP);
		strcpy((char *)config.TcpServer_Port,(char *)macUser_Esp32_TcpServer_Port);
		strcpy((char *)config.LocalIP,(char *)macUser_Esp32_LocalID);
		strcpy((char *)config.LocalGATEWAY,(char *)macUser_Esp32_LocalGATAWAY);
		strcpy((char *)config.LocalMASK,(char *)macUser_Esp32_LocalMASK);
		config.DHCP=0;
		saveConfig();
		__disable_fault_irq();
		NVIC_SystemReset();
	}
}


void  Led_time_task(void *arg)
{
	for(;;)
	{
		time_counter_beacon++;
		RTC_GetTime(RTC_Format_BIN,&time);
		RTC_GetDate(RTC_Format_BIN,&date);

//		if(lastday!=date.RTC_Date)  //һ������һ��
////		if(lasthour!=time.RTC_Hours)  //һСʱ����һ��
//		{
//			__disable_fault_irq();
//			NVIC_SystemReset();
//		}
		if(config.DataToBoardMode==GATAWAYMODE)//���ز���Ҫ�������
		{
			if(time_counter_beacon>180)  //180����һ��������
			{
				time_counter_beacon=0;
				if(isReceiveBeaconMessage())
				{
					DeleteBeaconMessage(); 
				}
				else
				{
					__disable_fault_irq();
					NVIC_SystemReset();
				}
			}
		}
		LED1_CLR();
		OSTimeDlyHMSM(0,0,0,500);//OSTimeDly(500);
		LED0_CLR();
		OSTimeDlyHMSM(0,0,0,500);//1ms��ദ��һ������
		update_reset_statue();
	}
}


volatile  uint32_t retry_send_buf_time=0;//�ط���������
extern    uint32_t receive_echo;


void  Send_Delay(uint16_t  delay_ms)
{
	OSTimeDlyHMSM(0,0,0,delay_ms);
}


void  Send_TXDBUF_task(void *arg)
{
	for(;;)
	{
		if(reply_flag==1)
		{	
			if(config.DataToBoardMode==BRIDGEMODE)
			{
				Send_Delay(config.send_delay_ms);    //��Χ��1-1000ms
			}
			command_reply_temp1();
			reply_flag=0;
		}
		if(!isTxdBufEmpty()) //����б���Ҫ����
		{
			if(config.DataToBoardMode==GATAWAYMODE) 
			{
				if(isInConnectNet())  //���Ҫ���ǣ������Ƿ�׼������
				{
					WritedatatoLAN(TXDBUF[TailIndex],TXDBUFLength[TailIndex]);
					Increase(TailIndex);
				}
				OSTimeDlyHMSM(0,0,0,10);	//��������أ��������˷��Ͷ���Ϊ10msһ����10��������෢��һ�Σ����Ҳ���Ҫack����
					
			}
			else if(config.DataToBoardMode==BRIDGEMODE)
			{
				if(module_ready==1) //��߿����м��Ƿ�׼���������������ʹ��ڻ�������
				{
					WritedatatoUSART6(TXDBUF[TailIndex],TXDBUFLength[TailIndex]);
					OSTimeDlyHMSM(0,0,1,0);//������м̣��������˷��Ͷ���Ϊ1000msһ����1000��������෢��һ�Σ�������Ҫack���ػ��ƣ����β�����������
					if((retry_send_buf_time>3)||(receive_echo))  //����յ�ack���ߣ��ط������ﵽ3�Σ��Ͳ�����
					{
						Increase(TailIndex);
						retry_send_buf_time=0;
						receive_echo=0;  //�ظ���־��0
					}
					retry_send_buf_time++;
				}
			}
		}
		else
		OSTimeDlyHMSM(0,0,0,1);	//1ms��ദ��һ������
	}	
}


void  start_task(void *pdata)
{
	u8 res;
	OS_CPU_SR cpu_sr;

	OSStatInit();  			//��ʼ��ͳ������
	OS_ENTER_CRITICAL();  	//���ж�
	
	
	res = OSTaskCreate(tcp_client_thread,(void*)0,(OS_STK*)&TCP_CLIENT_TASK_STK[TCP_CLIENT_STK_SIZE-1],TCP_CLIENT_PRIO);                //�������
	res = OSTaskCreate(analysis_netbuf,(void*)0,(OS_STK*)&ANALYSIS_NETBUF_TASK_STK[ANALYSIS_NETBUF_STK_SIZE-1],ANALYSIS_NETBUF_PRIO);   //������������
	res = OSTaskCreate(analysis_uart_3_buf,(void*)0,(OS_STK*)&ANALYSIS_UART_3_BUF_TASK_STK[ANALYSIS_UART_3_BUF_STK_SIZE-1],ANALYSIS_UART_3_BUF_PRIO);
	res = OSTaskCreate(analysis_uart_6_buf,(void*)0,(OS_STK*)&ANALYSIS_UART_6_BUF_TASK_STK[ANALYSIS_UART_6_BUF_STK_SIZE-1],ANALYSIS_UART_6_BUF_PRIO);
	res = OSTaskCreate(Led_time_task,(void*)0,(OS_STK*)&LED_TIME_TASK_STK[LED_TIME_STK_SIZE-1],LED_TIME_BUF_PRIO);
	res = OSTaskCreate(Send_TXDBUF_task,(void*)0,(OS_STK*)&SEND_TXDBUF_TASK_STK[SEND_TXDBUF_STK_SIZE-1],SEND_TXDBUF_BUF_PRIO);
	OSTaskSuspend(OS_PRIO_SELF);//����start_task����
	OS_EXIT_CRITICAL();  //���ж�	
}







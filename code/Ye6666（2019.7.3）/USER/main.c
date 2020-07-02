#include "ye6271_common.h"
#include "sys.h"
#include "delay.h"
#include "lwip_comm.h"
#include "LAN8720.h"
#include "usmart.h"
#include "timer.h"
#include "sram.h"
#include "malloc.h"
#include "lwip_comm.h"
#include "includes.h"
#include "lwipopts.h"
#include "udp_server.h"
#include "rs485.h"
#include "ad7606.h"
#include "spi.h"
#include "gpio.h"
#include "can.h"
#include "paramaters.h"
#include "app.h"

#define SPI_SPEEDP_MASTER  SPI_BaudRatePrescaler_2

extern  RTC_TimeTypeDef  time;
extern  RTC_DateTypeDef  date;
extern	uint32_t  lasthour;
extern	uint32_t  lastday;

#define START_TASK_PRIO		27
#define START_STK_SIZE		128
OS_STK START_TASK_STK[START_STK_SIZE];
void start_task(void *pdata); 

#define T1_TASK_PRIO		8
#define T1_STK_SIZE		    64
OS_STK	T1_TASK_STK[T1_STK_SIZE];
void T1_task(void *pdata);  

#define READ_SPI0_TASK_PRIO 		6
#define READ_SPI0_STK_SIZE		    128	
OS_STK READ_SPI0_TASK_STK[READ_SPI0_STK_SIZE];
void READ_SPI0_task(void *pdata);   


OS_EVENT *sem_taskT1;
struct PARAMETER Parameter;
struct CONFIG  config={0xAA55, //	uint16_t vaildsign;
	3,//uint8_t baundrate;   /* =0:4800    =1:9600 		=2:38400 		=3:115200 		 */ 
	1,//uint8_t addr; 
	0,//У���ʽ 0�����飬1��У��2żУ��
	8,//8λ����
	1,//1��ֹͣλ
	0x0603 ,
	0, //uint8_t parity;		// =0 : n,8,1   =1: o,8,1  =2: e,8,1	 ���ݸ�ʽ
	
	0, //uint8_t DisplayMode;  // ��ʾģʽ��=0���̶���=1 ѭ��
	{20,19,7,3,1,1,1,1,1,1,1,1}, //uint8_t interface_type[12]; // ��������
	{1,1,1,1,1,1,1,1,1,1,1,1},//uint8_t unit[12];  // ��λ
	{980,980,980,980,1000,1000,1000,1000,10000,10000,10000,10000},//uint32_t scale[12]; // ת��ϵ��
	{0,0,0,0,0,0,0,0,8192,8192,8192,8192},//uint32_t adjust[12]; // ����ϵ��
	{0,1,2,~0,~0,~0,~0,~0,3,4,5,6},//uint16_t interface_addr[12]; // modbus ��ַ �ϴ�
	{0,0,0,0,0,0,0,0,0,0,0,0},//	uint8_t abs[12]; // ����ֵ 1��
	0,//uint8_t means	;// ��ֵ����
	1,//uint16_t means_times; // ��ֵ��������
	20000,//uint16_t freq;  // ����Ƶ�� Hz
	4096,//uint16_t avr_count;
	0,0,0,0, //uint8_t yellow_ch,blue_ch,green_ch,red_ch; // ��ʾ����ͨ��
	2, // uint8_t reflash; // ˢ��ʱ�� 
	~0, //	uint16_t din_addr;  //  ����������Ĵ�����ַ
	~0, // uint16_t dout_addr; //  ����������Ĵ�����ַ
	20,20,0,0,//	uint32_t gate0,gate1,gate2,gate3;  // ������ֵ
	100, 100,0,0,//uint32_t backlash0,backlash1,backlash2,backlash3; // �����ز�
	500, 50, //uint32_t vibrate_gate,vibrate_backlash;  // ��������ֵ���ز�
	300, 30, // uint32_t force_gate,force_backlash,   // Ӧ��������ֵ�� �ز
	~0,~0, //	uint16_t max_addr0,max_addr1; ���ֵ��ַ
	300,
	300,
	300,
	0x4a,               //PGA
	2,                  //��������
	0x0081,             //�Ŵ���
	0x11,               //���书������
	0x01,               //AD����Ƶ��
	20,                 //AD����ʱ�䣬����������һ������
	0x59B6A4C3,         //8��00�֣�aabb�ĸ�ʽ��aa����ʱ�䣬bb������ӣ���Ĭ��Ϊ0
	0,                  //flash��ʼ��ַ
	0, //DHCP
	GATAWAYMODE,
	0,
	"wifi-sensor",    //"TP-LINK-SCZZB",//"yec-test",//"wifi-sensor",//"TP-LINK-sczzb2",//"hold-704",//"wifi-test1",//"yec-test",//"wifi-test",//"yec-test",//"zl_sensor",/////"yec-test",//"test3",//"qiangang2", //"qiangang1", //"qiangang1", /////
	"wifi-sensor",    //"china-yec",//"",//"wifi-sensor",//"18051061462",//"wifi-test",//"zl_sensor",///"china-yec",//"",////"",//"zl_sensor",/"lft13852578307",//"",//"",//"123456789",//"china-yec.com",// //
	"192.168.99.6",   //"192.168.99.3", //�������˵�IP��ַ  "192.168.0.18", //M
	"8712",           //�˿ں�
	"192.168.99.45",  //LocalIP
	"192.168.99.1",   //LocalGATEWAY
	"255.255.255.0",  //LocalMASK
	
	1,
	
	/*ģ��һ�Ĳ���*/
	0x8001,  //ģ��1�������ַ
	0x9001,		//ģ��1Ŀ���ַ
	0,       //У�鷽ʽ��8N1
	0x03,			//7������9600
	0x02,			//�������� 2.4k
	0x01,			//�ŵ� 1
	1,				//0���㴫��
	1,        //��������
	0,				//����ʱ�� ��������
	0,				//��ǰ�����
	0,				//0��20DB
	
	/*ģ����Ĳ���*/
	0x8002,  //ģ��1�������ַ
	0x9001,		//ģ��1Ŀ���ַ
	0,       //У�鷽ʽ��8N1
	0x03,			//7������9600
	0x02,			//�������� 2.4k
	0x01,			//�ŵ� 1
	1,				//0���㴫��
	1,        //��������
	0,				//����ʱ�� ��������
	0,				//��ǰ�����
	0,				//0��20DB
	
};


void  delay_ms_mcu(uint32_t time_ms)
{
	uint32_t ii=0,jj=0;
	for(ii=0;ii<time_ms;ii++)
	{
		for(jj=0;jj<20000;jj++)
			;
	}
}


OS_EVENT  *uart3_write;
OS_EVENT  *uart6_write;
OS_EVENT  *lan_write;
OS_EVENT  *txdbuff_write;

int  main(void)
{	
	delay_init(168);       	//��ʱ��ʼ��
//	delay_ms(1000);         //����ܹؼ������Ե�ʱ�򣬲�����������ԭ���Լ���
	My_RTC_Init();
	initEEPROM();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�жϷ�������
	
	//FSMC_SRAM_Init(); 		//SRAM��ʼ��
 	mymem_init(SRAMIN);  	//��ʼ���ڲ��ڴ��
	//mymem_init(SRAMEX);  	//��ʼ���ⲿ�ڴ��
	mymem_init(SRAMCCM); 	//��ʼ��CCM�ڴ��
	gpio_init();
	LED0_CLR();
	LED1_CLR();
	initMODULE_1();
	initMODULE_2();
	Parameter.current_module=0;
	Parameter.battery_reset_flag=0;
	Parameter.addr_node_high=0;
	Parameter.addr_node_lower=0;

	RTC_GetTime(RTC_Format_BIN,&time);
	RTC_GetDate(RTC_Format_BIN,&date);
	lasthour=time.RTC_Hours;
	lastday=date.RTC_Date;
	
    DisableModulePower();
	delay_ms_mcu(1000);
	EnableModulePower();
	LostConnectNet();//��ʼ����ʱ����Ĭ��û�����ӵ�

	//CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_7tq,6,CAN_Mode_Normal);	//CAN��ͨģʽ��ʼ��,��ͨģʽ,������500Kbps	
	OSInit(); 					//UCOS��ʼ��
//  delay_ms(100);	
//	if(read_system_parameters(&sys_para)){
//		set_default_system_parameters(&sys_para);
//	}

//	init_thread_data();
	uart3_write = OSSemCreate(0);  //���ó�ֵ���Բ����ͷ�
	uart6_write = OSSemCreate(0);
	lan_write = OSSemCreate(0);
	txdbuff_write = OSSemCreate(0);
	OSSemPost(uart3_write);
	OSSemPost(uart6_write);
	OSSemPost(lan_write);
	OSSemPost(txdbuff_write);
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
	OSStart(); //����UCOS
}


extern void  start_task(void *pdata);
extern void  T1_task(void *pdata);
extern void  analysis_uart_3_buf(void *arg);
extern void  analysis_netbuf(void *arg);














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
	0,//校验格式 0不检验，1奇校验2偶校验
	8,//8位长度
	1,//1个停止位
	0x0603 ,
	0, //uint8_t parity;		// =0 : n,8,1   =1: o,8,1  =2: e,8,1	 数据格式
	
	0, //uint8_t DisplayMode;  // 显示模式　=0　固定　=1 循环
	{20,19,7,3,1,1,1,1,1,1,1,1}, //uint8_t interface_type[12]; // 输入类型
	{1,1,1,1,1,1,1,1,1,1,1,1},//uint8_t unit[12];  // 单位
	{980,980,980,980,1000,1000,1000,1000,10000,10000,10000,10000},//uint32_t scale[12]; // 转换系数
	{0,0,0,0,0,0,0,0,8192,8192,8192,8192},//uint32_t adjust[12]; // 修正系数
	{0,1,2,~0,~0,~0,~0,~0,3,4,5,6},//uint16_t interface_addr[12]; // modbus 地址 上传
	{0,0,0,0,0,0,0,0,0,0,0,0},//	uint8_t abs[12]; // 绝对值 1打开
	0,//uint8_t means	;// 均值类型
	1,//uint16_t means_times; // 均值积算周期
	20000,//uint16_t freq;  // 采样频率 Hz
	4096,//uint16_t avr_count;
	0,0,0,0, //uint8_t yellow_ch,blue_ch,green_ch,red_ch; // 显示波形通道
	2, // uint8_t reflash; // 刷新时间 
	~0, //	uint16_t din_addr;  //  开关量输入寄存器地址
	~0, // uint16_t dout_addr; //  开关量输出寄存器地址
	20,20,0,0,//	uint32_t gate0,gate1,gate2,gate3;  // 动作阈值
	100, 100,0,0,//uint32_t backlash0,backlash1,backlash2,backlash3; // 动作回差
	500, 50, //uint32_t vibrate_gate,vibrate_backlash;  // 振动启动阈值，回差
	300, 30, // uint32_t force_gate,force_backlash,   // 应变启动阈值， 回差。
	~0,~0, //	uint16_t max_addr0,max_addr1; 最大值地址
	300,
	300,
	300,
	0x4a,               //PGA
	2,                  //工作周期
	0x0081,             //放大倍数
	0x11,               //发射功率设置
	0x01,               //AD采样频率
	20,                 //AD采样时间，跟工作周期一个道理
	0x59B6A4C3,         //8点00分，aabb的格式，aa代表时间，bb代表分钟，秒默认为0
	0,                  //flash起始地址
	0, //DHCP
	GATAWAYMODE,
	0,
	"wifi-sensor",    //"TP-LINK-SCZZB",//"yec-test",//"wifi-sensor",//"TP-LINK-sczzb2",//"hold-704",//"wifi-test1",//"yec-test",//"wifi-test",//"yec-test",//"zl_sensor",/////"yec-test",//"test3",//"qiangang2", //"qiangang1", //"qiangang1", /////
	"wifi-sensor",    //"china-yec",//"",//"wifi-sensor",//"18051061462",//"wifi-test",//"zl_sensor",///"china-yec",//"",////"",//"zl_sensor",/"lft13852578307",//"",//"",//"123456789",//"china-yec.com",// //
	"192.168.99.6",   //"192.168.99.3", //服务器端的IP地址  "192.168.0.18", //M
	"8712",           //端口号
	"192.168.99.45",  //LocalIP
	"192.168.99.1",   //LocalGATEWAY
	"255.255.255.0",  //LocalMASK
	
	1,
	
	/*模块一的参数*/
	0x8001,  //模块1的自身地址
	0x9001,		//模块1目标地址
	0,       //校验方式，8N1
	0x03,			//7波特率9600
	0x02,			//空中速率 2.4k
	0x01,			//信道 1
	1,				//0定点传输
	1,        //推挽输入
	0,				//唤醒时间 ，不配置
	0,				//带前向纠错
	0,				//0是20DB
	
	/*模块二的参数*/
	0x8002,  //模块1的自身地址
	0x9001,		//模块1目标地址
	0,       //校验方式，8N1
	0x03,			//7波特率9600
	0x02,			//空中速率 2.4k
	0x01,			//信道 1
	1,				//0定点传输
	1,        //推挽输入
	0,				//唤醒时间 ，不配置
	0,				//带前向纠错
	0,				//0是20DB
	
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
	delay_init(168);       	//延时初始化
//	delay_ms(1000);         //这个很关键，调试的时候，不会死，具体原因自己猜
	My_RTC_Init();
	initEEPROM();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断分组配置
	
	//FSMC_SRAM_Init(); 		//SRAM初始化
 	mymem_init(SRAMIN);  	//初始化内部内存池
	//mymem_init(SRAMEX);  	//初始化外部内存池
	mymem_init(SRAMCCM); 	//初始化CCM内存池
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
	LostConnectNet();//初始化的时候，是默认没有链接的

	//CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_7tq,6,CAN_Mode_Normal);	//CAN普通模式初始化,普通模式,波特率500Kbps	
	OSInit(); 					//UCOS初始化
//  delay_ms(100);	
//	if(read_system_parameters(&sys_para)){
//		set_default_system_parameters(&sys_para);
//	}

//	init_thread_data();
	uart3_write = OSSemCreate(0);  //设置初值可以不用释放
	uart6_write = OSSemCreate(0);
	lan_write = OSSemCreate(0);
	txdbuff_write = OSSemCreate(0);
	OSSemPost(uart3_write);
	OSSemPost(uart6_write);
	OSSemPost(lan_write);
	OSSemPost(txdbuff_write);
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
	OSStart(); //开启UCOS
}


extern void  start_task(void *pdata);
extern void  T1_task(void *pdata);
extern void  analysis_uart_3_buf(void *arg);
extern void  analysis_netbuf(void *arg);














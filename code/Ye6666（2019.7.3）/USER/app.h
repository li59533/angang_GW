#ifndef _APP_H_
#define _APP_H_

#include "stdint.h"
#include "stm32f4xx.h"
#include "stm32f4_flash.h"
#include "rtc.h"
#include "module_1.h"
#include "module_2.h"
#include "includes.h"

#define SetBit(c,bits) {c|=(1<<bits);}
#define ResetBit(c,bits) {c&= ~(1<<bits);}
#define TestBit(c,bits)  (c&(1<<bits))

#define      RX_BUFFER_SIZE                              256
#define      TX_BUFFER_SIZE                              256

extern uint32_t NetReceBufHeadIndex;
extern uint32_t NetReceBufTailIndex;

#define NetReceBufLine  1024
#define IncreaseNetBuf(x)   {x=(x+1)%NetReceBufLine;}
#define isNetReceBufFull() ((NetReceBufHeadIndex+1)%NetReceBufLine==NetReceBufTailIndex)
#define isNetReceBufEmpty() (NetReceBufHeadIndex==NetReceBufTailIndex)

/*******************数据发送缓冲区*********************/
#define TxdBufLine  25
#define Increase(x)   {x=(x+1)%TxdBufLine;}
#define isTxdBufFull() ((HeadIndex+1)%TxdBufLine==TailIndex)
#define isTxdBufEmpty() (HeadIndex==TailIndex)


//命令行  命令一定要小于70，多于128的命令，我都判定成非法命令了
#define COMMAND_STOP           0x00
#define COMMAND_START          0x01
#define COMMAND_ID             0x02
#define COMMAND_CHANNELKIND    0x13
#define COMMAND_REPLYCONFIG    0x14
#define COMMAND_SETCONFIG      0x15  

#define COMMAND_REPLYIP        0x04
#define COMMAND_SETIP          0x05  //设置IP地址

#define COMMAND_REPLY_RATE     0x06   //设置采样参数
#define COMMAND_SAMPLE_RATE    0x07   //设置采样参数
#define COMMAND_REPLY_SCALE    0x08
#define COMMAND_SET_SCALE      0x09
#define COMMAND_ADJUST_TEMP    0x69
#define COMMAND_ADJUST_ADC     0x68
#define COMMAND_SET_SNNUMBER   0x88

#define COMMAND_SELECT_MODULE  0x16
#define COMMAND_RECEIVE_BEACON 0x11    //心跳
#define COMMAND_BATTERY_RESET  0x17    //电池重置
#define COMMAND_DELAY_SET      0x18    //中继延时设定
#define COMMAND_Version_Num    0x19    //版本号

#define COMMAND_REPLYTEMP      0x45


#define COMMAND_REQUIRE_PERIODWAVE 0x30 //请求一次单次波形
#define COMMAND_SET_CHANNEL_CONDITION 0x0a  //设置通道字
#define COMMAND_SET_RUNMODE 0x0B  //设置工作模式

#define COMMAND_REQUIRE_TIME 0x48   //设置时间
#define COMMAND_SET_ALARM_TIME 0x49 
#define COMMAND_REPLY_ALARM_TIME 0x47

#define COMMAND_SET_TIME 0x50  //设置时间
#define COMMAND_SET_AP 0x51  //设置AP地址
#define COMMAND_SET_TCPSERVE 0x52  //设置server端的地址
#define COMMAND_REPLYAP 0x53
#define COMMAND_REPLYTCPSERVE 0x54
#define COMMAND_APPLYNETSET 0x55
#define COMMAND_REPLY_RUNMODE 0x56  //读取工作模式
#define COMMAND_REPLY_CHANNEL_CONDITION 0x57  //设置通道字


#define      macUser_Esp32_LocalID                        "192.168.100.11"                //连不上去时，备用的就是这个
#define      macUser_Esp32_LocalGATAWAY                   "192.168.100.1"           
#define      macUser_Esp32_LocalMASK                      "255.255.255.0"           


#define      macUser_Esp32_ApSsid                        "yec-test"                //要连接的热点的名称
#define      macUser_Esp32_ApPwd                         ""           //要连接的热点的密钥

#define      macUser_Esp32_TcpServer_IP                   "192.168.100.2"//"192.168.0.112"// //     //要连接的服务器的 IP
#define      macUser_Esp32_TcpServer_Port                 "8712"  //"8712"//             //要连接的服务器的端口




typedef struct CONFIG				 // 配置信息
{
	uint16_t vaildsign;

	uint16_t baundrate;    /* =0:4800    =1:9600 		=2:38400 		=3:115200  		 */
	uint16_t addr; 
	uint16_t ParityType;
	uint16_t DataLength;
	uint16_t StopBit;
	uint64_t SNnumber; 
	uint8_t parity;		// =0 : n,8,1   =1: o,8,1  =2: e,8,1	 数据格式
	
	uint8_t DisplayMode;  // 显示模式　=0　固定　=1 循环
	uint8_t interface_type[12]; // 输入类型
	uint8_t unit[12];  // 单位
	uint32_t scale[12]; // 转换系数
	uint32_t adjust[12]; // 修正值
	uint16_t interface_addr[12]; // modbus 地址 上传
	uint8_t abs[12]; // 绝对值
	uint8_t means	;// 均值类型
	uint16_t means_times; // 均值积算周期
	uint16_t freq;  // 采样频率 Hz
	uint16_t avr_count;
	uint8_t yellow_ch,blue_ch,green_ch,red_ch; // 显示波形通道
	uint8_t reflash; // 刷新时间 
	uint16_t din_addr;  //  开关量输入寄存器地址
	uint16_t dout_addr; //  开关量输出寄存器地址
	uint16_t gate0,gate1,gate2,gate3;  // 动作阈值
	uint32_t backlash0,backlash1,backlash2,backlash3; // 动作回差
	uint32_t vibrate_gate,vibrate_backlash;  // 振动启动阈值，回差
	uint32_t force_gate,force_backlash;  // 应变启动阈值， 回差。
	uint16_t max_addr0,max_addr1;        //
	uint16_t vlpsseconds;            //  
	uint16_t vlprseconds;           //
	uint16_t runseconds;           //
	uint16_t pga;                 //
	uint16_t workcycleseconds;   //  工作周期
	uint16_t fangda;            //  放大倍数
	uint16_t boardset;         // 发射功率设置
	uint16_t ADtime;          //AD单次采样时间
	uint16_t ADfrequence;    //AD采样频率
	  
	uint64_t alarmminutetime;  //开始报警时间
	uint32_t FLASH_WRADDR;
	uint8_t  DHCP;
	uint8_t DataToBoardMode;
	uint64_t current_boardtime;
	char APssid[20];
	char APpassword[20];
	char TcpServer_IP[20];
	char TcpServer_Port[10];
	char LocalIP[20];
	char LocalGATEWAY[20];
	char LocalMASK[20];
	
	uint16_t send_delay_ms;
	
	
	uint16_t module_1_source_addr;
	uint16_t module_1_destination_addr;
	uint8_t module_1_datacheck;//0:8N1; 1:8o1;  2:8E1;  3:8N1;
	uint8_t module_1_baudrate;//0:1200; 1:2400;  2:4800;  3:9600; 4:19200;  5:38400;  6:57600; 7:115200;
	uint8_t module_1_airspeed;//0:0.3K 1:1.2K 2:2.4k 3:4.8K 4:9.6K 5:19.2K
	uint8_t module_1_channel; //00H-1FH,频率 410~441MHz ；410MHz+CHAN * 1MHz
	uint8_t module_1_transmission_mode;//0:定点传输 1：透明传输
	uint8_t module_1_IO_workstyle; //0:推挽输出 1：开路输出
	uint8_t module_1_wakeup_time;//0:250ms 1;500ms 2：750ms 3�:1000  4：1250   5：1500 56:1750  72000
	uint8_t module_1_FEC;   //0:关闭 1：开启
	uint8_t module_1_power; //0:20dB 1:17dB 2:14dB 3:10dB
	
	uint16_t module_2_source_addr;
	uint16_t module_2_destination_addr;
	uint8_t module_2_datacheck;//0:8N1; 1:8o1;  2:8E1;  3:8N1;
	uint8_t module_2_baudrate;//0:1200; 1:2400;  2:4800;  3:9600; 4:19200;  5:38400;  6:57600; 7:115200;
	uint8_t module_2_airspeed;//0:0.3K 1:1.2K 2:2.4k 3:4.8K 4:9.6K 5:19.2K
	uint8_t module_2_channel; //00H-1FH,频率 410~441MHz ；410MHz+CHAN * 1MHz
	uint8_t module_2_transmission_mode;//0:定点传输 1：透明传输
	uint8_t module_2_IO_workstyle; //0:推挽输出 1：开路输出
	uint8_t module_2_wakeup_time;//0:250ms 1;500ms 2：750ms 3�:1000  4：1250   5：1500 56:1750  72000
	uint8_t module_2_FEC;   //0:关闭 1：开启
	uint8_t module_2_power; //0:20dB 1:17dB 2:14dB 3:10dB
	
}CONFIG;



extern  struct CONFIG  config;  //配置信息
extern uint8_t module_1_ReceivedTel_debug;
typedef enum {Coordinator_Mode=0,Normal_Mode,LowPower_Mode,Sleep_Mode,} MODULE_MODE;

typedef enum {U3=0,U6,LAN,} communication_style;  //通讯口方式
typedef enum {NoBrainTransmission=0,BrainTransmission=1,} Esp32TransmissionMode;
typedef enum {EnableDHCP=1,DisableDHCP=0,} DHCPmode;
typedef enum {PARAMETERMODE=1,WAVEMODE=2,FFTWAVEMODE=3,FFTPARAMETERMODE=4,IDLEMODE=5,LITEWAVEMODE=6,GATAWAYMODE,BRIDGEMODE,} DataToBoard_TYPE;
/* TYPE_HBR 全桥 */
typedef enum {TYPE_NONE=0,TYPE_MA=1,TYPE_V=2, TYPE_IEPE=3,TYPE_PT=4,TYPE_HBR=5,TYPE_AC_V=6,} INTERFACE_TYPE;
typedef enum {UNIT_NONE=0,UNIT_V=1,UNIT_A=2,UNIT_KV=3,UNIT_TEMP=4,UNIT_M=5,UNIT_M_S=6,UNIT_M_S2=7,UNIT_G=8,UNIT_MA=9,UNIT_DB=10,UNIT_MM_S=11,} INTERFACE_UNIT;
typedef enum {MEANS_NONE=0,
							MEANS_MEANS, // 均值
							MEANS_RMS, // 均方根值
							MEANS_P,  // 峰值
							MEANS_PP, // 峰峰值
} MEANS_TYPE;  
typedef enum {BAUND2400=0,BAUND4800,BAUND9600,BAUND19200,BAUND38400,} BAUND_RATE;
//typedef enum {BAM4E33=0,BAM4E31=1, BAM4I33=2,BAM4I31=3,BAM4U33=4,BAM4U31=5,BAM4H33=6,BAM4H31=7,BAM4P33=8,BAM4P31=9,BAM4Q33=10,BAM4Q31=11}MODEL ;
extern uint8_t 	brightness; 
extern	uint8_t FirstBlood; //解决温度上传冲突
typedef struct PARAMETER				 // 所有参数
{
	uint16_t vaildsign;
	int32_t int_v[12];
	int32_t int_av[12];
	int64_t s[12]; // 累积
	int64_t as[12]; // 累积
	float v[12];
	float fv[12];
	float vs[12];  //速度累加和，浮点型
	float av[12];
	float adate;                //加速度
	float vdate;                 //速度
	float xdate;                  //位移
	float pdate;                   //温度
	float scale_v[12];
	int32_t gate[12];
	int32_t backlash[12];
	uint8_t alarm[12]; // 报警标志
	uint16_t din;  // 低8位对应 8个通道输入
	uint16_t dout; // 低4位对应 4个输出通道 
	uint16_t status;
	int32_t ch1_int_max,ch2_int_max;
	int32_t ch1_int_max1,ch2_int_max2;
	float ch1_max,ch2_max;
	int32_t vibrate_gate1,vibrate_gate2,vibrate_backlash1,vibrate_backlash2;
	int32_t force_gate,force_backlash;
	int16_t selfpgaallow;
	uint16_t daytime;
	uint16_t minutetime;
	uint16_t selffangda;
	uint16_t alarmminutetime;
	uint8_t Esp32TransmissionMode;
	uint32_t sleeptime;
	uint8_t wakeupsourec;
	float Abs_average[12];
	float ReciprocalofADfrequence;
	float Inter_Skew[12]; //歪度
	float InterIIMarginIndex[12];//裕度指标中间变量
	float InterMAX[12];
	float InterMIN[12];
	float S_sum[12];
	float SS_sum[12];
	float SSS_sum[12];
	float SSSS_sum[12];
	float Abs_S_average[12];
	float PeakValue[12];  //峰峰值
	float EffectiveValue[12]; //有效值
	float Skew[12]; //歪度
	float MaxValue[12]; //峰值
	float Kurtosis[12]; //峭度
	float Mean[12]; //均值
	float WaveformIndex[12]; //波形指标
	float PeakIndex[12];//峰值指标
	float PulseIndex[12];//脉冲指标
	float MarginIndex[12];//裕度指标
	float KurtosisIndex[12];//峭度指标
	float Inter_MarginIndex[12]; //峭度
	float S_average[12]; //平均值总值
	float average[12]; //平均值
	float ReciprocalofEMUnumber; //采样频率的倒数，采样周期
	float ReciprocalofRange[12];//量程的倒数
	//
	float F_sum[12];     //s(k)之和
	float FS_sum[12];   //f(k)s(k)的和
	float FFS_sum[12];  //F(K)F(K)S(K)
	float FFFFS_sum[12];  //F(K)F(K)S(K)
	float F2_sum[12];     //feature2的中间和
	float F3_sum[12];     //feature3的中间和
	float F4_sum[12];     //feature4的中间和
	float F6_sum[12];     //feature6的中间和
	float F11_sum[12];     //feature11的中间和
	float F12_sum[12];     //feature12的中间和
	float F13_sum[12];     //feature12的中间和
	float F_feature1[12];
	float F_feature2[12];
	float F_feature3[12];
	float F_feature4[12];
	float F_feature5[12];
	float F_feature6[12];
	float F_feature7[12];
	float F_feature8[12];
	float F_feature9[12];
	float F_feature10[12];
	float F_feature11[12];
	float F_feature12[12];
	float F_feature13[12];
	uint32_t current_module;
	uint8_t  battery_reset_flag;
	uint8_t  addr_node_high;
	uint8_t  addr_node_lower;
}PARAMETER;

extern OS_EVENT *uart3_write;
extern OS_EVENT *uart6_write;
extern OS_EVENT *lan_write;
extern OS_EVENT *txdbuff_write;
extern OS_EVENT *sem_uart3_buf;
extern OS_EVENT *sem_uart6_buf;

extern void WritedatatoUSART6(uint8_t *sourdata,uint32_t length);
extern void WritedatatoUSART3(uint8_t *sourdata,uint32_t length);
extern void WritedatatoLAN(uint8_t *sourdata,uint32_t length);

extern  struct PARAMETER Parameter;
#define LPTMR_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_LpoClk)

#define isRunning() ((Parameter.status&0x01)!=0)
#define setRunning() {Parameter.status|=0x01;}
#define clrRunning() {Parameter.status&=~0x01;}

#define isFactoryLink() ((Parameter.status&0x02)!=0)
#define setFactoryLink() {Parameter.status|=0x02;}
#define clrFactoryLink() {Parameter.status&=~0x02;}

#define isAbleTempTransmission() ((Parameter.status&0x04)!=0)
#define EnableTempTransmission() {Parameter.status|=0x04;}
#define DisableTempTransmission() {Parameter.status&=~0x04;}

#define isAblePeroidWaveTransmission() ((Parameter.status&0x08)!=0)
#define EnablePeroidWaveTransmission() {Parameter.status|=0x08;}
#define DisablePeroidWaveTransmission() {Parameter.status&=~0x08;}


#define isInConnectNet() ((Parameter.status&0x10)!=0)
#define BuildConnectNet() {Parameter.status|=0x10;}
#define LostConnectNet() {Parameter.status&=~0x10;}

#define isAblePeroidWaveAutoTransmission() ((Parameter.status&0x20)!=0)
#define EnablePeroidWaveAutoTransmission() {Parameter.status|=0x20;}
#define DisablePeroidWaveAutoTransmission() {Parameter.status&=~0x20;}

#define isCollectingOverInParameterMode() ((Parameter.status&0x40)!=0)  //正在采集，不上传数据
#define overCollectingInParameterMode() {Parameter.status|=0x40;}
#define onCollectingInParameterMode() {Parameter.status&=~0x40;}

#define isReceiveBeaconMessage() ((Parameter.status&0x80)!=0) 
#define ReceiveBeaconMessage() {Parameter.status|=0x80;}
#define DeleteBeaconMessage() {Parameter.status&=~0x80;}


#define EnableModulePower()  {GPIO_SetBits(GPIOF, GPIO_Pin_13);}  //是否给esp32供电
#define DisableModulePower() {GPIO_ResetBits(GPIOF, GPIO_Pin_13);}

#define LED0_SET()  {GPIO_ResetBits(GPIOF, GPIO_Pin_9);}  //是否供电模拟部分
#define LED0_CLR()  {GPIO_SetBits(GPIOF, GPIO_Pin_9);}

#define LED1_SET()  {GPIO_ResetBits(GPIOF, GPIO_Pin_10);}  //呼吸等，GPIOA 10
#define LED1_CLR()  {GPIO_SetBits(GPIOF, GPIO_Pin_10);}

#define PSRAM_CS_HIGH() {GPIO_SetBits(GPIOD, 1u << 15);}  //呼吸等，GPIOA 10
#define PSRAM_CS_LOW() {GPIO_ResetBits(GPIOD, 1u << 15);}


#endif

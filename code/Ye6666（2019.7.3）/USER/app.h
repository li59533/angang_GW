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

/*******************Êý¾Ý·¢ËÍ»º³åÇø*********************/
#define TxdBufLine  25
#define Increase(x)   {x=(x+1)%TxdBufLine;}
#define isTxdBufFull() ((HeadIndex+1)%TxdBufLine==TailIndex)
#define isTxdBufEmpty() (HeadIndex==TailIndex)


//ÃüÁîÐÐ  ÃüÁîÒ»¶¨ÒªÐ¡ÓÚ70£¬¶àÓÚ128µÄÃüÁî£¬ÎÒ¶¼ÅÐ¶¨³É·Ç·¨ÃüÁîÁË
#define COMMAND_STOP           0x00
#define COMMAND_START          0x01
#define COMMAND_ID             0x02
#define COMMAND_CHANNELKIND    0x13
#define COMMAND_REPLYCONFIG    0x14
#define COMMAND_SETCONFIG      0x15  

#define COMMAND_REPLYIP        0x04
#define COMMAND_SETIP          0x05  //ÉèÖÃIPµØÖ·

#define COMMAND_REPLY_RATE     0x06   //ÉèÖÃ²ÉÑù²ÎÊý
#define COMMAND_SAMPLE_RATE    0x07   //ÉèÖÃ²ÉÑù²ÎÊý
#define COMMAND_REPLY_SCALE    0x08
#define COMMAND_SET_SCALE      0x09
#define COMMAND_ADJUST_TEMP    0x69
#define COMMAND_ADJUST_ADC     0x68
#define COMMAND_SET_SNNUMBER   0x88

#define COMMAND_SELECT_MODULE  0x16
#define COMMAND_RECEIVE_BEACON 0x11    //ÐÄÌø
#define COMMAND_BATTERY_RESET  0x17    //µç³ØÖØÖÃ
#define COMMAND_DELAY_SET      0x18    //ÖÐ¼ÌÑÓÊ±Éè¶¨
#define COMMAND_Version_Num    0x19    //°æ±¾ºÅ

#define COMMAND_REPLYTEMP      0x45


#define COMMAND_REQUIRE_PERIODWAVE 0x30 //ÇëÇóÒ»´Îµ¥´Î²¨ÐÎ
#define COMMAND_SET_CHANNEL_CONDITION 0x0a  //ÉèÖÃÍ¨µÀ×Ö
#define COMMAND_SET_RUNMODE 0x0B  //ÉèÖÃ¹¤×÷Ä£Ê½

#define COMMAND_REQUIRE_TIME 0x48   //ÉèÖÃÊ±¼ä
#define COMMAND_SET_ALARM_TIME 0x49 
#define COMMAND_REPLY_ALARM_TIME 0x47

#define COMMAND_SET_TIME 0x50  //ÉèÖÃÊ±¼ä
#define COMMAND_SET_AP 0x51  //ÉèÖÃAPµØÖ·
#define COMMAND_SET_TCPSERVE 0x52  //ÉèÖÃserver¶ËµÄµØÖ·
#define COMMAND_REPLYAP 0x53
#define COMMAND_REPLYTCPSERVE 0x54
#define COMMAND_APPLYNETSET 0x55
#define COMMAND_REPLY_RUNMODE 0x56  //¶ÁÈ¡¹¤×÷Ä£Ê½
#define COMMAND_REPLY_CHANNEL_CONDITION 0x57  //ÉèÖÃÍ¨µÀ×Ö


#define      macUser_Esp32_LocalID                        "192.168.100.11"                //Á¬²»ÉÏÈ¥Ê±£¬±¸ÓÃµÄ¾ÍÊÇÕâ¸ö
#define      macUser_Esp32_LocalGATAWAY                   "192.168.100.1"           
#define      macUser_Esp32_LocalMASK                      "255.255.255.0"           


#define      macUser_Esp32_ApSsid                        "yec-test"                //ÒªÁ¬½ÓµÄÈÈµãµÄÃû³Æ
#define      macUser_Esp32_ApPwd                         ""           //ÒªÁ¬½ÓµÄÈÈµãµÄÃÜÔ¿

#define      macUser_Esp32_TcpServer_IP                   "192.168.100.2"//"192.168.0.112"// //     //ÒªÁ¬½ÓµÄ·þÎñÆ÷µÄ IP
#define      macUser_Esp32_TcpServer_Port                 "8712"  //"8712"//             //ÒªÁ¬½ÓµÄ·þÎñÆ÷µÄ¶Ë¿Ú




typedef struct CONFIG				 // ÅäÖÃÐÅÏ¢
{
	uint16_t vaildsign;

	uint16_t baundrate;    /* =0:4800    =1:9600 		=2:38400 		=3:115200  		 */
	uint16_t addr; 
	uint16_t ParityType;
	uint16_t DataLength;
	uint16_t StopBit;
	uint64_t SNnumber; 
	uint8_t parity;		// =0 : n,8,1   =1: o,8,1  =2: e,8,1	 Êý¾Ý¸ñÊ½
	
	uint8_t DisplayMode;  // ÏÔÊ¾Ä£Ê½¡¡=0¡¡¹Ì¶¨¡¡=1 Ñ­»·
	uint8_t interface_type[12]; // ÊäÈëÀàÐÍ
	uint8_t unit[12];  // µ¥Î»
	uint32_t scale[12]; // ×ª»»ÏµÊý
	uint32_t adjust[12]; // ÐÞÕýÖµ
	uint16_t interface_addr[12]; // modbus µØÖ· ÉÏ´«
	uint8_t abs[12]; // ¾ø¶ÔÖµ
	uint8_t means	;// ¾ùÖµÀàÐÍ
	uint16_t means_times; // ¾ùÖµ»ýËãÖÜÆÚ
	uint16_t freq;  // ²ÉÑùÆµÂÊ Hz
	uint16_t avr_count;
	uint8_t yellow_ch,blue_ch,green_ch,red_ch; // ÏÔÊ¾²¨ÐÎÍ¨µÀ
	uint8_t reflash; // Ë¢ÐÂÊ±¼ä 
	uint16_t din_addr;  //  ¿ª¹ØÁ¿ÊäÈë¼Ä´æÆ÷µØÖ·
	uint16_t dout_addr; //  ¿ª¹ØÁ¿Êä³ö¼Ä´æÆ÷µØÖ·
	uint16_t gate0,gate1,gate2,gate3;  // ¶¯×÷ãÐÖµ
	uint32_t backlash0,backlash1,backlash2,backlash3; // ¶¯×÷»Ø²î
	uint32_t vibrate_gate,vibrate_backlash;  // Õñ¶¯Æô¶¯ãÐÖµ£¬»Ø²î
	uint32_t force_gate,force_backlash;  // Ó¦±äÆô¶¯ãÐÖµ£¬ »Ø²î¡£
	uint16_t max_addr0,max_addr1;        //
	uint16_t vlpsseconds;            //  
	uint16_t vlprseconds;           //
	uint16_t runseconds;           //
	uint16_t pga;                 //
	uint16_t workcycleseconds;   //  ¹¤×÷ÖÜÆÚ
	uint16_t fangda;            //  ·Å´ó±¶Êý
	uint16_t boardset;         // ·¢Éä¹¦ÂÊÉèÖÃ
	uint16_t ADtime;          //ADµ¥´Î²ÉÑùÊ±¼ä
	uint16_t ADfrequence;    //AD²ÉÑùÆµÂÊ
	  
	uint64_t alarmminutetime;  //¿ªÊ¼±¨¾¯Ê±¼ä
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
	uint8_t module_1_channel; //00H-1FH,ÆµÂÊ 410~441MHz £»410MHz+CHAN * 1MHz
	uint8_t module_1_transmission_mode;//0:¶¨µã´«Êä 1£ºÍ¸Ã÷´«Êä
	uint8_t module_1_IO_workstyle; //0:ÍÆÍìÊä³ö 1£º¿ªÂ·Êä³ö
	uint8_t module_1_wakeup_time;//0:250ms 1;500ms 2£º750ms 3£:1000  4£º1250   5£º1500 56:1750  72000
	uint8_t module_1_FEC;   //0:¹Ø±Õ 1£º¿ªÆô
	uint8_t module_1_power; //0:20dB 1:17dB 2:14dB 3:10dB
	
	uint16_t module_2_source_addr;
	uint16_t module_2_destination_addr;
	uint8_t module_2_datacheck;//0:8N1; 1:8o1;  2:8E1;  3:8N1;
	uint8_t module_2_baudrate;//0:1200; 1:2400;  2:4800;  3:9600; 4:19200;  5:38400;  6:57600; 7:115200;
	uint8_t module_2_airspeed;//0:0.3K 1:1.2K 2:2.4k 3:4.8K 4:9.6K 5:19.2K
	uint8_t module_2_channel; //00H-1FH,ÆµÂÊ 410~441MHz £»410MHz+CHAN * 1MHz
	uint8_t module_2_transmission_mode;//0:¶¨µã´«Êä 1£ºÍ¸Ã÷´«Êä
	uint8_t module_2_IO_workstyle; //0:ÍÆÍìÊä³ö 1£º¿ªÂ·Êä³ö
	uint8_t module_2_wakeup_time;//0:250ms 1;500ms 2£º750ms 3£:1000  4£º1250   5£º1500 56:1750  72000
	uint8_t module_2_FEC;   //0:¹Ø±Õ 1£º¿ªÆô
	uint8_t module_2_power; //0:20dB 1:17dB 2:14dB 3:10dB
	
}CONFIG;



extern  struct CONFIG  config;  //ÅäÖÃÐÅÏ¢
extern uint8_t module_1_ReceivedTel_debug;
typedef enum {Coordinator_Mode=0,Normal_Mode,LowPower_Mode,Sleep_Mode,} MODULE_MODE;

typedef enum {U3=0,U6,LAN,} communication_style;  //Í¨Ñ¶¿Ú·½Ê½
typedef enum {NoBrainTransmission=0,BrainTransmission=1,} Esp32TransmissionMode;
typedef enum {EnableDHCP=1,DisableDHCP=0,} DHCPmode;
typedef enum {PARAMETERMODE=1,WAVEMODE=2,FFTWAVEMODE=3,FFTPARAMETERMODE=4,IDLEMODE=5,LITEWAVEMODE=6,GATAWAYMODE,BRIDGEMODE,} DataToBoard_TYPE;
/* TYPE_HBR È«ÇÅ */
typedef enum {TYPE_NONE=0,TYPE_MA=1,TYPE_V=2, TYPE_IEPE=3,TYPE_PT=4,TYPE_HBR=5,TYPE_AC_V=6,} INTERFACE_TYPE;
typedef enum {UNIT_NONE=0,UNIT_V=1,UNIT_A=2,UNIT_KV=3,UNIT_TEMP=4,UNIT_M=5,UNIT_M_S=6,UNIT_M_S2=7,UNIT_G=8,UNIT_MA=9,UNIT_DB=10,UNIT_MM_S=11,} INTERFACE_UNIT;
typedef enum {MEANS_NONE=0,
							MEANS_MEANS, // ¾ùÖµ
							MEANS_RMS, // ¾ù·½¸ùÖµ
							MEANS_P,  // ·åÖµ
							MEANS_PP, // ·å·åÖµ
} MEANS_TYPE;  
typedef enum {BAUND2400=0,BAUND4800,BAUND9600,BAUND19200,BAUND38400,} BAUND_RATE;
//typedef enum {BAM4E33=0,BAM4E31=1, BAM4I33=2,BAM4I31=3,BAM4U33=4,BAM4U31=5,BAM4H33=6,BAM4H31=7,BAM4P33=8,BAM4P31=9,BAM4Q33=10,BAM4Q31=11}MODEL ;
extern uint8_t 	brightness; 
extern	uint8_t FirstBlood; //½â¾öÎÂ¶ÈÉÏ´«³åÍ»
typedef struct PARAMETER				 // ËùÓÐ²ÎÊý
{
	uint16_t vaildsign;
	int32_t int_v[12];
	int32_t int_av[12];
	int64_t s[12]; // ÀÛ»ý
	int64_t as[12]; // ÀÛ»ý
	float v[12];
	float fv[12];
	float vs[12];  //ËÙ¶ÈÀÛ¼ÓºÍ£¬¸¡µãÐÍ
	float av[12];
	float adate;                //¼ÓËÙ¶È
	float vdate;                 //ËÙ¶È
	float xdate;                  //Î»ÒÆ
	float pdate;                   //ÎÂ¶È
	float scale_v[12];
	int32_t gate[12];
	int32_t backlash[12];
	uint8_t alarm[12]; // ±¨¾¯±êÖ¾
	uint16_t din;  // µÍ8Î»¶ÔÓ¦ 8¸öÍ¨µÀÊäÈë
	uint16_t dout; // µÍ4Î»¶ÔÓ¦ 4¸öÊä³öÍ¨µÀ 
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
	float Inter_Skew[12]; //Íá¶È
	float InterIIMarginIndex[12];//Ô£¶ÈÖ¸±êÖÐ¼ä±äÁ¿
	float InterMAX[12];
	float InterMIN[12];
	float S_sum[12];
	float SS_sum[12];
	float SSS_sum[12];
	float SSSS_sum[12];
	float Abs_S_average[12];
	float PeakValue[12];  //·å·åÖµ
	float EffectiveValue[12]; //ÓÐÐ§Öµ
	float Skew[12]; //Íá¶È
	float MaxValue[12]; //·åÖµ
	float Kurtosis[12]; //ÇÍ¶È
	float Mean[12]; //¾ùÖµ
	float WaveformIndex[12]; //²¨ÐÎÖ¸±ê
	float PeakIndex[12];//·åÖµÖ¸±ê
	float PulseIndex[12];//Âö³åÖ¸±ê
	float MarginIndex[12];//Ô£¶ÈÖ¸±ê
	float KurtosisIndex[12];//ÇÍ¶ÈÖ¸±ê
	float Inter_MarginIndex[12]; //ÇÍ¶È
	float S_average[12]; //Æ½¾ùÖµ×ÜÖµ
	float average[12]; //Æ½¾ùÖµ
	float ReciprocalofEMUnumber; //²ÉÑùÆµÂÊµÄµ¹Êý£¬²ÉÑùÖÜÆÚ
	float ReciprocalofRange[12];//Á¿³ÌµÄµ¹Êý
	//
	float F_sum[12];     //s(k)Ö®ºÍ
	float FS_sum[12];   //f(k)s(k)µÄºÍ
	float FFS_sum[12];  //F(K)F(K)S(K)
	float FFFFS_sum[12];  //F(K)F(K)S(K)
	float F2_sum[12];     //feature2µÄÖÐ¼äºÍ
	float F3_sum[12];     //feature3µÄÖÐ¼äºÍ
	float F4_sum[12];     //feature4µÄÖÐ¼äºÍ
	float F6_sum[12];     //feature6µÄÖÐ¼äºÍ
	float F11_sum[12];     //feature11µÄÖÐ¼äºÍ
	float F12_sum[12];     //feature12µÄÖÐ¼äºÍ
	float F13_sum[12];     //feature12µÄÖÐ¼äºÍ
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

#define isCollectingOverInParameterMode() ((Parameter.status&0x40)!=0)  //ÕýÔÚ²É¼¯£¬²»ÉÏ´«Êý¾Ý
#define overCollectingInParameterMode() {Parameter.status|=0x40;}
#define onCollectingInParameterMode() {Parameter.status&=~0x40;}

#define isReceiveBeaconMessage() ((Parameter.status&0x80)!=0) 
#define ReceiveBeaconMessage() {Parameter.status|=0x80;}
#define DeleteBeaconMessage() {Parameter.status&=~0x80;}


#define EnableModulePower()  {GPIO_SetBits(GPIOF, GPIO_Pin_13);}  //ÊÇ·ñ¸øesp32¹©µç
#define DisableModulePower() {GPIO_ResetBits(GPIOF, GPIO_Pin_13);}

#define LED0_SET()  {GPIO_ResetBits(GPIOF, GPIO_Pin_9);}  //ÊÇ·ñ¹©µçÄ£Äâ²¿·Ö
#define LED0_CLR()  {GPIO_SetBits(GPIOF, GPIO_Pin_9);}

#define LED1_SET()  {GPIO_ResetBits(GPIOF, GPIO_Pin_10);}  //ºôÎüµÈ£¬GPIOA 10
#define LED1_CLR()  {GPIO_SetBits(GPIOF, GPIO_Pin_10);}

#define PSRAM_CS_HIGH() {GPIO_SetBits(GPIOD, 1u << 15);}  //ºôÎüµÈ£¬GPIOA 10
#define PSRAM_CS_LOW() {GPIO_ResetBits(GPIOD, 1u << 15);}


#endif

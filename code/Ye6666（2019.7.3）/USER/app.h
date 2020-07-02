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

/*******************���ݷ��ͻ�����*********************/
#define TxdBufLine  25
#define Increase(x)   {x=(x+1)%TxdBufLine;}
#define isTxdBufFull() ((HeadIndex+1)%TxdBufLine==TailIndex)
#define isTxdBufEmpty() (HeadIndex==TailIndex)


//������  ����һ��ҪС��70������128������Ҷ��ж��ɷǷ�������
#define COMMAND_STOP           0x00
#define COMMAND_START          0x01
#define COMMAND_ID             0x02
#define COMMAND_CHANNELKIND    0x13
#define COMMAND_REPLYCONFIG    0x14
#define COMMAND_SETCONFIG      0x15  

#define COMMAND_REPLYIP        0x04
#define COMMAND_SETIP          0x05  //����IP��ַ

#define COMMAND_REPLY_RATE     0x06   //���ò�������
#define COMMAND_SAMPLE_RATE    0x07   //���ò�������
#define COMMAND_REPLY_SCALE    0x08
#define COMMAND_SET_SCALE      0x09
#define COMMAND_ADJUST_TEMP    0x69
#define COMMAND_ADJUST_ADC     0x68
#define COMMAND_SET_SNNUMBER   0x88

#define COMMAND_SELECT_MODULE  0x16
#define COMMAND_RECEIVE_BEACON 0x11    //����
#define COMMAND_BATTERY_RESET  0x17    //�������
#define COMMAND_DELAY_SET      0x18    //�м���ʱ�趨
#define COMMAND_Version_Num    0x19    //�汾��

#define COMMAND_REPLYTEMP      0x45


#define COMMAND_REQUIRE_PERIODWAVE 0x30 //����һ�ε��β���
#define COMMAND_SET_CHANNEL_CONDITION 0x0a  //����ͨ����
#define COMMAND_SET_RUNMODE 0x0B  //���ù���ģʽ

#define COMMAND_REQUIRE_TIME 0x48   //����ʱ��
#define COMMAND_SET_ALARM_TIME 0x49 
#define COMMAND_REPLY_ALARM_TIME 0x47

#define COMMAND_SET_TIME 0x50  //����ʱ��
#define COMMAND_SET_AP 0x51  //����AP��ַ
#define COMMAND_SET_TCPSERVE 0x52  //����server�˵ĵ�ַ
#define COMMAND_REPLYAP 0x53
#define COMMAND_REPLYTCPSERVE 0x54
#define COMMAND_APPLYNETSET 0x55
#define COMMAND_REPLY_RUNMODE 0x56  //��ȡ����ģʽ
#define COMMAND_REPLY_CHANNEL_CONDITION 0x57  //����ͨ����


#define      macUser_Esp32_LocalID                        "192.168.100.11"                //������ȥʱ�����õľ������
#define      macUser_Esp32_LocalGATAWAY                   "192.168.100.1"           
#define      macUser_Esp32_LocalMASK                      "255.255.255.0"           


#define      macUser_Esp32_ApSsid                        "yec-test"                //Ҫ���ӵ��ȵ������
#define      macUser_Esp32_ApPwd                         ""           //Ҫ���ӵ��ȵ����Կ

#define      macUser_Esp32_TcpServer_IP                   "192.168.100.2"//"192.168.0.112"// //     //Ҫ���ӵķ������� IP
#define      macUser_Esp32_TcpServer_Port                 "8712"  //"8712"//             //Ҫ���ӵķ������Ķ˿�




typedef struct CONFIG				 // ������Ϣ
{
	uint16_t vaildsign;

	uint16_t baundrate;    /* =0:4800    =1:9600 		=2:38400 		=3:115200  		 */
	uint16_t addr; 
	uint16_t ParityType;
	uint16_t DataLength;
	uint16_t StopBit;
	uint64_t SNnumber; 
	uint8_t parity;		// =0 : n,8,1   =1: o,8,1  =2: e,8,1	 ���ݸ�ʽ
	
	uint8_t DisplayMode;  // ��ʾģʽ��=0���̶���=1 ѭ��
	uint8_t interface_type[12]; // ��������
	uint8_t unit[12];  // ��λ
	uint32_t scale[12]; // ת��ϵ��
	uint32_t adjust[12]; // ����ֵ
	uint16_t interface_addr[12]; // modbus ��ַ �ϴ�
	uint8_t abs[12]; // ����ֵ
	uint8_t means	;// ��ֵ����
	uint16_t means_times; // ��ֵ��������
	uint16_t freq;  // ����Ƶ�� Hz
	uint16_t avr_count;
	uint8_t yellow_ch,blue_ch,green_ch,red_ch; // ��ʾ����ͨ��
	uint8_t reflash; // ˢ��ʱ�� 
	uint16_t din_addr;  //  ����������Ĵ�����ַ
	uint16_t dout_addr; //  ����������Ĵ�����ַ
	uint16_t gate0,gate1,gate2,gate3;  // ������ֵ
	uint32_t backlash0,backlash1,backlash2,backlash3; // �����ز�
	uint32_t vibrate_gate,vibrate_backlash;  // ��������ֵ���ز�
	uint32_t force_gate,force_backlash;  // Ӧ��������ֵ�� �ز
	uint16_t max_addr0,max_addr1;        //
	uint16_t vlpsseconds;            //  
	uint16_t vlprseconds;           //
	uint16_t runseconds;           //
	uint16_t pga;                 //
	uint16_t workcycleseconds;   //  ��������
	uint16_t fangda;            //  �Ŵ���
	uint16_t boardset;         // ���书������
	uint16_t ADtime;          //AD���β���ʱ��
	uint16_t ADfrequence;    //AD����Ƶ��
	  
	uint64_t alarmminutetime;  //��ʼ����ʱ��
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
	uint8_t module_1_channel; //00H-1FH,Ƶ�� 410~441MHz ��410MHz+CHAN * 1MHz
	uint8_t module_1_transmission_mode;//0:���㴫�� 1��͸������
	uint8_t module_1_IO_workstyle; //0:������� 1����·���
	uint8_t module_1_wakeup_time;//0:250ms 1;500ms 2��750ms 3�:1000  4��1250   5��1500 56:1750  72000
	uint8_t module_1_FEC;   //0:�ر� 1������
	uint8_t module_1_power; //0:20dB 1:17dB 2:14dB 3:10dB
	
	uint16_t module_2_source_addr;
	uint16_t module_2_destination_addr;
	uint8_t module_2_datacheck;//0:8N1; 1:8o1;  2:8E1;  3:8N1;
	uint8_t module_2_baudrate;//0:1200; 1:2400;  2:4800;  3:9600; 4:19200;  5:38400;  6:57600; 7:115200;
	uint8_t module_2_airspeed;//0:0.3K 1:1.2K 2:2.4k 3:4.8K 4:9.6K 5:19.2K
	uint8_t module_2_channel; //00H-1FH,Ƶ�� 410~441MHz ��410MHz+CHAN * 1MHz
	uint8_t module_2_transmission_mode;//0:���㴫�� 1��͸������
	uint8_t module_2_IO_workstyle; //0:������� 1����·���
	uint8_t module_2_wakeup_time;//0:250ms 1;500ms 2��750ms 3�:1000  4��1250   5��1500 56:1750  72000
	uint8_t module_2_FEC;   //0:�ر� 1������
	uint8_t module_2_power; //0:20dB 1:17dB 2:14dB 3:10dB
	
}CONFIG;



extern  struct CONFIG  config;  //������Ϣ
extern uint8_t module_1_ReceivedTel_debug;
typedef enum {Coordinator_Mode=0,Normal_Mode,LowPower_Mode,Sleep_Mode,} MODULE_MODE;

typedef enum {U3=0,U6,LAN,} communication_style;  //ͨѶ�ڷ�ʽ
typedef enum {NoBrainTransmission=0,BrainTransmission=1,} Esp32TransmissionMode;
typedef enum {EnableDHCP=1,DisableDHCP=0,} DHCPmode;
typedef enum {PARAMETERMODE=1,WAVEMODE=2,FFTWAVEMODE=3,FFTPARAMETERMODE=4,IDLEMODE=5,LITEWAVEMODE=6,GATAWAYMODE,BRIDGEMODE,} DataToBoard_TYPE;
/* TYPE_HBR ȫ�� */
typedef enum {TYPE_NONE=0,TYPE_MA=1,TYPE_V=2, TYPE_IEPE=3,TYPE_PT=4,TYPE_HBR=5,TYPE_AC_V=6,} INTERFACE_TYPE;
typedef enum {UNIT_NONE=0,UNIT_V=1,UNIT_A=2,UNIT_KV=3,UNIT_TEMP=4,UNIT_M=5,UNIT_M_S=6,UNIT_M_S2=7,UNIT_G=8,UNIT_MA=9,UNIT_DB=10,UNIT_MM_S=11,} INTERFACE_UNIT;
typedef enum {MEANS_NONE=0,
							MEANS_MEANS, // ��ֵ
							MEANS_RMS, // ������ֵ
							MEANS_P,  // ��ֵ
							MEANS_PP, // ���ֵ
} MEANS_TYPE;  
typedef enum {BAUND2400=0,BAUND4800,BAUND9600,BAUND19200,BAUND38400,} BAUND_RATE;
//typedef enum {BAM4E33=0,BAM4E31=1, BAM4I33=2,BAM4I31=3,BAM4U33=4,BAM4U31=5,BAM4H33=6,BAM4H31=7,BAM4P33=8,BAM4P31=9,BAM4Q33=10,BAM4Q31=11}MODEL ;
extern uint8_t 	brightness; 
extern	uint8_t FirstBlood; //����¶��ϴ���ͻ
typedef struct PARAMETER				 // ���в���
{
	uint16_t vaildsign;
	int32_t int_v[12];
	int32_t int_av[12];
	int64_t s[12]; // �ۻ�
	int64_t as[12]; // �ۻ�
	float v[12];
	float fv[12];
	float vs[12];  //�ٶ��ۼӺͣ�������
	float av[12];
	float adate;                //���ٶ�
	float vdate;                 //�ٶ�
	float xdate;                  //λ��
	float pdate;                   //�¶�
	float scale_v[12];
	int32_t gate[12];
	int32_t backlash[12];
	uint8_t alarm[12]; // ������־
	uint16_t din;  // ��8λ��Ӧ 8��ͨ������
	uint16_t dout; // ��4λ��Ӧ 4�����ͨ�� 
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
	float Inter_Skew[12]; //���
	float InterIIMarginIndex[12];//ԣ��ָ���м����
	float InterMAX[12];
	float InterMIN[12];
	float S_sum[12];
	float SS_sum[12];
	float SSS_sum[12];
	float SSSS_sum[12];
	float Abs_S_average[12];
	float PeakValue[12];  //���ֵ
	float EffectiveValue[12]; //��Чֵ
	float Skew[12]; //���
	float MaxValue[12]; //��ֵ
	float Kurtosis[12]; //�Ͷ�
	float Mean[12]; //��ֵ
	float WaveformIndex[12]; //����ָ��
	float PeakIndex[12];//��ֵָ��
	float PulseIndex[12];//����ָ��
	float MarginIndex[12];//ԣ��ָ��
	float KurtosisIndex[12];//�Ͷ�ָ��
	float Inter_MarginIndex[12]; //�Ͷ�
	float S_average[12]; //ƽ��ֵ��ֵ
	float average[12]; //ƽ��ֵ
	float ReciprocalofEMUnumber; //����Ƶ�ʵĵ�������������
	float ReciprocalofRange[12];//���̵ĵ���
	//
	float F_sum[12];     //s(k)֮��
	float FS_sum[12];   //f(k)s(k)�ĺ�
	float FFS_sum[12];  //F(K)F(K)S(K)
	float FFFFS_sum[12];  //F(K)F(K)S(K)
	float F2_sum[12];     //feature2���м��
	float F3_sum[12];     //feature3���м��
	float F4_sum[12];     //feature4���м��
	float F6_sum[12];     //feature6���м��
	float F11_sum[12];     //feature11���м��
	float F12_sum[12];     //feature12���м��
	float F13_sum[12];     //feature12���м��
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

#define isCollectingOverInParameterMode() ((Parameter.status&0x40)!=0)  //���ڲɼ������ϴ�����
#define overCollectingInParameterMode() {Parameter.status|=0x40;}
#define onCollectingInParameterMode() {Parameter.status&=~0x40;}

#define isReceiveBeaconMessage() ((Parameter.status&0x80)!=0) 
#define ReceiveBeaconMessage() {Parameter.status|=0x80;}
#define DeleteBeaconMessage() {Parameter.status&=~0x80;}


#define EnableModulePower()  {GPIO_SetBits(GPIOF, GPIO_Pin_13);}  //�Ƿ��esp32����
#define DisableModulePower() {GPIO_ResetBits(GPIOF, GPIO_Pin_13);}

#define LED0_SET()  {GPIO_ResetBits(GPIOF, GPIO_Pin_9);}  //�Ƿ񹩵�ģ�ⲿ��
#define LED0_CLR()  {GPIO_SetBits(GPIOF, GPIO_Pin_9);}

#define LED1_SET()  {GPIO_ResetBits(GPIOF, GPIO_Pin_10);}  //�����ȣ�GPIOA 10
#define LED1_CLR()  {GPIO_SetBits(GPIOF, GPIO_Pin_10);}

#define PSRAM_CS_HIGH() {GPIO_SetBits(GPIOD, 1u << 15);}  //�����ȣ�GPIOA 10
#define PSRAM_CS_LOW() {GPIO_ResetBits(GPIOD, 1u << 15);}


#endif

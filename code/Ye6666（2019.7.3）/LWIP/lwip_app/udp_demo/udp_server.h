#ifndef __UDP_SERVER_H
#define __UDP_SERVER_H
#include "sys.h"
#include "includes.h"
#include "ye6271_common.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//NETCONN API编程方式的UDP测试代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/8/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   


 
#define UDP_SERVER_RX_BUFSIZE		512	//定义udp最大接收数据长度
#define UDP_SERVER_PORT			8089	//定义udp连接的本地端口号

extern u8 udp_flag;		//UDP数据发送标志位

typedef struct{
	u16 port;
	OS_EVENT *sem;
	//OS_EVENT *sem2;
	u8 current_write_bank;
	u8* bank[2];
}THREAD_CONTEX;

extern THREAD_CONTEX thread_contex[SLAVE_MACHINE_NUM];
void init_thread_data(void);


#endif




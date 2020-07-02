#ifndef __CAN_H
#define __CAN_H	 
#include "sys.h"	    
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//CAN驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/7
//版本：V1.0 
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

	
u8 CAN1_Mode_Init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode);//CAN初始化
u8 CAN1_Send_Msg(u8* msg,u8 len,u16 stdID);
u8 CAN1_Receive_Msg(u8 *buf);							//接收数据
u8 command_stop(u8 Slave_Num);
u8 command_start(u8 Slave_Num);
u8 command_set_sampleRate(u8 Slave_Num,u32 sample_rate);

#endif


















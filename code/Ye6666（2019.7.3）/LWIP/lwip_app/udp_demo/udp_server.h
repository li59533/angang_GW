#ifndef __UDP_SERVER_H
#define __UDP_SERVER_H
#include "sys.h"
#include "includes.h"
#include "ye6271_common.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//NETCONN API��̷�ʽ��UDP���Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/8/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   


 
#define UDP_SERVER_RX_BUFSIZE		512	//����udp���������ݳ���
#define UDP_SERVER_PORT			8089	//����udp���ӵı��ض˿ں�

extern u8 udp_flag;		//UDP���ݷ��ͱ�־λ

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




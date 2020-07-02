#include "ye6271_common.h"
#include "command.h"
#include "delay.h"
#include "can.h"
#include "paramaters.h"

//返回非0是错误
u8 analy_cmd(unsigned char *sch,int slen)
{
	u16 *p16;
	u32 *p32;
	u16 cmd;
	int i;
	u16 sum;
	unsigned char *pcmd = sch;
	int len =slen;

	len -= sizeof(u16);
	sum = 0;
	p16 = (u16 *)pcmd;
	for(i=0;i<len/sizeof(u16);i++){
		sum += *p16;
		p16++;
	}
	if(sum != *p16){
		return 1;
	}

	p16 = (u16 *)pcmd;
	//T
	cmd = p16[0];
	if(cmd > MAX_COMMAND){
		return 1;
	}
	len -= sizeof(u16);
	pcmd += sizeof(u16);
	
	//L
	p32 = (u32 *)pcmd;
	len -= sizeof(u32);
	if( p32[0] != len ){
		return 1;
	}
	pcmd += sizeof(u32);

	//V
	switch( cmd ){
		case COMMAND_STOP:
			p16 = (u16 *)pcmd;
			if(len != sizeof(u16))
				return 1;
			for(i=0;i<SLAVE_MACHINE_NUM;i++){
				if(p16[0] & (1<<i) ){
					if(command_stop(i)){
						p16[0] &=( ~(1<<i) );
					}
				}
			}
			break;
		case COMMAND_START:
			p16 = (u16 *)pcmd;
			if(len != sizeof(u16))
				return 1;
			for(i=0;i<SLAVE_MACHINE_NUM;i++){
				if(p16[0] & (1<<i) ){
					if(command_start(i)){
						p16[0] &=( ~(1<<i) );
					}
				}
			}
			break;
		case COMMAND_SAMPLE_RATE_1:
			p16 = (u16 *)pcmd;
			pcmd += sizeof(u16);
			len -=sizeof(u16);
			p32 = (u32 *)pcmd;
			len -= sizeof(u32);
			for(i=0;i<SLAVE_MACHINE_NUM;i++){
				if(p16[0] & (1<<i) ){
					if( (len<0) || command_set_sampleRate(i,*p32) ){
						p16[0] &= (~(1<<i) );
					}
					{
						u32 uiTIMxCLK;
						u16 usPrescaler;
						u16 usPeriod;
						u32 _ulFreq;

						_ulFreq = *p32;
						uiTIMxCLK = SystemCoreClock / 2;
						_ulFreq*=2;
						if (_ulFreq < 3000)
						{
							usPrescaler = 100;
							usPeriod =  (uiTIMxCLK / 100) / _ulFreq;		/* 自动重装的值 */
						}
						else	/* 大于4K的频率，无需分频 */
						{
							usPrescaler = 1;					/* 分频比 = 1 */
							usPeriod = uiTIMxCLK / _ulFreq;	/* 自动重装的值 */
						}
						_ulFreq = uiTIMxCLK/(usPrescaler*usPeriod);
						*p32 = _ulFreq/2;
					}
					
					len -= sizeof(u32);
					p32++;
				}
			}
			break;
		case COMMAND_IP:
			if(len !=3*sizeof(u32))
				return 1;
			sys_para.ip[0] = pcmd[0];
			sys_para.ip[1] = pcmd[1];
			sys_para.ip[2] = pcmd[2];
			sys_para.ip[3] = pcmd[3];
			pcmd+=sizeof(u32);
			sys_para.netmask[0] = pcmd[0];
			sys_para.netmask[1] = pcmd[1];
			sys_para.netmask[2] = pcmd[2];
			sys_para.netmask[3] = pcmd[3];
			pcmd+=sizeof(u32);
			sys_para.gateway[0] = pcmd[0];
			sys_para.gateway[1] = pcmd[1];
			sys_para.gateway[2] = pcmd[2];
			sys_para.gateway[3] = pcmd[3];
			write_system_parameters(&sys_para);
			break;
		default:
			return 1;
	}

	pcmd = sch;
	len =slen;

	
	sum = 0;
	p16 = (u16 *)pcmd;
	for(i=0;i<(len-sizeof(u16))/sizeof(u16);i++){
		sum += *p16;
		p16++;
	}
	*p16 = sum;	
	return 0;
}





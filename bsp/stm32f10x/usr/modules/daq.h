#ifndef _DAQ_H
#define _DAQ_H

#include "sys_conf.h"
enum
{
	TARGET_MODE_RETURN=0,	//�ط�
	TARGET_MODE_SUPPLY,		//�ͷ�
	TARGET_MODE_REMOTE,		//Զ��
};
enum
{
		AVERAGE_TEMP_MODE =0 ,//ƽ���¶�
		MAX_TEMP_MODE,				//����¶�
};
int16_t get_current_temp(uint8_t type);

int16_t get_current_max_temp(uint8_t type);
int16_t get_current_min_temp(uint8_t type);
uint16_t get_current_hum(uint8_t type);


#endif

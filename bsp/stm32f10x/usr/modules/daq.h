#ifndef _DAQ_H
#define _DAQ_H

#include "sys_conf.h"
enum
{
	TARGET_MODE_RETURN=0,	//回风
	TARGET_MODE_SUPPLY,		//送风
	TARGET_MODE_REMOTE,		//远程
};
enum
{
		AVERAGE_TEMP_MODE =0 ,//平均温度
		MAX_TEMP_MODE,				//最大温度
};
int16_t get_current_temp(uint8_t type);

int16_t get_current_max_temp(uint8_t type);
int16_t get_current_min_temp(uint8_t type);
uint16_t get_current_hum(uint8_t type);


#endif

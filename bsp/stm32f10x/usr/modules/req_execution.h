#ifndef __REQ_EXE_H__
#define __REQ_EXE_H__
#include "stdint.h"

#define HUM_CURRENT_UNIT                  1.19
enum
{
		HUM_FSM_STATE_IDLE = 0,
		HUM_FSM_STATE_CHECK,
		HUM_FSM_STATE_WARM,
		HUM_FSM_STATE_DRAIN,//排水
		HUM_FSM_STATE_HUM,//加湿
		HUM_FSM_STATE_FILL,//注水
		HUM_FSM_STATE_FLUSH,//冲刷换水
		HUM_FSM_STARTCHECK_FILL,//上电检测注水	
		HUM_FSM_STARTCHECK_DRAIN,//上电检测排水	
};

enum
{
		HUM_TYPE_FIX,
		HUM_TYPE_P,
		HUM_TYPE_INFRARED,//红外加湿
		HUM_TYPE_OTHER,//外置加湿器
};






enum
{
		COMPRESSOR_FSM_STATE_IDLE=0,
		COMPRESSOR_FSM_STATE_INIT,
		COMPRESSOR_FSM_STATE_STARTUP,
		COMPRESSOR_FSM_STATE_NORMAL,
		COMPRESSOR_FSM_STATE_SHUTING,
		COMPRESSOR_FSM_STATE_STOP,
};

//EV状态机
enum
{
		EV_FSM_STATE_IDLE=0,
		EV_FSM_STATE_INIT,
		EV_FSM_STATE_STARTUP,
		EV_FSM_STATE_NORMAL,
		EV_FSM_STATE_SHUTING,
		EV_FSM_STATE_STOP,
};

//风机状态机
enum
{
		FSM_FAN_IDLE=0,
		FSM_FAN_INIT,
		FSM_FAN_START_UP,
		FSM_FAN_NORM,
		FSM_FAN_SHUT
};


enum
{
    WATERVALVE_FSM_STATE_STOP = 0,
    WATERVALVE_FSM_STATE_STARTUP,
    WATERVALVE_FSM_STATE_WARMUP,
    WATERVALVE_FSM_STATE_WARMUP_SHUTING,
    WATERVALVE_FSM_STATE_NORMAL,
    WATERVALVE_FSM_STATE_SHUTING,
};
//ICT
enum
{
		ICT_IDLE=0,
		ICT_START,
		ICT_TEST,//
		ICT_STOP,//
};
enum
{
		ICT_FSM_START=0,
		ICT_FSM_V12,
		ICT_FSM_V10,
		ICT_FSM_V33,//
		ICT_FSM_VFG,//
		ICT_FSM_VAO,//
};
#define ICT_V_MAX   30
#define ICT_V_MIN   15
#define ICT_I_MAX   20
#define ICT_VI_OFFSET   10
#define ICT_VSP_OFFSET   30

enum
{
		ICT_ST_NO=0,
		ICT_ST_START,
		ICT_ST_OK,//
		ICT_ST_ERR,//
};

enum
{
		FAN_MODE_FIX=0,//定速模式
		FAN_MODE_PRESS_DIFF,//压差模式
		FAN_MODE_AVR_RETURN,//回风平均
		FAN_MODE_AVR_SUPPLY,//送风平均
  	FAN_MODE_TEMP_DIFF,//温差平均
		FAN_MODE_MAX_RETURN,//回风热点
		FAN_MODE_MAX_SUPPLY,//送风热点
		FAN_MODE_TEMP_MAX_DIFF,//温差热点
    FAM_MODE_INV_COMP,//变频跟踪
};
void hum_capacity_calc(void);
void req_execution(int16_t target_req_temp,int16_t target_req_hum,int16_t target_req_fan);
void req_bitmap_op(uint8_t component_bpos, uint8_t action);
int16_t get_inv_comp_freq_down_signal(uint8_t Type);
void Ex_Fan_Ctrl_Temp(void);
#endif //__REQ_EXE_H__


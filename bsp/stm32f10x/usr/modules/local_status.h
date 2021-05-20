#ifndef __LOCAL_REG_H__
#define __LOCAL_REG_H__

#include "stdint.h"
#include "sys_conf.h"

enum
{
		RESET_REQ = 0,
		LOCAL_REQ,
		TEAM_REQ,
		TARGET_REQ,
		MAX_REQ,
		REQ_MAX_CNT,
};

enum
{
		T_REQ = 0,
		H_REQ,
		F_REQ,
		REQ_MAX_LEVEL,
};

//global FSM states definition
enum
{
		T_FSM_TOP_ID = 0,
		T_FSM_STANALONE_ID,
		T_FSM_TEAM_ID,
		H_FSM_MAX_ID_CNT
};

//global top-level FSM states definition
enum
{
		T_FSM_STATE_IDLE=0,
		T_FSM_STATE_STANDALONE,
		T_FSM_STATE_TEAM,
		T_FSM_MAX_CNT,
};

enum
{
		BITMAP_REQ=0,
		BITMAP_ALARM,
		BITMAP_MANUAL,
		BITMAP_FINAL,
		BITMAP_MASK,
		BITMAP_MAX_CNT,
};

enum
{
		FAN_FSM_STATE=0,
		COMPRESS_SIG_FSM_STATE,
		COMPRESS1_FSM_STATE,
		COMPRESS2_FSM_STATE,
		HEATER_FSM_STATE,
		DEHUMER_FSM_STATE,
		HUMIDIFIER_FSM_STATE,
    WATERVALVE_FSM_STATE,
		EV_FSM_STATE,
		L_FSM_STATE_MAX_NUM,
};
//风机类型
enum
{
		FAN_TPYE_AC=0,
		FAN_TPYE_EC
};

enum
{
		COMP_CONSTANT_FRE=0,//定频
		COMP_QABP,//变频
};

enum
{
		T_FSM_SIG_IDLE=0,
		T_FSM_SIG_STANDALONE,
		T_FSM_SIG_TEAM,
		T_FSM_SIG_SHUT
};

/*
@signal:	system operating mode 
	1:	power on signal
	2:	power down signal
	3:	sys on signal
	4:	sys down signal
*/

typedef struct
{
		int16_t p_saved;
		int16_t i_saved;
		int16_t req_saved;
}pid_reg_st;

typedef struct
{
    int16_t set_point;   //Desired Value
    double  proportion; //Proportional Const
    double  integral;   //Integral Const
    double  derivative; //Derivative Const
    int16_t last_error;  //Error[-1]
    int16_t prev_error;  //Error[-2]
}pid_param_st;

typedef struct
{
		pid_reg_st temp;
		pid_reg_st hum;
}pid_st;

typedef struct
{
		uint16_t 	Fan_Dehumer_Delay;//风机档位延时,Alair，20161113
		uint8_t   Fan_Dehumer_State;//风机档位,Alair，20161113
		uint8_t   Fan_Gear;//风机档位,Alair，20161113
		uint8_t   Fan_default_cnt;//异常风机个数
		uint8_t   Fan_Start;//风机启动
}Fan_st;

typedef struct 
{
		int16_t 	require[REQ_MAX_CNT][REQ_MAX_LEVEL];		
		uint16_t 	bitmap[BITMAP_MAX_CNT];
		int16_t 	ao_list[AO_MAX_CNT][BITMAP_MAX_CNT];
//    int16_t 	pwm_list[PWM_MAX_CNT][BITMAP_MAX_CNT];
		uint16_t 	comp_timeout[DO_MAX_CNT];
		uint16_t	t_fsm_state;
		uint16_t	t_fsm_signal;
		int16_t   ec_fan_diff_reg;
		int16_t   ec_fan_suc_temp;
		uint16_t  authen_cd;
		uint16_t 	comp_startup_interval;
		uint16_t	debug_flag;
		uint16_t	debug_tiemout;
		uint16_t 	l_fsm_state[L_FSM_STATE_MAX_NUM];
		pid_st		pid;
		Fan_st		Fan;
    uint16_t  watervalve_warmup_delay;
		uint8_t   Humer_Delay;//加湿延时,Alair，20161113
		uint8_t   Flush_State;//换水状态,Alair，20161113
		uint8_t   HumCheck;//加湿器上电快速自检,Alair，20200931
		uint8_t   Comp_State;//压缩机开关信号,Alair，20161125
		uint8_t   Comp_Delay;//变频压机滞后关闭时间
		uint16_t	u16Team_NOReq;//群控需求
		uint16_t	u16Uart_Timeout;
		uint16_t	u16EF_Flag;//外风机启动信号
		uint32_t	u32L_Time;//设置Unix时间
    uint8_t 	u8AlarmClose_dev; //关机		
    uint8_t 	u8ICT_PowerKey; //开始测试
    uint8_t 	u8ICT_Fsm; //测试
    uint8_t 	u8ICT_Delay; //测试
    uint8_t 	u8ICT_Start; //测试
		uint8_t 	u8ICT_Cnt[2]; //测试
}local_reg_st;

enum
{
		FAN_DEHUMER_IDLE= 0,
		FAN_DEHUMER_START ,
		FAN_DEHUMER_NORMAL,
};
//风机档位,Alair，20161113
enum
{
		HUM_FLUSH = 0,
		HUM_DRAIN,
		HUM_FILL,
};
//换水状态,Alair，20161113
enum
{
		FAN_GEAR_NO = 0,
		FAN_GEAR_LOW,
		FAN_GEAR_MID,
		FAN_GEAR_HIGH,
};
#define FAN_GEAR_START FAN_GEAR_LOW

#endif //__LOCAL_REG_H__

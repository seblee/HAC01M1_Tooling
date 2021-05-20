#ifndef __SYS_CONF
#define __SYS_CONF

#include "sys_def.h"
#include "alarms.h"
#include "string.h"

enum
{
    DO_HUM_BPOS = 0,  //加湿器
    DO_FILL_BPOS,     //注水阀
    DO_DRAIN_BPOS,    //排水阀
    DO_DEHUM1_BPOS,   //除湿阀
    DO_PHASE_P_BPOS,  //正相序
    DO_PHASE_N_BPOS,  //逆相序
    DO_COMP1_BPOS,    //压机1启动
    DO_COMP2_BPOS,    //压机2
    DO_PUMMP_BPOS,    //泵
    DO_HGBP_BPOS,     //卸载阀、外风机
    DO_EV1_BPOS,      //电子膨胀阀1
    DO_EV2_BPOS,      //电子膨胀阀2
    DO_FAN_BPOS,      //内风机
    DO_RH1_BPOS,      //电加热1
    DO_RH2_BPOS,      //电加热2
    DO_ALARM_BPOS,    //公共告警
    DO_FILLTER_DUMMY_BPOS,
    DO_FAN2_DUMMY_BPOS,
    DO_FAN3_DUMMY_BPOS,
    DO_FAN4_DUMMY_BPOS,
    DO_FAN5_DUMMY_BPOS,
    DO_FAN6_DUMMY_BPOS,
    DO_FAN7_DUMMY_BPOS,
    DO_FAN8_DUMMY_BPOS,
    DO_OUT_FAN_DUMMY_BPOS,
    DO_MAX_CNT,
};
#define DO_WATER_VALVE_DUMMY_BPOS DO_COMP1_BPOS

#define DO_FAN_LOW_BPOS DO_FAN_BPOS  //风机低档

#define DO_CONDENSE_BPOS DO_DEHUM1_BPOS  //冷凝器输出

// ICT测试
#define DO_POWER_DC5V_BPOS DO_COMP1_BPOS  //
#define DO_POWER_AC24V_BPOS DO_FAN_BPOS   //
#define DO_POWER_ON_BPOS DO_ALARM_BPOS    //

// application delay
/*
#define		MODBUS_MASTER_THREAD_DELAY  500
#define		MODBUS_SLAVE_THREAD_DELAY   1500
#define		TCOM_THREAD_DELAY	          1015
#define		TEAM_THREAD_DELAY           1020
#define		MBM_FSM_THREAD_DELAY				1025
#define		DI_THREAD_DELAY							1030
#define		DAQ_THREAD_DELAY						1035
#define		CORE_THREAD_DELAY						2100
#define		SURV_THREAD_DELAY						1045
#define		CPAD_THREAD_DELAY					  1050
#define		BKG_THREAD_DELAY            2200
#define		TESTCASE_THREAD_DELAY		    1060
*/
#define MODBUS_MASTER_THREAD_DELAY 500
#define MODBUS_SLAVE_THREAD_DELAY 2000
#define TCOM_THREAD_DELAY 1150
#define TEAM_THREAD_DELAY 1200
#define MBM_FSM_THREAD_DELAY 1250
#define DI_THREAD_DELAY 1300
#define DAQ_THREAD_DELAY 1350
#define CORE_THREAD_DELAY 3000
#define SURV_THREAD_DELAY 1400
#define CPAD_THREAD_DELAY 1600
#define BKG_THREAD_DELAY 2200
#define TESTCASE_THREAD_DELAY 1650

// enum
//{
//		AI_SENSOR1 = 0,
//		AI_SENSOR2,
//		AI_NTC1,
//		AI_NTC2,
//		AI_NTC3,
//		AI_NTC4,
//		AI_MAX_CNT
//};
//#define AI_SENSOR_NUM   	2		//传感器数量
//#define AI_NTC_NUM   			4		//NTC数量

//#define AI_WATER_VALVE_FB   	AI_SENSOR1		//水阀反馈信号
//#define AI_COOL_NTC1         AI_NTC3
//#define AI_COOL_NTC2         AI_NTC4

//#define AI_HI_PRESS_SENSOR1  	AI_SENSOR2		//回路1高压传感器
//#define AI_LO_PRESS_SENSOR1  	AI_SENSOR1		//回路1低压传感器

////ntc
//#define AI_RETURN_NTC1       AI_NTC1
//#define AI_SUPPLY_NTC1       AI_NTC2

//#define AI_COMP_RETURN1      AI_NTC3				//回路1回气温度
//#define AI_COMP_EXHAUST1     AI_NTC4				//回路1排气温度

enum
{
    AI_SENSOR1 = 0,
    AI_SENSOR2,
    AI_SENSOR3,
    AI_SENSOR4,
    AI_SENSOR5,
    AI_NTC1,
    AI_NTC2,
    AI_NTC3,
    AI_NTC4,
    AI_NTC5,
    AI_NTC6,
    AI_NTC7,
    AI_NTC8,
    AI_MAX_CNT
};

#define AI_MAX_CNT_M3 6              // M3 AI数量
#define AI_MAX_CNT_MR AI_MAX_CNT_M3  //实际AI数量

#define AI_SENSOR_NUM 5  //传感器数量
#define AI_NTC_NUM 8     // NTC数量

//#define AI_AIR_FLOW_DIFF			AI_SENSOR1		//静压差传感器
#define AI_WATER_FLOW AI_SENSOR2           //水流量传感器
#define AI_WATER_VALVE_FB AI_SENSOR3       //水阀反馈信号
#define AI_HI_PRESS_SENSOR1 AI_SENSOR4     //回路1高压传感器
#define AI_HI_PRESS_SENSOR2 AI_SENSOR5     //回路2高压传感器
#define AI_LO_PRESS_SENSOR1 AI_SENSOR3     //回路1低压传感器
#define AI_LO_PRESS_SENSOR2 AI_SENSOR2     //回路2低压
#define AI_MAJOR_WATER_FLOW AI_SENSOR2     //?
#define AI_HI_PRESS_CONDENSATE AI_SENSOR4  //冷凝压力即高压压力

// ntc
#define AI_RETURN_NTC1 AI_NTC1
#define AI_RETURN_NTC2 AI_NTC2

#define AI_SUPPLY_NTC1 AI_NTC3
#define AI_SUPPLY_NTC2 AI_NTC4

#define AI_COOL_NTC1 AI_NTC5
#define AI_COOL_NTC2 AI_NTC6

#define AI_COMP_RETURN1 AI_NTC5   //回路1回气温度
#define AI_COMP_EXHAUST1 AI_NTC6  //回路1排气温度
#define AI_COMP_RETURN2 AI_NTC7   //回路2回气温度
#define AI_COMP_EXHAUST2 AI_NTC8  //回路2排气温度

// enum
//{
//		AO_EC_FAN = 0,//EC风机
////		AO_PREV_1,
////		AO_WATER_VALVE,
//    AO_EX_FAN,//冷凝风机
//    AO_EC_COMPRESSOR,//变频压机
//		AO_MAX_CNT,
//		AO_INV_FAN, //虚拟输出 变频
//};

////#define AO_EX_FAN2    AO_PREV_1//冷凝风机2
//#define AO_WATER_VALVE    AO_EX_FAN//水阀
//#define ABNORMAL_VALUE    0x7FFF//异常值
enum
{
    AO_EC_FAN = 0,  // EC风机
    AO_PREV_1,
    AO_WATER_VALVE,
    //    AO_CONDENSATE,//冷凝风机
    AO_EX_FAN,         //冷凝风机
    AO_EC_COMPRESSOR,  //变频压机
    AO_MAX_CNT,
    AO_INV_FAN,  //虚拟输出 变频
};

#define AO_EX_FAN2 AO_PREV_1   //冷凝风机2
#define ABNORMAL_VALUE 0x7FFF  //异常值

//手动测试模式
enum
{
    MANUAL_TEST_UNABLE = 0,     //退出测试模式
    MANUAL_MODE_ENABLE = 0x01,  //手动模式
    TEST_MODE_ENABLE   = 0x02,  //测试模式
};
///////////////////////////////////////////////////////////////
// system configuration
///////////////////////////////////////////////////////////////

typedef struct
{
    uint16_t id;
    uint16_t* reg_ptr;
    int16_t min;
    uint16_t max;
    uint16_t dft;
    uint8_t permission;
    uint8_t rw;
    uint8_t (*chk_ptr)(uint16_t pram);
} conf_reg_map_st;

typedef struct
{
    uint16_t id;
    uint16_t* reg_ptr;
    uint16_t dft;
    // uint8_t		rw;
} sts_reg_map_st;

typedef struct
{
    uint16_t ain;
    uint16_t din[2];
    uint16_t din_pusl;
    uint16_t aout;
    uint16_t dout;
    uint16_t mb_comp;
    //		uint16_t  mb_TH;
    //		uint16_t  return_temp_mask;
    //		uint16_t  supply_temp_mask;
    //		uint16_t  cab_temp_mask;
    uint16_t din_bitmap_polarity[2];
    //    uint16_t  pwm_out;

} dev_mask_st;

typedef struct
{
    uint16_t voltage;
    uint16_t frequency;
    uint16_t power_mode;  // 1三相电还是0单相电
    uint16_t PH_Ver;      //电源板软件版本
    uint16_t PH_Vol;      //加湿器水位电压判断
} power_supply_st;

// system_set
/*
@operating_mode:	system operating mode
    0:	power-off
    1:	standalone
    2:	team
    3:	shut_down
@standalone_mode:
    0:	automatic
    1:	manual
    2:	testing
@team_mode:
    0:	mode1
    1:	mode2
    2:	mode3
    3:	mode4
@cool_type:
    0:	wind cool
    1:	water cool
    2:	chilled water
@cpad_baudrate:
    0:	9600
    1:	19200
    2:	38400
    3:	57600
@surv_baudrate:
    0:	9600
    1:	19200
@surv_addr:	1~32
*/
enum
{
    COOL_TYPE_MODULE_WIND = 0,  //常规风冷
    COOL_TYPE_MODULE_WATER,     //常规冷冻水
    COOL_TYPE_MODULE_MIX,
    COOL_TYPE_COLUMN_WIND,   //列间风冷
    COOL_TYPE_COLUMN_WATER,  //列间冷冻水
    COOL_TYPE_HUMIDITY = 9,  //恒湿机
};
enum
{
    POWER_MODE_THREBLE = 0,
    POWER_MODE_SINGLE,
    POWER_MODE_NONE,
};
#define ALARM_TOTAL_WORD 6
typedef struct
{
    uint16_t temp;
    uint16_t hum;
} temp_sensor_cali_st;

typedef struct
{
    uint16_t power_mode;                             // power-off or power-on
    uint16_t standalone_timer;                       // automatic, manual
    uint16_t cool_type;                              // cooling type
    uint16_t cpad_baudrate;                          // control pad communication baudrate
    uint16_t surv_baudrate;                          // surveillance communication baudrate
    uint16_t surv_addr;                              // surveillance communication address
    uint16_t diagnose_mode_en;                       // diagnose mode enalbe
    uint16_t alarm_bypass_en;                        // diagnose mode enalbe
    uint16_t testing_mode_en;                        // test mode enalbe
    uint16_t power_mode_mb_en;                       // modbuss power mode control enable
    uint16_t cancel_alarm_mb_en;                     // cancel all alarm enable
    uint16_t alarm_remove_bitmap[ALARM_TOTAL_WORD];  // reless alarm
    uint16_t ntc_cali[AI_NTC_NUM];                   // NTC cali
    uint16_t ai_cali[AI_SENSOR_NUM];                 // ai_ cali
    uint16_t LED_Num;                                // LED数量
    uint16_t Alarm_Beep;                             //消音
    temp_sensor_cali_st temp_sensor_cali[TEMP_HUM_SENSOR_NUM];
    uint16_t DI_Reuse;        // DI复用
    uint16_t u16Clear_ALARM;  //清除告警
    uint16_t u16Reset;        //恢复原始参数
    uint16_t u16Clear_RT;     //清除部件时间
} conf_general_st;

// status_set
/*
@permission_level: control pad accesssible user permission level
    0:	lowest
    1:	above lowest
    2:	below highest
    3:	highest
@running_mode: control pad accesssible user permission level
    0:	standalone_power-off
    1:	standalone_on
    2:	team_poweroff
    3:	team_power_on

@running_mode: control pad accesssible user permission level
    bit0:	fatal error
    bit1:	internal modbus bus communication error
    bit2:	survallance modbus bus communication error
    bit3:	can bus communication error
*/
enum
{
    SYS_ERR_INIT = 0,
    SYS_ERR_TEAM,
    SYS_ERR_MBM,
    SYS_ERR_MBS,
    SYS_ERR_CAN,
    SYS_ERR_CPAD,
};

enum
{
    GEN_STS_REG_NO = 0,
    MBM_COM_STS_REG_NO,
    WORK_MODE_STS_REG_NO,  //机组工作状态
    SENSOR_STS_REG_NO
};

enum
{
    ALARM_COM = 0,
    ALARM_NTC,
};

enum
{
    PWR_STS_BPOS = 0,
    FAN_STS_BPOS,
    HEATING_STS_BPOS,
    COOLING_STS_BPOS,
    HUMING_STS_BPOS,
    DEMHUM_STS_BPOS,
    COOL_VALVE_BPOS,
    TEAM_STANDALONE_STS_BPOS = 8,
    TEAM_STS_BPOS,
    ALARM_STUSE_BPOS = 14,
    ALARM_BEEP_BPOS  = 15  //告警、蜂鸣器位
};

typedef struct
{
    uint16_t permission_level;  // user authentication level
    uint16_t running_mode;      // automatic, manual or testing
    uint16_t sys_error_bitmap;  // system error status
} status_general_st;

enum
{
    P_ALOGORITHM = 0,
    PID_ALOGORITHM,
    FUZZY_ALOGORITHM
};

enum
{
    HUM_RELATIVE = 0,
    HUM_ABSOLUTE
};
enum
{
    WATER_LEVEL_NO = 0,         //水少
    WATER_LEVEL_LOW,            //低水位
    WATER_LEVEL_REPLENISHMENT,  //补水水位
    WATER_LEVEL_HIGH,           //高水位
    WATER_LEVEL_OVERFLOW,       //溢水
    WATER_LEVEL_OTHER,          //异常
};
// meter tem_hum
typedef struct
{
    uint16_t supply_air_temp;
    uint16_t return_air_temp;
    uint16_t remote_air_temp;
    uint16_t supply_air_max_temp;
    uint16_t return_air_max_temp;
    uint16_t remote_air_max_temp;
    uint16_t supply_air_hum;
    uint16_t return_air_hum;
    uint16_t remote_air_hum;
    uint16_t supply_air_min_temp;
    uint16_t Target_air_temp;
    uint16_t Target_air_hum;
    uint16_t Current_air_temp;
    uint16_t Current_air_hum;
    uint16_t Current_fan_temp;
} sys_tem_hum_st;
// algorithm
typedef struct
{
    uint16_t temp_calc_mode;
    uint16_t temp_ctrl_mode;
    uint16_t hum_ctrl_mode;
    uint16_t ctrl_target_mode;
    uint16_t supply_air_temp;
    uint16_t return_air_temp;
    uint16_t remote_air_temp;
    uint16_t supply_air_hum;
    uint16_t return_air_hum;
    uint16_t remote_air_hum;
    uint16_t temp_precision;
    uint16_t hum_precision;
    uint16_t temp_deadband;
    uint16_t hum_deadband;
    uint16_t sample_interval;
    uint16_t temp_integ;
    uint16_t temp_diff;
    uint16_t pid_action_max;
    uint16_t temp_req_out_max;
    uint16_t InOut_diff;
} algorithm_st;

// compressor
typedef struct
{
    uint16_t type;
    uint16_t dehum_level;
    uint16_t startup_delay;
    uint16_t stop_delay;
    uint16_t min_runtime;
    uint16_t min_stoptime;
    uint16_t startup_lowpress_shield;
    uint16_t alter_mode;
    uint16_t alter_time;
    uint16_t start_interval;
    uint16_t ev_ahead_start_time;
    uint16_t ev_Delay_shut_time;
    uint16_t speed_upper_lim;
    uint16_t speed_lower_lim;
    uint16_t ec_comp_start_req;
    uint16_t startup_freq;
    uint16_t high_press_threshold;
    uint16_t high_press_hyst;
    uint16_t ret_oil_period;
    uint16_t ret_oil_freq;
    uint16_t low_freq_switch_period;
    uint16_t low_freq_threshold;
    uint16_t step;
    uint16_t step_period;
    uint16_t Exhaust_temperature;            //排气温度
    uint16_t Exhaust_temperature_hystersis;  //排气温度回差
    uint16_t Dehumidity_Freq;
} compressor_st;

//降频
enum
{
    HIGH_PRESS = 0,
    EXHAUST_TEMP,
};

//高压采集取样
enum
{
    GET_ZERO = 0,
    GET_ONE,
    GET_TWO,
    GET_AVG,
};

typedef struct
{
    char inv_hipress_flag;
    char inv_hipress_tmp;
    char inv_hipress_stop_flag;
    uint32_t inv_start_time[3];
    uint32_t inv_stop_time[3];
    uint16 inv_alarm_counter;
    uint16_t avg_hi_press[4];  //高压压力
    uint8_t counter_hipress;
    uint8_t counter_hi_Temperature;
    uint16_t Inv_hi_Temperature[4];     //排气温度
    uint8_t Inv_hi_temp_Flag;           //排气温度降频标识
    uint8_t Inv_hi_temp_Count;          //排气温度计数
    uint8_t Inv_hi_temp_Stop;           //排气温度停机
    uint32_t Inv_hi_temp_Starttime[3];  //排气温度降频时间
    uint32_t Inv_hi_temp_Stoptime[3];   //排气温度降频恢复时间
} inv_compress_alarm_st;

// compressor
typedef struct
{
    uint16_t auto_mode_en;
    uint16_t max_opening;
    uint16_t min_opening;
    uint16_t set_opening;
    uint16_t start_req;
    //	uint16_t mod_priority;
    uint16_t action_delay;
    uint16_t temp_act_delay;
    //  uint16_t trace_mode;
    uint16_t act_threashold;
} watervalve_st;

// fan
typedef struct
{
    uint16_t type;
    uint16_t mode;
    uint16_t num;
    uint16_t adjust_step;
    uint16_t startup_delay;
    uint16_t cold_start_delay;
    uint16_t stop_delay;
    uint16_t set_speed;
    uint16_t min_speed;
    uint16_t max_speed;
    uint16_t dehum_ratio;
    uint16_t hum_min_speed;
    uint16_t set_flow_diff;
    uint16_t flow_diff_deadzone;
    uint16_t flow_diff_step;
    uint16_t flow_diff_delay;
    uint16_t target_suc_temp;
    uint16_t suc_temp_deadzone;
    uint16_t suc_temp_step;
    uint16_t suc_temp_delay;
    uint16_t noload_down;  //无负载风机降速使能
    uint16_t target_temp;
    uint16_t temp_dead_band;
    uint16_t temp_precision;
    uint16_t temp_add_fan_en;
    uint16_t tem_add_fan_delay;
    uint16_t inv_step;
    uint16_t fan_k;
    uint16_t CFM_Enable;  //风量显示使能
    uint16_t CFM_Para_A;  //风量参数A
    uint16_t CFM_Para_B;
    uint16_t CFM_Para_C;
} fan_st;

// heater
typedef struct
{
    uint16_t pwm_period1;
    uint16_t start_req;
    uint16_t pwm_period2;
} heater_st;

// humidifier
typedef struct
{
    uint16_t hum_type;  // 0额定加湿，1比例带加湿。
    uint16_t hum_en;
    uint16_t hum_cool_k;    //制冷加湿系数
    uint16_t hum_capacity;  //
    uint16_t hum_cap_type;
    uint16_t hum_real_cap;
    uint16_t flush_time;
    uint16_t flush_interval;
    uint16_t min_fill_interval;  //注水间隔
    uint16_t drain_timer;        //排水时间
    uint16_t fill_cnt;
    uint16_t hum_time;
    uint16_t current_percentage;
    uint16_t HumStart;  //加湿快速启动
    uint16_t min_percentage;
    uint16_t water_conductivity;
} humidifier_st;

// humidifier

// dehum_dev
typedef struct
{
    uint16_t stop_dehum_temp;
    uint16_t stop_dehum_diff;    //除湿回差
    uint16_t Dehumer_Superheat;  //除湿过热度
    uint16_t DehumPriority;      //
} dehum_st;
// team set
typedef struct
{
    uint16_t team_en;        // team enable
    uint16_t mode;           // team mode 0,1,2,3
    uint16_t addr;           // team id
    uint16_t baudrate;       // team communication baudrate
    uint16_t total_num;      // units number in the team
    uint16_t backup_num;     // backup units
    uint16_t rotate_period;  // upper byte:0:no rotate;1:daily;2:weekly;lower byte:week day(0:sunday,1:monday...)
    uint16_t rotate_time;    // upper byte:hour;lower byte:minite;
    uint16_t rotate_num;
    uint16_t rotate_manual;
    uint16_t cascade_enable;
    uint16_t team_fan_mode;
    uint16_t fault_power_en;
} team_st;

///////////////////////////////////////////////////////////////
// system status
///////////////////////////////////////////////////////////////
//#define AI_INPUT_MAX 	64

//#define	AI_SUPPLY_AIR_TEMP_1_POS		0
//#define	AI_SUPPLY_AIR_TEMP_2_POS		1
//#define	AI_RETURN_AIR_TEMP_1_POS		2
//#define	AI_RETURN_AIR_TEMP_2_POS		3
//#define	AI_SUPPLY_AIR_HUM_1_POS			4
//#define	AI_SUPPLY_AIR_HUM_2_POS			5
//#define	AI_RETURN_AIR_HUM_1_POS			6
//#define	AI_RETURN_AIR_HUM_2_POS			7
//#define	AI_AD_1_POS									8
//#define	AI_AD_2_POS									9
//#define	AI_AD_3_POS									10
//#define	AI_AD_4_POS									11
//#define	AI_AD_5_POS									12
//#define	AI_AD_6_POS									13
//#define	AI_AD_7_POS									14
//#define	AI_AD_8_POS									15
//#define	AI_AD_9_POS									16
//#define	AI_AD_10_POS								17
//#define	AI_HUM_CURRENT_POS					18
//#define	AI_HUM_CONDUCT_POS					19
//#define	AI_POWER_V_PA_POS						20
//#define	AI_POWER_V_PB_POS						21
//#define	AI_POWER_V_PC_POS						22
//#define	AI_POWER_CURR_POS						23
//#define	AI_POWER_FREQ_POS						24

// analog_in
typedef struct
{
    uint16_t ai_data[AI_MAX_CNT];
    uint64_t ai_mask;
} ain_st;

#define DI_COMP_1_HI_TEMP_POS ((uint32_t)0x00000001 << 0)
#define DI_COMP_2_HI_TEMP_POS ((uint32_t)0x00000001 << 1)
#define DI_COMP_1_LOW_TEMP_POS ((uint32_t)0x00000001 << 2)
#define DI_COMP_2_LOW_TEMP_POS ((uint32_t)0x00000001 << 3)
#define DI_COMP_1_DISC_TEMP_POS ((uint32_t)0x00000001 << 4)
#define DI_COMP_2_DISC_TEMP_POS ((uint32_t)0x00000001 << 5)
#define DI_FAN_1_OVF_POS ((uint32_t)0x00000001 << 6)
#define DI_FAN_2_OVF_POS ((uint32_t)0x00000001 << 7)
#define DI_FAN_3_OVF_POS ((uint32_t)0x00000001 << 8)
#define DI_AIR_LOSS_POS ((uint32_t)0x00000001 << 9)
#define DI_FILTER_CLOG_POS ((uint32_t)0x00000001 << 10)
#define DI_WATER_OVER_FLOW_POS ((uint32_t)0x00000001 << 11)
#define DI_RMT_SHUT_POS ((uint32_t)0x00000001 << 12)
#define DI_HUM_WATER_LV ((uint32_t)0x00000001 << 13)
#define DI_RESERVE_2_POS ((uint32_t)0x00000001 << 14)
#define DI_RESERVE_3_POS ((uint32_t)0x00000001 << 15)

#define ST_PWR_PA_AB_POS ((uint32_t)0x00000001 << 16)
#define ST_PWR_PB_AB_POS ((uint32_t)0x00000001 << 17)
#define ST_PWR_PC_AB_POS ((uint32_t)0x00000001 << 18)
#define ST_HUM_WL_H_POS ((uint32_t)0x00000001 << 19)
#define ST_HUM_HC_H_POS ((uint32_t)0x00000001 << 20)
#define ST_HUM_WQ_L_POS ((uint32_t)0x00000001 << 21)

// Digtal input status
/*
bit map:
bit0: 	compressor 1 hi temp valve
bit1: 	compressor 2 hi temp valve
bit2: 	compressor 1 low temp valve
bit3: 	compressor 2 low temp valve
bit4: 	compressor 1 discharge temp valve
bit5:		compressor 2 discharge temp valve
bit6: 	fan 1 overload valve
bit7: 	fan 2 overload valve
bit8:		fan 3 overload valve
bit9:		air lost valve
bit10:	filter clog valve
bit11:	water overflow valve
bit12:	remote shut valve
bit13:	reserve1
bit14:	reserve2
bit15:	reserve3

bit16:	power phase A error
bit17:	power phase B error
bit18:	power phase C error
bit19:	humidifier water level high
bit20:	humidifier heating current high
bit21:	humidifier conductivity low
*/

typedef struct
{
    uint32_t din_data;
    uint32_t din_mask;
} din_st;

///////////////////////////////////////////////////////////////
// system output status
///////////////////////////////////////////////////////////////

// analog_out
// this feature is not yet determined, reserve interface for future application
typedef struct
{
    int16_t ec_fan[3];
    int16_t vf_compressor[2];
    int16_t reserve_aout[2];
} aout_st;

// Digital output definition
typedef struct
{
    int16_t fan_out[MAX_FAN_NUM];
    int16_t compressor_out[MAX_COMPRESSOR_NUM];
    int16_t heater_out[MAX_HEATER_NUM];
    int16_t liq_val_bypass_out[MAX_COMPRESSOR_NUM];
    int16_t hot_gas_bypass_out[MAX_COMPRESSOR_NUM];
    int16_t humidifier_out;
    int16_t dehumidification_out;
    int16_t water_injection_out;
    int16_t common_alarm_out;
    int16_t scr_out;
    int16_t usr_out[DO_MAX_CNT];
} dout_st;

///////////////////////////////////////////////////////////////
// system log
///////////////////////////////////////////////////////////////
// alarm status
typedef struct
{
    int16_t alarm_id;
    time_t trigger_time;
} alarm_status_st;

// alarm history
typedef struct
{
    int16_t alarm_id;
    time_t trigger_time;
    time_t clear_time;
} alarm_history_st;

// alarm system runtime log, record components accumulative running time
/*
@comp_id:
    0:	compressor 1
    1:	compressor 2
    2:	fan 1
    3:	fan 2
    4:	fan 3
    5:	heater 1
    6:	heater 2
    7:	humidifier
@action:
    0:	deactivated
    1:	activated
@trigger_time:
    sys_time
*/

typedef struct
{
    int16_t comp1_runtime_day;
    int16_t comp1_runtime_min;
    int16_t comp2_runtime_day;
    int16_t comp2_runtime_min;
    int16_t fan1_runtime_day;
    int16_t fan1_runtime_min;
    int16_t fan2_runtime_day;
    int16_t fan2_runtime_min;
    int16_t fan3_runtime_day;
    int16_t fan3_runtime_min;
    int16_t heater1_runtime_day;
    int16_t heater1_runtime_min;
    int16_t heater2_runtime_day;
    int16_t heater2_runtime_min;
    int16_t humidifier_runtime_day;
    int16_t humidifier_runtime_min;
} sys_runtime_log_st;

// alarm system runtime log, record components change of output states
/*
@comp_id:
    0:	compressor 1
    1:	compressor 2
    2:	fan 1
    3:	fan 2
    4:	fan 3
    5:	heater 1
    6:	heater 2
    7:	humidifier
@action:
    0:	deactivated
    1:	activated
@trigger_time:
    sys_time
*/
typedef struct
{
    uint16_t comp_id;
    uint16_t action;
    time_t trigger_time;
    time_t clear_time;
} sys_status_log_st;

///////////////////////////////////////////////////////////////
// alarms definition
///////////////////////////////////////////////////////////////

// alarms: acl definition
/*
@id:			alarm id
@delay:		trigger&clear delay
@timeout:	delay timeout count down
@trigger_time:	alarm trigger time
@enable mode:	alarm enable mode
`0x00:		enable
`0x01:		suspend
`0x02:		forbid
@enable mask:	alarm enable mask
'0x03:	all mode enable
'0x02:	enable or forbid
'0x01:	enable or suspend
'0x00:	only enable
@alarm_param:	related paramter(eg. threshold)
@void (*alarm_proc): designated alarm routine check function
*/
// typedef struct alarm_acl_td
//{
//	uint16_t 					id;
//	uint16_t					delay;
//	uint16_t 					timeout;
//	uint16_t 					state;
//	time_t						trigger_time;
//	uint16_t					enable_mode;
//	uint16_t					enable_mask;
//	uint16_t 					alarm_param;
//	uint16_t (*alarm_proc)(struct alarm_acl_td* str_ptr);
//}alarm_acl_st;

typedef struct
{
    uint16_t id;
    uint16_t delay;
    uint16_t enable_mode;
    uint16_t alarm_param;
} alarm_acl_conf_st;

// typedef struct alarm_acl_td
//{
//	uint16_t 					id;
//	uint16_t 					state;
//	time_t						trigger_time;
//	uint16_t (*alarm_proc)(struct alarm_acl_td* str_ptr);
//}alarm_acl_status_st;

// team struct definition
// team status
/*
@temp:	sampled temperature
@hum:		sampled humidity
@alarm_sts:
    `bit0:cri_alarm:	critical alarm, which will cause online set to go offline
        0:	no alarm
        1:	alarm active
    `bit1:norm_alarm:	sampled temperature
        0:	no alarm
        1:	alarm active
    `bit2:hitemp_alarm:
        0:	no alarm
        1:	alarm active
    `bit7:alarm_flag:
        0:	no alarm
        1:	alarm active
@comp_num:
    `compressor_num:	number of compressors
    `heater_num:			number of heaters
@run_state:
    0:	standby
    1:	running
@offline_count:
    0:	standby
    1:	running
*/
typedef struct
{
    int16_t temp;
    int16_t hum;
    uint8_t alarm_sts;   // 0:critical_alarm;1:normal_alarm;2:hitemp_alarm;7:alarm_flag
    uint8_t comp_num;    //[3:0]:compressor_num;[7:4]:heater_num;
    uint8_t run_states;  // 7:run status;[6:0]:offline time count
} team_status_st;

// team configuration
/*
@temp_set:							set temperature
@temp_precision_set:		set temperature precisiton
@temp_deadband_set:			set temperature deadband
@hum_set:								set humidity
@hum_precision_set:			set humidity precisiton
@hum_deadband_set:			set humidity deadband
@hitemp_alarm_set:			set high temperature alarm threshold
@lotemp_alarm_set:			set low temperature alarm threshold
@hihum_alarm_set:				set high humidity alarm threshold
@lohum_alarm_set:				set low humidity alarm threshold
*/
typedef struct
{
    int16_t temp_set;
    int16_t temp_precision_set;
    int16_t temp_deadband_set;
    int16_t hum_set;
    int16_t hum_precision_set;
    int16_t hum_deadband_set;
    int16_t hitemp_alarm_set;
    int16_t lotemp_alarm_set;
    int16_t hihum_alarm_set;
    int16_t lohum_alarm_set;
} team_param_st;

// team requirement
/*
@temp_req:				temperature requirement
@hum_req:					humidity requirement
@temp_ctrl_mode:	temperature control mode set
    0:	P algorithm
    1:	PID algorithm
    2:	reserve
@hum_ctrl_mode:		humidity control mode set
    0:	P algorithm
    1:	reserve
@team_mode:			team operating mode set
    0:	standalone mode
    1:	average req mode
    2:	uni-direction mode
@run_enable:			run permission
    0:	disable
    1:	enable
*/
typedef struct
{
    int16_t temp_req;
    int16_t hum_req;
    int8_t temp_ctrl_mode;
    int8_t hum_ctrl_mode;
    int8_t team_mode;
    int8_t run_enable;
} team_req_st;

// system memory map
// system memory configuration map
typedef struct sys_conf_map
{
    int16_t id;
    void* str_ptr;
    int16_t length;
} sys_conf_map_st;

typedef struct sys_status_map
{
    int16_t id;
    int16_t* addr;
    int16_t length;
} sys_status_map_st;
typedef struct
{
    uint16_t ext_fan_cnt;
    uint16_t ext_fan_min_press;  //压力下限
    uint16_t ext_fan_max_press;  //压力上限
    uint16_t ext_fan_prop_start;
    uint16_t ext_fan_prop_band;
    uint16_t ext_fan_prop_hyst;
    uint16_t ext_fan_prop_plat;
    uint16_t ext_fan_min_speed;
    uint16_t ext_fan_max_speed;
    uint16_t ext_fan1_set_speed;
    uint16_t ext_fan2_set_speed;
} ext_fan_st;

typedef struct
{
    uint16_t PMINID;       //最小压力
    uint16_t PMAXID;       //最大压力
    uint16_t PSETID;       //压力下限
    uint16_t PBANDID;      //调节范围
    uint16_t PHYSTID;      //调节回差
    uint16_t Buff[5];      //内存错误
    uint16_t PSTEPID;      //保持值
    uint16_t MINSPEEDID;   //最小转速
    uint16_t MAXSPEEDID;   //最大转速
                           //		uint16_t Min_Speed;//最小转速
                           //		uint16_t Max_Speed;//最大转速
    uint16_t FAN_TYPE;     //风机类型
    uint16_t Start_Temp;   //启动温度,最低温度
    uint16_t Stop_Temp;    //停止温度
    uint16_t Max_Temp;     //最大温度
    uint16_t Hyster_Temp;  //温度回差
} Ex_Fan_st;

typedef struct
{
    uint16_t on_req;
    uint16_t hysterisis;
    uint16_t max_on_time;
    uint16_t min_off_time;
    //		uint16_t Hgbp_SkyLight;//0-天窗,1-HGBP
} hgbp_st;

typedef struct
{
    uint16_t mioor_alarm_tmp;
    uint16_t critical_alarm_tmp;
} ext_tmp_st;

typedef struct
{
    uint16_t Data;
    uint8_t Flag;
} Type_Conf;

//配置寄存器
typedef struct
{
    Type_Conf Set_Frequency;  //设置频率
} INV_Conf;

//配置寄存器
typedef struct
{
    Type_Conf Set_Dehumidity_Supperheart;  //除湿过热度
    Type_Conf Set_Freon;                   //制冷剂类型
    Type_Conf ntc_cali[2];                 //设置NTC温度校正值
    Type_Conf ai_cali[2];                  //设置压力校正值
    Type_Conf Set_Superheat[2];            //过热度
    Type_Conf Set_Manual_Mode;             //模式
    Type_Conf Set_Manual_Steps[2];         //步数
} EEV_Conf;

typedef struct
{
    Type_Conf Set_Speed_Mode;  //调速模式
    Type_Conf Set_Comm_Speed;  //速度
} EX_FAN_Conf;
// modbus master data structure
typedef struct
{
    INV_Conf INV;           //变频器
    EEV_Conf EEV;           //电子膨胀阀
    EX_FAN_Conf EX_FAN[2];  //外风机
} mbm_Conf_st;

typedef struct
{
    conf_general_st general;
    dev_mask_st dev_mask;
    algorithm_st algorithm;
    compressor_st compressor;
    watervalve_st water_valve;
    fan_st fan;
    heater_st heater;
    humidifier_st humidifier;
    dehum_st dehumer;
    team_st team;
    power_supply_st ac_power_supply;
    alarm_acl_conf_st alarm[ACL_TOTAL_NUM];
    //	ext_fan_st            ext_fan_inst;
    Ex_Fan_st Ex_Fan;
    //	hgbp_st								hgbp;
    ext_tmp_st ext_tmp;
    mbm_Conf_st mbm_Conf;
} config_st;

typedef struct
{
    uint16_t dev_sts;
    uint16_t conductivity;
    uint16_t hum_current;
    uint16_t water_level;
    uint16_t u16HumCurrent;
} mbm_hum_st;

typedef struct
{
    uint16_t dev_sts;
    uint16_t pa_volt;
    uint16_t pb_volt;
    uint16_t pc_volt;
    uint16_t freq;
    uint16_t pe_bitmap;  //相序
    uint16_t p_cur[3];
} mbm_pwr_st;

typedef struct
{
    uint16_t dev_sts;
    uint16_t temp;
    uint16_t hum;
} mbm_tnh_st;

typedef struct
{
    uint16_t Temp;
    uint16_t Hum;
} Com_tnh_st;

// modbus master data structure
typedef struct
{
    uint16_t dev_sts;
    uint16_t di_bit_map;
    uint16_t press;
    uint16_t temp;
    uint16_t fan_speed;
    uint16_t do_bit_map_r;
    uint16_t compress_speed_r;
    uint16_t do_bit_map_w;
    uint16_t compress_speed_w;
} mbm_outsidec_st;
// EX_FAN外风机
typedef struct
{
    //
    uint16_t Fre;             //频率
    uint16_t M_Voltage;       //母线电压
    uint16_t P_Current[3];    //相电流
    uint16_t P_Pressure[2];   //冷凝压力
    uint16_t Temp;            //温度
    uint16_t Humidity;        //湿度
    uint16_t V_Analog;        //模拟电压
    uint16_t Alarm;           //告警状态
    uint16_t DI;              //数字输入
    uint16_t DO;              //数字输出
    uint16_t PIM_Temp;        // PIM温度
    uint16_t Set_Speed_Mode;  //调速模式
    uint16_t Set_Comm_Speed;  //通信输出频率百分比
} mbm_EX_FAN_St;

// INV变频器状态寄存器
typedef struct
{
    //
    uint16_t Comm;         //通信设定值
    uint16_t Fre;          //频率
    uint16_t M_Voltage;    //母线电压
    uint16_t Out_Voltage;  //输出电压
    uint16_t Out_Current;  //输出电流
    uint16_t Out_Power;    //输出功率
    uint16_t Out_Torque;   //输出转矩
    uint16_t Speed;        //运行速度
    uint16_t DI;           //
    uint16_t DO;           //
    uint16_t AI[3];        // AI电压
    uint16_t Inv_Err;      //变频器故障
    uint16_t Comm_Err;     //通信故障
} mbm_INV_St;

// EEV变频器状态寄存器
typedef struct
{
    uint16_t run_mode;              //运行模式
    uint16_t status_bitmap;         // bit0 自动/手动 bit 1:双路/单路
    uint16_t alarm_num;             //状态映射位
    uint16_t alarm_bitmap;          //告警数
    uint16_t suction_temp_avg[2];   //吸气温度平均值
    uint16_t vapor_pressure1_avg;   //蒸发压力1平均值
    uint16_t vapor_temp1_avg;       //蒸发压力1转饱和温度的平均值
    uint16_t vapor_pressure2_avg;   //
    uint16_t vapor_temp2_avg;       //
    uint16_t superHeat[2];          //过热度的值
    uint16_t valve_opening_cur[2];  //阀的当前开度
    uint16_t valve_steps_cur[2];    //阀的当前步数
    uint16_t cur_steps_inc[2];      //阀1需要增加的步数
    uint16_t valve_ctrl_status[2];  //阀的控制状态
    uint16_t ain[4];                //蒸发压力和吸气温度的检测值
    //配置寄存器 0x13
    uint16_t Set_Dehumidity_Supperheart;  //除湿过热度
    uint16_t Set_Freon;                   //制冷剂类型
    uint16_t Set_ntc_cali[2];
    uint16_t Set_ai_cali[2];
    uint16_t Set_Superheat[2];     //过热度
    uint16_t Set_Manual_Mode;      //模式
    uint16_t Set_Manual_Steps[2];  //步数
} mbm_EEV_St;

typedef struct
{
    uint8_t Err_Master0[13];  //异常计数
} mbm_Error_St;
// modbus master data structure
typedef struct
{
    //	uint8_t         timer;
    mbm_hum_st hum;     // 4
    mbm_pwr_st pwr[2];  // 5
    //	mbm_outsidec_st outside_control;
    mbm_tnh_st tnh[TEMP_HUM_SENSOR_NUM];  // 16
    mbm_EX_FAN_St EX_FAN[2];
    mbm_EEV_St EEV;
    //	mbm_INV_St			INV;
    mbm_Error_St Err_M;
} mbm_sts_st;

typedef union
{
    time_t u32Systime;
    uint16_t u16Sys_time[2];
} S_time;
// system information
typedef struct
{
    uint16_t status_reg_num;
    uint16_t config_reg_num;
    uint16_t software_ver;
    uint16_t hardware_ver;
    uint16_t serial_no[4];
    uint16_t man_date[2];
    S_time Sys_Time;
} sys_info_st;

typedef struct
{
    uint16_t low;
    uint16_t high;
} run_time_st;

typedef struct
{
    uint16_t pwr_off_alarm;
    uint16_t critical_cnt;
    uint16_t major_cnt;
    uint16_t mioor_cnt;
    uint16_t total_cnt;
} alarm_state_cnt_st;

typedef struct
{
    uint16_t work_mode;
    uint16_t limit_day;
    uint16_t runing_day;
    uint16_t runing_hour;
    uint16_t runing_sec;
    uint16_t runing_State;
    uint8_t pass_word[4];
} work_mode_st;

typedef struct
{
    uint16_t Day;       //管控天数  1-4级管控天数及密码
    uint16_t Password;  //管控密码
    time_t Starttime;   //开始时刻
} Grade_st;

typedef struct
{
    uint16_t Grade_Manage;      //管控阶段
    uint16_t Password_Poweron;  //开机管控密码
                                //		uint16_t Password_Grade[4][2];//1-4级管控天数及密码
                                //		time_t   Start_Grade[4];//开始时刻
    Grade_st Password_Grade[4];
    uint16_t Remain_day;  //当前阶段剩余天数
    uint16_t Run_day;     //当前阶段运行天数
    uint16_t Run_hour;    //当前阶段运行小时
    uint16_t Run_second;  //当前阶段运行秒
    uint16_t Run_State;   //运行状态
} ControlPassword_st;

typedef enum
{
    RETURN_AIR_PLUSS_MODE = 0,
    SET_FAN_SPEED_MODE,
} return_air_mode_st;
typedef struct
{
    uint16_t timer;
    return_air_mode_st return_air_work_mode;
} return_air_sta_st;

typedef struct
{
    uint16_t u16Status;
    uint16_t u16Test;
    uint16_t u16Fsm;
    uint16_t u16Vout[10];
    uint16_t u16DI[5];
    uint16_t u16Buff[5];
} ICT_st;

typedef struct
{
    sys_info_st sys_info;
    status_general_st general;      // 3
    mbm_sts_st mbm;                 // 25
    uint16_t ain[AI_MAX_CNT];       // 10
    uint16_t aout[AO_MAX_CNT];      // 6
    uint16_t CFM;                   //总风量
                                    //    uint16_t							pwmout[PWM_MAX_CNT];						//2
    uint16_t din_bitmap[2];         // 1
    uint16_t dout_bitmap;           // 1
    uint16_t status_remap[4];       // 4
    uint16_t alarm_bitmap[6];       // 6
    uint16_t Alarm_COM_NTC_BIT[2];  // 2
    uint16_t flash_program_flag;    // 1
    run_time_st run_time[DO_MAX_CNT];
    alarm_state_cnt_st alarm_status_cnt;
    sys_tem_hum_st sys_tem_hum;
    work_mode_st sys_work_mode;
    uint16_t flow_diff_timer;
    return_air_sta_st return_air_status;
    inv_compress_alarm_st inv_compress_alarm;
    uint16_t Hum_Water_Level;            // 1
    ControlPassword_st ControlPassword;  // 5级密码管控
    uint16_t Ex_Fan_Speed[2];            // 2
    ICT_st ICT;
} status_st;

typedef struct
{
    config_st config;
    status_st status;
} sys_reg_st;

// yxq

////modebusó2?t???????ù
// typedef enum
//{
//	RETURN_TEM_HUM_SENSOR1_BPOS=0,
//	RETURN_TEM_HUM_SENSOR2_BPOS,
//	RETURN_TEM_HUM_SENSOR3_BPOS,
//	RETURN_TEM_HUM_SENSOR4_BPOS,
//	SUPPLY_TEM_HUM_SENSOR1_BPOS,
//	SUPPLY_TEM_HUM_SENSOR2_BPOS,
//	TEM_HUM_SENSOR_RESERVE1_BPOS,
//	TEM_HUM_SENSOR_RESERVE2_BPOS,
//	//
//	HUM_MODULE_BPOS,
//	POWER_MODULE_BPOS,
//	MBM_DEV_OSC_BPOS,//outside control board
//
//}DEV_MASK_MB_BPOS;

typedef enum
{
    RETURN_TEM_HUM_SENSOR_BPOS = 0,
    SUPPLY_TEM_HUM_SENSOR_BPOS,
    TEM_HUM_SENSOR3_BPOS,
    TEM_HUM_SENSOR4_BPOS,
    TEM_HUM_SENSOR5_BPOS,
    TEM_HUM_SENSOR6_BPOS,
    TEM_HUM_SENSOR7_BPOS,
    TEM_HUM_SENSOR8_BPOS,
    //
    HUM_MODULE_BPOS,
    POWER_MODULE_BPOS,
    EX_FAN_ADDR,       //
    EEV_ADDR,          //
    EX_FAN2_ADDR,      //
    MBM_DEV_OSC_BPOS,  // outside control board

} DEV_MASK_MB_BPOS;

#define RETURN_TEM_HUM_2_BPOS TEM_HUM_SENSOR3_BPOS  //回风2
#define SUPPLY_TEM_HUM_2_BPOS TEM_HUM_SENSOR4_BPOS  //送风2
#define RETURN_TEM_HUM_3_BPOS TEM_HUM_SENSOR4_BPOS  //回风3
#define SUPPLY_TEM_HUM_3_BPOS TEM_HUM_SENSOR5_BPOS  //送风3

typedef enum
{
    MBM_RETURN_TEM_HUM_SENSOR = 0x01,
    MBM_SUPPLY_TEM_HUM_SENSOR = 0x02,
    //
    MBM_HUM_MODULE   = 0x100,
    MBM_POWER_MODULE = 0x200,

} MB_DEV_MASK;

#endif  //	__SYS_CONF

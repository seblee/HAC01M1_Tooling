#include <rtthread.h>
#include <components.h>
#include "sys_conf.h"
#include "calc.h"
#include "sys_def.h"
#include "i2c_bsp.h"
#include "authentication.h"
#include "local_status.h"
#include "global_var.h"
#include "event_record.h"
#include "password.h"
#include "team.h"
#include "reg_map_check.h"
#include "mb_event_cpad.h"
#include "reg_map_check.h"
// configuration parameters
uint16_t conf_reg[CONF_REG_MAP_NUM];
uint16_t test_reg[CONF_REG_MAP_NUM];

typedef enum
{
    INIT_LOAD_USR = 0,
    INIT_LOAD_FACT,
    INIT_LOAD_DEBUT,
    INIT_LOAD_DEFAULT,
} init_state_em;

#define EE_FLAG_LOAD_USR 0xdf
#define EE_FLAG_LOAD_FACT 0x1b
#define EE_FLAG_LOAD_DFT 0x9b
#define EE_FLAG_LOAD_DEBUT 0xff

#define EE_FLAG_EMPTY 0x00
#define EE_FLAG_OK 0x01
#define EE_FLAG_ERROR 0x02

alarm_acl_conf_st g_alarm_acl_inst[MAX_ALARM_ACL_NUM];  // alarm check list declairation
sys_reg_st g_sys;                                       // global parameter declairation
local_reg_st l_sys;                                     // local status declairation

extern team_local_st team_local_inst;

// configuration register map declairation

const conf_reg_map_st conf_reg_map_inst[CONF_REG_MAP_NUM] = {
    // 			id			mapped registers																			 min					max					default				permission	r/w
    // chk_prt
    {0, &g_sys.config.general.power_mode, 0, 1, 0, 0, 1, NULL},
    {1, &g_sys.config.general.cool_type, 0, 9, COOL_TYPE_MODULE_WIND, 3, 1, NULL},
    //		{				1,			&g_sys.config.general.cool_type,												 0,					  9,
    //COOL_TYPE_COLUMN_WIND,						3,					1,      NULL    },
    {2, &g_sys.config.general.surv_baudrate, 0, 5, 1, 1, 1, NULL},
    {3, &g_sys.config.general.surv_addr, 1, 128, 1, 1, 1, NULL},
    {4, &g_sys.config.general.diagnose_mode_en, 0, 1, 0, 2, 1, diagnose_mode_chk},
    {5, &g_sys.config.general.alarm_bypass_en, 0, 1, 0, 3, 1, NULL},
    {MANUAL_TSET, &g_sys.config.general.testing_mode_en, 0, 1, 0, 4, 1, NULL},
    {7, &g_sys.config.general.power_mode_mb_en, 0, 1, 1, 3, 1, NULL},
    //		{				8,      &g_sys.config.dev_mask.din_bitmap_polarity[0],					 0,				  	0xffff,			0x4ACC,
    //3,					1,      NULL   	},// DI极性,恒湿机
    {8, &g_sys.config.dev_mask.din_bitmap_polarity[0], 0, 0xffff, 0x2C0, 3, 1, NULL},  // DI极性
    //		{				8,      &g_sys.config.dev_mask.din_bitmap_polarity[0],					 0,				  	0xffff,			0xFFFC,
    //3,					1,      NULL   	},// DI极性
    //		{				8,      &g_sys.config.dev_mask.din_bitmap_polarity[0],					 0,				  	0xffff,			0xAC0,
    //3,					1,      NULL   	},// DI极性,变频列间
    {9, &g_sys.config.dev_mask.din_bitmap_polarity[1], 0, 0xffff, 0x00, 3, 1, NULL},
    //		{				9,			&g_sys.config.dev_mask.din_bitmap_polarity[1],					 0,				  	0xffff,			0x03,
    //3,					1,      NULL   	},
    {10, &g_sys.config.dev_mask.ain, 0, 0xffff, 0x0000, 3, 1, NULL},
    //		{				10,			&g_sys.config.dev_mask.ain,													     0,				  	0xffff,			0x0408,
    //3,					1,      NULL   	},
    {11, &g_sys.config.dev_mask.din[0], 0, 0xffff, 0x03, 3, 1, NULL},  // DI屏蔽位
    {12, &g_sys.config.dev_mask.din[1], 0, 0xffff, 0x0000, 3, 1, NULL},
    {13, &g_sys.config.dev_mask.aout, 0, 0x003f, 0x0001, 3, 1, NULL},  //模拟输出
    //		{				14,			&g_sys.config.dev_mask.mb_comp,													 0,				  	0xFFFF,			0x0A01,
    //3,					1,      NULL   	},
    //		{				14,			&g_sys.config.dev_mask.mb_comp,													 0,				  	0xFFFF,			0x0201,
    //3,					1,      NULL   	},//通信位
    {14, &g_sys.config.dev_mask.mb_comp, 0, 0xFFFF, 0x1200, 3, 1, NULL},  //通信位
    {15, &g_sys.config.dev_mask.dout, 0, 0xffff, 0xB04F, 3, 1, NULL},
    //		{				15,			&g_sys.config.dev_mask.dout,														 0,				  	0xffff,			0x1F7,
    //3,				1,      NULL   	},//数字输出
    {16, &g_sys.config.ac_power_supply.power_mode, 0, 2, POWER_MODE_THREBLE, 2, 1, NULL},
    {17, &g_sys.config.ac_power_supply.voltage, 1000, 3500, 2200, 2, 1, NULL},
    {18, &g_sys.config.ac_power_supply.frequency, 500, 600, 500, 2, 1, NULL},
    {19, &g_sys.config.algorithm.temp_calc_mode, 0, 1, 0, 0, 1, NULL},
    {20, &g_sys.config.algorithm.remote_air_temp, 100, 450, 230, 1, 1, NULL},
    {21, &g_sys.config.algorithm.remote_air_hum, 0, 1000, 500, 1, 1, NULL},
    {22, &g_sys.config.algorithm.temp_ctrl_mode, 0, 1, 0, 1, 1, NULL},
    {23, &g_sys.config.algorithm.hum_ctrl_mode, 0, 1, 0, 1, 1, NULL},
    {24, &g_sys.config.algorithm.ctrl_target_mode, 0, 2, 0, 1, 1, NULL},
    {25, &g_sys.config.algorithm.supply_air_temp, 50, 350, 150, 1, 1, NULL},
    {26, &g_sys.config.algorithm.return_air_temp, 150, 450, 230, 1, 1, NULL},
    {27, &g_sys.config.algorithm.supply_air_hum, 0, 1000, 500, 1, 1, NULL},
    {28, &g_sys.config.algorithm.return_air_hum, 200, 900, 500, 1, 1, NULL},
    {29, &g_sys.config.algorithm.temp_precision, 1, 100, 10, 1, 1, NULL},
    //		{				29,			&g_sys.config.algorithm.temp_precision,									 1,			  	  100,				20,
    //1,					1,      NULL   	},
    {30, &g_sys.config.algorithm.hum_precision, 1, 150, 50, 1, 1, NULL},
    {31, &g_sys.config.algorithm.temp_deadband, 0, 50, 10, 1, 1, NULL},
    //		{				31,			&g_sys.config.algorithm.temp_deadband,									 0,				  	50,					5,
    //1,					1,      NULL   	},
    {32, &g_sys.config.algorithm.hum_deadband, 10, 100, 50, 1, 1, NULL},
    {33, &g_sys.config.algorithm.sample_interval, 1, 300, 25, 2, 1, NULL},
    {34, &g_sys.config.algorithm.temp_integ, 0, 9000, 90, 2, 1, NULL},
    {35, &g_sys.config.algorithm.temp_diff, 0, 9000, 5, 2, 1, NULL},
    {36, &g_sys.config.algorithm.pid_action_max, 0, 200, 100, 2, 1, NULL},
    {37, &g_sys.config.algorithm.temp_req_out_max, 0, 200, 20, 2, 1, NULL},
    //		{				38,			&g_sys.config.dev_mask.pwm_out,													 0,				  	0x0003,			0,
    //0,					1,      NULL   	},
    {38, NULL, 0, 0x0003, 0, 0, 1, NULL},
    {39, &g_sys.config.general.ntc_cali[0], 0, 0xffff, 0, 2, 1, NULL},
    {40, &g_sys.config.general.ntc_cali[1], 0, 0xffff, 0, 2, 1, NULL},
    {41, &g_sys.config.general.ntc_cali[2], 0, 0xffff, 0, 2, 1, NULL},
    {42, &g_sys.config.general.ntc_cali[3], 0, 0xffff, 0, 2, 1, NULL},
    {43, &g_sys.config.general.ntc_cali[4], 0, 0xffff, 0, 2, 1, NULL},
    {44, &g_sys.config.general.ntc_cali[5], 0, 0xffff, 0, 2, 1, NULL},
    {45, &g_sys.config.general.temp_sensor_cali[0].temp, 0, 0xffff, 0, 2, 1, NULL},
    {46, &g_sys.config.general.temp_sensor_cali[0].hum, 0, 0xffff, 0, 2, 1, NULL},
    {47, &g_sys.config.general.temp_sensor_cali[1].temp, 0, 0xffff, 0, 2, 1, NULL},
    {48, &g_sys.config.general.temp_sensor_cali[1].hum, 0, 0xffff, 0, 2, 1, NULL},
    {49, &g_sys.config.general.temp_sensor_cali[2].temp, 0, 0xffff, 0, 2, 1, NULL},
    {50, &g_sys.config.general.temp_sensor_cali[2].hum, 0, 0xffff, 0, 2, 1, NULL},
    {51, &g_sys.config.general.temp_sensor_cali[3].temp, 0, 0xffff, 0, 2, 1, NULL},
    {52, &g_sys.config.general.temp_sensor_cali[3].hum, 0, 0xffff, 0, 2, 1, NULL},
    {53, &g_sys.config.general.temp_sensor_cali[4].temp, 0, 0xffff, 0, 2, 1, NULL},
    {54, &g_sys.config.general.temp_sensor_cali[4].hum, 0, 0xffff, 0, 2, 1, NULL},
    {55, &g_sys.config.general.temp_sensor_cali[5].temp, 0, 0xffff, 0, 2, 1, NULL},
    {56, &g_sys.config.general.temp_sensor_cali[5].hum, 0, 0xffff, 0, 2, 1, NULL},
    {57, &g_sys.config.compressor.ret_oil_period, 60, 600, 180, 2, 1, NULL},
    {58, &g_sys.config.compressor.ret_oil_freq, 50, 100, 70, 2, 1, NULL},
    {59, &g_sys.config.dehumer.stop_dehum_temp, 50, 250, 180, 2, 1, NULL},
    {60, &g_sys.config.compressor.high_press_threshold, 200, 500, 400, 2, 1, NULL},
    {61, &g_sys.config.compressor.high_press_hyst, 0, 200, 40, 2, 1, NULL},
    {62, &g_sys.config.compressor.alter_mode, 0, 1, 1, 2, 1, NULL},
    {63, &g_sys.config.compressor.alter_time, 100, 1000, 100, 2, 1, NULL},
    {64, &g_sys.config.compressor.start_interval, 10, 600, 20, 2, 1, NULL},
    //		{				65,			&g_sys.config.compressor.type,										 	     0,				  	1,					0,
    //2,					1,      NULL   	},
    {65, &g_sys.config.compressor.type, 0, 1, COMP_CONSTANT_FRE, 2, 1, NULL},  //变频
    {66, &g_sys.config.compressor.step_period, 1, 600, 1, 2, 1, NULL},
    {67, &g_sys.config.compressor.startup_delay, 5, 600, 30, 2, 1, NULL},
    {68, &g_sys.config.compressor.stop_delay, 10, 90, 20, 2, 1, NULL},
    {69, &g_sys.config.compressor.min_runtime, 60, 600, 180, 2, 1, NULL},
    {70, &g_sys.config.compressor.min_stoptime, 60, 600, 180, 2, 1, NULL},
    {71, &g_sys.config.compressor.startup_lowpress_shield, 60, 600, 120, 2, 1, NULL},
    {72, &g_sys.config.compressor.speed_upper_lim, 50, 100, 80, 2, 1, comp_uper_spd_chk},
    {73, &g_sys.config.compressor.speed_lower_lim, 10, 70, 30, 2, 1, comp_low_spd_chk},
    {74, &g_sys.config.compressor.ec_comp_start_req, 0, 100, 30, 2, 1, NULL},
    {75, &g_sys.config.compressor.step, 1, 10, 1, 2, 1, NULL},
    {76, &g_sys.config.water_valve.auto_mode_en, 0, 1, 1, 2, 1, NULL},
    {77, &g_sys.config.water_valve.set_opening, 0, 100, 80, 2, 1, water_valve_set_chk},
    {78, &g_sys.config.water_valve.min_opening, 0, 100, 28, 2, 1, water_valve_min_chk},
    {79, &g_sys.config.water_valve.max_opening, 0, 100, 100, 2, 1, water_valve_max_chk},
    {80, &g_sys.config.water_valve.start_req, 1, 100, 10, 2, 1, NULL},
    {81, &g_sys.config.water_valve.action_delay, 1, 999, 5, 2, 1, NULL},
    {82, &g_sys.config.water_valve.temp_act_delay, 5, 999, 60, 2, 1, NULL},
    {83, &g_sys.config.water_valve.act_threashold, 1, 20, 5, 2, 1, NULL},
    {84, &g_sys.config.fan.target_suc_temp, 80, 250, 200, 1, 1, NULL},
    {85, &g_sys.config.fan.suc_temp_deadzone, 10, 150, 50, 1, 1, NULL},
    {86, &g_sys.config.fan.suc_temp_step, 1, 15, 5, 1, 1, NULL},
    {87, &g_sys.config.fan.suc_temp_delay, 30, 600, 30, 1, 1, NULL},
    {88, &g_sys.config.fan.type, 0, 1, FAN_TPYE_EC, 1, 1, NULL},
    {89, &g_sys.config.fan.mode, 0, 8, 0, 1, 1, NULL},
    //		{				90,			&g_sys.config.fan.num,																	 1,			    	8,					1,
    //2,					1,      NULL   	},
    {90, &g_sys.config.fan.num, 1, 8, 1, 2, 1, NULL},
    {91, &g_sys.config.fan.startup_delay, 1, 300, 5, 1, 1, NULL},
    {92, &g_sys.config.fan.stop_delay, 1, 300, 20, 1, 1, NULL},
    {93, &g_sys.config.fan.set_speed, 0, 100, 70, 2, 1, fan_set_spd_chk},
    {94, &g_sys.config.fan.min_speed, 0, 100, 50, 2, 1, fan_low_spd_chk},
    {95, &g_sys.config.fan.max_speed, 0, 100, 90, 2, 1, fan_uper_spd_chk},
    {96, &g_sys.config.fan.dehum_ratio, 20, 100, 80, 2, 1, NULL},
    {97, &g_sys.config.fan.hum_min_speed, 50, 80, 50, 2, 1, NULL},
    {98, &g_sys.config.fan.cold_start_delay, 20, 180, 40, 2, 1, NULL},
    {99, &g_sys.config.fan.set_flow_diff, 1, 20, 4, 2, 1, NULL},
    {100, &g_sys.config.fan.flow_diff_deadzone, 1, 10, 2, 2, 1, NULL},
    {101, &g_sys.config.fan.flow_diff_step, 1, 10, 3, 2, 1, NULL},
    {102, &g_sys.config.fan.flow_diff_delay, 5, 30, 10, 2, 1, NULL},
    {103, &g_sys.config.fan.noload_down, 0, 1, 1, 2, 1, NULL},
    {104, &g_sys.config.fan.adjust_step, 1, 10, 1, 2, 1, NULL},
    {105, &g_sys.config.humidifier.hum_type, 0, 3, 1, 2, 1, NULL},
    {106, &g_sys.config.humidifier.hum_cap_type, 0, 420, 50, 2, 1, NULL},
    //		{				107,		&g_sys.config.humidifier.flush_time,							       1,				  	120,				3,
    //2,					1,      NULL   	},
    {107, &g_sys.config.humidifier.flush_time, 1, 120, 30, 2, 1, NULL},
    //		{				108,		&g_sys.config.humidifier.flush_interval,						     1,					  120,				3,
    //2,					1,      NULL   	},
    {108, &g_sys.config.humidifier.flush_interval, 1, 120, 100, 2, 1, NULL},
    {109, &g_sys.config.humidifier.hum_en, 0, 1, 1, 2, 1, NULL},
    {110, &g_sys.config.humidifier.HumStart, 0, 1, 0, 2, 1, NULL},
    {111, &g_sys.config.humidifier.drain_timer, 40, 300, 120, 2, 1, NULL},
    {112, &g_sys.config.humidifier.fill_cnt, 5, 50, 8, 2, 1, NULL},
    //		{				113,		&g_sys.config.humidifier.hum_time,									     1,				    600,			  240,
    //2,					1,      NULL   	},
    {113, &g_sys.config.humidifier.hum_time, 1, 600, 40, 2, 1, NULL},
    {114, &g_sys.config.humidifier.current_percentage, 60, 100, 80, 2, 1, NULL},
    {115, &g_sys.config.humidifier.hum_cool_k, 60, 100, 60, 2, 1, NULL},
    {116, &g_sys.config.humidifier.min_percentage, 60, 75, 60, 2, 1, NULL},
    {117, &g_sys.config.humidifier.hum_capacity, 10, 420, 50, 2, 1, NULL},
    {118, &g_sys.config.compressor.low_freq_switch_period, 5, 600, 120, 2, 1, NULL},
    {119, &g_sys.config.compressor.low_freq_threshold, 20, 70, 50, 2, 1, NULL},
    {120, &g_sys.config.compressor.ev_ahead_start_time, 0, 180, 0, 2, 1, NULL},
    {121, &g_sys.config.compressor.ev_Delay_shut_time, 0, 180, 5, 2, 1, NULL},
    {122, &g_sys.config.compressor.startup_freq, 30, 100, 50, 2, 1, NULL},
    {123, &g_sys.config.humidifier.water_conductivity, 1, 300, 100, 2, 1, NULL},
    {124, &g_sys.config.fan.temp_add_fan_en, 0, 1, 0, 2, 1, NULL},
    {125, &g_sys.config.fan.tem_add_fan_delay, 0, 3600, 300, 2, 1, NULL},
    //		{				126,		&g_sys.config.hgbp.on_req,															 10,				  50,					30,
    //0,					1,      NULL   	},
    //		{				127,		&g_sys.config.hgbp.hysterisis,													 10,				  25,					15,
    //0,					1,      NULL   	},
    //		{				128,		&g_sys.config.hgbp.max_on_time,													 5,				  	300,				15,
    //0,					1,      NULL   	},
    //		{				129,		&g_sys.config.hgbp.min_off_time,												 30,				  900,				300,
    //0,					1,      NULL    },
    {126, NULL, 10, 50, 30, 0, 1, NULL},
    {127, NULL, 10, 25, 15, 0, 1, NULL},
    {128, NULL, 5, 300, 15, 0, 1, NULL},
    {129, NULL, 30, 900, 300, 0, 1, NULL},
    {130, &g_sys.config.team.team_en, 0, 1, 0, 1, 1, NULL},
    {131, &g_sys.config.team.mode, 0, 3, 0, 1, 1, NULL},
    {132, &g_sys.config.team.addr, 1, 32, 1, 1, 1, NULL},
    {133, &g_sys.config.team.baudrate, 0, 2, 0, 1, 1, NULL},
    {134, &g_sys.config.team.total_num, 2, 32, 2, 1, 1, team_total_num_chk},
    {135, &g_sys.config.team.backup_num, 0, 16, 0, 1, 1, team_back_num_chk},
    {136, &g_sys.config.team.rotate_period, 0, 9, 0, 1, 1, NULL},
    {137, &g_sys.config.team.rotate_time, 0, 0xffff, 0, 1, 1, NULL},
    {138, &g_sys.config.team.rotate_num, 0, 0, 0, 1, 1, NULL},
    {139, &g_sys.config.team.rotate_manual, 0, 0, 0, 1, 1, NULL},
    {140, &g_sys.config.team.cascade_enable, 0, 1, 0, 1, 1, NULL},
    {141, &g_sys.config.team.team_fan_mode, 0, 3, 0, 0, 1, NULL},
    {142, &g_sys.config.general.ai_cali[0], 0, 0xffff, 0, 2, 1, NULL},
    {143, &g_sys.config.general.ai_cali[1], 0, 0xffff, 0, 2, 1, NULL},
    {144, &g_sys.config.general.ai_cali[2], 0, 0xffff, 0, 2, 1, NULL},
    {145, &g_sys.config.fan.target_temp, 30, 400, 230, 0, 1, NULL},
    {146, &g_sys.config.fan.temp_dead_band, 10, 50, 15, 0, 1, NULL},
    {147, &g_sys.config.fan.temp_precision, 10, 50, 10, 0, 1, NULL},
    {148, &g_sys.config.general.ai_cali[3], 0, 0xffff, 0, 2, 1, NULL},
    {149, &g_sys.config.general.LED_Num, 0, 100, 0, 2, 1, NULL},
    {150, &g_sys.config.team.fault_power_en, 0, 1, 1, 2, 1, NULL},
    {151, NULL, 0, 3600, 0, 2, 1, NULL},
    {152, NULL, 0, 3600, 0, 2, 1, NULL},
    {153, NULL, 0, 3600, 0, 2, 1, NULL},
    {154, NULL, 0, 3600, 0, 2, 1, NULL},
    {155, NULL, 0, 3600, 0, 2, 1, NULL},
    {156, NULL, 0, 3600, 0, 2, 1, NULL},
    {157, NULL, 0, 3600, 0, 2, 1, NULL},
    {158, &g_sys.config.dehumer.stop_dehum_diff, 0, 100, 20, 2, 1, NULL},
    {159, NULL, 0, 3600, 0, 2, 1, NULL},
    {160, NULL, 0, 3600, 0, 2, 1, NULL},
    {161, NULL, 0, 3600, 0, 2, 1, NULL},
    {162, &g_sys.config.heater.pwm_period1, 10, 1000, 100, 2, 1, NULL},
    {163, &g_sys.config.heater.start_req, 0, 100, 50, 2, 1, NULL},
    {164, &g_sys.config.heater.pwm_period2, 10, 1000, 100, 2, 1, NULL},
    {165, &g_sys.config.general.cancel_alarm_mb_en, 0, 1, 0, 3, 1, NULL},
    {166, &g_sys.config.fan.fan_k, 0, 200, 100, 1, 1, NULL},
    {167, &g_sys.config.dev_mask.din_pusl, 0, 0xffff, 0x0000, 3, 1, NULL},  //列间变频
    //		{				167,		&g_sys.config.dev_mask.din_pusl,												 0,				  	0xffff,			0x0000,
    //3,					1,      NULL   	},
    {168, &l_sys.bitmap[BITMAP_MANUAL], 0, 0xffff, 0, 2, 1, NULL},
    {169, (uint16_t *)&l_sys.ao_list[AO_EC_FAN][BITMAP_MANUAL], 0, 100, 0, 0, 1, NULL},
    {170, (uint16_t *)&l_sys.ao_list[AO_EC_COMPRESSOR][BITMAP_MANUAL], 0, 100, 0, 0, 1, NULL},
    {171, NULL, 0, 100, 0, 0, 1, NULL},
    {172, NULL, 0, 100, 0, 0, 1, NULL},
    {173, (uint16_t *)&l_sys.ao_list[AO_EX_FAN][BITMAP_MANUAL], 0, 100, 0, 0, 1, NULL},
    //		{				174,		(uint16_t *)&l_sys.pwm_list[PWM_OUT0][BITMAP_MANUAL],		 0,			  		100,				0,
    //0,					1,      NULL   	},
    //		{				175,		(uint16_t *)&l_sys.pwm_list[PWM_OUT1][BITMAP_MANUAL],		 0,			  		100,				0,
    //0,					1,      NULL   	},
    {174, NULL, 0, 100, 0, 0, 1, NULL},
    {175, NULL, 0, 100, 0, 0, 1, NULL},
    {176, &g_sys.config.alarm[ACL_HI_TEMP_RETURN].enable_mode, 0, 7, 4, 2, 1, NULL},
    {177, &g_sys.config.alarm[ACL_HI_TEMP_RETURN].delay, 5, 100, 30, 2, 1, NULL},
    {178, &g_sys.config.alarm[ACL_HI_TEMP_RETURN].alarm_param, 200, 550, 350, 2, 1, return_temp_hiacl_chk},
    {179, &g_sys.config.alarm[ACL_LO_TEMP_RETURN].enable_mode, 0, 7, 4, 2, 1, NULL},
    {180, &g_sys.config.alarm[ACL_LO_TEMP_RETURN].delay, 5, 100, 30, 2, 1, NULL},
    {181, &g_sys.config.alarm[ACL_LO_TEMP_RETURN].alarm_param, 50, 280, 150, 2, 1, return_temp_lowacl_chk},
    {182, &g_sys.config.alarm[ACL_HI_HUM_RETURN].enable_mode, 0, 7, 4, 2, 1, NULL},
    {183, &g_sys.config.alarm[ACL_HI_HUM_RETURN].delay, 5, 100, 30, 2, 1, NULL},
    {184, &g_sys.config.alarm[ACL_HI_HUM_RETURN].alarm_param, 0, 2000, 800, 2, 1, return_hum_hiacl_chk},
    {185, &g_sys.config.alarm[ACL_LO_HUM_RETURN].enable_mode, 0, 7, 4, 2, 1, NULL},
    {186, &g_sys.config.alarm[ACL_LO_HUM_RETURN].delay, 5, 100, 30, 2, 1, NULL},
    {187, &g_sys.config.alarm[ACL_LO_HUM_RETURN].alarm_param, 0, 2000, 350, 2, 1, return_hum_lowacl_chk},
    {188, &g_sys.config.alarm[ACL_HI_TEMP_SUPPLY].enable_mode, 0, 7, 4, 2, 1, NULL},
    {189, &g_sys.config.alarm[ACL_HI_TEMP_SUPPLY].delay, 5, 100, 30, 2, 1, NULL},
    {190, &g_sys.config.alarm[ACL_HI_TEMP_SUPPLY].alarm_param, 200, 450, 350, 2, 1, supply_temp_hiacl_chk},
    {191, &g_sys.config.alarm[ACL_LO_TEMP_SUPPLY].enable_mode, 0, 7, 4, 2, 1, NULL},
    {192, &g_sys.config.alarm[ACL_LO_TEMP_SUPPLY].delay, 5, 100, 30, 2, 1, NULL},
    {193, &g_sys.config.alarm[ACL_LO_TEMP_SUPPLY].alarm_param, 50, 200, 150, 2, 1, supply_temp_lowacl_chk},
    {194, &g_sys.config.alarm[ACL_HI_HUM_SUPPLY].enable_mode, 0, 7, 2, 2, 1, NULL},
    {195, &g_sys.config.alarm[ACL_HI_HUM_SUPPLY].delay, 5, 100, 30, 2, 1, NULL},
    {196, &g_sys.config.alarm[ACL_HI_HUM_SUPPLY].alarm_param, 0, 2000, 990, 2, 1, supply_hum_hiacl_chk},
    {197, &g_sys.config.alarm[ACL_LO_HUM_SUPPLY].enable_mode, 0, 7, 2, 2, 1, NULL},
    {198, &g_sys.config.alarm[ACL_LO_HUM_SUPPLY].delay, 5, 100, 30, 2, 1, NULL},
    {199, &g_sys.config.alarm[ACL_LO_HUM_SUPPLY].alarm_param, 0, 2000, 350, 2, 1, supply_hum_lowacl_chk},
    {200, &g_sys.config.alarm[ACL_NTC_INVALID].enable_mode, 0, 7, 4, 2, 1, NULL},
    {201, &g_sys.config.alarm[ACL_NTC_INVALID].delay, 5, 100, 5, 2, 1, NULL},
    {202, &g_sys.config.alarm[ACL_NTC_INVALID].alarm_param, 0, 0, 0, 2, 1, NULL},
    //		{				203,		&g_sys.config.alarm[ACL_HI_TMP1].enable_mode,						 0,					  7,					5,
    //2,					1,      NULL   	},
    //		{				204,		&g_sys.config.alarm[ACL_HI_TMP1].delay,									 5,					  100,				10,
    //2,					1,      NULL   	},
    //		{				205,		&g_sys.config.alarm[ACL_HI_TMP1].alarm_param,					   100,					350,			  350,
    //2,					1,      NULL 	  },
    {203, NULL, 0, 3600, 0, 2, 1, NULL},
    {204, NULL, 0, 3600, 0, 2, 1, NULL},
    {205, NULL, 0, 3600, 0, 2, 1, NULL},
    //		{				206,		&g_sys.config.alarm[ACL_HI_TMP2].enable_mode,					   0,					  7,					4,
    //2,					1,      NULL    },
    //		{				207,		&g_sys.config.alarm[ACL_HI_TMP2].delay,								   5,					  100,				10,
    //2,					1,      NULL    },
    //		{				208,		&g_sys.config.alarm[ACL_HI_TMP2].alarm_param,					   0,					  0,			    0,
    //2,				  1,      NULL    },
    {206, NULL, 0, 3600, 0, 2, 1, NULL},
    {207, NULL, 0, 3600, 0, 2, 1, NULL},
    {208, NULL, 0, 3600, 0, 2, 1, NULL},
    {209, &g_sys.config.alarm[ACL_FAN_OVERLOAD1].enable_mode, 0, 7, 0, 2, 1, NULL},
    {210, &g_sys.config.alarm[ACL_FAN_OVERLOAD1].delay, 3, 100, 30, 2, 1, NULL},
    {211, &g_sys.config.alarm[ACL_FAN_OVERLOAD1].alarm_param, 1, 8, 1, 2, 1, NULL},
    {212, &g_sys.config.alarm[ACL_FAN_OVERLOAD2].enable_mode, 0, 7, 0, 2, 1, NULL},
    {213, &g_sys.config.alarm[ACL_FAN_OVERLOAD2].delay, 3, 100, 30, 2, 1, NULL},
    {214, &g_sys.config.alarm[ACL_FAN_OVERLOAD2].alarm_param, 0, 1000, 450, 2, 1, NULL},
    {215, &g_sys.config.alarm[ACL_FAN_OVERLOAD3].enable_mode, 0, 7, 4, 2, 1, NULL},
    {216, &g_sys.config.alarm[ACL_FAN_OVERLOAD4].enable_mode, 0, 7, 4, 2, 1, NULL},
    {217, &g_sys.config.alarm[ACL_FAN_OVERLOAD5].enable_mode, 0, 7, 4, 2, 1, NULL},
    {218, &g_sys.config.alarm[ACL_FAN_OVERLOAD6].enable_mode, 0, 7, 4, 2, 1, NULL},
    {219, &g_sys.config.alarm[ACL_FAN_OVERLOAD7].enable_mode, 0, 7, 4, 2, 1, NULL},
    {220, &g_sys.config.alarm[ACL_FAN_OVERLOAD8].enable_mode, 0, 7, 4, 2, 1, NULL},
    {221, &g_sys.config.alarm[ACL_FAN_OT1].enable_mode, 0, 7, 5, 2, 1, NULL},
    {222, &g_sys.config.alarm[ACL_FAN_OT1].delay, 0, 0, 0, 2, 1, NULL},
    {223, &g_sys.config.alarm[ACL_FAN_OT1].alarm_param, 0, 3650, 3650, 2, 1, NULL},
    {224, &g_sys.config.alarm[ACL_FAN_OT2].enable_mode, 0, 7, 5, 2, 1, NULL},
    {225, &g_sys.config.alarm[ACL_FAN_OT3].enable_mode, 0, 7, 5, 2, 1, NULL},
    {226, &g_sys.config.alarm[ACL_FAN_OT4].enable_mode, 0, 7, 5, 2, 1, NULL},
    {227, &g_sys.config.alarm[ACL_FAN_OT5].enable_mode, 0, 7, 5, 2, 1, NULL},
    {228, &g_sys.config.alarm[ACL_FAN_OT6].enable_mode, 0, 7, 5, 2, 1, NULL},
    {229, &g_sys.config.alarm[ACL_FAN_OT7].enable_mode, 0, 7, 5, 2, 1, NULL},
    {230, &g_sys.config.alarm[ACL_FAN_OT8].enable_mode, 0, 7, 5, 2, 1, NULL},

    {231, &g_sys.config.Ex_Fan.PMINID, 0, 500, 0, 2, 1, NULL},
    {232, &g_sys.config.Ex_Fan.PMAXID, 300, 500, 345, 2, 1, NULL},
    {233, &g_sys.config.Ex_Fan.PSETID, 100, 400, 135, 2, 1, NULL},
    {234, &g_sys.config.Ex_Fan.PBANDID, 10, 70, 45, 2, 1, NULL},
    {235, &g_sys.config.Ex_Fan.PHYSTID, 5, 100, 10, 2, 1, NULL},

    {236, &g_sys.config.Ex_Fan.PSTEPID, 30, 70, 50, 2, 1, NULL},
    {237, &g_sys.config.Ex_Fan.MINSPEEDID, 10, 100, 30, 2, 1, NULL},
    {238, &g_sys.config.Ex_Fan.MAXSPEEDID, 10, 100, 100, 2, 1, NULL},

    {239, &g_sys.config.Ex_Fan.FAN_TYPE, 0, 1, 0, 2, 1, NULL},
    {240, &g_sys.config.Ex_Fan.Start_Temp, 0, 0xffff, 100, 2, 1, NULL},
    {241, &g_sys.config.Ex_Fan.Hyster_Temp, 10, 300, 100, 2, 1, NULL},
    {242, &g_sys.config.Ex_Fan.Stop_Temp, 0, 0xffff, 80, 2, 1, NULL},
    {243, &g_sys.config.Ex_Fan.Max_Temp, 0, 0xffff, 130, 2, 1, NULL},
    {244, &g_sys.config.algorithm.InOut_diff, 0, 550, 50, 2, 1, NULL},

    {245, &g_sys.config.alarm[ACL_HI_PRESS1].enable_mode, 0, 7, 0, 2, 1, NULL},
    {246, &g_sys.config.alarm[ACL_HI_PRESS1].delay, 3, 100, 3, 2, 1, NULL},
    {247, &g_sys.config.alarm[ACL_HI_PRESS1].alarm_param, 0, 0, 0, 2, 1, NULL},
    {248, &g_sys.config.alarm[ACL_HI_LOCK1].enable_mode, 0, 7, 0, 2, 1, NULL},
    {249, &g_sys.config.alarm[ACL_HI_LOCK1].delay, 0, 0, 0, 2, 1, NULL},
    {250, &g_sys.config.alarm[ACL_HI_LOCK1].alarm_param, 0, 0, 0, 2, 1, NULL},
    {251, &g_sys.config.alarm[ACL_LO_PRESS1].enable_mode, 0, 7, 4, 2, 1, NULL},
    {252, &g_sys.config.alarm[ACL_LO_PRESS1].delay, 1, 500, 120, 2, 1, NULL},
    {253, &g_sys.config.alarm[ACL_LO_PRESS1].alarm_param, 60, 300, 120, 2, 1, NULL},
    {254, &g_sys.config.alarm[ACL_LO_LOCK1].enable_mode, 0, 7, 0, 2, 1, NULL},
    {255, &g_sys.config.alarm[ACL_LO_LOCK1].delay, 0, 0, 0, 2, 1, NULL},
    {256, &g_sys.config.alarm[ACL_LO_LOCK1].alarm_param, 0, 0, 0, 2, 1, NULL},
    {257, &g_sys.config.alarm[ACL_EXTMP1].enable_mode, 0, 7, 0, 2, 1, NULL},
    {258, &g_sys.config.alarm[ACL_EXTMP1].delay, 3, 100, 3, 2, 1, NULL},
    {259, &g_sys.config.alarm[ACL_EXTMP1].alarm_param, 0, 1300, 1100, 2, 1, NULL},
    {260, &g_sys.config.alarm[ACL_EXTMP_LOCK1].enable_mode, 0, 7, 0, 2, 1, NULL},
    {261, &g_sys.config.alarm[ACL_EXTMP_LOCK1].delay, 0, 0, 0, 2, 1, NULL},
    {262, &g_sys.config.alarm[ACL_EXTMP_LOCK1].alarm_param, 0, 0, 0, 2, 1, NULL},
    {263, &g_sys.config.alarm[ACL_SHORT_TERM1].enable_mode, 0, 7, 4, 2, 1, NULL},
    {264, &g_sys.config.alarm[ACL_SHORT_TERM1].delay, 0, 0, 0, 2, 1, NULL},
    {265, &g_sys.config.alarm[ACL_SHORT_TERM1].alarm_param, 0, 0, 0, 2, 1, NULL},
    {266, &g_sys.config.alarm[ACL_COMPRESSOR_OT1].enable_mode, 0, 7, 5, 2, 1, NULL},
    {267, &g_sys.config.alarm[ACL_COMPRESSOR_OT1].delay, 0, 0, 0, 2, 1, NULL},
    {268, &g_sys.config.alarm[ACL_COMPRESSOR_OT1].alarm_param, 1800, 7300, 3650, 2, 1, NULL},
    {269, &g_sys.config.alarm[ACL_INV_HIPRESS].enable_mode, 0, 7, 0, 2, 1, NULL},
    {270, &g_sys.config.alarm[ACL_INV_HIPRESS].delay, 5, 100, 10, 2, 1, NULL},
    {271, &g_sys.config.alarm[ACL_INV_HIPRESS].alarm_param, 0, 1000, 430, 2, 1, NULL},
    {272, &g_sys.config.alarm[ACL_HI_PRESS2].enable_mode, 0, 7, 0, 2, 1, NULL},
    {273, &g_sys.config.alarm[ACL_HI_PRESS2].delay, 3, 100, 3, 2, 1, NULL},
    {274, &g_sys.config.alarm[ACL_HI_PRESS2].alarm_param, 0, 0, 0, 2, 1, NULL},
    {275, &g_sys.config.alarm[ACL_HI_LOCK2].enable_mode, 0, 7, 0, 2, 1, NULL},
    {276, &g_sys.config.alarm[ACL_HI_LOCK2].delay, 0, 0, 0, 2, 1, NULL},
    {277, &g_sys.config.alarm[ACL_HI_LOCK2].alarm_param, 0, 0, 0, 2, 1, NULL},
    {278, &g_sys.config.alarm[ACL_LO_PRESS2].enable_mode, 0, 7, 4, 2, 1, NULL},
    {279, &g_sys.config.alarm[ACL_LO_PRESS2].delay, 3, 500, 120, 2, 1, NULL},
    {280, &g_sys.config.alarm[ACL_LO_PRESS2].alarm_param, 60, 300, 120, 2, 1, NULL},
    {281, &g_sys.config.alarm[ACL_LO_LOCK2].enable_mode, 0, 7, 0, 2, 1, NULL},
    {282, &g_sys.config.alarm[ACL_LO_LOCK2].delay, 0, 0, 0, 2, 1, NULL},
    {283, &g_sys.config.alarm[ACL_LO_LOCK2].alarm_param, 0, 0, 0, 2, 1, NULL},
    {284, &g_sys.config.alarm[ACL_EXTMP2].enable_mode, 0, 7, 4, 2, 1, NULL},
    {285, &g_sys.config.alarm[ACL_EXTMP2].delay, 3, 100, 3, 2, 1, NULL},
    {286, &g_sys.config.alarm[ACL_EXTMP2].alarm_param, 0, 0, 0, 2, 1, NULL},
    {287, &g_sys.config.alarm[ACL_EXTMP_LOCK2].enable_mode, 0, 7, 0, 2, 1, NULL},
    {288, &g_sys.config.alarm[ACL_EXTMP_LOCK2].delay, 0, 0, 0, 2, 1, NULL},
    {289, &g_sys.config.alarm[ACL_EXTMP_LOCK2].alarm_param, 0, 0, 0, 2, 1, NULL},
    {290, &g_sys.config.alarm[ACL_SHORT_TERM2].enable_mode, 0, 7, 4, 2, 1, NULL},
    {291, &g_sys.config.alarm[ACL_SHORT_TERM2].delay, 0, 0, 0, 2, 1, NULL},
    {292, &g_sys.config.alarm[ACL_SHORT_TERM2].alarm_param, 0, 0, 0, 2, 1, NULL},
    {293, &g_sys.config.alarm[ACL_COMPRESSOR_OT2].enable_mode, 0, 7, 5, 2, 1, NULL},
    {294, &g_sys.config.alarm[ACL_COMPRESSOR_OT2].delay, 0, 0, 0, 2, 1, NULL},
    {295, &g_sys.config.alarm[ACL_COMPRESSOR_OT2].alarm_param, 1800, 7300, 3650, 2, 1, NULL},
    {296, &g_sys.config.alarm[ACL_HUM_DEFAULT].enable_mode, 0, 7, 4, 2, 1, NULL},
    {297, &g_sys.config.alarm[ACL_HUM_DEFAULT].delay, 0, 1, 1, 2, 1, NULL},
    {298, &g_sys.config.alarm[ACL_HUM_DEFAULT].alarm_param, 0, 600, 460, 2, 1, NULL},
    {299, &g_sys.config.alarm[ACL_HUM_OCURRENT].enable_mode, 0, 7, 4, 2, 1, NULL},
    {300, &g_sys.config.alarm[ACL_HUM_OCURRENT].delay, 3, 100, 3, 2, 1, NULL},
    {301, &g_sys.config.alarm[ACL_HUM_OCURRENT].alarm_param, 1000, 30000, 9000, 2, 1, NULL},
    {302, &g_sys.config.alarm[ACL_HUM_HI_LEVEL].enable_mode, 0, 7, 2, 2, 1, NULL},
    {303, &g_sys.config.alarm[ACL_HUM_HI_LEVEL].delay, 5, 100, 5, 2, 1, NULL},
    {304, &g_sys.config.alarm[ACL_HUM_HI_LEVEL].alarm_param, 0, 0, 0, 2, 1, NULL},
    {305, &g_sys.config.alarm[ACL_HUM_LO_LEVEL].enable_mode, 0, 7, 4, 2, 1, NULL},
    {306, &g_sys.config.alarm[ACL_HUM_LO_LEVEL].delay, 1, 1000, 5, 2, 1, NULL},
    {307, &g_sys.config.alarm[ACL_HUM_LO_LEVEL].alarm_param, 5, 50, 40, 2, 1, NULL},
    {308, &g_sys.config.alarm[ACL_HUM_OT].enable_mode, 0, 7, 2, 2, 1, NULL},
    {309, &g_sys.config.alarm[ACL_HUM_OT].delay, 0, 0, 0, 2, 1, NULL},
    {310, &g_sys.config.alarm[ACL_HUM_OT].alarm_param, 4, 366, 30, 2, 1, NULL},
    {311, &g_sys.config.alarm[ACL_WATER_ELEC_HI].enable_mode, 0, 7, 2, 2, 1, NULL},
    {312, &g_sys.config.alarm[ACL_WATER_ELEC_HI].delay, 5, 100, 5, 2, 1, NULL},
    {313, &g_sys.config.alarm[ACL_WATER_ELEC_HI].alarm_param, 600, 2000, 1000, 2, 1, water_elec_hi_chk},
    {CLEAR_ALARM, &g_sys.config.general.u16Clear_ALARM, 0, 0xffff, 0, 2, 1, NULL},
    {FACTORY_RESET, &g_sys.config.general.u16Reset, 0, 0xffff, 0, 2, 1, NULL},
    {CLEAR_RT, &g_sys.config.general.u16Clear_RT, 0, 0xffff, 0, 2, 1, NULL},
    {317, &g_sys.config.alarm[ACL_HEATER_OD].enable_mode, 0, 7, 0, 2, 1, NULL},
    {318, &g_sys.config.alarm[ACL_HEATER_OD].delay, 3, 100, 3, 2, 1, NULL},
    {319, &g_sys.config.alarm[ACL_HEATER_OD].alarm_param, 0, 0, 0, 2, 1, NULL},
    {320, &g_sys.config.alarm[ACL_HEATER_OT1].enable_mode, 0, 7, 5, 2, 1, NULL},
    {321, &g_sys.config.alarm[ACL_HEATER_OT1].delay, 0, 0, 0, 2, 1, NULL},
    {322, &g_sys.config.alarm[ACL_HEATER_OT1].alarm_param, 1800, 5000, 3650, 2, 1, NULL},
    {323, &g_sys.config.alarm[ACL_HEATER_OT2].enable_mode, 0, 7, 5, 2, 1, NULL},
    {324, &g_sys.config.alarm[ACL_HEATER_OT2].delay, 0, 0, 0, 2, 1, NULL},
    {325, &g_sys.config.alarm[ACL_HEATER_OT2].alarm_param, 1800, 5000, 3650, 2, 1, NULL},
    //		{			  326,    &g_sys.config.alarm[ACL_WATERPAN].enable_mode,		 				0,					  7,					0,
    //2,					1,      NULL   	},
    //		{			  327,    &g_sys.config.alarm[ACL_WATERPAN].delay,					 				1,					100,					5,
    //2,					1,      NULL   	},
    //		{			  328,    &g_sys.config.alarm[ACL_WATERPAN].alarm_param,		 				0,					  1,		    	0,
    //2,					1,      NULL   	},
    {326, NULL, 0, 3600, 0, 2, 1, NULL},
    {327, NULL, 0, 3600, 0, 2, 1, NULL},
    {328, NULL, 0, 3600, 0, 2, 1, NULL},
    {329, &g_sys.config.general.Alarm_Beep, 0, 1, 0, 2, 1, NULL},
    {330, &g_sys.config.alarm[ACL_POWER_LOSS].enable_mode, 0, 7, 5, 2, 1, NULL},
    {331, &g_sys.config.alarm[ACL_POWER_LOSS].delay, 3, 15, 3, 2, 1, NULL},
    {332, &g_sys.config.alarm[ACL_POWER_LOSS].alarm_param, 0, 5000, 1540, 2, 1, NULL},
    {333, &g_sys.config.alarm[ACL_POWER_EP].enable_mode, 0, 7, 4, 2, 1, NULL},
    {334, &g_sys.config.alarm[ACL_POWER_EP].delay, 3, 100, 10, 2, 1, NULL},
    {335, &g_sys.config.alarm[ACL_POWER_EP].alarm_param, 0, 0, 0, 2, 1, NULL},
    {336, &g_sys.config.alarm[ACL_POWER_HI_FD].enable_mode, 0, 7, 4, 2, 1, NULL},
    {337, &g_sys.config.alarm[ACL_POWER_HI_FD].delay, 3, 100, 10, 2, 1, NULL},
    {338, &g_sys.config.alarm[ACL_POWER_HI_FD].alarm_param, 0, 1000, 525, 2, 1, power_frq_hi_chk},
    {339, &g_sys.config.alarm[ACL_POWER_LO_FD].enable_mode, 0, 7, 4, 2, 1, NULL},
    {340, &g_sys.config.alarm[ACL_POWER_LO_FD].delay, 3, 100, 10, 2, 1, NULL},
    {341, &g_sys.config.alarm[ACL_POWER_LO_FD].alarm_param, 0, 1000, 475, 2, 1, power_frq_lo_chk},
    {342, &g_sys.config.alarm[ACL_POWER_A_HIGH].enable_mode, 0, 7, 4, 2, 1, NULL},
    {343, &g_sys.config.alarm[ACL_POWER_A_HIGH].delay, 3, 100, 10, 2, 1, NULL},
    {344, &g_sys.config.alarm[ACL_POWER_A_HIGH].alarm_param, 0, 5000, 2530, 2, 1, power_a_hi_chk},
    {345, &g_sys.config.alarm[ACL_POWER_B_HIGH].enable_mode, 0, 7, 4, 2, 1, NULL},
    {346, &g_sys.config.alarm[ACL_POWER_B_HIGH].delay, 3, 100, 10, 2, 1, NULL},
    {347, &g_sys.config.alarm[ACL_POWER_B_HIGH].alarm_param, 0, 5000, 2530, 2, 1, power_b_hi_chk},
    {348, &g_sys.config.alarm[ACL_POWER_C_HIGH].enable_mode, 0, 7, 4, 2, 1, NULL},
    {349, &g_sys.config.alarm[ACL_POWER_C_HIGH].delay, 3, 100, 10, 2, 1, NULL},
    {350, &g_sys.config.alarm[ACL_POWER_C_HIGH].alarm_param, 0, 5000, 2530, 2, 1, power_c_hi_chk},
    {351, &g_sys.config.alarm[ACL_POWER_A_LOW].enable_mode, 0, 7, 4, 2, 1, NULL},
    {352, &g_sys.config.alarm[ACL_POWER_A_LOW].delay, 3, 100, 10, 2, 1, NULL},
    {353, &g_sys.config.alarm[ACL_POWER_A_LOW].alarm_param, 0, 5000, 1870, 2, 1, power_a_lo_chk},
    {354, &g_sys.config.alarm[ACL_POWER_B_LOW].enable_mode, 0, 7, 4, 2, 1, NULL},
    {355, &g_sys.config.alarm[ACL_POWER_B_LOW].delay, 3, 100, 10, 2, 1, NULL},
    {356, &g_sys.config.alarm[ACL_POWER_B_LOW].alarm_param, 0, 5000, 1870, 2, 1, power_b_lo_chk},
    {357, &g_sys.config.alarm[ACL_POWER_C_LOW].enable_mode, 0, 7, 4, 2, 1, NULL},
    {358, &g_sys.config.alarm[ACL_POWER_C_LOW].delay, 3, 100, 10, 2, 1, NULL},
    {359, &g_sys.config.alarm[ACL_POWER_C_LOW].alarm_param, 0, 5000, 1870, 2, 1, power_c_lo_chk},
    {360, &g_sys.config.alarm[ACL_POWER_A_OP].enable_mode, 0, 7, 4, 2, 1, NULL},
    {361, &g_sys.config.alarm[ACL_POWER_A_OP].delay, 3, 100, 10, 2, 1, NULL},
    {362, &g_sys.config.alarm[ACL_POWER_A_OP].alarm_param, 0, 5000, 132, 2, 1, NULL},
    {363, &g_sys.config.alarm[ACL_POWER_B_OP].enable_mode, 0, 7, 4, 2, 1, NULL},
    {364, &g_sys.config.alarm[ACL_POWER_B_OP].delay, 3, 100, 10, 2, 1, NULL},
    {365, &g_sys.config.alarm[ACL_POWER_B_OP].alarm_param, 0, 5000, 132, 2, 1, NULL},
    {366, &g_sys.config.alarm[ACL_POWER_C_OP].enable_mode, 0, 7, 4, 2, 1, NULL},
    {367, &g_sys.config.alarm[ACL_POWER_C_OP].delay, 3, 100, 10, 2, 1, NULL},
    {368, &g_sys.config.alarm[ACL_POWER_C_OP].alarm_param, 0, 5000, 132, 2, 1, NULL},
    {369, &g_sys.config.alarm[ACL_AIR_LOSS].enable_mode, 0, 7, 0, 2, 1, NULL},
    {370, &g_sys.config.alarm[ACL_AIR_LOSS].delay, 1, 100, 10, 2, 1, NULL},
    {371, &g_sys.config.alarm[ACL_AIR_LOSS].alarm_param, 20, 600, 60, 2, 1, NULL},
    {372, &g_sys.config.alarm[ACL_FILTER_OT].enable_mode, 0, 7, 4, 2, 1, NULL},
    {373, &g_sys.config.alarm[ACL_FILTER_OT].delay, 0, 0, 0, 2, 1, NULL},
    {374, &g_sys.config.alarm[ACL_FILTER_OT].alarm_param, 10, 3600, 90, 2, 1, NULL},
    {375, &g_sys.config.alarm[ACL_FILTER_CLOG].enable_mode, 0, 7, 0, 2, 1, NULL},
    {376, &g_sys.config.alarm[ACL_FILTER_CLOG].delay, 30, 100, 60, 2, 1, NULL},
    {377, &g_sys.config.alarm[ACL_FILTER_CLOG].alarm_param, 0, 0, 0, 2, 1, NULL},
    {378, &g_sys.config.alarm[ACL_REMOTE_SHUT].enable_mode, 0, 7, 4, 2, 1, NULL},
    {379, &g_sys.config.alarm[ACL_REMOTE_SHUT].delay, 3, 100, 5, 2, 1, NULL},
    {380, &g_sys.config.alarm[ACL_REMOTE_SHUT].alarm_param, 0, 0, 0, 2, 1, NULL},
    {381, &g_sys.config.alarm[ACL_WATER_OVERFLOW].enable_mode, 0, 7, 0, 2, 1, NULL},
    {382, &g_sys.config.alarm[ACL_WATER_OVERFLOW].delay, 1, 100, 5, 2, 1, NULL},
    {383, &g_sys.config.alarm[ACL_WATER_OVERFLOW].alarm_param, 0, 1, 0, 2, 1, NULL},
    {384, &g_sys.config.alarm[ACL_GROUP_CONTROL_FAIL].enable_mode, 0, 7, 4, 2, 1, NULL},
    {385, &g_sys.config.alarm[ACL_GROUP_CONTROL_FAIL].delay, 30, 100, 30, 2, 1, NULL},
    {386, &g_sys.config.alarm[ACL_GROUP_CONTROL_FAIL].alarm_param, 0, 0, 0, 2, 1, NULL},
    {387, &g_sys.config.alarm[ACL_INV_FAULT].enable_mode, 0, 7, 4, 2, 1, NULL},
    {388, &g_sys.config.alarm[ACL_INV_FAULT].delay, 5, 100, 5, 2, 1, NULL},
    {389, &g_sys.config.alarm[ACL_INV_FAULT].alarm_param, 0, 0, 0, 2, 1, NULL},
    {390, &g_sys.config.alarm[ACL_SMOKE_ALARM].enable_mode, 0, 7, 0, 2, 1, NULL},
    {391, &g_sys.config.alarm[ACL_SMOKE_ALARM].delay, 5, 100, 5, 2, 1, NULL},
    {392, &g_sys.config.alarm[ACL_SMOKE_ALARM].alarm_param, 0, 0, 0, 2, 1, NULL},
    {393, &g_sys.config.alarm[ACL_USR_ALARM1].enable_mode, 0, 7, 2, 2, 1, NULL},
    {394, &g_sys.config.alarm[ACL_USR_ALARM1].delay, 5, 100, 5, 2, 1, NULL},
    {395, &g_sys.config.alarm[ACL_USR_ALARM1].alarm_param, 0, 0, 0, 2, 1, NULL},
    {396, &g_sys.config.alarm[ACL_MBM_HARD_FAULT].enable_mode, 0, 7, 4, 2, 1, NULL},
    {397, &g_sys.config.alarm[ACL_MBM_HARD_FAULT].delay, 5, 100, 5, 2, 1, NULL},
    {398, &g_sys.config.alarm[ACL_MBM_HARD_FAULT].alarm_param, 0, 0, 0, 2, 1, NULL},
    {399, &g_sys.config.alarm[ACL_MBM_COM_LOSS].enable_mode, 0, 7, 4, 2, 1, NULL},
    {400, &g_sys.config.alarm[ACL_MBM_COM_LOSS].delay, 5, 100, 20, 2, 1, NULL},
    {401, &g_sys.config.alarm[ACL_MBM_COM_LOSS].alarm_param, 0, 0, 0, 2, 1, NULL},
    {402, &g_sys.config.alarm[ACL_BACK_POWER].enable_mode, 0, 7, 2, 2, 1, NULL},
    {403, &g_sys.config.alarm[ACL_BACK_POWER].delay, 5, 100, 5, 2, 1, NULL},
    {404, &g_sys.config.alarm[ACL_BACK_POWER].alarm_param, 0, 0, 0, 2, 1, NULL},
    {405, &g_sys.config.alarm[ACL_COIL_HI_TEM1].enable_mode, 0, 7, 4, 2, 1, NULL},
    {406, &g_sys.config.alarm[ACL_COIL_HI_TEM1].delay, 5, 100, 30, 2, 1, NULL},
    {407, &g_sys.config.alarm[ACL_COIL_HI_TEM1].alarm_param, 70, 300, 150, 2, 1, NULL},
    {408, &g_sys.config.alarm[ACL_COIL_VALVE_DEFAULT].enable_mode, 0, 7, 2, 2, 1, NULL},
    {409, &g_sys.config.alarm[ACL_COIL_VALVE_DEFAULT].delay, 5, 150, 100, 2, 1, NULL},
    {410, &g_sys.config.alarm[ACL_COIL_VALVE_DEFAULT].alarm_param, 5, 20, 10, 2, 1, NULL},
    {411, &g_sys.config.alarm[ACL_COIL_BLOCKING].enable_mode, 0, 7, 4, 2, 1, NULL},
    {412, &g_sys.config.alarm[ACL_COIL_BLOCKING].delay, 5, 100, 70, 2, 1, NULL},
    {413, &g_sys.config.alarm[ACL_COIL_BLOCKING].alarm_param, 0, 1000, 60, 2, 1, NULL},
    {414, NULL, 0, 3600, 0, 2, 1, NULL},
    {415, NULL, 0, 3600, 0, 2, 1, NULL},
    {416, NULL, 0, 3600, 0, 2, 1, NULL},
    {417, NULL, 0, 3600, 0, 2, 1, NULL},
    {418, NULL, 0, 3600, 0, 2, 1, NULL},
    {419, NULL, 0, 3600, 0, 2, 1, NULL},
    {420, NULL, 0, 3600, 0, 2, 1, NULL},
    {421, NULL, 0, 3600, 0, 2, 1, NULL},
    {422, NULL, 0, 3600, 0, 2, 1, NULL},
    {423, NULL, 0, 3600, 0, 2, 1, NULL},
    {424, NULL, 0, 3600, 0, 2, 1, NULL},
    {425, NULL, 0, 3600, 0, 2, 1, NULL},
    {426, &g_sys.config.alarm[ACL_EX_FAN_CURRENT].enable_mode, 0, 7, 4, 2, 1, NULL},
    {427, &g_sys.config.alarm[ACL_EX_FAN_CURRENT].delay, 0, 100, 5, 2, 1, NULL},
    {428, &g_sys.config.alarm[ACL_EX_FAN_CURRENT].alarm_param, 0, 3600, 800, 2, 1, NULL},
    {429, &g_sys.config.alarm[ACL_EX_FAN2_CURRENT].enable_mode, 0, 7, 4, 2, 1, NULL},
    {430, &g_sys.config.alarm[ACL_EX_FAN2_CURRENT].delay, 0, 100, 30, 2, 1, NULL},
    {431, &g_sys.config.alarm[ACL_EX_FAN2_CURRENT].alarm_param, 0, 3600, 800, 2, 1, NULL},
    {432, &g_sys.config.alarm[ACL_WATER_PUMP_HI_LIVEL].enable_mode, 0, 7, 4, 2, 1, NULL},
    {433, &g_sys.config.alarm[ACL_WATER_PUMP_HI_LIVEL].delay, 40, 150, 80, 2, 1, NULL},
    {434, &g_sys.config.alarm[ACL_WATER_PUMP_HI_LIVEL].alarm_param, 0, 0, 0, 2, 1, NULL},
    {435, &g_sys.config.alarm[ACL_FLOW_DIFF].enable_mode, 0, 7, 5, 2, 1, NULL},
    {436, &g_sys.config.alarm[ACL_FLOW_DIFF].delay, 40, 150, 80, 2, 1, NULL},
    {437, &g_sys.config.alarm[ACL_FLOW_DIFF].alarm_param, 0, 0, 0, 2, 1, NULL},
    {438, &g_sys.config.alarm[ACL_HUM_OD].enable_mode, 0, 7, 4, 2, 1, NULL},
    {439, &g_sys.config.alarm[ACL_HUM_OD].delay, 3, 100, 30, 2, 1, NULL},
    {440, &g_sys.config.alarm[ACL_HUM_OD].alarm_param, 0, 0, 0, 2, 1, NULL},
    {441, &g_sys.config.alarm[ACL_NTC_EVAPORATE].enable_mode, 0, 7, 4, 2, 1, NULL},
    {442, &g_sys.config.alarm[ACL_NTC_EVAPORATE].delay, 1, 100, 5, 2, 1, NULL},
    {443, &g_sys.config.alarm[ACL_NTC_EVAPORATE].alarm_param, 0, 500, 30, 2, 1, NULL},
    {444, NULL, 0, 3600, 0, 2, 1, NULL},
    {445, NULL, 0, 3600, 0, 2, 1, NULL},
    {446, NULL, 0, 3600, 0, 2, 1, NULL},
    {447, NULL, 0, 3600, 0, 2, 1, NULL},
    {448, &g_sys.config.ac_power_supply.PH_Ver, 0, 1, 0, 2, 1, NULL},
    {449, &g_sys.config.ac_power_supply.PH_Vol, 0, 6000, 300, 2, 1, NULL},
    {450, &g_sys.config.fan.inv_step, 1, 10, 5, 0, 1, NULL},
    //		{			  451,		&g_sys.config.ext_tmp.mioor_alarm_tmp,							 	  500,					1500,		 1100,
    //2,					1,      NULL},
    //		{			  452,		&g_sys.config.ext_tmp.critical_alarm_tmp,					 		  500,					1500,		 1200,
    //2,					1,      NULL},
    {451, NULL, 0, 3600, 0, 2, 1, NULL},
    {452, NULL, 0, 3600, 0, 2, 1, NULL},
    {453, &g_sys.config.compressor.Exhaust_temperature, 500, 5000, 950, 2, 1, NULL},
    {454, &g_sys.config.compressor.Exhaust_temperature_hystersis, 0, 5000, 100, 2, 1, NULL},
    {455, &g_sys.config.compressor.Dehumidity_Freq, 30, 100, 60, 2, 1, NULL},
    {456, NULL, 0, 0, 0, 2, 1, NULL},
    {457, &g_sys.config.general.DI_Reuse, 0, 1, 0, 2, 1, NULL},
    {458, &g_sys.config.dehumer.Dehumer_Superheat, 60, 200, 120, 2, 1, NULL},
    {459, &g_sys.config.dehumer.DehumPriority, 0, 1, 0, 2, 1, NULL},
    {460, &g_sys.config.fan.CFM_Enable, 0, 1, 0, 2, 1, NULL},
    {461, &g_sys.config.fan.CFM_Para_A, 0, 0xffff, 10, 2, 1, NULL},
    {462, &g_sys.config.fan.CFM_Para_B, 0, 0xffff, 100, 2, 1, NULL},
    {463, &g_sys.config.fan.CFM_Para_C, 0, 0xffff, 2000, 2, 1, NULL},
    {464, NULL, 0, 0, 0, 2, 1, NULL},
    {465, &g_sys.config.mbm_Conf.EEV.ntc_cali[0].Data, 0, 0xffff, 0, 2, 1, NULL},
    {466, &g_sys.config.mbm_Conf.EEV.ntc_cali[1].Data, 0, 0xffff, 0, 2, 1, NULL},
    {467, &g_sys.config.mbm_Conf.EEV.ai_cali[0].Data, 0, 0xffff, 0, 2, 1, NULL},
    {468, &g_sys.config.mbm_Conf.EEV.ai_cali[1].Data, 0, 0xffff, 0, 2, 1, NULL},
    {469, &g_sys.config.mbm_Conf.EEV.Set_Superheat[0].Data, 0, 200, 60, 2, 1, NULL},
    {470, &g_sys.config.mbm_Conf.EEV.Set_Superheat[1].Data, 0, 200, 60, 2, 1, NULL},
    {471, &g_sys.config.mbm_Conf.EEV.Set_Manual_Mode.Data, 0, 1, 0, 2, 1, NULL},
    {472, &g_sys.config.mbm_Conf.EEV.Set_Manual_Steps[0].Data, 0, 600, 250, 2, 1, NULL},
    {473, &g_sys.config.mbm_Conf.EEV.Set_Manual_Steps[1].Data, 0, 600, 250, 2, 1, NULL},

    //		{			  478,    NULL,			 	                                             0,					  0,			    0,
    //2,					1,      NULL   	},
    //		{			  479,    NULL,			 	                                             0,				    0,			  	0,
    //2,					1,      NULL   	},
    //		{			  480,    NULL,						 	                                       0,					  0,				  0,
    //2,					1,      NULL   	},
    //		{			  481,    NULL,			 	                                             0,					  0,			    0,
    //2,					1,      NULL   	},
    //		{			  482,    NULL,			 	                                             0,				    0,			  	0,
    //2,					1,      NULL   	},
    //		{			  483,    NULL,						 	                                       0,					  0,				  0,
    //2,					1,      NULL   	},
    //		{			  484,    NULL,			 	                                             0,					  0,			    0,
    //2,					1,      NULL   	},
    //		{			  485,    NULL,			 	                                             0,				    0,			  	0,
    //2,					1,      NULL   	},
    //		{			  486,    NULL,						 	                                       0,					  0,				  0,
    //2,					1,      NULL   	},
    //		{			  487,    NULL,			 	                                             0,					  0,			    0,
    //2,					1,      NULL   	},
    //		{			  488,    NULL,			 	                                             0,				    0,			  	0,
    //2,					1,      NULL   	},
    //		{			  489,    NULL,						 	                                       0,					  0,				  0,
    //2,					1,      NULL   	},
    //		{			  490,    NULL,						 	                                       0,					  0,				  0,
    //2,					1,      NULL   	},
    //		{			  491,    NULL,			 	                                             0,					  0,			    0,
    //2,					1,      NULL   	},
    //		{			  492,    NULL,			 	                                             0,				    0,			  	0,
    //2,					1,      NULL   	},
    //		{			  493,    NULL,						 	                                       0,					  0,				  0,
    //2,					1,      NULL   	},
    //		{			  494,    NULL,			 	                                             0,					  0,			    0,
    //2,					1,      NULL   	},
    //		{			  495,    NULL,			 	                                             0,				    0,			  	0,
    //2,					1,      NULL   	},
};

// status register map declairation
const sts_reg_map_st status_reg_map_inst[STATUS_REG_MAP_NUM] = {
    // 			id			mapped registers														default
    {0, &g_sys.status.sys_info.status_reg_num, STATUS_REG_MAP_NUM},
    {1, &g_sys.status.sys_info.config_reg_num, CONF_REG_MAP_NUM},
    {2, &g_sys.status.sys_info.software_ver, SOFTWARE_VER},
    {3, &g_sys.status.sys_info.hardware_ver, HARDWARE_VER},
    {4, &g_sys.status.sys_info.serial_no[3], SERIAL_NO_3},
    {5, &g_sys.status.sys_info.serial_no[2], SERIAL_NO_2},
    {6, &g_sys.status.sys_info.serial_no[1], SERIAL_NO_1},
    {7, &g_sys.status.sys_info.serial_no[0], SERIAL_NO_0},
    {8, &g_sys.status.sys_info.man_date[1], MAN_DATA_1},
    {9, &g_sys.status.sys_info.man_date[0], MAN_DATA_0},
    {10, &g_sys.status.sys_info.Sys_Time.u16Sys_time[0], 0},
    {11, &g_sys.status.sys_info.Sys_Time.u16Sys_time[1], 0},
    {12, NULL, 0},
    {13, NULL, 0},
    {14, &g_sys.status.sys_tem_hum.Target_air_temp, 0},
    {15, &g_sys.status.sys_tem_hum.Target_air_hum, 0},
    {16, &g_sys.status.sys_tem_hum.Current_air_temp, 0},
    {17, &g_sys.status.sys_tem_hum.Current_air_hum, 0},
    {18, &g_sys.status.sys_tem_hum.Current_fan_temp, 0},
    {19, &g_sys.status.alarm_status_cnt.pwr_off_alarm, 0},
    {20, &g_sys.status.general.sys_error_bitmap, 0},
    {21, &g_sys.status.general.permission_level, 0},
    {22, &g_sys.status.general.running_mode, 0},
    {23, &g_sys.status.status_remap[0], 0},
    {24, &g_sys.status.status_remap[1], 0},
    {25, &g_sys.status.status_remap[2], 0},
    {26, &g_sys.status.status_remap[3], 0},
    {27, &g_sys.config.team.addr, 0},
    {28, &g_sys.status.alarm_status_cnt.mioor_cnt, 0},
    {29, &g_sys.status.alarm_status_cnt.critical_cnt, 0},
    {30, &g_sys.status.alarm_status_cnt.major_cnt, 0},
    {31, &g_sys.status.alarm_status_cnt.total_cnt, 0},
    {32, &g_sys.status.alarm_bitmap[0], 0},
    {33, &g_sys.status.alarm_bitmap[1], 0},
    {34, &g_sys.status.alarm_bitmap[2], 0},
    {35, &g_sys.status.alarm_bitmap[3], 0},
    {36, &g_sys.status.alarm_bitmap[4], 0},
    {37, &g_sys.status.alarm_bitmap[5], 0},
    {38, &g_sys.status.mbm.tnh[0].temp, 0},
    {39, &g_sys.status.mbm.tnh[0].hum, 0},
    {40, &g_sys.status.mbm.tnh[1].temp, 0},
    {41, &g_sys.status.mbm.tnh[1].hum, 0},
    {42, &g_sys.status.mbm.tnh[2].temp, 0},  //回风2
    {43, &g_sys.status.mbm.tnh[2].hum, 0},
    {44, &g_sys.status.mbm.tnh[3].temp, 0},  //送风2
    {45, &g_sys.status.mbm.tnh[3].hum, 0},
    {46, &g_sys.status.mbm.tnh[4].temp, 0},  //回风3
    {47, &g_sys.status.mbm.tnh[4].hum, 0},
    {48, &g_sys.status.mbm.tnh[5].temp, 0},  //送风3
    {49, &g_sys.status.mbm.tnh[5].hum, 0},
    {50, &g_sys.status.mbm.tnh[6].temp, 0},
    {51, &g_sys.status.mbm.tnh[6].hum, 0},
    {52, &g_sys.status.mbm.tnh[7].temp, 0},
    {53, &g_sys.status.mbm.tnh[7].hum, 0},
    {54, &g_sys.status.Alarm_COM_NTC_BIT[0], 0},
    {55, &g_sys.status.Alarm_COM_NTC_BIT[1], 0},
    {56, NULL, 0},
    {57, &g_sys.status.sys_tem_hum.remote_air_temp, 0},
    {58, &g_sys.status.sys_tem_hum.remote_air_hum, 0},
    {59, &g_sys.status.sys_tem_hum.supply_air_temp, 0},
    {60, &g_sys.status.sys_tem_hum.supply_air_hum, 0},
    {61, &g_sys.status.sys_tem_hum.return_air_temp, 0},
    {62, &g_sys.status.sys_tem_hum.return_air_hum, 0},
    {63, &g_sys.status.mbm.hum.dev_sts, 0},
    {64, &g_sys.status.mbm.hum.conductivity, 0},
    {65, &g_sys.status.mbm.hum.u16HumCurrent, 0},
    {66, &g_sys.status.mbm.hum.water_level, 0},
    {67, NULL, 0},
    {68, &g_sys.status.mbm.pwr[0].pa_volt, 0},
    {69, &g_sys.status.mbm.pwr[0].pb_volt, 0},
    {70, &g_sys.status.mbm.pwr[0].pc_volt, 0},
    {71, &g_sys.status.mbm.pwr[0].freq, 0},
    {72, &g_sys.status.mbm.pwr[0].dev_sts, 0},
    {73, &g_sys.status.mbm.pwr[0].p_cur[0], 0},
    {74, &g_sys.status.mbm.pwr[0].p_cur[1], 0},
    {75, &g_sys.status.mbm.pwr[0].p_cur[2], 0},
    {76, &g_sys.status.ICT.u16Vout[0], 0},
    {77, &g_sys.status.ICT.u16Vout[1], 0},
    {78, &g_sys.status.ICT.u16Vout[2], 0},
    {79, &g_sys.status.ICT.u16Vout[3], 0},
    {80, &g_sys.status.ICT.u16Vout[4], 0},
    {81, &g_sys.status.ICT.u16Vout[5], 0},
    {82, &g_sys.status.ICT.u16DI[0], 0},
    {83, &g_sys.status.ICT.u16DI[1], 0},
    {84, &g_sys.status.ICT.u16Status, 0},
    {85, &g_sys.status.ICT.u16Test, 0},
    {86, &g_sys.status.ICT.u16Fsm, 0},
    {87, &g_sys.status.run_time[DO_FAN5_DUMMY_BPOS].low, 0},
    {88, &g_sys.status.run_time[DO_FAN6_DUMMY_BPOS].high, 0},
    {89, &g_sys.status.run_time[DO_FAN6_DUMMY_BPOS].low, 0},
    {90, &g_sys.status.run_time[DO_FAN7_DUMMY_BPOS].high, 0},
    {91, &g_sys.status.run_time[DO_FAN7_DUMMY_BPOS].low, 0},
    {92, &g_sys.status.run_time[DO_FAN8_DUMMY_BPOS].high, 0},
    {93, &g_sys.status.run_time[DO_FAN8_DUMMY_BPOS].low, 0},
    //    {				80,		  NULL,			                                  0},
    //    {				81,		  NULL,			                                  0},
    //    {				82,		  NULL,			                                  0},
    //    {				83,		  NULL,			                                  0},
    //    {				84,		  NULL,			                                  0},
    //    {				85,		  NULL,			                                  0},
    //    {				86,		  NULL,			                                  0},
    //    {				87,		  NULL,			                                  0},
    //    {				88,		  NULL,			                                  0},
    //    {				89,		  NULL,			                                  0},
    //    {				90,		  NULL,			                                  0},
    //    {				91,		  NULL,			                                  0},
    //    {				92,		  NULL,			                                  0},
    //    {				93,		  NULL,			                                  0},
    {94, NULL, 0},
    {95, &g_sys.status.inv_compress_alarm.avg_hi_press[GET_AVG], 0},
    {96, NULL, 0},
    {97, NULL, 0},
    {98, &g_sys.status.ain[0], 0},  //模拟输入状态
    {99, &g_sys.status.ain[1], 0},
    {100, &g_sys.status.ain[2], 0},
    {101, &g_sys.status.ain[3], 0},
    {102, &g_sys.status.ain[4], 0},
    {103, &g_sys.status.ain[5], 0},
    {104, &g_sys.status.ain[6], 0},
    {105, &g_sys.status.ain[7], 0},
    {106, &g_sys.status.ain[8], 0},
    {107, &g_sys.status.ain[9], 0},
    {108, &g_sys.status.ain[10], 0},
    {109, &g_sys.status.ain[11], 0},
    {110, &g_sys.status.ain[12], 0},
    {111, &g_sys.status.aout[0], 0},  //模拟输出状态
    {112, &g_sys.status.aout[1], 0},
    {113, &g_sys.status.aout[2], 0},
    {114, &g_sys.status.aout[3], 0},
    {115, &g_sys.status.aout[4], 0},
    {116, &g_sys.status.CFM, 0},  //总风量
    {117, &g_sys.status.din_bitmap[0], 0},
    {118, &g_sys.status.din_bitmap[1], 0},
    {119, &g_sys.status.dout_bitmap, 0},
    {120, &g_sys.status.run_time[DO_FAN_BPOS].high, 0},
    {121, &g_sys.status.run_time[DO_FAN_BPOS].low, 0},
    {122, &g_sys.status.run_time[DO_COMP1_BPOS].high, 0},
    {123, &g_sys.status.run_time[DO_COMP1_BPOS].low, 0},
    {124, &g_sys.status.run_time[DO_COMP2_BPOS].high, 0},
    {125, &g_sys.status.run_time[DO_COMP2_BPOS].low, 0},
    {126, &g_sys.status.run_time[DO_HUM_BPOS].high, 0},
    {127, &g_sys.status.run_time[DO_HUM_BPOS].low, 0},
    {128, &g_sys.status.run_time[DO_RH1_BPOS].high, 0},
    {129, &g_sys.status.run_time[DO_RH1_BPOS].low, 0},
    {130, &g_sys.status.run_time[DO_RH2_BPOS].high, 0},
    {131, &g_sys.status.run_time[DO_RH2_BPOS].low, 0},
    //		{				130,		NULL, 0},
    //		{				131,		NULL, 0},
    {132, &g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].high, 0},
    {133, &g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low, 0},
    {134, &g_sys.status.flash_program_flag, 0},
    //		{				135,		&g_sys.status.sys_work_mode.work_mode,			0},
    //		{				136,		&g_sys.status.sys_work_mode.limit_day,			0},
    //		{				137,		&g_sys.status.sys_work_mode.runing_day,		  0},
    //		{				138,		&g_sys.status.sys_work_mode.runing_hour,	  0},
    //    {				139,		&g_sys.status.sys_work_mode.runing_State,   0},
    {135, NULL, 0},
    {136, NULL, 0},
    {137, NULL, 0},
    {138, NULL, 0},
    {139, NULL, 0},
    {140, (uint16_t *)&team_local_inst.team_fsm, 0},
    {141, ((uint16_t *)&team_local_inst.team_bitmap[TEAM_BITMAP_MALFUN_DEV] + 0), 0},
    {142, ((uint16_t *)&team_local_inst.team_bitmap[TEAM_BITMAP_MALFUN_DEV] + 1), 0},
    {143, ((uint16_t *)&team_local_inst.team_bitmap[TEAM_BITMAP_ONLINE] + 0), 0},
    {144, ((uint16_t *)&team_local_inst.team_bitmap[TEAM_BITMAP_ONLINE] + 1), 0},
    {145, ((uint16_t *)&team_local_inst.team_bitmap[TEAM_BITMAP_FINAL_OUT] + 0), 0},
    {146, ((uint16_t *)&team_local_inst.team_bitmap[TEAM_BITMAP_FINAL_OUT] + 1), 0},
    {147, &team_local_inst.team_work_status[TEAM_WORK_STATUS_0_3], 0},
    {148, &team_local_inst.team_work_status[TEAM_WORK_STATUS_4_7], 0},
    {149, &team_local_inst.team_work_status[TEAM_WORK_STATUS_8_11], 0},
    {150, &team_local_inst.team_work_status[TEAM_WORK_STATUS_12_15], 0},
    {151, &team_local_inst.team_work_status[TEAM_WORK_STATUS_16_19], 0},
    {152, &team_local_inst.team_work_status[TEAM_WORK_STATUS_20_23], 0},
    {153, &team_local_inst.team_work_status[TEAM_WORK_STATUS_24_27], 0},
    {154, &team_local_inst.team_work_status[TEAM_WORK_STATUS_28_31], 0},
    {155, &team_local_inst.team_config[TEAM_CONF_CNT], 0},
    {156, &team_local_inst.team_config[TEAM_CONF_TEMP_PRECISION], 0},
    {157, &team_local_inst.team_config[TEAM_CONF_TEMP_DEADBAND], 0},
    {158, &team_local_inst.team_config[TEAM_CONF_TEMP_SETVAL], 0},
    {159, &team_local_inst.team_config[TEAM_CONF_HUM_PRECISION], 0},
    {160, &team_local_inst.team_config[TEAM_CONF_HUM_DEADBAND], 0},
    {161, &team_local_inst.team_config[TEAM_CONF_HUM_SETVAL], 0},
    {162, &team_local_inst.team_config[TEAM_CONF_ALARM_HI_TEMP_PARAM], 0},
    {163, &team_local_inst.team_config[TEAM_CONF_ALARM_LO_TEMP_PARAM], 0},
    {164, &team_local_inst.team_config[TEAM_CONF_ALARM_HI_HUM_PARAM], 0},
    {165, &team_local_inst.team_config[TEAM_CONF_ALARM_LO_HUM_PARAM], 0},
    {166, &team_local_inst.team_config[TEAM_CONF_MODE], 0},
    {167, NULL, 0},
    {168, &team_local_inst.team_config[TEAM_CONF_ROTATE_PERIOD], 0},
    {169, &team_local_inst.team_config[TEAM_CONF_ROTATE_TIME], 0},
    {170, &g_sys.config.team.addr, 0},
    {171, &team_local_inst.team_config[TEAM_CONF_FAN_MODE], 0},
    {172, &team_local_inst.team_config[TEAM_CONF_FAN_TARGET_TEMP], 0},
    {173, &team_local_inst.team_config[TEAM_CONF_FAN_TEMP_DEADBAND], 0},
    {174, &team_local_inst.team_config[TEAM_CONF_FAN_TEMP_PRECISION], 0},
    {175, &team_local_inst.team_config[TEAM_CONF_FAN_SET_FLOW_DIFF], 0},
    {176, &team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_DEADZONE], 0},
    {177, &team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_STEP], 0},
    {178, &team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_DELAY], 0},
    {179, NULL, 0},
    // EX_FAN
    {180, &g_sys.status.mbm.EX_FAN[0].Fre, 0},
    {181, &g_sys.status.mbm.EX_FAN[0].M_Voltage, 0},
    {182, &g_sys.status.mbm.EX_FAN[0].P_Current[0], 0},
    {183, &g_sys.status.mbm.EX_FAN[0].P_Current[1], 0},
    {184, &g_sys.status.mbm.EX_FAN[0].P_Current[2], 0},
    {185, &g_sys.status.mbm.EX_FAN[1].Fre, 0},
    {186, &g_sys.status.mbm.EX_FAN[1].M_Voltage, 0},
    {187, &g_sys.status.mbm.EX_FAN[1].P_Current[0], 0},
    {188, &g_sys.status.mbm.EX_FAN[1].P_Current[1], 0},
    {189, &g_sys.status.mbm.EX_FAN[1].P_Current[2], 0},

    {190, &g_sys.status.ControlPassword.Grade_Manage, 0},
    {191, &g_sys.status.ControlPassword.Password_Poweron, 0},
    {192, &g_sys.status.ControlPassword.Password_Grade[0].Day, 0},
    {193, &g_sys.status.ControlPassword.Password_Grade[0].Password, 0},
    {194, &g_sys.status.ControlPassword.Password_Grade[1].Day, 0},
    {195, &g_sys.status.ControlPassword.Password_Grade[1].Password, 0},
    {196, &g_sys.status.ControlPassword.Password_Grade[2].Day, 0},
    {197, &g_sys.status.ControlPassword.Password_Grade[2].Password, 0},
    {198, &g_sys.status.ControlPassword.Password_Grade[3].Day, 0},
    {199, &g_sys.status.ControlPassword.Password_Grade[3].Password, 0},
    {200, &g_sys.status.ControlPassword.Remain_day, 0},
    {201, &g_sys.status.ControlPassword.Run_day, 0},
    {202, &g_sys.status.ControlPassword.Run_hour, 0},
    {203, &g_sys.status.ControlPassword.Run_second, 0},
    {204, &g_sys.status.ControlPassword.Run_State, 0},
    // EEV
    {205, &g_sys.status.mbm.EEV.run_mode, 0},
    {206, &g_sys.status.mbm.EEV.alarm_bitmap, 0},
    {207, &g_sys.status.mbm.EEV.suction_temp_avg[0], 0},
    {208, &g_sys.status.mbm.EEV.suction_temp_avg[1], 0},
    {209, &g_sys.status.mbm.EEV.vapor_pressure1_avg, 0},
    {210, &g_sys.status.mbm.EEV.vapor_temp1_avg, 0},
    {211, &g_sys.status.mbm.EEV.vapor_pressure2_avg, 0},
    {212, &g_sys.status.mbm.EEV.vapor_temp2_avg, 0},
    {213, &g_sys.status.mbm.EEV.superHeat[0], 0},
    {214, &g_sys.status.mbm.EEV.superHeat[1], 0},
    {215, &g_sys.status.mbm.EEV.valve_opening_cur[0], 0},
    {216, &g_sys.status.mbm.EEV.valve_opening_cur[1], 0},
    {217, &g_sys.status.mbm.EEV.valve_steps_cur[0], 0},
    {218, &g_sys.status.mbm.EEV.valve_steps_cur[1], 0},
    {219, &g_sys.status.mbm.EEV.cur_steps_inc[0], 0},
    {220, &g_sys.status.mbm.EEV.cur_steps_inc[1], 0},
    {221, &g_sys.status.mbm.EEV.valve_ctrl_status[0], 0},
    {222, &g_sys.status.mbm.EEV.valve_ctrl_status[1], 0},
    //		//INV
    //		{				223,	 &g_sys.status.mbm.INV.Comm,						              				0},
    //		{				224,	 &g_sys.status.mbm.INV.Fre,						              				0},
    //		{				225,	 &g_sys.status.mbm.INV.M_Voltage, 0},
    //		{				226,	 &g_sys.status.mbm.INV.Out_Voltage, 0},
    //		{				227,	 &g_sys.status.mbm.INV.Out_Current, 0},
    //		{				228,	 &g_sys.status.mbm.INV.Out_Power, 0},
    //		{				229,	 &g_sys.status.mbm.INV.Out_Torque, 0},
    //		{				230,	 &g_sys.status.mbm.INV.Speed,						              				0},
    //		{				231,	 &g_sys.status.mbm.INV.DI,						              				0},
    //		{				232,	 &g_sys.status.mbm.INV.DO,						              				0},
    //		{				233,	 &g_sys.status.mbm.INV.AI[0],						              				0},
    //		{				234,	 &g_sys.status.mbm.INV.AI[1],						              				0},
    //		{				235,	 &g_sys.status.mbm.INV.AI[2],					              				0},
    //		{				236,	 &g_sys.status.mbm.INV.Inv_Err,						              				0},
    //		{				237,	 &g_sys.status.mbm.INV.Comm_Err, 0},

};

/**
  * @brief  get eeprom program status
  * @param  None
  * @retval
        `EE_FLAG_OK:		configuration data valid in eeprom
        `EE_FLAG_EMPTY:	eeprom empty
  */

static init_state_em get_ee_status(void)
{
    init_state_em em_init_state;
    uint8_t ee_pflag;
    rt_thread_delay(10);                        // wait for eeprom power on
    I2C_EE_BufRead(&ee_pflag, STS_EE_ADDR, 1);  //启动区
    switch (ee_pflag)
    {
        case (EE_FLAG_LOAD_USR): {
            em_init_state = INIT_LOAD_USR;
            break;
        }

        case (EE_FLAG_LOAD_FACT): {
            em_init_state = INIT_LOAD_FACT;
            break;
        }
        case (EE_FLAG_LOAD_DFT): {
            em_init_state = INIT_LOAD_DEFAULT;
            break;
        }
        default: {
            em_init_state = INIT_LOAD_DEBUT;
            break;
        }
    }
    return em_init_state;
}

/**
  * @brief 	save system configurable variables initialization
    * @param  0:load usr1 eeprom
                        1:load usr2 eeprom
                        2:load facotry eeprom
                        3:load default eeprom
    * @retval err_cnt: mismatch read/write data count
  */

uint16_t set_load_flag(uint8_t ee_load_flag)
{
    uint8_t ee_flag;
    switch (ee_load_flag)
    {
        case (0): {
            ee_flag = EE_FLAG_LOAD_USR;
            break;
        }
        case (1): {
            ee_flag = EE_FLAG_LOAD_FACT;
            break;
        }
        case (2): {
            ee_flag = EE_FLAG_LOAD_DEBUT;  //原始参数
            break;
        }
        default: {
            ee_flag = EE_FLAG_LOAD_DFT;
            break;
        }
    }
    I2C_EE_BufWrite(&ee_flag, STS_EE_ADDR, 1);
    return 1;
}

/**
  * @brief 	save system configurable variables initialization
    * @param  addr_sel:
                        `0: save current configuration to usr1 eeprom address
                        `1:	save current configuration to usr2 eeprom address
                        `2:	save current configuration to facotry eeprom address
    * @retval err_cnt: mismatch read/write data count
  */
uint16_t save_conf_reg(uint8_t addr_sel)
{
    //		uint16_t conf_reg[CONF_REG_MAP_NUM];
    //		uint16_t test_reg[CONF_REG_MAP_NUM];
    uint16_t i, j, err_cnt, chk_res;
    uint16_t ee_save_addr;
    uint8_t ee_flag, req;

    ee_save_addr = 0;
    err_cnt      = 0;

    switch (addr_sel)
    {
        case (0): {
            ee_flag = EE_FLAG_LOAD_USR;
            break;
        }
        case (1): {
            ee_save_addr = CONF_REG_FACT_ADDR;
            ee_flag      = EE_FLAG_LOAD_FACT;
            break;
        }
        default: {
            return 0xff;
        }
    }

    for (i = 0; i < CONF_REG_MAP_NUM; i++)  // set configration reg with default value
    {
        conf_reg[i] = *(conf_reg_map_inst[i].reg_ptr);
    }
    if (ee_flag == EE_FLAG_LOAD_USR)
    {
        req = 0;
        for (j = 0; j < 3; j++)
        {
            if (j == 0)
            {
                ee_save_addr = CONF_REG_EE1_ADDR;
            }
            else if (j == 1)
            {
                ee_save_addr = CONF_REG_EE2_ADDR;
            }
            else
            {
                ee_save_addr = CONF_REG_EE3_ADDR;
            }

            I2C_EE_BufWrite((uint8_t *)conf_reg, ee_save_addr,
                            (CONF_REG_MAP_NUM)*2);  // save configuration data to eeprom
            for (i = 0; i < 10; i++)
                ;
            I2C_EE_BufRead((uint8_t *)test_reg, ee_save_addr, (CONF_REG_MAP_NUM)*2);
            for (i = 0; i < CONF_REG_MAP_NUM; i++)
            {
                if (conf_reg[i] != test_reg[i])
                {
                    err_cnt++;
                }
            }
            if (err_cnt == 0)
            {
                chk_res = checksum_u16(conf_reg, CONF_REG_MAP_NUM);  // set parameter checksum
                I2C_EE_BufWrite((uint8_t *)&chk_res, ee_save_addr + (CONF_REG_MAP_NUM * 2), 2);
                //
                //								rt_kprintf("\nchk_res_addr = %d",ee_save_addr+(CONF_REG_MAP_NUM*2));
                //								rt_kprintf("\nchk_res = %d",chk_res);

                I2C_EE_BufWrite(&ee_flag, STS_EE_ADDR, 1);  // set eeprom program flag
            }
            else
            {
                req++;
            }
        }
        if (req < 2)
        {
            err_cnt = 0;
        }
        else
        {
            err_cnt = req;
        }
    }
    else
    {
        I2C_EE_BufWrite((uint8_t *)conf_reg, ee_save_addr, (CONF_REG_MAP_NUM)*2);  // save configuration data to eeprom
        for (i = 0; i < 10; i++)
            ;
        I2C_EE_BufRead((uint8_t *)test_reg, ee_save_addr, (CONF_REG_MAP_NUM)*2);
        for (i = 0; i < CONF_REG_MAP_NUM; i++)
        {
            if (conf_reg[i] != test_reg[i])
            {
                err_cnt++;
            }
        }
        if (err_cnt == 0)
        {
            chk_res = checksum_u16(conf_reg, CONF_REG_MAP_NUM);  // set parameter checksum
            I2C_EE_BufWrite((uint8_t *)&chk_res, ee_save_addr + (CONF_REG_MAP_NUM * 2), 2);
            I2C_EE_BufWrite(&ee_flag, STS_EE_ADDR, 1);  // set eeprom program flag
        }
    }

    return err_cnt;
}

static uint16_t init_load_default(void)
{
    uint16_t i, ret;
    ret = 1;
    for (i = 0; i < CONF_REG_MAP_NUM; i++)  // initialize global variable with default values
    {
        if (conf_reg_map_inst[i].reg_ptr != NULL)
        {
            *(conf_reg_map_inst[i].reg_ptr) = conf_reg_map_inst[i].dft;
        }
    }
    authen_init();
    ret = 1;
    g_sys.status.status_remap[0] |= 0x0001;

    return ret;
}

/**
 * @brief  load system configuration data from eeprom
 * @param  None
 * @retval None
 */

static uint16_t conf_reg_read_ee(uint16_t addr)
{
    uint16_t reg;
    uint16_t ee_err, ret;
    reg    = 0;
    ee_err = 0;
    ret    = 0;
    ee_err = eeprom_compare_read(addr, &reg);

    if ((conf_reg_map_inst[addr].reg_ptr != NULL) &&
        ((reg < conf_reg_map_inst[addr].min) || (reg > conf_reg_map_inst[addr].max)))
    {
        *(conf_reg_map_inst[addr].reg_ptr) = (conf_reg_map_inst[addr].dft);
        ret                                = 0;
    }
    else
    {
        *(conf_reg_map_inst[addr].reg_ptr) = reg;
        ret                                = 1;
    }

    if ((ee_err != 0) || (ret == 0))
    {
        ret = 1;
    }
    else
    {
        ret = 0;
    }
    return ret;
}

static uint16_t init_load_user_conf(void)
{
    uint16_t i, sum, sum_reg;
    sum = 0;

    for (i = 0; i < CONF_REG_MAP_NUM; i++)
    {
        sum_reg = sum;
        sum += conf_reg_read_ee(i);
        if (sum != sum_reg)
        {
            rt_kprintf("Err addr:%d\n", i);
        }
    }

    if (sum == 0)
    {
        return (1);
    }
    else
    {
        return (0);
    }
}

static uint16_t init_load_factory_conf(void)
{
    uint16_t buf_reg[CONF_REG_MAP_NUM + 1];
    uint16_t i;
    uint16_t chk_res;
    uint16_t ee_load_addr;
    ee_load_addr = CONF_REG_FACT_ADDR;

    I2C_EE_BufRead((uint8_t *)buf_reg, ee_load_addr,
                   (CONF_REG_MAP_NUM + 1) * 2);  // read eeprom data & checksum to data buffer
    rt_thread_delay(1);                          // wait for i2c opeartion comletion
    chk_res = checksum_u16(buf_reg, (CONF_REG_MAP_NUM + 1));
    if (chk_res != 0)  // eeprom configuration data checksum fail
    {
        for (i = 0; i < CONF_REG_MAP_NUM; i++)  // initialize global variable with default values
        {
            if (conf_reg_map_inst[i].reg_ptr != NULL)
            {
                *(conf_reg_map_inst[i].reg_ptr) = conf_reg_map_inst[i].dft;
            }
        }
        return 0;
    }
    else
    {
        for (i = 0; i < CONF_REG_MAP_NUM; i++)
        {
            *(conf_reg_map_inst[i].reg_ptr) = buf_reg[i];
        }

        return 1;
    }
}

/**
 * @brief  initialize system status reg data
 * @param  None
 * @retval None
 */
static void init_load_status(void)
{
    uint16_t i;
    for (i = 0; i < STATUS_REG_MAP_NUM; i++)
    {
        if (status_reg_map_inst[i].reg_ptr != NULL)
        {
            *(status_reg_map_inst[i].reg_ptr) = status_reg_map_inst[i].dft;
        }
    }

    I2C_EE_BufRead((uint8_t *)&g_sys.status.run_time, STS_REG_EE1_ADDR,
                   sizeof(g_sys.status.run_time));  // read legacy checksum data
}

/**
  * @brief  system configurable variables initialization
  * @param  None
  * @retval
            1:	load default data
            2:	load eeprom data
  */
uint16_t sys_global_var_init(void)
{
    uint16_t ret;
    init_state_em em_init_state;
    em_init_state = get_ee_status();  // get eeprom init status
    // TEST
    em_init_state = INIT_LOAD_DEBUT;
    init_load_status();
    switch (em_init_state)
    {
        case (INIT_LOAD_USR):  // load usr1 data
        {
            ret = init_load_user_conf();
            if (ret == 1)
            {
                g_sys.status.status_remap[0] |= 0x01;
                rt_kprintf("Usr conf file loaded successfully.\n");
            }
            else
            {
                g_sys.status.status_remap[0] &= 0xFFFE;
                rt_kprintf("Usr conf file loaded failed.\n");
            }
            break;
        }
        case (INIT_LOAD_FACT):  // load factory data
        {
            ret = init_load_factory_conf();
            if (ret == 1)
            {
                save_conf_reg(0);
                set_load_flag(0);
                g_sys.status.status_remap[0] |= 0x01;
                rt_kprintf("Factory conf file loaded successfully.\n");
            }
            else
            {
                g_sys.status.status_remap[0] &= 0xFFFE;
                rt_kprintf("Factory conf file loaded failed.\n");
            }
            break;
        }
        case (INIT_LOAD_DEBUT):  // resotre default configuration data, include reset password to default values
        {
            ret = init_load_default();
            if (ret == 1)
            {
                g_sys.status.status_remap[0] |= 0x01;
                save_conf_reg(0);
                save_conf_reg(1);
                set_load_flag(0);
                // reset dev run time
                reset_runtime(0xff);
                rt_kprintf("INIT_LOAD_DEBUT loaded successfully.\n");
            }
            else
            {
                g_sys.status.status_remap[0] &= 0xFFFE;
                rt_kprintf("INIT_LOAD_DEBUT loaded failed.\n");
            }
            break;
        }
        default:  // resotre default configuration data, include reset password to default values
        {
            ret = init_load_default();
            if (ret == 1)
            {
                g_sys.status.status_remap[0] |= 0x01;
                rt_kprintf("Default conf data load successfully.\n");
            }
            else
            {
                g_sys.status.status_remap[0] &= 0xFFFE;
                rt_kprintf("Default conf file loaded failed.\n");
            }
            break;
        }
    }
    //测试模式和诊断模式复位。
    g_sys.config.general.diagnose_mode_en = 0;
    g_sys.config.general.alarm_bypass_en  = 0;
    g_sys.config.general.testing_mode_en  = 0;

    return ret;
}

uint16_t sys_local_var_init(void)
{
    uint16_t i, j;

    for (i = 0; i < REQ_MAX_CNT; i++)
    {
        for (j = 0; j < REQ_MAX_LEVEL; j++)
        {
            l_sys.require[i][j] = 0;
        }
    }
    // MAX REQ initialization
    l_sys.require[MAX_REQ][T_REQ] = 100;
    l_sys.require[MAX_REQ][H_REQ] = 0;

    l_sys.t_fsm_state = 0;

    for (i = 0; i < DO_MAX_CNT; i++)
    {
        l_sys.bitmap[i] = 0;
    }

    for (i = 0; i < DO_MAX_CNT; i++)
    {
        l_sys.comp_timeout[i] = 0;
    }

    for (i = 0; i < AO_MAX_CNT; i++)
    {
        for (j = 0; j < BITMAP_MAX_CNT; j++)
        {
            l_sys.ao_list[i][j] = 0;
        }
    }

    for (i = 0; i < L_FSM_STATE_MAX_NUM; i++)
    {
        l_sys.l_fsm_state[i] = 0;
    }
    l_sys.debug_flag    = 0;
    l_sys.debug_tiemout = DEBUG_TIMEOUT_MAX;
    //		l_sys.debug_flag = 1;
    //		l_sys.debug_tiemout = DEBUG_TIMEOUT_MAX;
    l_sys.t_fsm_signal            = 0;
    l_sys.watervalve_warmup_delay = 0;
    l_sys.ec_fan_diff_reg         = 0;
    l_sys.ec_fan_suc_temp         = 0;
    l_sys.HumCheck                = 0;
    //		g_sys.config.ac_power_supply.PH_Ver=1;

    g_sys.config.dev_mask.din_bitmap_polarity[0] = 0;
    g_sys.config.dev_mask.ain                    = 0x1F;
    g_sys.config.dev_mask.mb_comp                = 0x100;

    return 1;
}

/**
 * @brief  get current system permission
 * @param  None
 * @retval current system permission level
 */
uint16_t get_sys_permission(void)
{
    return g_sys.status.general.permission_level;
}

static int16_t eeprom_singel_write(uint16 base_addr, uint16 reg_offset_addr, uint16 wr_data, uint16_t rd_data)
{
    int16_t err_no;
    uint16_t wr_data_buf;
    uint16_t cs_data, ee_rd_cheksum;
    wr_data_buf = wr_data;

    err_no = I2C_EE_BufRead((uint8_t *)&cs_data, base_addr + (CONF_REG_MAP_NUM << 1), 2);
    //计算check_sum
    ee_rd_cheksum = cs_data ^ rd_data ^ wr_data;
    // 写寄存器
    err_no += I2C_EE_BufWrite((uint8_t *)&wr_data_buf, base_addr + (reg_offset_addr << 1), 2);
    // 写校验
    err_no += I2C_EE_BufWrite((uint8_t *)&ee_rd_cheksum, base_addr + (CONF_REG_MAP_NUM << 1), 2);

    return err_no;
}

int16_t eeprom_compare_read(uint16 reg_offset_addr, uint16_t *rd_data)
{
    uint16_t rd_buf0;
    uint16_t rd_buf1;
    uint16_t rd_buf2;
    int16_t ret;

    I2C_EE_BufRead((uint8_t *)&rd_buf0, CONF_REG_EE1_ADDR + (reg_offset_addr << 1), 2);
    I2C_EE_BufRead((uint8_t *)&rd_buf1, CONF_REG_EE2_ADDR + (reg_offset_addr << 1), 2);
    I2C_EE_BufRead((uint8_t *)&rd_buf2, CONF_REG_EE3_ADDR + (reg_offset_addr << 1), 2);

    // normal situation
    if ((rd_buf0 == rd_buf1) && (rd_buf2 == rd_buf1))
    {
        if (rd_buf0)
            *rd_data = rd_buf0;
        ret = 0;
    }
    else if ((rd_buf0 == rd_buf1) || (rd_buf0 == rd_buf2) || (rd_buf1 == rd_buf2))
    {
        *rd_data = rd_buf0;
        if (rd_buf0 == rd_buf1)  // buf2!= buf1
        {
            *rd_data = rd_buf0;
            eeprom_singel_write(CONF_REG_EE3_ADDR, reg_offset_addr, rd_buf0, rd_buf2);
        }
        else if (rd_buf0 == rd_buf2)  // buf2 = buf0, buf1错
        {
            *rd_data = rd_buf2;
            eeprom_singel_write(CONF_REG_EE2_ADDR, reg_offset_addr, rd_buf2, rd_buf1);
        }
        else  //(rd_buf1 == rd_buf2)
        {
            *rd_data = rd_buf1;
            eeprom_singel_write(CONF_REG_EE1_ADDR, reg_offset_addr, rd_buf1, rd_buf0);
        }
        rt_kprintf("eeprom_compare_read :reg_offset_addr_ERRO= %d \n", reg_offset_addr);
        ret = 0;
    }
    else  //三个都错误
    {
        *rd_data = ABNORMAL_VALUE;
        ret      = 1;
        rt_kprintf("eeprom_compare_read :reg_offset_addr_ALL_ERRO= %d \n", reg_offset_addr);
    }
    return (ret);
}

int16_t eeprom_tripple_write(uint16 reg_offset_addr, uint16 wr_data, uint16_t rd_data)
{
    int16_t err_no;
    uint16_t wr_data_buf;
    uint16_t cs_data, ee_rd_cheksum;
    wr_data_buf = wr_data;

    err_no = eeprom_compare_read(CONF_REG_MAP_NUM, &cs_data);

    if (err_no == 0)
    {
        ee_rd_cheksum = cs_data ^ rd_data ^ wr_data;
    }
    else
    {
        rt_kprintf("eeprom_tripple_write : ERRO \n");
        return -1;
    }
    err_no = 0;

    // write data to eeprom
    err_no += I2C_EE_BufWrite((uint8_t *)&wr_data_buf, CONF_REG_EE1_ADDR + (reg_offset_addr << 1), 2);
    err_no += I2C_EE_BufWrite((uint8_t *)&wr_data_buf, CONF_REG_EE2_ADDR + (reg_offset_addr << 1), 2);
    err_no += I2C_EE_BufWrite((uint8_t *)&wr_data_buf, CONF_REG_EE3_ADDR + (reg_offset_addr << 1), 2);

    // write checksum data to eeprom
    err_no += I2C_EE_BufWrite((uint8_t *)&ee_rd_cheksum, CONF_REG_EE1_ADDR + (CONF_REG_MAP_NUM * 2), 2);
    err_no += I2C_EE_BufWrite((uint8_t *)&ee_rd_cheksum, CONF_REG_EE2_ADDR + (CONF_REG_MAP_NUM * 2), 2);
    err_no += I2C_EE_BufWrite((uint8_t *)&ee_rd_cheksum, CONF_REG_EE3_ADDR + (CONF_REG_MAP_NUM * 2), 2);

    return err_no;
}

/**
 * @brief  write register map with constraints.
 * @param  reg_addr: reg map address.
 * @param  wr_data: write data.
 * @param  permission_flag:
 *   This parameter can be one of the following values:
 *     @arg PERM_PRIVILEGED: write opertion can be performed dispite permission level
 *     @arg PERM_INSPECTION: write operation could only be performed when pass permission check
 * @retval
 *   This parameter can be one of the following values:
 *     @arg 1: write operation success
 *     @arg 0: write operation fail
 */
uint16 reg_map_write(uint16 reg_addr, uint16 *wr_data, uint8_t wr_cnt, uint8_t User_ID)
{
    uint16_t i;
    uint16_t err_code;
    uint16_t sys_permission;
    uint16_t ee_wr_data, ee_rd_data;
    err_code = CPAD_ERR_NOERR;

    sys_permission = get_sys_permission();
    // modebus_slave permission_high_lev
    sys_permission = 4;
    if (User_ID == USER_MODEBUS_SLAVE)
    {
        sys_permission = 3;
    }
    if ((reg_addr + wr_cnt) > CONF_REG_MAP_NUM)  // address range check
    {
        err_code = CPAD_ERR_ADDR_OR;
        rt_kprintf("MB_SLAVE CPAD_ERR_ADDR_OR1 failed\n");
        return err_code;
    }

    for (i = 0; i < wr_cnt; i++)  // permission check
    {
        if (conf_reg_map_inst[reg_addr + i].permission > sys_permission)
        {
            err_code = CPAD_ERR_PERM_OR;
            rt_kprintf("CPAD_ERR_PERM_OR1 failed\n");
            return err_code;
        }
    }

    for (i = 0; i < wr_cnt; i++)  // writablility check
    {
        if (conf_reg_map_inst[reg_addr + i].rw != 1)
        {
            err_code = CPAD_ERR_WR_OR;
            rt_kprintf("CPAD_ERR_WR_OR02 failed\n");
            return err_code;
        }
    }

    for (i = 0; i < wr_cnt; i++)  // min_max limit check
    {
        if ((*(wr_data + i) > conf_reg_map_inst[reg_addr + i].max) ||
            (*(wr_data + i) < conf_reg_map_inst[reg_addr + i].min))  // min_max limit check
        {
            err_code = CPAD_ERR_DATA_OR;
            rt_kprintf("CPAD_ERR_WR_OR03 failed\n");
            return err_code;
        }

        if (conf_reg_map_inst[reg_addr + i].chk_ptr != NULL)
        {
            if (conf_reg_map_inst[reg_addr + i].chk_ptr(*(wr_data + i)) == 0)
            {
                err_code = CPAD_ERR_CONFLICT_OR;
                rt_kprintf("CHK_PTR:CPAD_ERR_WR_OR failed\n");
                return err_code;
            }
        }
    }

    for (i = 0; i < wr_cnt; i++)  // data write
    {
        ee_rd_data = *(conf_reg_map_inst[reg_addr + i].reg_ptr);  // buffer legacy reg data
        ee_wr_data = *(wr_data + i);                              // buffer current write data

        *(conf_reg_map_inst[reg_addr + i].reg_ptr) = *(wr_data + i);  // write data to designated register

        //				I2C_EE_BufWrite((uint8_t *)&ee_wr_data,CONF_REG_EE1_ADDR+((i+reg_addr)<<1),2);//write data to
        //eeprom
        //
        //				I2C_EE_BufRead((uint8_t *)&ee_rd_cheksum,CONF_REG_EE1_ADDR+(CONF_REG_MAP_NUM*2),2);	//read legacy
        //checksum data 				ee_rd_cheksum = ee_rd_cheksum^ee_rd_data^ee_wr_data; 				I2C_EE_BufWrite((uint8_t
        //*)&ee_rd_cheksum,CONF_REG_EE1_ADDR+(CONF_REG_MAP_NUM*2),2);//write checksum data to eeprom 写从机标识
        if (User_ID == USER_CPAD)
        {
            AC_Conf_Write(reg_addr, 2);
        }
        eeprom_tripple_write(i + reg_addr, ee_wr_data, ee_rd_data);
        // event_Recrd
        add_eventlog_fifo(i + reg_addr, (User_ID << 8) | g_sys.status.general.permission_level, ee_rd_data, ee_wr_data);
        //	rt_kprintf("reg_map_write I2C_EE_BufRead ADDR \n");
    }
    return err_code;
}
/**
 * @brief  read register map.
 * @param  reg_addr: reg map address.
 * @param  *rd_data: read data buffer ptr.
 * @retval
 *   This parameter can be one of the following values:
 *     @arg 1: write operation success
 *     @arg 0: read operation fail
 */
uint16 reg_map_read(uint16 reg_addr, uint16 *reg_data, uint8_t read_cnt)
{
    uint16_t i;
    uint16_t err_code;
    extern USHORT cpad_usSRegHoldBuf[CMD_REG_SIZE];
    err_code = CPAD_ERR_NOERR;
    if ((reg_addr & 0x8000) != 0)
    {
        reg_addr &= ABNORMAL_VALUE;
        if (reg_addr > STATUS_REG_MAP_NUM)  // address out of range
        {
            err_code = CPAD_ERR_ADDR_OR;
        }
        else
        {
            for (i = 0; i < read_cnt; i++)
            {
                *(reg_data + i) = *(status_reg_map_inst[reg_addr + i].reg_ptr);  // read data from designated register
            }
        }
    }
    else if ((reg_addr & 0x4000) != 0)
    {
        reg_addr &= 0x3fff;
        if (reg_addr > CMD_REG_SIZE)  // address out of range
        {
            err_code = CPAD_ERR_ADDR_OR;
        }
        else
        {
            for (i = 0; i < read_cnt; i++)
            {
                *(reg_data + i) = cpad_usSRegHoldBuf[reg_addr + i];  // read data from designated register
            }
        }
    }
    else
    {
        reg_addr = reg_addr;
        if (reg_addr > CONF_REG_MAP_NUM)  // address out of range
        {
            err_code = CPAD_ERR_ADDR_OR;
        }
        else
        {
            for (i = 0; i < read_cnt; i++)
            {
                *(reg_data + i) = *(conf_reg_map_inst[reg_addr + i].reg_ptr);  // read data from designated register
            }
        }
    }
    return err_code;
}

/**
 * @brief  show register map content.
 * @param  reg_addr: reg map address.
 * @param  *rd_data: register read count.
 * @retval none
 */
static void show_reg_map(uint16_t reg_addr, uint16_t reg_cnt)
{
    uint16_t reg_buf[32];
    uint16_t i;
    reg_map_read(reg_addr, reg_buf, reg_cnt);
    rt_kprintf("Reg map info:\n");
    for (i = 0; i < reg_cnt; i++)
    {
        rt_kprintf("addr:%d;data:%x\n", (reg_addr + i) & 0x3fff, reg_buf[i]);
    }
}

static uint16_t write_reg_map(uint16_t reg_addr, uint16_t data)
{
    uint16_t ret;
    ret = reg_map_write(reg_addr, &data, 1, 0);
    return ret;
}

//内存修改E2
uint16_t RAM_Write_Reg(uint16_t reg_addr, uint16_t data, uint8_t u8Num)
{
    uint16_t ret;
    ret = reg_map_write(reg_addr, &data, u8Num, USER_DEFAULT);
    return ret;
}

static void read_eeprom(uint16_t offset, uint16_t rd_cnt)
{
    uint8_t rd_reg[32];
    uint16_t i;
    I2C_EE_BufRead((uint8_t *)rd_reg, offset, rd_cnt);
    for (i = 0; i < rd_cnt; i++)
    {
        rt_kprintf("addr:%d,data:%x\n", i + offset, rd_reg[i]);
    }
}

uint8_t reset_runtime(uint16_t param)
{
    uint8_t i, req;

    req = 1;
    if (param == 0xff)
    {
        for (i = 0; i < DO_MAX_CNT; i++)
        {
            g_sys.status.run_time[i].low  = 0;
            g_sys.status.run_time[i].high = 0;
        }
    }

    else
    {
        if (param < (DO_MAX_CNT))
        {
            rt_kprintf("param= %d \n", param);
            g_sys.status.run_time[param].low  = 0;
            g_sys.status.run_time[param].high = 0;
        }
        else
        {
            req = 0;
        }
    }

    if (req == 1)
    {
        I2C_EE_BufWrite((uint8_t *)&g_sys.status.run_time, STS_REG_EE1_ADDR, sizeof(g_sys.status.run_time));
    }

    return (req);
}

uint8_t load_factory_pram(void)
{
    uint8_t req;
    req = 0;
    req = init_load_factory_conf();
    authen_init();
    save_conf_reg(0);
    set_load_flag(0);
    rt_thread_delay(1000);
    NVIC_SystemReset();
    return (req);
}

void write_passward1(uint16_t work_mode)
{
    uint8_t pass[4] = {0x12, 0x34, 0x56, 0x78};

    write_passward(pass, work_mode, 2);

    I2C_EE_BufRead((uint8_t *)&g_sys.status.sys_work_mode.work_mode, WORK_MODE_EE_ADDR, sizeof(work_mode_st));

    rt_kprintf("g_sys.status.sys_work_mode.work_mode = %d\n", g_sys.status.sys_work_mode.work_mode);
    rt_kprintf("g_sys.status.sys_work_mode.limit_time = %d\n", g_sys.status.sys_work_mode.limit_day);
    rt_kprintf("g_sys.status.sys_work_mode.runing_time = %d\n", g_sys.status.sys_work_mode.runing_day);
    rt_kprintf("g_sys.status.sys_work_mode.runing_hour = %d\n", g_sys.status.sys_work_mode.runing_hour);
    rt_kprintf("g_sys.status.sys_work_mode.runing_sec = %d\n", g_sys.status.sys_work_mode.runing_sec);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[0] = %d\n", g_sys.status.sys_work_mode.pass_word[0]);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[1] = %d\n", g_sys.status.sys_work_mode.pass_word[1]);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[2] = %d\n", g_sys.status.sys_work_mode.pass_word[2]);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[3] = %d\n", g_sys.status.sys_work_mode.pass_word[3]);
}

void cpad_work_mode1(uint16_t work_mode)
{
    // cpad_work_mode(work_mode,2);
    rt_kprintf("g_sys.status.sys_work_mode.work_mode = %d\n", g_sys.status.sys_work_mode.work_mode);
    rt_kprintf("g_sys.status.sys_work_mode.limit_time = %d\n", g_sys.status.sys_work_mode.limit_day);
    rt_kprintf("g_sys.status.sys_work_mode.runing_time = %d\n", g_sys.status.sys_work_mode.runing_day);
    rt_kprintf("g_sys.status.sys_work_mode.runing_hour = %d\n", g_sys.status.sys_work_mode.runing_hour);
    rt_kprintf("g_sys.status.sys_work_mode.runing_sec = %d\n", g_sys.status.sys_work_mode.runing_sec);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[0] = %d\n", g_sys.status.sys_work_mode.pass_word[0]);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[1] = %d\n", g_sys.status.sys_work_mode.pass_word[1]);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[2] = %d\n", g_sys.status.sys_work_mode.pass_word[2]);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[3] = %d\n", g_sys.status.sys_work_mode.pass_word[3]);
}

void test_fsm(void)
{
    //		I2C_EE_BufRead((uint8_t*)&g_sys.status.sys_work_mode.work_mode,WORK_MODE_EE_ADDR,sizeof(work_mode_st));
    rt_kprintf("g_sys.status.sys_work_mode.work_mode = %d\n", g_sys.status.sys_work_mode.work_mode);
    rt_kprintf("g_sys.status.sys_work_mode.limit_time = %d\n", g_sys.status.sys_work_mode.limit_day);
    rt_kprintf("g_sys.status.sys_work_mode.runing_time = %d\n", g_sys.status.sys_work_mode.runing_day);
    rt_kprintf("g_sys.status.sys_work_mode.runing_hour = %d\n", g_sys.status.sys_work_mode.runing_hour);
    rt_kprintf("g_sys.status.sys_work_mode.runing_sec = %d\n", g_sys.status.sys_work_mode.runing_sec);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[0] = %d\n", g_sys.status.sys_work_mode.pass_word[0]);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[1] = %d\n", g_sys.status.sys_work_mode.pass_word[1]);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[2] = %d\n", g_sys.status.sys_work_mode.pass_word[2]);
    rt_kprintf("g_sys.status.sys_work_mode.pass_word[3] = %d\n", g_sys.status.sys_work_mode.pass_word[3]);
}

static void sys_dbg(uint16_t flag)
{
    if (flag == 1)
    {
        l_sys.debug_flag    = DEBUG_ON_FLAG;
        l_sys.debug_tiemout = DEBUG_TIMEOUT_MAX;
    }
    else if (flag == 223)
    {
        l_sys.debug_flag    = DEBUG_ON_FLAG;
        l_sys.debug_tiemout = DEBUG_TIMEOUT_NA;
    }
    else
    {
        l_sys.debug_flag    = DEBUG_OFF_FLAG;
        l_sys.debug_tiemout = 0;
    }
}

void eeprom_addr(void)
{
    rt_kprintf("user1 conf  start =%d ,end =%d ,size = %d\n", CONF_REG_EE1_ADDR, CONF_REG_EE2_ADDR,
               (CONF_REG_EE2_ADDR - CONF_REG_EE1_ADDR));
    rt_kprintf("user2 conf  start =%d ,end =%d ,size = %d\n", CONF_REG_EE2_ADDR, CONF_REG_EE3_ADDR,
               (CONF_REG_EE3_ADDR - CONF_REG_EE2_ADDR));
    rt_kprintf("user3 conf  start =%d ,end =%d ,size = %d\n", CONF_REG_EE3_ADDR, CONF_REG_FACT_ADDR,
               (CONF_REG_FACT_ADDR - CONF_REG_EE3_ADDR));
    rt_kprintf("fact  conf  start =%d ,end =%d ,size = %d\n", CONF_REG_FACT_ADDR, STS_REG_EE1_ADDR,
               (STS_REG_EE1_ADDR - CONF_REG_FACT_ADDR));
    rt_kprintf("status reg1 start =%d ,end =%d ,size = %d\n", STS_REG_EE1_ADDR, STS_REG_EE2_ADDR,
               (STS_REG_EE2_ADDR - STS_REG_EE1_ADDR));
    rt_kprintf("status reg2 start =%d ,end =%d ,size = %d\n", STS_REG_EE2_ADDR, WORK_MODE_EE_ADDR,
               (WORK_MODE_EE_ADDR - STS_REG_EE2_ADDR));
    rt_kprintf("work_mode   start =%d ,end =%d ,size = %d\n", WORK_MODE_EE_ADDR, AlARM_TABLE_ADDR,
               (AlARM_TABLE_ADDR - WORK_MODE_EE_ADDR));
    rt_kprintf("alarm_table start =%d ,end =%d ,size = %d\n", AlARM_TABLE_ADDR, TEM_HUM_PT_ADDR,
               (TEM_HUM_PT_ADDR - AlARM_TABLE_ADDR));
    rt_kprintf("TEM_HUM_REC start =%d ,end =%d ,size = %d\n", TEM_HUM_PT_ADDR, ALARM_REC_PT_ADDR,
               (ALARM_REC_PT_ADDR - TEM_HUM_PT_ADDR));
    rt_kprintf("ALARM_REC   start =%d ,end =%d ,size = %d\n", ALARM_REC_PT_ADDR, EVENT_REC_PT_ADDR,
               (EVENT_REC_PT_ADDR - ALARM_REC_PT_ADDR));
    rt_kprintf("EVENT_REC   start =%d ,end =%d ,size = %d\n", EVENT_REC_PT_ADDR, EE_REC_END,
               (EE_REC_END - EVENT_REC_PT_ADDR));
}
FINSH_FUNCTION_EXPORT(eeprom_addr, show_ee_addr_table);
FINSH_FUNCTION_EXPORT(sys_dbg, system debug switchs.);
// FINSH_FUNCTION_EXPORT(test_fsm, test_test_fasm.);
// FINSH_FUNCTION_EXPORT(write_passward1, test_test_cpad_work_mode.);
// FINSH_FUNCTION_EXPORT(cpad_work_mode1, test_test_write_passward.);
FINSH_FUNCTION_EXPORT(reset_runtime, reset run_time eeprom &regs.);
FINSH_FUNCTION_EXPORT(show_reg_map, show registers map.);
FINSH_FUNCTION_EXPORT(write_reg_map, write data into conf registers.);
FINSH_FUNCTION_EXPORT(set_load_flag, set sys init load option.);
FINSH_FUNCTION_EXPORT(save_conf_reg, save current conf reg data.);
FINSH_FUNCTION_EXPORT(read_eeprom, read eeprom content eeprom flag.);

#include <rtthread.h>
#include "sys_conf.h"
#include "calc.h"
#include "local_status.h"
#include "team.h"
#include "i2c_bsp.h"
#include "rtc_bsp.h"
#include "event_record.h"
#include "daq.h"
#include "sys_status.h"
#include "watchdog_bsp.h"
#include "password.h"
#include "req_execution.h"
#include "dio_bsp.h"

static void sys_comp_cooldown(void);
static void run_time_process(void);
static void team_tab_cooldown(void);
static void team_timer(void);
static void check_team_config(void);
static void sys_debug_timeout(void);
//static void analog_dummy_out(void);
static void ec_fan_diff_req(void);
static void ec_fan_suc_temp(void);
//extern void work_mode_manage(void);
static void temp_hum_calc(void);
static void test_mode_init_data(void);
static void inv_compress_alarm_req(void);
static void hipress_avg_calc(void);
static void BKG_Var_init(void);


//掉电提示
void power_loss_delay(void)
{
		static uint16_t delay =0;
	
		if(delay ++ >30)
		{
				delay = 11;
			  sys_option_di_sts(DI_POWER_LOSS_BPOS,0);
			
		}
}

time_t Get_Systime(void)
{
		time_t System_Time;
		
		get_local_time(&System_Time);
		g_sys.status.sys_info.Sys_Time.u32Systime=System_Time;
		return System_Time;
}

/**
  * @brief 	output control module components cooldown 
	* @param  none
	* @retval none
  */
void bkg_thread_entry(void* parameter)
{				
		//初始化温湿度曲线记录 
		extern sys_reg_st	g_sys;

		rt_thread_delay(BKG_THREAD_DELAY);
		init_tem_hum_record();
		watchdog_init();
		BKG_Var_init();		
		while(1)
		{
      led_toggle();
			team_tab_cooldown();
			team_timer();
			sys_comp_cooldown();
			run_time_process();
      check_team_config();
			ec_fan_diff_req();
			ec_fan_suc_temp();
			user_eventlog_add();
			user_alarmlog_add();
			temp_hum_calc();
				
			work_mode_manage();
			add_hum_temp_log();
			sys_running_mode_update();
			sys_debug_timeout();
	
			inv_compress_alarm_req();
			hipress_avg_calc();
// test mode
			test_mode_init_data();
			power_loss_delay();
			
			Get_Systime();//获取系统时间
			dog();
			rt_thread_delay(1000);
		}
}


static void BKG_Var_init(void)
{
			//临时写到此处
		g_sys.status.inv_compress_alarm.inv_alarm_counter = 0;
//		g_sys.status.inv_compress_alarm.avg_hi_press[0] = 0;
//		g_sys.status.inv_compress_alarm.avg_hi_press[1] = 0;
//		g_sys.status.inv_compress_alarm.avg_hi_press[2] = 0;
		memset(g_sys.status.inv_compress_alarm.avg_hi_press,0x00,sizeof(g_sys.status.inv_compress_alarm.avg_hi_press));
		g_sys.status.inv_compress_alarm.counter_hipress = 0;

		g_sys.status.inv_compress_alarm.Inv_hi_temp_Count = 0;	
		g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag = 0;	
		g_sys.status.inv_compress_alarm.counter_hi_Temperature=0;
		memset(g_sys.status.inv_compress_alarm.Inv_hi_Temperature,0x00,sizeof(g_sys.status.inv_compress_alarm.Inv_hi_Temperature));
	
}

static void temp_hum_calc(void)
{
		extern sys_reg_st					g_sys; 
		g_sys.status.sys_tem_hum.return_air_hum = get_current_hum(TARGET_MODE_RETURN);
		g_sys.status.sys_tem_hum.return_air_temp = get_current_temp(TARGET_MODE_RETURN);	
		g_sys.status.sys_tem_hum.supply_air_hum = get_current_hum(TARGET_MODE_SUPPLY);
		g_sys.status.sys_tem_hum.supply_air_temp = get_current_temp(TARGET_MODE_SUPPLY);
		g_sys.status.sys_tem_hum.remote_air_hum = get_current_hum(TARGET_MODE_REMOTE);
		g_sys.status.sys_tem_hum.remote_air_temp = get_current_temp(TARGET_MODE_REMOTE);
		g_sys.status.sys_tem_hum.return_air_max_temp = get_current_max_temp(TARGET_MODE_RETURN);
		g_sys.status.sys_tem_hum.supply_air_max_temp = get_current_max_temp(TARGET_MODE_SUPPLY);
		g_sys.status.sys_tem_hum.remote_air_max_temp = get_current_max_temp(TARGET_MODE_REMOTE);
	  g_sys.status.sys_tem_hum.supply_air_min_temp = get_current_min_temp(TARGET_MODE_SUPPLY);
//				rt_kprintf("return_air_hum=%x,return_air_temp= %x\n",g_sys.status.sys_tem_hum.return_air_hum,g_sys.status.sys_tem_hum.return_air_temp);			
}



static void test_mode_init_data(void)
{
		extern sys_reg_st					g_sys; 
		extern local_reg_st 				l_sys;	
	
		if(g_sys.config.general.testing_mode_en == 1)//测试模式
		{
//				g_sys.config.dev_mask.din_bitmap_polarity[0]=0xB0;
				g_sys.config.dev_mask.din_bitmap_polarity[0]=0x2C0;
				g_sys.config.dev_mask.din_bitmap_polarity[1]=0x00;
				g_sys.config.dev_mask.ain =0xffff;
				g_sys.config.dev_mask.din[0] = 0xffff;
				g_sys.config.dev_mask.din[1] = 0xffff;
				g_sys.config.dev_mask.aout = 0xffff;
        g_sys.config.dev_mask.mb_comp = 0xffff;
        g_sys.config.dev_mask.dout = 0xffff;
			
				l_sys.bitmap[BITMAP_MANUAL] = 0xffff;
			  l_sys.ao_list[AO_EC_FAN][BITMAP_MANUAL] = 50;  
			  l_sys.ao_list[AO_EC_COMPRESSOR][BITMAP_MANUAL] = 50;
			  l_sys.ao_list[AO_WATER_VALVE][BITMAP_MANUAL] = 50;
//			  l_sys.ao_list[AO_PREV_1][BITMAP_MANUAL] = 100;
			  l_sys.ao_list[AO_EX_FAN][BITMAP_MANUAL] = 50;
//				l_sys.pwm_list[PWM_OUT0][BITMAP_MANUAL] = 100;
//			  l_sys.pwm_list[PWM_OUT1][BITMAP_MANUAL] = 100;
			
				g_sys.config.team.team_en=1;
			
		}
		
}


//static void ec_fan_timer_init(void)
//{
//	g_sys.status.flow_diff_timer = g_sys.config.fan.flow_diff_delay;
//	g_sys.status.return_air_status.timer= 0; 
//}
//static void ec_fan_timer_run(void)
//{
//		if(g_sys.status.return_air_status.timer>0)
//		{
//			 g_sys.status.return_air_status.timer --;
//		}
//		else
//		{
//				g_sys.status.return_air_status.timer = 0;
//		}
//		if(g_sys.status.flow_diff_timer>0)
//		{
//			 g_sys.status.flow_diff_timer --;
//		}
//		else
//		{
//				g_sys.status.flow_diff_timer = 0;
//		}
//}
static void ec_fan_diff_req(void)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st 	l_sys;
		extern team_local_st team_local_inst;
		int16_t ec_fan_diff_tmp;
		static uint16_t timer_cd = 0;
		uint16_t set_diff,diff_deaszone,fan_step,diff_delay;
	
		if(((team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STATUS]&(0x0001<<TEAM_STS_CONFD_BPOS))==0)||
		(g_sys.config.team.team_en == 0)||(g_sys.config.team.addr >(team_local_inst.team_config[TEAM_CONF_CNT]&0x00ff)))
		{
				set_diff = g_sys.config.fan.set_flow_diff;		
				diff_deaszone = g_sys.config.fan.flow_diff_deadzone;
				fan_step = g_sys.config.fan.flow_diff_step;    
				diff_delay =  g_sys.config.fan.flow_diff_delay;
		}
		else
		{
				set_diff = team_local_inst.team_config[TEAM_CONF_FAN_SET_FLOW_DIFF];		
				diff_deaszone = team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_DEADZONE];
				fan_step = team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_STEP];    
				diff_delay = team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_DELAY];
		}	
		
		
		if(((g_sys.config.dev_mask.ain) & (0x01<<AI_SENSOR1))&&(g_sys.config.fan.mode == FAN_MODE_PRESS_DIFF)
			&&(g_sys.status.ain[AI_SENSOR1]!=ABNORMAL_VALUE))				
		{
				//Alair,20170318，AI复用
				if(g_sys.config.dev_mask.ain&0x8000)
				{
						return;
				}			
				
				if(timer_cd != 0)
				{
						timer_cd--;
						ec_fan_diff_tmp = l_sys.ec_fan_diff_reg;
				}
				else
				{
						timer_cd = diff_delay;
						if(g_sys.status.ain[AI_SENSOR1] > ( set_diff + diff_deaszone))
						{
								ec_fan_diff_tmp = l_sys.ec_fan_diff_reg - fan_step;
						}
						else if(g_sys.status.ain[AI_SENSOR1] < (set_diff - diff_deaszone))
						{
								ec_fan_diff_tmp = l_sys.ec_fan_diff_reg + fan_step;
						}
						else
						{
								ec_fan_diff_tmp = l_sys.ec_fan_diff_reg;
						}
				}
				
				l_sys.ec_fan_diff_reg = lim_min_max((int16_t)(g_sys.config.fan.min_speed - g_sys.config.fan.set_speed),(int16_t)(g_sys.config.fan.max_speed - g_sys.config.fan.set_speed),ec_fan_diff_tmp);
		}
		else
		{
				l_sys.ec_fan_diff_reg = 0;
		}
}

enum
{
		FSM_SUC_TEMP_IDLE,
		FSM_SUC_TEMP_DECRESS
};

//回气温度降风速控制
static void ec_fan_suc_temp(void)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st 	l_sys;
		int16_t suc_temp_reg;
		static uint16_t timer_cd = 0;	
		static uint16_t suc_temp_fsm = FSM_SUC_TEMP_IDLE;
	
		if(((g_sys.config.dev_mask.ain) & (0x01<<AI_COMP_RETURN1))&&((g_sys.config.general.cool_type == COOL_TYPE_MODULE_WIND)||(g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND)))
		{
				if(timer_cd != 0)
				{
						timer_cd--;
						suc_temp_reg = l_sys.ec_fan_suc_temp;
				}
				else 
				{
						timer_cd = g_sys.config.fan.suc_temp_delay;
						switch(suc_temp_fsm)
						{
								case FSM_SUC_TEMP_IDLE:
								{
										if(g_sys.status.ain[AI_COMP_RETURN1] > g_sys.config.fan.target_suc_temp)
										{
												suc_temp_fsm = FSM_SUC_TEMP_DECRESS;
												suc_temp_reg = -g_sys.config.fan.suc_temp_step; 
										}
										else
										{
												suc_temp_fsm = FSM_SUC_TEMP_IDLE;
												suc_temp_reg = 0; 
										}
										break;
								}
								case FSM_SUC_TEMP_DECRESS:
								{
										if(g_sys.status.ain[AI_COMP_RETURN1] <= (g_sys.config.fan.target_suc_temp-g_sys.config.fan.suc_temp_deadzone))
										{
												suc_temp_fsm = FSM_SUC_TEMP_IDLE;
												suc_temp_reg = 0; 
										}
										else
										{
												suc_temp_fsm = FSM_SUC_TEMP_DECRESS;
												suc_temp_reg = l_sys.ec_fan_suc_temp - g_sys.config.fan.suc_temp_step; 
										}
										break;									
								}
								default:
								{
										suc_temp_fsm = FSM_SUC_TEMP_IDLE;
										suc_temp_reg = 0; 
										break;
								}								
						}			
				}
				l_sys.ec_fan_suc_temp = lim_min_max((int16_t)(g_sys.config.fan.min_speed - g_sys.config.fan.set_speed),(int16_t)(g_sys.config.fan.max_speed - g_sys.config.fan.set_speed),suc_temp_reg);
		}
		else
		{
				l_sys.ec_fan_suc_temp = 0;
		}
}

static void inv_compress_alarm_req(void)
{
//Alair,20170311,取消三次高压停压机
		return;
//			time_t now;
//			get_local_time(&now);
//			if((get_inv_comp_freq_down_signal(HIGH_PRESS) > 0) && (g_sys.status.inv_compress_alarm.inv_hipress_tmp == 0))
//			{
//				g_sys.status.inv_compress_alarm.inv_alarm_counter ++;
//				g_sys.status.inv_compress_alarm.inv_start_time[0] = g_sys.status.inv_compress_alarm.inv_start_time[1];
//				g_sys.status.inv_compress_alarm.inv_start_time[1] = g_sys.status.inv_compress_alarm.inv_start_time[2];
//				g_sys.status.inv_compress_alarm.inv_start_time[2] = now;

//				g_sys.status.inv_compress_alarm.inv_hipress_tmp = get_inv_comp_freq_down_signal(HIGH_PRESS);
//				g_sys.status.inv_compress_alarm.inv_hipress_flag = 0;
//				if(g_sys.status.inv_compress_alarm.inv_alarm_counter > 2)
//				{
//					g_sys.status.inv_compress_alarm.inv_alarm_counter = 0;
//					g_sys.status.inv_compress_alarm.inv_hipress_flag = 1;
////					rt_kprintf("triggle!\r\n");
//				}
//			}else if((get_inv_comp_freq_down_signal(HIGH_PRESS) == 0) && (g_sys.status.inv_compress_alarm.inv_hipress_tmp == 1))
//			{
//				g_sys.status.inv_compress_alarm.inv_hipress_tmp = 0;
//				g_sys.status.inv_compress_alarm.inv_hipress_stop_flag = 1;
////				rt_kprintf("time:%d\n",g_sys.status.inv_compress_alarm.inv_stop_time[0]);
////				g_sys.status.inv_compress_alarm.inv_hipress_flag = 0;
//			}	
//			else if(get_inv_comp_freq_down_signal(HIGH_PRESS) < 0)
//				{
//					g_sys.status.inv_compress_alarm.inv_hipress_tmp = 0;
//					g_sys.status.inv_compress_alarm.inv_hipress_tmp = 0;
//					g_sys.status.inv_compress_alarm.inv_stop_time[0] = 0xffffffff;
//					g_sys.status.inv_compress_alarm.inv_stop_time[1] = 0xffffffff;
//					g_sys.status.inv_compress_alarm.inv_stop_time[2] = 0xffffffff;
//				}
//				
//			if((get_inv_comp_freq_down_signal(HIGH_PRESS) == 0) && g_sys.status.inv_compress_alarm.inv_hipress_stop_flag == 1)
//			{
//				g_sys.status.inv_compress_alarm.inv_stop_time[0] = g_sys.status.inv_compress_alarm.inv_stop_time[1];
//				g_sys.status.inv_compress_alarm.inv_stop_time[1] = g_sys.status.inv_compress_alarm.inv_stop_time[2];
//				g_sys.status.inv_compress_alarm.inv_stop_time[2] = now;
//				g_sys.status.inv_compress_alarm.inv_hipress_stop_flag = 0;
//			}
}

static void hipress_avg_calc(void)
{
	uint16_t Buffer;
	//压力取值
	if(g_sys.status.ain[AI_HI_PRESS_SENSOR1]>=ABNORMAL_VALUE)
	{
		g_sys.status.inv_compress_alarm.avg_hi_press[GET_AVG]=0;		
	}
	else
	{	
		//压力取值
		g_sys.status.inv_compress_alarm.avg_hi_press[GET_ZERO] = g_sys.status.inv_compress_alarm.avg_hi_press[GET_ONE];
		g_sys.status.inv_compress_alarm.avg_hi_press[GET_ONE] = g_sys.status.inv_compress_alarm.avg_hi_press[GET_TWO];
		g_sys.status.inv_compress_alarm.avg_hi_press[GET_TWO] = g_sys.status.ain[AI_HI_PRESS_SENSOR1];
		
	//	rt_kprintf("counter_hipress:%d\n",g_sys.status.inv_compress_alarm.counter_hipress);
		g_sys.status.inv_compress_alarm.counter_hipress ++;
		if(g_sys.status.inv_compress_alarm.counter_hipress > 2)
		{
			g_sys.status.inv_compress_alarm.counter_hipress = 0;
			Buffer=g_sys.status.inv_compress_alarm.avg_hi_press[GET_ZERO];
			Buffer+=g_sys.status.inv_compress_alarm.avg_hi_press[GET_ONE];
			Buffer+=g_sys.status.inv_compress_alarm.avg_hi_press[GET_TWO];
			g_sys.status.inv_compress_alarm.avg_hi_press[GET_AVG] = Buffer / 3;
		}
	}
//	rt_kprintf("g_sys.status.inv_compress_alarm.avg_hi_press[GET_AVG]:%d\n",g_sys.status.inv_compress_alarm.avg_hi_press[GET_AVG]);
//		rt_kprintf("g_sys.status.ain[AI_HI_PRESS_SENSOR1]:%d\n",g_sys.status.ain[AI_HI_PRESS_SENSOR1]);
	
	//Alair,20170301，排气温度取值
	if(g_sys.status.ain[AI_COMP_EXHAUST1]>=ABNORMAL_VALUE)
	{
		g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_AVG]=0;		
	}
	else
	{	
		g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_ZERO] = g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_ONE];
		g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_ONE] = g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_TWO];
		g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_TWO] = g_sys.status.ain[AI_COMP_EXHAUST1];
		
	//	rt_kprintf("counter_hipress:%d\n",g_sys.status.inv_compress_alarm.counter_hipress);
		g_sys.status.inv_compress_alarm.counter_hi_Temperature ++;
		if(g_sys.status.inv_compress_alarm.counter_hi_Temperature > 2)
		{
			g_sys.status.inv_compress_alarm.counter_hi_Temperature = 0;
			Buffer=g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_ZERO];
			Buffer+=g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_ONE];
			Buffer+=g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_TWO];
			g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_AVG] = Buffer / 3;
		}	
	}
	
}

static void team_tab_cooldown(void)
{
		extern team_local_st team_local_inst;
		uint16_t i;
		for(i = 0;i < TEAM_MAX_SLAVE_NUM; i++)
		{
				if(team_local_inst.team_table[i][TEAM_TAB_TIMEOUT] > 0)
				{
						team_local_inst.team_table[i][TEAM_TAB_TIMEOUT]--;
				}		
		}
}


static void team_timer(void)
{
		extern sys_reg_st		g_sys;
		extern team_local_st team_local_inst;

		if((team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STATUS]&(0x0001<<TEAM_STS_RUN_BPOS)) != 0)
		{
				team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STOP_TIME] = 0;
				team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_RUN_TIME]++;
		}
		else
		{
				team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STOP_TIME] ++;
				team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_RUN_TIME] = 0;				
		}
    
    if(team_local_inst.rotate_timeout < ROTATE_TIMEOUT_CNT)
    {
        team_local_inst.rotate_timeout++;
    }
		
		if(team_local_inst.run_sts_cd > 0)
		{
				team_local_inst.run_sts_cd--;
		}
}


/**
  * @brief 	system components runtime counter 
	* @param  none
	* @retval none
  */
static void time_calc(uint16_t*  sec,uint16_t* h)
{
		uint16_t second;
		uint16_t hour;
	
		second = *sec;
		hour = *h;
		if((second&0x0fff) >= 3600)
		{
				
				second = (second &0xf000)+0x1000;
				
				if(second == 0 )
				{
						hour++;
					  *h = hour;
				}
			 *sec = second;
		}
}
////HUAWEI
//static void fans_run_time(void)
//{
//		uint8_t i,base_pos;
//		extern sys_reg_st		g_sys; 

//		if((g_sys.status.dout_bitmap&(0x0001<<DO_FAN_BPOS)) != 0)
//		{		
//				
//				base_pos= DO_FAN2_DUMMY_BPOS;
//				// fan 2,3,4,5
//				for(i =0 ; i < 4;  i++)
//				{
//						if(get_alarm_bitmap(ACL_FAN_OVERLOAD2 + i) == 0)
//						{
//								g_sys.status.run_time[base_pos+i].low++;
//							
//								time_calc(&g_sys.status.run_time[base_pos+i].low,&g_sys.status.run_time[base_pos+i].high);
//						}
//				}
//			//列间冷冻水
//			if(g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER)
//			{
//					if(g_sys.config.dev_mask.din[0]&(0x01<<DI_FAN06_OD_BPOS))
//					{
//							if(get_alarm_bitmap(ACL_FAN_OVERLOAD6 ) == 0)
//							{
//									g_sys.status.run_time[DO_FAN6_DUMMY_BPOS].low++;
//									time_calc(&g_sys.status.run_time[DO_FAN6_DUMMY_BPOS].low,&g_sys.status.run_time[DO_FAN6_DUMMY_BPOS].high);
//							}
//							
//					}
//					if(g_sys.config.dev_mask.din[0]&(0x01<<DI_FAN07_OD_BPOS))
//					{
//							if(get_alarm_bitmap(ACL_FAN_OVERLOAD7 ) == 0)
//							{
//									g_sys.status.run_time[DO_FAN7_DUMMY_BPOS].low++;
//									time_calc(&g_sys.status.run_time[DO_FAN7_DUMMY_BPOS].low,&g_sys.status.run_time[DO_FAN7_DUMMY_BPOS].high);
//							}		
//					}
//					if(g_sys.config.dev_mask.din[0]&(0x01<<DI_FAN08_OD_BPOS))
//					{
//								if(get_alarm_bitmap(ACL_FAN_OVERLOAD8 ) == 0)
//								{
//										g_sys.status.run_time[DO_FAN8_DUMMY_BPOS].low++;
//										time_calc(&g_sys.status.run_time[DO_FAN8_DUMMY_BPOS].low,&g_sys.status.run_time[DO_FAN8_DUMMY_BPOS].high);
//								}		
//							
//					}
//			}		
//		}
//}

	
static void run_time_process(void)
{
		extern sys_reg_st		g_sys; 
    extern team_local_st team_local_inst;
//		time_t now;
		uint8_t i;
		static uint16_t u16_Sec=0;
		
		for(i=0; i<DO_FILLTER_DUMMY_BPOS; i++)
		{
				if((g_sys.status.dout_bitmap&(0x0001<<i)) != 0)
				{
						g_sys.status.run_time[i].low++;
						
						time_calc(&g_sys.status.run_time[i].low,&g_sys.status.run_time[i].high);
				}				
		}
		//过滤网运行时间累计
		if((g_sys.status.dout_bitmap&(0x0001<<DO_FAN_BPOS)) != 0)
		{	
				g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low++;
				time_calc(&g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low,&g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].high);
		}
//		// HAUWEI 风机运行时间扩展
//		fans_run_time();
		u16_Sec++;
		if(u16_Sec%900==0)
		{
				u16_Sec=0;
				I2C_EE_BufWrite((uint8_t *)&g_sys.status.run_time,STS_REG_EE1_ADDR,sizeof(g_sys.status.run_time));		//when, fan is working update eeprom every minite		
				//bitmapset backup
        I2C_EE_BufWrite((uint8_t *)&team_local_inst.team_bitmap[TEAM_BITMAP_FINAL_OUT],STS_REG_EE2_ADDR,sizeof(team_local_inst.team_bitmap[TEAM_BITMAP_FINAL_OUT]));			
		}
//		get_local_time(&now);
//		if((now&0x0000007f) == 0x7f)
//		{
//				I2C_EE_BufWrite((uint8_t *)&g_sys.status.run_time,STS_REG_EE1_ADDR,sizeof(g_sys.status.run_time));		//when, fan is working update eeprom every minite		
//		}
//    // bitmapset backup
//    if((now & 0x0000007f) == 0x7f)
//    {
//        I2C_EE_BufWrite((uint8_t *)&team_local_inst.team_bitmap[TEAM_BITMAP_FINAL_OUT],STS_REG_EE2_ADDR,sizeof(team_local_inst.team_bitmap[TEAM_BITMAP_FINAL_OUT]));
//    }
		
//		rt_kprintf("[DO_FAN_BPOS]high= %x,[DO_FAN_BPOS]low= %x,[DO_RH1_BPOS]high= %x,[DO_RH1_BPOS]low= %x,[DO_RH2_BPOS]high= %x,[DO_RH2_BPOS]low= %x,[0]high= %x,[0]low= %x,[0]high= %x ,[0]low= %x,[0]high= %x,[0]low= %x\n",g_sys.status.run_time[DO_FAN_BPOS].high,g_sys.status.run_time[DO_FAN_BPOS].low
//		,g_sys.status.run_time[DO_RH1_BPOS].high,g_sys.status.run_time[DO_RH1_BPOS].low,g_sys.status.run_time[DO_RH2_BPOS].high,g_sys.status.run_time[DO_RH2_BPOS].low
//		,g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].high,g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low,g_sys.status.run_time[DO_HUM_BPOS].high,g_sys.status.run_time[DO_HUM_BPOS].low
//		,g_sys.status.run_time[DO_COMP1_BPOS].high,g_sys.status.run_time[DO_COMP1_BPOS].low);		
}

static void sys_comp_cooldown(void)
{
		extern local_reg_st	l_sys;
		uint16_t i;

		for(i=0;i<DO_MAX_CNT;i++)
		{
				if(l_sys.comp_timeout[i]>0)
				{	
						l_sys.comp_timeout[i]--;
				}	
				else
				{
						l_sys.comp_timeout[i]=0;
				}
		}	
		
		if(l_sys.comp_startup_interval > 0)
		{
				l_sys.comp_startup_interval--;
		}
		else
		{
				l_sys.comp_startup_interval = 0;
		}
		
		if(l_sys.watervalve_warmup_delay > 0)
		{
				l_sys.watervalve_warmup_delay--;
		}
		else
		{
				l_sys.watervalve_warmup_delay = 0;
		}
	
		if(l_sys.Humer_Delay > 0)
		{
				l_sys.Humer_Delay--;
		}
		else
		{
				l_sys.Humer_Delay = 0;
		}
		//Alair,20170302
		if(g_sys.config.compressor.type == COMP_QABP)
		{				
				if(l_sys.Comp_Delay > 0)
				{
						l_sys.Comp_Delay--;
				}
				else
				{
						l_sys.Comp_Delay = 0;
				}
		}
}


static void check_team_config(void)
{
    extern sys_reg_st     g_sys;
    extern team_local_st  team_local_inst;
    static uint8_t check_timeout;
    if((team_local_inst.team_fsm == TEAM_FSM_MASTER) && (g_sys.config.team.addr == 1))
    {
        if(check_timeout > CHECK_TEAM_CONFIG_TIMEOUT)
        {
            if((team_local_inst.team_config[TEAM_CONF_TEMP_DEADBAND]        != g_sys.config.algorithm.temp_deadband)    ||
               (team_local_inst.team_config[TEAM_CONF_TEMP_PRECISION]       != g_sys.config.algorithm.temp_precision)   ||
               ( check_target_temp_hum())||
               (team_local_inst.team_config[TEAM_CONF_HUM_DEADBAND]         != g_sys.config.algorithm.hum_deadband)     ||
               (team_local_inst.team_config[TEAM_CONF_HUM_PRECISION]        != g_sys.config.algorithm.hum_precision)    ||
               (team_local_inst.team_config[TEAM_CONF_ALARM_HI_TEMP_PARAM]  != g_sys.config.alarm[ACL_HI_TEMP_RETURN].alarm_param)  ||
               (team_local_inst.team_config[TEAM_CONF_ALARM_LO_TEMP_PARAM]  != g_sys.config.alarm[ACL_LO_TEMP_RETURN].alarm_param)  ||
               (team_local_inst.team_config[TEAM_CONF_ALARM_HI_HUM_PARAM]   != g_sys.config.alarm[ACL_HI_HUM_RETURN].alarm_param)   ||
               (team_local_inst.team_config[TEAM_CONF_ALARM_LO_HUM_PARAM]   != g_sys.config.alarm[ACL_LO_HUM_RETURN].alarm_param)		||
							 (team_local_inst.team_config[TEAM_CONF_FAN_MODE] 						!= g_sys.config.team.team_fan_mode)		||
							 (team_local_inst.team_config[TEAM_CONF_FAN_TARGET_TEMP] 			!= g_sys.config.fan.target_temp)		||
							 (team_local_inst.team_config[TEAM_CONF_FAN_TEMP_DEADBAND] 		!= g_sys.config.fan.temp_dead_band)		||
							 (team_local_inst.team_config[TEAM_CONF_FAN_TEMP_PRECISION]		!= g_sys.config.fan.temp_precision)	||
							 (team_local_inst.team_config[TEAM_CONF_FAN_SET_FLOW_DIFF]		!= g_sys.config.fan.set_flow_diff)	||
						 	 (team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_DEADZONE]!= g_sys.config.fan.flow_diff_deadzone)	||
							 (team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_STEP]		!= g_sys.config.fan.flow_diff_step)	||
							 (team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_DELAY]	!= g_sys.config.fan.flow_diff_delay)	)
            {
//                rt_kprintf("%x %x %x %x %x %x %x %x %x %x\n", g_sys.config.algorithm.temp_deadband,
//                                                              g_sys.config.algorithm.temp_precision,
//                                                              (g_sys.config.algorithm.ctrl_target_mode == 0)? g_sys.config.algorithm.return_air_temp:g_sys.config.algorithm.supply_air_temp,
//                                                              g_sys.config.algorithm.hum_deadband,
//                                                              g_sys.config.algorithm.hum_precision,
//                                                              (g_sys.config.algorithm.ctrl_target_mode == 0)? g_sys.config.algorithm.return_air_hum:g_sys.config.algorithm.supply_air_hum,
//                                                              g_sys.config.alarm[ACL_HI_TEMP_RETURN].alarm_param,
//                                                              g_sys.config.alarm[ACL_LO_TEMP_RETURN].alarm_param,
//                                                              g_sys.config.alarm[ACL_HI_HUM_RETURN].alarm_param,
//                                                              g_sys.config.alarm[ACL_LO_HUM_RETURN].alarm_param);
//                rt_kprintf("%x %x %x %x %x %x %x %x %x %x\n", team_local_inst.team_config[TEAM_CONF_TEMP_DEADBAND],
//                                                              team_local_inst.team_config[TEAM_CONF_TEMP_PRECISION],
//                                                              team_local_inst.team_config[TEAM_CONF_TEMP_SETVAL],
//                                                              team_local_inst.team_config[TEAM_CONF_HUM_DEADBAND],
//                                                              team_local_inst.team_config[TEAM_CONF_HUM_PRECISION],
//                                                              team_local_inst.team_config[TEAM_CONF_HUM_SETVAL],
//                                                              team_local_inst.team_config[TEAM_CONF_ALARM_HI_TEMP_PARAM],
//                                                              team_local_inst.team_config[TEAM_CONF_ALARM_LO_TEMP_PARAM],
//                                                              team_local_inst.team_config[TEAM_CONF_ALARM_HI_HUM_PARAM],
//                                                              team_local_inst.team_config[TEAM_CONF_ALARM_LO_HUM_PARAM]);
              
                team_local_inst.team_config[TEAM_CONF_TEMP_DEADBAND]        = g_sys.config.algorithm.temp_deadband;
                team_local_inst.team_config[TEAM_CONF_TEMP_PRECISION]       = g_sys.config.algorithm.temp_precision;
                team_local_inst.team_config[TEAM_CONF_HUM_DEADBAND]         = g_sys.config.algorithm.hum_deadband;
                team_local_inst.team_config[TEAM_CONF_HUM_PRECISION]        = g_sys.config.algorithm.hum_precision;
                team_local_inst.team_config[TEAM_CONF_ALARM_HI_TEMP_PARAM]  = g_sys.config.alarm[ACL_HI_TEMP_RETURN].alarm_param;
                team_local_inst.team_config[TEAM_CONF_ALARM_LO_TEMP_PARAM]  = g_sys.config.alarm[ACL_LO_TEMP_RETURN].alarm_param;
                team_local_inst.team_config[TEAM_CONF_ALARM_HI_HUM_PARAM]   = g_sys.config.alarm[ACL_HI_HUM_RETURN].alarm_param;
                team_local_inst.team_config[TEAM_CONF_ALARM_LO_HUM_PARAM]   = g_sys.config.alarm[ACL_LO_HUM_RETURN].alarm_param;
								team_local_inst.team_config[TEAM_CONF_FAN_MODE]							= g_sys.config.team.team_fan_mode;
								team_local_inst.team_config[TEAM_CONF_FAN_TARGET_TEMP]			= g_sys.config.fan.target_temp;
								team_local_inst.team_config[TEAM_CONF_FAN_TEMP_DEADBAND]		= g_sys.config.fan.temp_dead_band;
								team_local_inst.team_config[TEAM_CONF_FAN_TEMP_PRECISION]		= g_sys.config.fan.temp_precision;
								team_local_inst.team_config[TEAM_CONF_FAN_SET_FLOW_DIFF]		= g_sys.config.fan.set_flow_diff;
								team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_DEADZONE]= g_sys.config.fan.flow_diff_deadzone;
								team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_STEP]		= g_sys.config.fan.flow_diff_step;
								team_local_inst.team_config[TEAM_CONF_FAN_FLOW_DIFF_DELAY]	= g_sys.config.fan.flow_diff_delay;
                team_clear_confd();
                rt_kprintf("config has changed!\n");
            }
            check_timeout = 0;
        }
        check_timeout++;
    }
    if(team_local_inst.master_send_param_protect > 0)
    {
        team_local_inst.master_send_param_protect--;
    }
}

static void sys_debug_timeout(void)
{
		extern local_reg_st	l_sys;
		if(l_sys.debug_tiemout == DEBUG_TIMEOUT_NA)
		{
				return;
		}
		else if(l_sys.debug_tiemout > 0)
		{
				l_sys.debug_tiemout --;
		}
		else
		{
				l_sys.debug_flag = DEBUG_OFF_FLAG;
				l_sys.debug_tiemout = 0;
		}
}


#include <rtthread.h>
#include "sys_conf.h"
#include "kits/fifo.h"
#include "local_status.h"
#include "alarms.h"
#include "team.h"
#include "output_ctrl.h"
#include "require_calc.h"
#include "req_execution.h"
#include "sys_status.h"
#include "dio_bsp.h"
//static void top_fsm(void);
static void req_process(void);

void core_thread_entry(void* parameter)
{
		rt_thread_delay(CORE_THREAD_DELAY);
		alarm_acl_init(); 
    inc_pid_param_init();
		sys_option_di_sts(DI_POWER_LOSS_BPOS,1);
		
		while(1)
		{		 
//				alarm_acl_exe();
//				top_fsm();
				req_process();				
				oc_update();
				rt_thread_delay(200);
		}			
}

//static void top_fsm_signal_gen(void)
//{
//		extern sys_reg_st		g_sys;
//		extern local_reg_st l_sys;
//		
//		
//		
//}

 void top_fsm(void)
{
		extern sys_reg_st		g_sys;
		extern local_reg_st l_sys;
		extern team_local_st team_local_inst;
		switch(l_sys.t_fsm_state)
		{
				case(T_FSM_STATE_IDLE):
				{
						if((g_sys.config.team.team_en == 0)&&												//power not enabled
								(g_sys.config.general.power_mode == 1)&&										//power enabled
								((g_sys.status.general.sys_error_bitmap & 0x0001) == 0))		//SYS_ERR_FATAL
						{
								l_sys.t_fsm_state = T_FSM_STATE_STANDALONE;
						}
						else if((g_sys.config.team.team_en == 1)&&
								((g_sys.status.general.sys_error_bitmap & 0x0003) == 0))		//SYS_ERR_FATAL||SYS_ERR_TEAM
						{
								l_sys.t_fsm_state = T_FSM_STATE_TEAM;
						}
						else
						{	
								l_sys.t_fsm_state = T_FSM_STATE_IDLE;
						}							
						break;
				}
				case(T_FSM_STATE_STANDALONE):
				{
						if(	((g_sys.config.general.power_mode == 0))||
								((g_sys.status.general.sys_error_bitmap & 0x0001) != 0))		//SYS_ERR_FATAL
						{
								l_sys.t_fsm_state = T_FSM_STATE_IDLE;
						}
						else if((g_sys.config.team.team_en == 1)&&
								((g_sys.status.general.sys_error_bitmap & 0x0003) == 0))		//SYS_ERR_FATAL||SYS_ERR_TEAM
						{
								l_sys.t_fsm_state = T_FSM_STATE_TEAM;
						}
						else
						{	
								l_sys.t_fsm_state = T_FSM_STATE_STANDALONE;
						}							
						break;
				}
				case(T_FSM_STATE_TEAM):
				{
						if( ((g_sys.config.team.team_en == 0)&&(g_sys.config.general.power_mode == 0))||		//power down and team disable
								((g_sys.status.general.sys_error_bitmap & 0x0001) != 0))				//SYS_ERR_FATAL
						{
								l_sys.t_fsm_state = T_FSM_STATE_IDLE;
						}
						else if(((g_sys.config.general.power_mode == 1)&&(g_sys.config.team.team_en == 0))||	//power on and team disable
								((g_sys.status.general.sys_error_bitmap & 0x0002) == 0x0002))		//SYS_ERR_TEAM
						{
								l_sys.t_fsm_state = T_FSM_STATE_STANDALONE;
						}
						else 
						{	
								l_sys.t_fsm_state = T_FSM_STATE_TEAM;
						}							
						break;
				}
				default:
				{	
						l_sys.t_fsm_state = T_FSM_STATE_IDLE;
				}				
		}		
}

//需求处理
static void req_process(void)
{
		extern local_reg_st l_sys;
		extern sys_reg_st		g_sys;
		extern team_local_st  team_local_inst;
		int16_t target_temp_req,target_hum_req,target_fan_req; 		 
		
//		req_update();		//caculate local and team requirement
//		switch(l_sys.t_fsm_state)		//switch target requirement according to top fsm state
//		{
//				case(T_FSM_STATE_IDLE):
//				{
//						target_temp_req = l_sys.require[RESET_REQ][T_REQ];
//						target_hum_req = l_sys.require[RESET_REQ][H_REQ];
//						target_fan_req = l_sys.require[RESET_REQ][F_REQ];
//						break;
//				}
//				case(T_FSM_STATE_STANDALONE):
//				{
//						target_temp_req = l_sys.require[LOCAL_REQ][T_REQ];
//						target_hum_req = l_sys.require[LOCAL_REQ][H_REQ];
//						target_fan_req = l_sys.require[LOCAL_REQ][F_REQ];
//						break;
//				}
//				case(T_FSM_STATE_TEAM):
//				{
//						target_temp_req = l_sys.require[TEAM_REQ][T_REQ];
//						target_hum_req = l_sys.require[TEAM_REQ][H_REQ];
//						target_fan_req = l_sys.require[TEAM_REQ][F_REQ];
//						//从机,Alair,20180605
//						if(team_local_inst.team_fsm==TEAM_FSM_SLAVE)
//						{
//						    //无加热
//								if((devinfo_get_heater_level() == 0)&&(target_temp_req<0))
//								{
//										target_temp_req =0;
//										target_fan_req =0;
//										l_sys.u16Team_NOReq|=0x01;
//								}
//								else
//								{
//										l_sys.u16Team_NOReq&=~0x01;									
//								}
//								
//						    //无加湿
//								if(g_sys.config.ac_power_supply.PH_Ver==0)
//								{
//											if(((g_sys.config.humidifier.hum_en == 0)||(sys_get_mbm_online(POWER_MODULE_BPOS) == 0))&&(target_hum_req <= -100))
//											{
//													target_temp_req =0;
//													target_fan_req =0;
//													l_sys.u16Team_NOReq|=0x02;
//											}
//											else
//											{
//													l_sys.u16Team_NOReq&=~0x02;									
//											}
//								}
//								else
//								{
//									if((sys_get_mbm_online(HUM_MODULE_BPOS) == 0)&&(target_hum_req <= -100))
//									{
//											target_temp_req =0;
//											target_fan_req =0;
//											l_sys.u16Team_NOReq|=0x02;
//									}
//									else
//									{
//											l_sys.u16Team_NOReq&=~0x02;									
//									}
//								}
//								
//						}
//						break;
//				}
//				default:
//				{
//						target_temp_req = l_sys.require[RESET_REQ][T_REQ];
//						target_hum_req = l_sys.require[RESET_REQ][H_REQ];	
//						target_fan_req = l_sys.require[RESET_REQ][F_REQ];
//						break;
//				}
//		}
//		
//		if((g_sys.status.alarm_status_cnt.pwr_off_alarm==1)&&(l_sys.t_fsm_state==T_FSM_STATE_TEAM)&&
//			 ((team_local_inst.team_config[TEAM_CONF_MODE] >> TEAM_CONF_TEAM_POWER_OFF_EN) & 0x0001))		//主机下发的故障是否关机参数
//		{
//						target_temp_req = l_sys.require[LOCAL_REQ][T_REQ];
//						target_hum_req = l_sys.require[LOCAL_REQ][H_REQ];
//						target_fan_req = l_sys.require[LOCAL_REQ][F_REQ];
//		}
//		
//		
//		if(0)//if H_T sensor fail
//		{
//				l_sys.require[TARGET_REQ][T_REQ] = l_sys.require[RESET_REQ][T_REQ];
//				l_sys.require[TARGET_REQ][H_REQ] = l_sys.require[RESET_REQ][H_REQ];			
//				l_sys.require[TARGET_REQ][F_REQ] = l_sys.require[RESET_REQ][F_REQ];	
//		}
//		else
//		{
//				l_sys.require[TARGET_REQ][T_REQ] = target_temp_req;
//				l_sys.require[TARGET_REQ][H_REQ] = target_hum_req;
//				l_sys.require[TARGET_REQ][F_REQ] = target_fan_req;
//		}

		req_execution(target_temp_req,target_hum_req,target_fan_req);		//execute requirement 				
}

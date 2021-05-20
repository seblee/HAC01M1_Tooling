#include "sys_def.h" 
#include "reg_map_check.h"
#include "user_mb_app.h"
#include "global_var.h"
extern sys_reg_st					g_sys; 

uint8_t diagnose_mode_chk(uint16_t pram)
{
	if(pram ==1)
	{
		if(g_sys.config.general.power_mode!=0)
		{
				g_sys.config.general.power_mode = 0;
				RAM_Write_Reg(EE_POWERON, g_sys.config.general.power_mode, 1);			
		}
	}
	return(1);
}
//compressor0_uper_speed check
uint8_t comp_uper_spd_chk(uint16_t pram)
{
	if(pram > g_sys.config.compressor.speed_lower_lim)
	{
			return(1);
	}
	return(0);
}
//compressor0 lower speed check
uint8_t comp_low_spd_chk(uint16_t pram) 
{
	if(pram < g_sys.config.compressor.speed_upper_lim)
	{
			return(1);
	}
	return(0);
}

uint8_t water_valve_set_chk(uint16_t param)
{
    if((param <= g_sys.config.water_valve.max_opening) &&
       (param >= g_sys.config.water_valve.min_opening))
    {
        return (1);
    }
    return (0);
}

uint8_t water_valve_min_chk(uint16_t param)
{
    if(param < g_sys.config.water_valve.max_opening)
    {
        return(1);
    }
    return (0);
}

uint8_t water_valve_max_chk(uint16_t param)
{
    if(param > g_sys.config.water_valve.min_opening)
    {
        return (1);
    }
    return (0);
}

uint8_t fan_set_spd_chk(uint16_t pram)
{
	if((pram <= g_sys.config.fan.max_speed)&&(pram >= g_sys.config.fan.min_speed))
	{
			return(1);
	}
	return(0);
}
uint8_t fan_low_spd_chk(uint16_t pram)
{
	if((pram <= g_sys.config.fan.max_speed)&&(pram <= g_sys.config.fan.set_speed))
	{
			return(1);
	}
	return(0);
}
uint8_t fan_uper_spd_chk(uint16_t pram)
{
	if((pram >= g_sys.config.fan.min_speed)&&(pram >= g_sys.config.fan.set_speed))
	{
			return(1);
	}
	return(0);
}
uint8_t fan_dehum_min_spd_chk(uint16_t pram)
{
	if((pram <= g_sys.config.fan.max_speed)&&(pram >= g_sys.config.fan.min_speed)&&(pram <= g_sys.config.fan.set_speed))
	{
			return(1);
	}
	return(0);
}

uint8_t team_total_num_chk(uint16_t pram)
{
	if(pram > g_sys.config.team.backup_num)
	{
			return(1);
	}
	return(0);
}
uint8_t team_back_num_chk(uint16_t pram)
{
	if(pram <= (g_sys.config.team.total_num>>1))
	{
			return(1);
	}
	return(0);
}

uint8_t return_temp_hiacl_chk(uint16_t pram)
{
	if(pram > g_sys.config.alarm[ACL_LO_TEMP_RETURN].alarm_param)
	{
			return(1);
	}
	return(0);
}

uint8_t return_temp_lowacl_chk(uint16_t pram)
{
	if(pram < g_sys.config.alarm[ACL_HI_TEMP_RETURN].alarm_param)
	{
			return(1);
	}
	return(0);
}
uint8_t return_hum_hiacl_chk(uint16_t pram)
{
	if(pram > g_sys.config.alarm[ACL_LO_HUM_RETURN].alarm_param)
	{
			return(1);
	}
	return(0);
}
uint8_t return_hum_lowacl_chk(uint16_t pram)
{
	if(pram < g_sys.config.alarm[ACL_HI_HUM_RETURN].alarm_param)
	{
			return(1);
	}
	return(0);
}

uint8_t supply_temp_hiacl_chk(uint16_t pram)
{
	if(pram > g_sys.config.alarm[ACL_LO_TEMP_SUPPLY].alarm_param)
	{
			return(1);
	}
	return(0);
}

uint8_t supply_temp_lowacl_chk(uint16_t pram)
{
	if(pram < g_sys.config.alarm[ACL_HI_TEMP_SUPPLY].alarm_param)
	{
			return(1);
	}
	return(0);
}
uint8_t supply_hum_hiacl_chk(uint16_t pram)
{
	if(pram > g_sys.config.alarm[ACL_LO_HUM_SUPPLY].alarm_param)
	{
			return(1);
	}
	return(0);
}
 uint8_t supply_hum_lowacl_chk(uint16_t pram)
{
	if(pram < g_sys.config.alarm[ACL_HI_HUM_SUPPLY].alarm_param)
	{
			return(1);
	}
	return(0);
}

uint8_t water_elec_hi_chk(uint16_t pram)
{
		if(pram > g_sys.config.alarm[ACL_WATER_ELEC_LO].alarm_param)
		{
				return(1);
		}
		return(0);
}
uint8_t water_elec_lo_chk(uint16_t pram)
{
	if(pram < g_sys.config.alarm[ACL_WATER_ELEC_HI].alarm_param)
	{
			return(1);
	}
	return(0);
}
uint8_t power_frq_hi_chk(uint16_t pram)
{
		if(pram > g_sys.config.alarm[ACL_POWER_LO_FD].alarm_param)
		{
				return(1);
		}
		return(0);
}
uint8_t power_frq_lo_chk(uint16_t pram)
{
	if(pram < g_sys.config.alarm[ACL_POWER_HI_FD].alarm_param)
	{
			return(1);
	}
	return(0);
}

uint8_t power_a_hi_chk(uint16_t pram)
{
	if(pram > g_sys.config.alarm[ACL_POWER_A_LOW].alarm_param)
		{
				return(1);
		}
		return(0);
}
uint8_t power_a_lo_chk(uint16_t pram)
{
	if(pram < g_sys.config.alarm[ACL_POWER_A_HIGH].alarm_param)
	{
			return(1);
	}
	return(0);
}
uint8_t power_b_hi_chk(uint16_t pram)
{
	if(pram > g_sys.config.alarm[ACL_POWER_B_LOW].alarm_param)
		{
				return(1);
		}
		return(0);
}
uint8_t power_b_lo_chk(uint16_t pram)
{
	if(pram < g_sys.config.alarm[ACL_POWER_B_HIGH].alarm_param)
	{
			return(1);
	}
	return(0);
}
uint8_t power_c_hi_chk(uint16_t pram)
{
	if(pram > g_sys.config.alarm[ACL_POWER_C_LOW].alarm_param)
		{
				return(1);
		}
		return(0);
}
uint8_t power_c_lo_chk(uint16_t pram)
{
	if(pram < g_sys.config.alarm[ACL_POWER_C_HIGH].alarm_param)
	{
			return(1);
	}
	return(0);
}
//uint8_t cool0_temp_hi_chk(uint16_t pram)
//{
//	if(pram > g_sys.config.alarm[ACL_COIL_BLOCKING].alarm_param)
//		{
//				return(1);
//		}
//		return(0);
//}
uint8_t cool0_temp_lo_chk(uint16_t pram)
{
	if(pram < g_sys.config.alarm[ACL_COIL_HI_TEM1].alarm_param)
	{
			return(1);
	}
	return(0);
}
uint8_t cool1_temp_hi_chk(uint16_t pram)
{
	if(pram > g_sys.config.alarm[ACL_FAN_OT6].alarm_param)
		{
				return(1);
		}
		return(0);
}
uint8_t cool1_temp_lo_chk(uint16_t pram)
{
	if(pram < g_sys.config.alarm[ACL_FAN_OVERLOAD7].alarm_param)
	{
			return(1);
	}
	return(0);
}

uint8_t AC_Conf_Write(uint16_t pram,uint8_t Type)
{
	extern	mbm_read_st mbm_read_table[];	
	uint8_t k,Ret=0;
	
	switch(Type)
	{
		case 0x00:
		{
			if(((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EX_FAN_ADDR)&0x0001) == 1)
			{
				if(mbm_read_table[MBM_DEV_EX_FAN_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt >0)
				{
					for(k=0;k < mbm_read_table[MBM_DEV_EX_FAN_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt;k++)
					{
							*(mbm_read_table[MBM_DEV_EX_FAN_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].conf_Flag)=FALSE;
					}	
					Ret=1;	
				}
			}	
//			if(((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EX_FAN2_ADDR)&0x0001) == 1)
//			{			
//				if(mbm_read_table[MBM_DEV_EX_FAN2_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt >0)
//				{				
//					for(k=0;k < mbm_read_table[MBM_DEV_EX_FAN2_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt;k++)
//					{
//							*(mbm_read_table[MBM_DEV_EX_FAN2_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].conf_Flag)=FALSE;
//					}	
//					Ret=1;	
//				}
//			}	
			//EEV
			if(((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EEV_ADDR)&0x0001) == 1)
			{
				if(mbm_read_table[MBM_DEV_EEV_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt >0)
				{
					for(k=0;k < mbm_read_table[MBM_DEV_EEV_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt;k++)
					{
							*(mbm_read_table[MBM_DEV_EEV_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].conf_Flag)=FALSE;
					}	
					Ret=1;	
				}
			}				
			break;
		}
		case 0x01:
		{
			if(((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EX_FAN_ADDR)&0x0001) == 1)
			{
				if(mbm_read_table[MBM_DEV_EX_FAN_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt >0)
				{
					for(k=0;k < mbm_read_table[MBM_DEV_EX_FAN_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt;k++)
					{
							*(mbm_read_table[MBM_DEV_EX_FAN_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].conf_Flag)=TRUE;
					}
					Ret=1;						
				}
			}	
//			if(((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EX_FAN2_ADDR)&0x0001) == 1)
//			{			
//				if(mbm_read_table[MBM_DEV_EX_FAN2_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt >0)
//				{				
//					for(k=0;k < mbm_read_table[MBM_DEV_EX_FAN2_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt;k++)
//					{
//							*(mbm_read_table[MBM_DEV_EX_FAN2_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].conf_Flag)=TRUE;
//					}	
//					Ret=1;	
//				}
//			}	
			//EEV
			if(((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EEV_ADDR)&0x0001) == 1)
			{
				if(mbm_read_table[MBM_DEV_EEV_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt >0)
				{
					for(k=0;k < mbm_read_table[MBM_DEV_EEV_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt;k++)
					{
							*(mbm_read_table[MBM_DEV_EEV_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].conf_Flag)=TRUE;
					}	
					Ret=1;	
				}
			}				
			break;
		}
		case 0x02:
		{
			if(((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EX_FAN_ADDR)&0x0001) == 1)
			{
				if(mbm_read_table[MBM_DEV_EX_FAN_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt >0)
				{
	//							g_sys.status.general.TEST|=0x40;
					for(k=0;k < mbm_read_table[MBM_DEV_EX_FAN_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt;k++)
					{
						if(pram == mbm_read_table[MBM_DEV_EX_FAN_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].Conf_addr)
						{
	//									g_sys.status.general.TEST|=0x80;
							*(mbm_read_table[MBM_DEV_EX_FAN_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].conf_Flag)=TRUE;
						}
					}	
					Ret=1;						
				}	
			}
//			if(((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EX_FAN2_ADDR)&0x0001) == 1)
//			{			
//				if(mbm_read_table[MBM_DEV_EX_FAN2_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt >0)
//				{
//	//							g_sys.status.general.TEST|=0x40;
//					for(k=0;k < mbm_read_table[MBM_DEV_EX_FAN2_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt;k++)
//					{
//						if(pram == mbm_read_table[MBM_DEV_EX_FAN2_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].Conf_addr)
//						{
//	//									g_sys.status.general.TEST|=0x80;
//							*(mbm_read_table[MBM_DEV_EX_FAN2_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].conf_Flag)=TRUE;
//						}
//					}	
//					Ret=1;					
//				}	
//			}	
			//EEV
			if(((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EEV_ADDR)&0x0001) == 1)
			{			
				if(mbm_read_table[MBM_DEV_EEV_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt >0)
				{
					for(k=0;k < mbm_read_table[MBM_DEV_EEV_ADDR-MB_MASTER_DISCRETE_OFFSET].reg_w_cnt;k++)
					{
						if(pram == mbm_read_table[MBM_DEV_EEV_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].Conf_addr)
						{
							*(mbm_read_table[MBM_DEV_EEV_ADDR-MB_MASTER_DISCRETE_OFFSET].w_pt[k].conf_Flag)=TRUE;
						}
					}	
					Ret=1;					
				}	
			}				
			break;
		}
		default:
			break;
	}
	return(Ret);
}



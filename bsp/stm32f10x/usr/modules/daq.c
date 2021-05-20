#include <rtthread.h>
#include <components.h>
#include "daq.h"
#include "sys_conf.h"
#include "sys_status.h"
#include "calc.h"
#include "local_status.h"
#include "adc_bsp.h"
#include "user_mb_app.h"
#include "dio_bsp.h"

#define NTC_TEMP_SCALE   191
#define NTC_TEMP_OFFSET  39		
#define NTC_TEMP_DT 15
#define K_FACTOR_HI_PRESS 174
#define K_FACTOR_LO_PRESS 232

extern  sys_reg_st g_sys;

const uint16_t ntc_lookup_tab[NTC_TEMP_SCALE] = 
{
	193,204,215,227,239,252,265,279,294,309,324,340,357,375,393,411,431,451,471,
	492,514,537,560,584,609,635,661,687,715,743,772,801,831,862,893,925,957,991,
  1024,1058,1093,1128,1164,1200,1236,1273,1310,1348,1386,1424,1463,1501,1540,
  1579,1618,1657,1697,1736,1775,1815,1854,1893,1932,1971,2010,2048,2087,2125,
  2163,2200,2238,2274,2311,2347,2383,2418,2453,2488,2522,2556,2589,2622,2654,
  2685,2717,2747,2777,2807,2836,2865,2893,2921,2948,2974,3000,3026,3051,3075,
  3099,3123,3146,3168,3190,3212,3233,3253,3273,3293,3312,3331,3349,3367,3384,
  3401,3418,3434,3450,3465,3481,3495,3510,3524,3537,3551,3564,3576,3588,3600,
  3612,3624,3635,3646,3656,3667,3677,3686,3696,3705,3714,3723,3732,3740,3748,
  3756,3764,3772,3779,3786,3793,3800,3807,3813,3820,3826,3832,3838,3844,3849,
  3855,3860,3865,3870,3875,3880,3884,3889,3893,3898,3902,3906,3910,3914,3918,
  3922,3925,3929,3932,3936,3939,3942,3945,3949,3952,3954,3957,3960,3963,3966,
  3968,3971,3973
};





static int16_t calc_ntc(uint16_t adc_value,int16_t adjust)
{	
		int16_t ntc_temp;
		int16_t index;
		uint16_t offset;
		adc_value= 4096 - adc_value;
		index = bin_search((uint16_t *)ntc_lookup_tab, (NTC_TEMP_SCALE-1), adc_value);  		
		if(index < 0)
		{
				return ABNORMAL_VALUE;				
		}
		else
		{
				offset = (adc_value-ntc_lookup_tab[index-1])*10 / (ntc_lookup_tab[index]-ntc_lookup_tab[index-1]);
				ntc_temp = ( index- NTC_TEMP_OFFSET)*10 + offset + adjust - NTC_TEMP_DT;
				return ntc_temp;
		}	
			
}

static int16_t get_average_temp(uint8_t type)
{
	 uint8_t index=0,i;
	 int16_t sum=0;	 

    if(type == TARGET_MODE_RETURN)
    {
      //硬件
    //配置信息 温度传感器 前四个为回风，NTC 前面两个 为回风温度
    //	g_sys.config.
    //报警信息
    //alrmbit= get_alarm_status_bitmap(ACL_TNH_MALFUNC>>4,ACL_TNH_MALFUNC&0x0f);
    //TEM_SENSOR

				if(((g_sys.config.dev_mask.mb_comp) & (0X01<<MBM_DEV_A1_ADDR))&&
					(sys_get_mbm_online(MBM_DEV_A1_ADDR) == 1))//配置位 online
				{
						if(sys_get_remap_status(SENSOR_STS_REG_NO,MBM_DEV_A1_ADDR) == 0 )//报警位
						{
							sum +=(int16_t) (g_sys.status.mbm.tnh[MBM_DEV_A1_ADDR].temp);
							index++;
						}
				}				
//				rt_kprintf("sum=%x,index= %x,MBM_DEV_A1_ADDR= %x\n",sum,index,sys_get_mbm_online(MBM_DEV_A1_ADDR));		
//				//alair,20170224
//				if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND)&&(g_sys.config.compressor.type == COMP_QABP))		//列间变频
//				{
//						//回风2
//						if(((g_sys.config.dev_mask.mb_comp) & (0X01<<RETURN_TEM_HUM_2_BPOS))&&
//							(sys_get_mbm_online(RETURN_TEM_HUM_2_BPOS) == 1))//配置位 online
//						{
//								if(sys_get_remap_status(SENSOR_STS_REG_NO,RETURN_TEM_HUM_2_BPOS) == 0 )//报警位
//								{
//									sum +=(int16_t) (g_sys.status.mbm.tnh[RETURN_TEM_HUM_2_BPOS].temp);
//									index++;
//								}
//						}		
//						//回风3
//						if(((g_sys.config.dev_mask.mb_comp) & (0X01<<RETURN_TEM_HUM_3_BPOS))&&
//							(sys_get_mbm_online(RETURN_TEM_HUM_3_BPOS) == 1))//配置位 online
//						{
//								if(sys_get_remap_status(SENSOR_STS_REG_NO,RETURN_TEM_HUM_3_BPOS) == 0 )//报警位
//								{
//									sum +=(int16_t) (g_sys.status.mbm.tnh[RETURN_TEM_HUM_3_BPOS].temp);
//									index++;
//								}
//						}								
//				}
        //NTC 
    
        if((g_sys.config.dev_mask.ain) & (0x01<<AI_RETURN_NTC1))
        {
          if(sys_get_remap_status(SENSOR_STS_REG_NO,AI_RETURN_NTC1) == 0)//报警位
					{
							sum +=(int16_t)( g_sys.status.ain[AI_RETURN_NTC1]);
							index++;
					}
        }
//				rt_kprintf("sum=%x,index= %x,MBM_DEV_A1_ADDR= %x,SENSOR_STS_REG_NO= %x\n",sum,index,sys_get_mbm_online(MBM_DEV_A1_ADDR),sys_get_remap_status(SENSOR_STS_REG_NO,DEV_RETUREN_NTC1_FAULT_BPOS));		
    }
    else if(type == TARGET_MODE_SUPPLY)
    {
        //TEM_SENSOR
        if(((g_sys.config.dev_mask.mb_comp) & (0X01<<MBM_DEV_A2_ADDR))&&
          (sys_get_mbm_online(MBM_DEV_A2_ADDR) == 1))//配置位 online
        {
            if(sys_get_remap_status(SENSOR_STS_REG_NO,MBM_DEV_A2_ADDR) == 0)//报警位
            {
                sum +=(int16_t)( g_sys.status.mbm.tnh[MBM_DEV_A2_ADDR].temp);
                index++;
            }
        }
						
					//NTC 
						if((g_sys.config.dev_mask.ain) & (0x01<<AI_SUPPLY_NTC1))
						{
								if(sys_get_remap_status(SENSOR_STS_REG_NO,AI_SUPPLY_NTC1) == 0)//报警位
								{
										sum += (int16_t)(g_sys.status.ain[AI_SUPPLY_NTC1]);
										index++;
								}
						}
		}
    else//REMOTE TARGET
    {
        if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND)||((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER))) 
        {
            for(i = TEM_HUM_SENSOR3_BPOS ;i <= TEM_HUM_SENSOR8_BPOS;i++ )
            {
                if(((g_sys.config.dev_mask.mb_comp) & (0X01<<i))&&
                (sys_get_mbm_online(i) == 1))//配置位 online
                {
                    if(sys_get_remap_status(SENSOR_STS_REG_NO,i) == 0 )//报警位
                    {
                        sum +=(int16_t) (g_sys.status.mbm.tnh[i].temp);
                        index++;
                    }
                }
            } 
        }
    }
    if(index!=0)
    {
        return(sum/index);
    }
    else
    {
        return(ABNORMAL_VALUE);
    }
}
    
static uint16_t get_max_temp(uint8_t type)
{
	 uint8_t index=0,i;
	 int16_t meter,max=0x8000;	 

	 if(type == TARGET_MODE_RETURN)
	 {
	 	 //硬件
		 //配置信息 温度传感器 前四个为回风，NTC 前面两个 为回风温度
		 //	g_sys.config.
		 //报警信息
		//alrmbit= get_alarm_status_bitmap(ACL_TNH_MALFUNC>>4,ACL_TNH_MALFUNC&0x0f);
		//TEM_SENSOR
		 if(((g_sys.config.dev_mask.mb_comp) & (0X01<<MBM_DEV_A1_ADDR))&&
		 	(sys_get_mbm_online(MBM_DEV_A1_ADDR) == 1))//配置位 online
		 {
				if(sys_get_remap_status(SENSOR_STS_REG_NO,MBM_DEV_A1_ADDR) == 0 )//报警位
				{
					max =(int16_t) (g_sys.status.mbm.tnh[MBM_DEV_A1_ADDR].temp);
					index++;
				}
		 }
		 
		 //NTC 
		 if((g_sys.config.dev_mask.ain) & (0x01<<AI_RETURN_NTC1))
		 {
			 if(sys_get_remap_status(SENSOR_STS_REG_NO,DEV_RETUREN_NTC1_FAULT_BPOS) == 0)//报警位
				{
						meter =(int16_t)( g_sys.status.ain[AI_RETURN_NTC1]);
						if(meter > max)
						{
								max =  meter;
						}
						index++;
				}
		 } 
 
	 }
	 else if(type == TARGET_MODE_SUPPLY)
	 {
	 	//TEM_SENSOR
		 if(((g_sys.config.dev_mask.mb_comp) & (0X01<<MBM_DEV_A2_ADDR))&&
		 	(sys_get_mbm_online(MBM_DEV_A2_ADDR) == 1))//配置位 online
		 {
				if(sys_get_remap_status(SENSOR_STS_REG_NO,MBM_DEV_A2_ADDR) == 0)//报警位
				{
					max =(int16_t)( g_sys.status.mbm.tnh[MBM_DEV_A2_ADDR].temp);
					index++;
				}
		 }

		 //NTC 
		 if((g_sys.config.dev_mask.ain) & (0x01<<AI_SUPPLY_NTC1))
		 {
				if(sys_get_remap_status(SENSOR_STS_REG_NO,DEV_SUPPLY_NTC1_FAULT_BPOS) == 0)//报警位
				{
					meter = (int16_t)(g_sys.status.ain[AI_SUPPLY_NTC1]);
					if(meter > max)
					{
								max =  meter;
					}
					index++;
				}
		 }		  
		
	 }
	 else //remote target
	 {
			if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND)||((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER))) 
			{
					for(i = TEM_HUM_SENSOR3_BPOS ;i <= TEM_HUM_SENSOR8_BPOS;i++ )
					{
							if(((g_sys.config.dev_mask.mb_comp) & (0X01<<i))&&
							(sys_get_mbm_online(i) == 1))//配置位 online
							{
								if(sys_get_remap_status(SENSOR_STS_REG_NO,i) == 0 )//报警位
								{
									meter =(int16_t) (g_sys.status.mbm.tnh[i].temp);
									if(meter > max )
									{
										  max  = meter;
									}
									index++;
								}
							}
					} 
			}
	 }
	if(index!=0)
	{
	 return(max);
	}
	else
	{
		return(ABNORMAL_VALUE);
	}
}


static uint16_t get_min_temp(uint8_t type)
{
	 uint8_t index=0,i;
	 int16_t meter,min=ABNORMAL_VALUE;	 

	 if(type == TARGET_MODE_RETURN)
	 {
	 	 //硬件
		 //配置信息 温度传感器 前四个为回风，NTC 前面两个 为回风温度
		 //	g_sys.config.
		 //报警信息
		//alrmbit= get_alarm_status_bitmap(ACL_TNH_MALFUNC>>4,ACL_TNH_MALFUNC&0x0f);
		//TEM_SENSOR
		 if(((g_sys.config.dev_mask.mb_comp) & (0X01<<MBM_DEV_A1_ADDR))&&
		 	(sys_get_mbm_online(MBM_DEV_A1_ADDR) == 1))//配置位 online
		 {
				if(sys_get_remap_status(SENSOR_STS_REG_NO,MBM_DEV_A1_ADDR) == 0 )//报警位
				{
					min =(int16_t) (g_sys.status.mbm.tnh[MBM_DEV_A1_ADDR].temp);
					index++;
				}
		 }	
		 
		 //NTC 
		 if((g_sys.config.dev_mask.ain) & (0x01<<AI_RETURN_NTC1))
		 {
			 if(sys_get_remap_status(SENSOR_STS_REG_NO,DEV_RETUREN_NTC1_FAULT_BPOS) == 0)//报警位
				{
						meter =(int16_t)( g_sys.status.ain[AI_RETURN_NTC1]);
						if(meter < min)
						{
								min =  meter;
						}
						index++;
				}
		 }
	 
 
	 }
	 else if(type == TARGET_MODE_SUPPLY)
	 {
	 	//TEM_SENSOR
		 if(((g_sys.config.dev_mask.mb_comp) & (0X01<<MBM_DEV_A2_ADDR))&&
		 	(sys_get_mbm_online(MBM_DEV_A2_ADDR) == 1))//配置位 online
		 {
				if(sys_get_remap_status(SENSOR_STS_REG_NO,MBM_DEV_A2_ADDR) == 0)//报警位
				{
					min =(int16_t)( g_sys.status.mbm.tnh[MBM_DEV_A2_ADDR].temp);
					index++;
				}
		 }
		 
		 //NTC 
		 if((g_sys.config.dev_mask.ain) & (0x01<<AI_SUPPLY_NTC1))
		 {
				if(sys_get_remap_status(SENSOR_STS_REG_NO,DEV_SUPPLY_NTC1_FAULT_BPOS) == 0)//报警位
				{
					meter = (int16_t)(g_sys.status.ain[AI_SUPPLY_NTC1]);
					if(meter < min)
					{
								min =  meter;
					}
					index++;
				}
		 }
		  
		
	 }
	 else //remote target
	 {
			if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND)||((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER))) 
			{
					for(i = TEM_HUM_SENSOR3_BPOS ;i <= TEM_HUM_SENSOR8_BPOS;i++ )
					{
							if(((g_sys.config.dev_mask.mb_comp) & (0X01<<i))&&
							(sys_get_mbm_online(i) == 1))//配置位 online
							{
								if(sys_get_remap_status(SENSOR_STS_REG_NO,i) == 0 )//报警位
								{
									meter =(int16_t) (g_sys.status.mbm.tnh[i].temp);
									if(meter < min )
									{
										  min  = meter;
									}
									index++;
								}
							}
					} 
			}
	 }
	if(index!=0)
	{
	 return(min);
	}
	else
	{
		return(ABNORMAL_VALUE);
	}
}









int16_t get_current_temp(uint8_t type)
{
		int16_t temp;
	 
		temp=get_average_temp(type);
		return(temp);
}

int16_t get_current_max_temp(uint8_t type)
{
		int16_t temp;
//		if(g_sys.config.algorithm.temp_calc_mode == MAX_TEMP_MODE)
//		{
//			temp=get_max_temp(type);
//		}
//		else
//		{
//			temp=get_average_temp(type);
//		}
		temp=get_max_temp(type);
		return(temp);
}

int16_t get_current_min_temp(uint8_t type)
{
			int16_t temp;
//		if(g_sys.config.algorithm.temp_calc_mode == MAX_TEMP_MODE)
//		{
//			temp=get_max_temp(type);
//		}
//		else
//		{
//			temp=get_average_temp(type);
//		}
		temp=get_min_temp(type);
		return(temp);
}



uint16_t get_current_hum(uint8_t type)
{
	uint16_t sum=0;
	uint8_t index=0,i;
	if(type == TARGET_MODE_RETURN)
	{
		  if(((g_sys.config.dev_mask.mb_comp) & (0X01<<MBM_DEV_A1_ADDR))&&
		 	(sys_get_mbm_online(MBM_DEV_A1_ADDR) == 1))//配置位 online
		 {
					if(sys_get_remap_status(SENSOR_STS_REG_NO,MBM_DEV_A1_ADDR) == 0)//报警位
					{
							sum += g_sys.status.mbm.tnh[MBM_DEV_A1_ADDR].hum;
							index++;
					}
		 } 

//		 //alair,20170224
//		if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND)&&(g_sys.config.compressor.type == COMP_QABP))		//列间变频
//		{
//				//回风2
//				if(((g_sys.config.dev_mask.mb_comp) & (0X01<<RETURN_TEM_HUM_2_BPOS))&&
//					(sys_get_mbm_online(RETURN_TEM_HUM_2_BPOS) == 1))//配置位 online
//				{
//						if(sys_get_remap_status(SENSOR_STS_REG_NO,RETURN_TEM_HUM_2_BPOS) == 0 )//报警位
//						{
//							sum +=(int16_t) (g_sys.status.mbm.tnh[RETURN_TEM_HUM_2_BPOS].hum);
//							index++;
//						}
//				}		
//				//回风3
//				if(((g_sys.config.dev_mask.mb_comp) & (0X01<<RETURN_TEM_HUM_3_BPOS))&&
//					(sys_get_mbm_online(RETURN_TEM_HUM_3_BPOS) == 1))//配置位 online
//				{
//						if(sys_get_remap_status(SENSOR_STS_REG_NO,RETURN_TEM_HUM_3_BPOS) == 0 )//报警位
//						{
//							sum +=(int16_t) (g_sys.status.mbm.tnh[RETURN_TEM_HUM_3_BPOS].hum);
//							index++;
//						}
//				}								
//		}
	}
	else if(type == TARGET_MODE_SUPPLY)
	{
		  if(((g_sys.config.dev_mask.mb_comp) & (0X01<<MBM_DEV_A2_ADDR))&&
		 	(sys_get_mbm_online(MBM_DEV_A2_ADDR) == 1))//配置位 onlin
		 {
					if(sys_get_remap_status(SENSOR_STS_REG_NO,MBM_DEV_A2_ADDR) == 0)//报警位
					{
							sum += g_sys.status.mbm.tnh[MBM_DEV_A2_ADDR].hum;
							index++;
					}
		 } 
	}
	else //remote target
	{
		  if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND)||((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER))) 
			{
					for(i = TEM_HUM_SENSOR3_BPOS ;i <= TEM_HUM_SENSOR8_BPOS;i++ )
					{
							if(((g_sys.config.dev_mask.mb_comp) & (0X01<<i))&&
							(sys_get_mbm_online(i) == 1))//配置位 online
							{
									if(sys_get_remap_status(SENSOR_STS_REG_NO,i) == 0 )//报警位
									{
											sum +=(int16_t) (g_sys.status.mbm.tnh[i].hum);
											index++;
									}
							}
					} 
			}
	}
	
	if(index!=0)
	{
		 return(sum/index);
	}
	else
	{
		return(ABNORMAL_VALUE);
	}

}

#define Rs  200

//static int16_t calc_10vai(uint16_t adc_value)
//{
//	int16_t relvaule;
//	relvaule = (adc_value*132)/4096;
//	return(relvaule);
//}

//static int16_t calc_ap_ai(uint16_t adc_value,int16_t cali)
//{
//	int16_t relvaule;	
//	relvaule = ((825*adc_value)/(40*Rs)) - 25 + (int16_t)(cali);//(adc_value*1330-100)/(1024*Rs);
////				rt_kprintf("adc_value= %X\n",adc_value);		
////				rt_kprintf("relvaule= %X\n",relvaule);	
//	if(relvaule<0)
//	{
//			relvaule = ABNORMAL_VALUE;
//	}
//	if(relvaule>250)
//	{
//			relvaule = 0x7fff;
//	}
//	return(relvaule);
//}

//calculate the hi pressure sensor analog input
//k_factor has 3-valid-digitals integer
  int16_t calc_hi_press_ai(uint16_t adc_value,uint16_t k_factor,int16_t cali)
{
    int32_t ret_val = 0;
    //As Vo/Vcc*100 = K*P+10, Vadc*4/Vcc*100 = K*P+10. Vadc = 3.3/4096 * N. 
    //(((3.3/4096)*N)*4)/Vcc*100 = K*P + 10;
    ret_val = ((3.3*adc_value*100*4)/(4096*5) - 10)*1000/k_factor + (int16_t)(cali); //unit is BAR(aka. 0.1MPaG)
	  if(ret_val <=-50)
		{
				ret_val =ABNORMAL_VALUE;
		}
    return (int16_t)ret_val;    
//    int32_t ret_val = 0;
//    //As Vo/Vcc*100 = K*P+10, Vadc*4/Vcc*100 = K*P+10. Vadc = 3.3/4096 * N. 
//    //(((3.3/4096)*N)*4)/Vcc*100 = K*P + 10;
//    ret_val = ((3.3*adc_value*100*4)/(4096*5) - 10)*1000/k_factor + (int16_t)(cali); //unit is BAR(aka. 0.1MPaG)
//		rt_kprintf("adc_value = %d,k_factor = %d,cali = %d,ret_val = %d\n",adc_value,k_factor,cali,ret_val);			

//	  if(ret_val <=0)
//		{
//				ret_val =ABNORMAL_VALUE;
//		}

//    return (int16_t)ret_val;   
}

////water flow supper voice wave
//static int16_t calc_water_flow_ai(uint16_t adc_value,int16_t cali)
//{
//		uint16_t q_max=450;//45.0m3/h
//		int32_t ret_val=0;
//		//AS Q = (Qmax/16)*(Ima-4)
//		//Ima= 1000*V200/R;
//		// V200= 4*Vadc
//		// Vadc =3.3*adc/4096
//		// Q =Qmax/16(16.5*adc/1024 - 4)
//		ret_val = (q_max/16)*((16.5*adc_value/1024)-4) + (int16_t)cali;
//		
//		 if(ret_val <=0) 
//		{
//				ret_val =ABNORMAL_VALUE;
//		}
//		if(ret_val >=q_max + cali+5)
//		{
//				ret_val =ABNORMAL_VALUE;
//		}
//    return (int16_t)ret_val;
//}

void ADCValProcess(uint16_t *ptrADCval,uint16_t *ptrADCbuf,uint8_t index)							
{
	uint8_t i=0;
	volatile uint16_t ADC_VOL_ave = 0;
	uint16_t ADC_Tmp[MAX_ADBUFEVERY];
	for(i=0;i<MAX_ADBUFEVERY;i++)
	{
		ADC_Tmp[i] = 0x0000;
		ADC_Tmp[i]=ptrADCbuf[i*AI_MAX_CNT_MR + index];	  
	}
	quick(ADC_Tmp,0,19);	//0~35标号,共36个
	for(i=2;i<18;i++)
	{
		ADC_VOL_ave+=ADC_Tmp[i];
	}
	ADC_VOL_ave >>= 4;
	ptrADCval[index] = ADC_VOL_ave;
}

#define NUM_0 8       //滤波次数
unsigned short AVGfilter(int8_t i8Type,int16_t i16Value)
{
		static int8_t i8Num[AI_MAX_CNT]={0};
		static int16_t i16Value_buf[AI_MAX_CNT][NUM_0];
//		unsigned char k=0;
		int16_t i16CvtValue;		
		if(i8Num[i8Type]<NUM_0)
		{
			i8Num[i8Type]++;
		}
		else
		{
			i8Num[i8Type]=0;		
		}		
		i16Value_buf[i8Type][i8Num[i8Type]] = i16Value;	
		i16CvtValue=MedianFilter((uint16_t *)i16Value_buf[i8Type],NUM_0);	
		
    return i16CvtValue;
}


void ai_sts_update(sys_reg_st*	gds_sys_ptr)
{
		extern volatile uint16_t ADC1ConvertedValue[AI_MAX_CNT];
//		extern volatile uint16_t ADC1Buff[AI_SENSOR_NUM * MAX_ADBUFEVERY];
		uint16_t ain_mask_bitmap;
		uint16_t i;
		uint16_t u16AI;
		uint16_t u16ADCRemapValue[AI_MAX_CNT];
	
		ain_mask_bitmap = gds_sys_ptr->config.dev_mask.ain;
		for(i = 0;i < AI_MAX_CNT_MR;i++)
		{
				//重映射
				u16AI=Sts_Remap(i,Rep_AI,0);	
				u16ADCRemapValue[u16AI]=ADC1ConvertedValue[i];					
		}
		for(i=0;i<AI_SENSOR5;i++)
		{
				gds_sys_ptr->status.ain[i] = ADC1ConvertedValue[i];
		}
//		rt_kprintf("ADC1ConvertedValue = %d,V[1] = %d,V[2] = %d,V[3] = %d,V[4] = %d,V[5] = %d,\n",ADC1ConvertedValue[0],ADC1ConvertedValue[1],ADC1ConvertedValue[2],ADC1ConvertedValue[3],ADC1ConvertedValue[4],ADC1ConvertedValue[5]);					

//rt_kprintf("u16ADCRemapValue = %X,R[3] = %X,R[4] = %X,R[5] = %X,R[7] = %X,R[9] = %X,\n",u16ADCRemapValue[2],u16ADCRemapValue[3],u16ADCRemapValue[4],u16ADCRemapValue[5],u16ADCRemapValue[7],u16ADCRemapValue[9]);					
   
//    //low_pressure sensor caculation
//		if((ain_mask_bitmap&(0x0001<<AI_LO_PRESS_SENSOR1)) != 0)
//		{
//        if((gds_sys_ptr->config.general.cool_type == COOL_TYPE_MODULE_WIND)||(gds_sys_ptr->config.general.cool_type == COOL_TYPE_COLUMN_WIND))
////            gds_sys_ptr->status.ain[AI_LO_PRESS_SENSOR1] = calc_hi_press_ai(ADC1ConvertedValue[AI_LO_PRESS_SENSOR1],K_FACTOR_LO_PRESS,gds_sys_ptr->config.general.ai_cali[AI_LO_PRESS_SENSOR1]);
//           gds_sys_ptr->status.ain[AI_LO_PRESS_SENSOR1] = calc_hi_press_ai(u16ADCRemapValue[AI_LO_PRESS_SENSOR1],K_FACTOR_LO_PRESS,gds_sys_ptr->config.general.ai_cali[AI_LO_PRESS_SENSOR1]);
//        else
//            gds_sys_ptr->status.ain[AI_LO_PRESS_SENSOR1] = ABNORMAL_VALUE;
//		}
//    //high_pressure sensor caculation    
//		if((ain_mask_bitmap&(0x0001<<AI_HI_PRESS_SENSOR1)) != 0)
//		{
////				gds_sys_ptr->status.ain[AI_HI_PRESS_SENSOR1] = calc_hi_press_ai(ADC1ConvertedValue[AI_HI_PRESS_SENSOR1],K_FACTOR_HI_PRESS,gds_sys_ptr->config.general.ai_cali[AI_HI_PRESS_SENSOR1]);
//				gds_sys_ptr->status.ain[AI_HI_PRESS_SENSOR1] = calc_hi_press_ai(u16ADCRemapValue[AI_HI_PRESS_SENSOR1],K_FACTOR_HI_PRESS,gds_sys_ptr->config.general.ai_cali[AI_HI_PRESS_SENSOR1]);
//		}       
    
		
		for(i=AI_NTC1;i<AI_MAX_CNT;i++)
		{
				gds_sys_ptr->status.ain[i] = (((ain_mask_bitmap>>i)&0x0001) != 0)?  calc_ntc(u16ADCRemapValue[i],gds_sys_ptr->config.general.ntc_cali[i-AI_NTC1]):0;
		}
//		//取平均值
//		for(i=0;i<AI_MAX_CNT;i++)
//		{
//				gds_sys_ptr->status.ain[i]=AVGfilter(i,gds_sys_ptr->status.ain[i]);
//		}
//		rt_kprintf("AI_NTC1 = %d,AI_NTC2 = %d,AI_NTC5 = %d,AI_NTC6 = %d,AI_NTC7 = %d,AI_NTC8 = %d\n",gds_sys_ptr->status.ain[AI_NTC1],gds_sys_ptr->status.ain[AI_NTC2],gds_sys_ptr->status.ain[AI_NTC5],
//		gds_sys_ptr->status.ain[AI_NTC6],gds_sys_ptr->status.ain[AI_NTC7],gds_sys_ptr->status.ain[AI_NTC8]);	
//		rt_kprintf("ain[2] = %d,ain[3] = %d,ain[4] = %d,ain[5] = %d,ain[6] = %d,ain[7] = %d\n",gds_sys_ptr->status.ain[2],gds_sys_ptr->status.ain[3],gds_sys_ptr->status.ain[4],gds_sys_ptr->status.ain[5],gds_sys_ptr->status.ain[6],gds_sys_ptr->status.ain[7]);			
}

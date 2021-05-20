#ifndef __DIO_H__
#define __DIO_H__
#include "stm32f10x.h"
#include "sys_conf.h"
enum
{
	Rep_DI=0,
	Rep_DO,
	Rep_AI,
	Rep_AO,
	Rep_MAX,
};

typedef struct 
{
	uint8_t u8M3_Bit; //STM32F103RCT6;
	uint8_t u8M1_Bit; //STM32F103VCT6;
}Bit_remap_st;

enum
{
		DI_HI_PRESS1_BPOS	 =0,
		DI_LO_PRESS1_BPOS,
	  DI_HI_PRESS2_BPOS,	
		DI_LO_PRESS2_BPOS,					
		DI_FAN01_OD_BPOS,						
		DI_AIR_FLOW_BPOS,					
		DI_SMOKE_BPOS,							
		DI_WATER_OVERFLOW_BPOS,				
		DI_HEATER_OD_BPOS,					
		DI_REMOTE_BPOS,						
		DI_FILTER_BPOS,						
		DI_REV_BPOS,
		DI_FAN02_OD_BPOS,
		DI_FAN03_OD_BPOS,
		DI_FAN04_OD_BPOS,					
		DI_FAN05_OD_BPOS,
		DI_HUM_DEFAULT_BPOS,
		DI_GROUP_DEFAULT_BPOS,
		DI_POWER_LOSS_BPOS,//电源掉电
		DI_DUMMY_BPOS,
		DI_RESERVE0,
		DI_RESERVE1,
		DI_RESERVE2,
		DI_RESERVE3,	
		DI_WATER_PAN_BPOS,      //接水盘溢出
		DI_HUM_OVERHEAT_BPOS,		
		DI_MAX_BPOS,	
};

//#define  DI_HI_PRESS2_BPOS            DI_AIR_FLOW_BPOS
//#define  DI_LO_PRESS2_BPOS            DI_SMOKE_BPOS
//
#define  DI_USR_BPOS            		DI_SMOKE_BPOS
#define  DI_FAN02_ERR_BPOS          DI_AIR_FLOW_BPOS
////冷热机(ZL)
//#define  DI_BK_POWER_BPOS						DI_SMOKE_BPOS

//ICT(测试键)
#define  DI_ICT_POWERON_BPOS			DI_HI_PRESS1_BPOS
#define  DI_ICT_POWEROFF_BPOS			DI_LO_PRESS1_BPOS

static uint16_t	drv_di_timer_init(void);
static void dio_reg_init(void);
static void di_reg_update(void);
//static void pwm_slow_timer_init(void);
static void drv_dio_bsp_init(void);

void drv_dio_init(void);
void di_sts_update(sys_reg_st*	gds_sys_ptr);
void dio_set_do(uint16_t channel_id, BitAction data);
//void slow_pwm_set(uint8_t channel, uint16_t dutycycle);
void led_toggle(void);
extern uint16_t Sts_Remap(uint16_t u16IN_Bit,uint8_t Rep_Type,uint8_t Rep_Dir);

#endif //__DIO_H__

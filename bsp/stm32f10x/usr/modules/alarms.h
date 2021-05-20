#ifndef __ALAMRS_H__
#define __ALAMRS_H__

#include "sys_conf.h"


#define ACL_INACTIVE 		0
#define ACL_PREACTIVE 	1
#define ACL_ACTIVE 			2
#define ACL_POSTACTIVE	3

#define ACL_ENABLE			0
#define ACL_SUPPRESS		1
#define ACL_DISABLE			2

//alarm acl def
enum
{
//出风和回风温湿度报警	
			ACL_HI_TEMP_RETURN = 0		,
			ACL_LO_TEMP_RETURN			  ,
			ACL_HI_HUM_RETURN			    ,
			ACL_LO_HUM_RETURN			    ,
			ACL_HI_TEMP_SUPPLY			  ,
			ACL_LO_TEMP_SUPPLY			  ,
			ACL_HI_HUM_SUPPLY			    ,
			ACL_LO_HUM_SUPPLY			    ,
			ACL_MBM_HARD_FAULT				,// modbus slave 硬件错误
			ACL_MBM_COM_LOSS				  ,//modbus slave 通信错误
			ACL_NTC_INVALID           ,
//风机过载
			ACL_FAN_OVERLOAD1				,
			ACL_FAN_OVERLOAD2				,
			ACL_FAN_OVERLOAD3				,
			ACL_FAN_OVERLOAD4				,
			ACL_FAN_OVERLOAD5				,
			ACL_EX_FAN_CURRENT      ,//外风机电流过大
			ACL_FAN_OT1						  ,
			ACL_FAN_OT2						  ,
			ACL_FAN_OT3						  ,
			ACL_FAN_OT4						  ,
			ACL_FAN_OT5						  ,
			ACL_EX_FAN2_CURRENT     ,
//压缩机1报警
			ACL_HI_PRESS1           ,						
			ACL_HI_LOCK1					  ,
			ACL_LO_PRESS1					  ,
			ACL_LO_LOCK1					  ,
			ACL_EXTMP1						  ,//排气高温 ACL_INV_HI_TEMP
			ACL_EXTMP_LOCK1				  ,
			ACL_SHORT_TERM1					,
			ACL_COMPRESSOR_OT1			,
//压缩机2报警
			ACL_HI_PRESS2						,
			ACL_HI_LOCK2						,
			ACL_LO_PRESS2						,
			ACL_LO_LOCK2						,
			ACL_EXTMP2							,
			ACL_EXTMP_LOCK2					,
			ACL_SHORT_TERM2				  ,
			ACL_COMPRESSOR_OT2			,
//加湿器故障报警
			ACL_HUM_OCURRENT        ,
			ACL_HUM_HI_LEVEL			  ,
			ACL_HUM_LO_LEVEL        ,
			ACL_HUM_OT					    ,
//加热器 运行超时
			ACL_HEATER_OD           ,//加热器过载
			ACL_HEATER_OT1			    ,
			ACL_HEATER_OT2				  ,
		//	ACL_HEATER_OT3				,
//电源类报警

			ACL_POWER_LOSS					,//ABC三相电电压低于门限电压
			ACL_POWER_EP						,	//reverse phase
			ACL_POWER_HI_FD					,	//freqency deviation
			ACL_POWER_LO_FD         ,
			ACL_POWER_A_HIGH				,
			ACL_POWER_B_HIGH				,
			ACL_POWER_C_HIGH				,
			ACL_POWER_A_LOW					,
			ACL_POWER_B_LOW					,
			ACL_POWER_C_LOW					,
			ACL_POWER_A_OP					,	//open phase
			ACL_POWER_B_OP					,	//open phase
			ACL_POWER_C_OP					,	//open phase
//气流丢失 单独函数实现
			ACL_AIR_LOSS				   ,
//过滤网赌
			ACL_FILTER_OT          ,
			ACL_FILTER_CLOG        ,
// 远程关机 报警
			ACL_REMOTE_SHUT				 ,
//地板积水
			ACL_WATER_OVERFLOW		 ,	
//群控失败告警
			ACL_GROUP_CONTROL_FAIL ,	
//变频器故障
			ACL_INV_FAULT        ,
// 注水电导率过高提示
			ACL_WATER_ELEC_HI      ,
//注水电导率过低
			ACL_WATER_ELEC_LO      ,
//烟雾告警
      ACL_SMOKE_ALARM         ,
//软件预留报警
			ACL_USR_ALARM1			   ,
//启动备用电源提示 (电源故障)
      ACL_BACK_POWER         ,	

//冷冻水阀(NTC温度模拟量报警)
			ACL_COIL_HI_TEM1        ,
			ACL_COIL_VALVE_DEFAULT   ,
			ACL_COIL_BLOCKING        ,
//列间冷冻水
			ACL_FAN_OVERLOAD6       ,
			ACL_FAN_OVERLOAD7       ,
			ACL_FAN_OVERLOAD8       ,
			ACL_FAN_OT6             ,
			ACL_FAN_OT7             ,
			ACL_FAN_OT8             , 
			
//水泵高水位告警
      ACL_WATER_PUMP_HI_LIVEL ,
			ACL_HUM_DEFAULT         ,
			ACL_FLOW_DIFF           ,
			ACL_HUM_OD              ,
			ACL_INV_HIPRESS					,//变频器高压
//			ACL_WATERPAN						,//接水盘
			ACL_TOTAL_NUM				    ,
			ACL_NTC_EVAPORATE       ,//蒸发器温度,Alair,20161118
			//Alair,20161221
			ACL_USR_ALARM2				  ,
			ACL_USR_ALARM3				  ,
			ACL_USR_ALARM4				  ,
////			ACL_MBM_HUMIDITY_ERROR		  ,//?????
////			ACL_MBM_POWER_ERROR				  ,//?????
//			ACL_TOTAL_NUM				    ,

};

//电源故障
#define   ACL_POWER_ERROR    ACL_BACK_POWER   

enum
{
	DEV_RETURN_SENSOR1_FAULT_BPOS=0,
	DEV_RETURN_SENSOR2_FAULT_BPOS,
	DEV_RETURN_SENSOR3_FAULT_BPOS,
	DEV_RETURN_SENSOR4_FAULT_BPOS,
	DEV_SUPPLY_SENSOR1_FAULT_BPOS,
	DEV_SUPPLY_SENSOR2_FAULT_BPOS,
	//
	DEV_TEM_HUM_RESERVE1_FAULT_BPOS,
	DEV_TEM_HUM_RESERVE2_FAULT_BPOS,
//	DEV_HUM_MODULE_FAULT_BPOS,
//	DEV_POWER_MODULE_FAULT_BPOS,
	//
	DEV_NTC7_BPOS,
	DEV_NTC8_BPOS,
	DEV_CIOL_NTC1_FAULT_BPOS,
	DEV_CIOL_NTC2_FAULT_BPOS,
	DEV_RETUREN_NTC1_FAULT_BPOS,
	DEV_RETUREN_NTC2_FAULT_BPOS,
	DEV_SUPPLY_NTC1_FAULT_BPOS,
	DEV_SUPPLY_NTC2_FAULT_BPOS,
};

#define ALARM_THRESHOLD		50//告警阈值


void alarm_acl_init(void);
void alarm_acl_exe(void);

uint8_t get_alarm_bitmap(uint8_t alarm_id);

uint8_t clear_alarm(uint8_t alarm_id);
uint8_t get_alarm_bitmap_mask(uint8_t component_bpos);
uint8_t get_alarm_bitmap_op(uint8_t component_bpos);

#endif //__ALAMRS_H__

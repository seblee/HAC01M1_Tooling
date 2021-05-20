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
//����ͻط���ʪ�ȱ���	
			ACL_HI_TEMP_RETURN = 0		,
			ACL_LO_TEMP_RETURN			  ,
			ACL_HI_HUM_RETURN			    ,
			ACL_LO_HUM_RETURN			    ,
			ACL_HI_TEMP_SUPPLY			  ,
			ACL_LO_TEMP_SUPPLY			  ,
			ACL_HI_HUM_SUPPLY			    ,
			ACL_LO_HUM_SUPPLY			    ,
			ACL_MBM_HARD_FAULT				,// modbus slave Ӳ������
			ACL_MBM_COM_LOSS				  ,//modbus slave ͨ�Ŵ���
			ACL_NTC_INVALID           ,
//�������
			ACL_FAN_OVERLOAD1				,
			ACL_FAN_OVERLOAD2				,
			ACL_FAN_OVERLOAD3				,
			ACL_FAN_OVERLOAD4				,
			ACL_FAN_OVERLOAD5				,
			ACL_EX_FAN_CURRENT      ,//������������
			ACL_FAN_OT1						  ,
			ACL_FAN_OT2						  ,
			ACL_FAN_OT3						  ,
			ACL_FAN_OT4						  ,
			ACL_FAN_OT5						  ,
			ACL_EX_FAN2_CURRENT     ,
//ѹ����1����
			ACL_HI_PRESS1           ,						
			ACL_HI_LOCK1					  ,
			ACL_LO_PRESS1					  ,
			ACL_LO_LOCK1					  ,
			ACL_EXTMP1						  ,//�������� ACL_INV_HI_TEMP
			ACL_EXTMP_LOCK1				  ,
			ACL_SHORT_TERM1					,
			ACL_COMPRESSOR_OT1			,
//ѹ����2����
			ACL_HI_PRESS2						,
			ACL_HI_LOCK2						,
			ACL_LO_PRESS2						,
			ACL_LO_LOCK2						,
			ACL_EXTMP2							,
			ACL_EXTMP_LOCK2					,
			ACL_SHORT_TERM2				  ,
			ACL_COMPRESSOR_OT2			,
//��ʪ�����ϱ���
			ACL_HUM_OCURRENT        ,
			ACL_HUM_HI_LEVEL			  ,
			ACL_HUM_LO_LEVEL        ,
			ACL_HUM_OT					    ,
//������ ���г�ʱ
			ACL_HEATER_OD           ,//����������
			ACL_HEATER_OT1			    ,
			ACL_HEATER_OT2				  ,
		//	ACL_HEATER_OT3				,
//��Դ�౨��

			ACL_POWER_LOSS					,//ABC������ѹ�������޵�ѹ
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
//������ʧ ��������ʵ��
			ACL_AIR_LOSS				   ,
//��������
			ACL_FILTER_OT          ,
			ACL_FILTER_CLOG        ,
// Զ�̹ػ� ����
			ACL_REMOTE_SHUT				 ,
//�ذ��ˮ
			ACL_WATER_OVERFLOW		 ,	
//Ⱥ��ʧ�ܸ澯
			ACL_GROUP_CONTROL_FAIL ,	
//��Ƶ������
			ACL_INV_FAULT        ,
// עˮ�絼�ʹ�����ʾ
			ACL_WATER_ELEC_HI      ,
//עˮ�絼�ʹ���
			ACL_WATER_ELEC_LO      ,
//����澯
      ACL_SMOKE_ALARM         ,
//���Ԥ������
			ACL_USR_ALARM1			   ,
//�������õ�Դ��ʾ (��Դ����)
      ACL_BACK_POWER         ,	

//�䶳ˮ��(NTC�¶�ģ��������)
			ACL_COIL_HI_TEM1        ,
			ACL_COIL_VALVE_DEFAULT   ,
			ACL_COIL_BLOCKING        ,
//�м��䶳ˮ
			ACL_FAN_OVERLOAD6       ,
			ACL_FAN_OVERLOAD7       ,
			ACL_FAN_OVERLOAD8       ,
			ACL_FAN_OT6             ,
			ACL_FAN_OT7             ,
			ACL_FAN_OT8             , 
			
//ˮ�ø�ˮλ�澯
      ACL_WATER_PUMP_HI_LIVEL ,
			ACL_HUM_DEFAULT         ,
			ACL_FLOW_DIFF           ,
			ACL_HUM_OD              ,
			ACL_INV_HIPRESS					,//��Ƶ����ѹ
//			ACL_WATERPAN						,//��ˮ��
			ACL_TOTAL_NUM				    ,
			ACL_NTC_EVAPORATE       ,//�������¶�,Alair,20161118
			//Alair,20161221
			ACL_USR_ALARM2				  ,
			ACL_USR_ALARM3				  ,
			ACL_USR_ALARM4				  ,
////			ACL_MBM_HUMIDITY_ERROR		  ,//?????
////			ACL_MBM_POWER_ERROR				  ,//?????
//			ACL_TOTAL_NUM				    ,

};

//��Դ����
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

#define ALARM_THRESHOLD		50//�澯��ֵ


void alarm_acl_init(void);
void alarm_acl_exe(void);

uint8_t get_alarm_bitmap(uint8_t alarm_id);

uint8_t clear_alarm(uint8_t alarm_id);
uint8_t get_alarm_bitmap_mask(uint8_t component_bpos);
uint8_t get_alarm_bitmap_op(uint8_t component_bpos);

#endif //__ALAMRS_H__

#include <rtthread.h>
#include "sys_conf.h"
#include "kits/fifo.h"
#include "alarms.h"
#include "local_status.h"
#include "global_var.h"
#include "req_execution.h"

#include "event_record.h"
#include "rtc_bsp.h"
#include "daq.h"
#include "dio_bsp.h"
#include "sys_status.h"
#include "team.h"
#include "sys_conf.h"
#include "led_bsp.h"
#include "password.h"
#include "user_mb_app.h"

#define ACL_ENMODE_ENABLE	0x0000
#define ACL_ENMODE_SUPPRESS	0x0001//����
#define ACL_ENMODE_DISABLE	0x0002

#define ACL_ENMODE_AUTO_RESET_ALARM     0X0004
#define ACL_ENMODE_HAND_RESET_ALARM     0X0000

#define MBM_DEV_STS_DEFAULT		0x8000

#define ACL_TEM_MAX 1400 //0.1��
#define ACL_TEM_MIN -280

#define ACL_HUM_MAX 1000 // 0.1%
#define ACL_HUM_MIN  0

#define IO_CLOSE 1
#define IO_OPEN 0

#define OVER_TIME_ACCURACY     	86400          //DATE



#define POWER_PARAM		0xFF80
#define POWER_DOT 0x7f
#define POWER_DOT_BIT 7

uint8_t hipress_flag_triggle = 0,buffer_stats = 0;

typedef struct alarm_acl_td
{
	uint16_t				  id;
	uint16_t 					state;
	uint16_t          alram_value;
	uint16_t					timeout;
	uint16_t 					enable_mask;
	uint16_t 					reset_mask;
	uint8_t           alarm_level;
	uint16_t          dev_type;
	uint16_t (*alarm_proc)(struct alarm_acl_td* str_ptr);	
	
}alarm_acl_status_st;


typedef enum
{
	SML_MIN_TYPE=0,
	THR_MAX_TYPE,
	OUT_MIN_MAX_TYPE,
	IN_MIN_MAX_TYPE,	
}Compare_type_st;


typedef struct
{
	uint32_t lock_time[3];//

	uint16_t last_state;
	char lock_flag;
	
}alarm_lock_st;

typedef struct
{
	char  cycle_flag;//�����ڴ�����־��
	uint16_t compress_state;//1\2ѹ����״̬��
	uint32_t start_time[10];//ѹ����1��2������ʱ��
	uint16 alarm_timer;
}compress_cycle_alarm_st;

#define  MAX_LOCK_CNT 6
typedef struct
{
	
 	uint8_t compress_high_flag[2]; //0��ʾѹ����1��ѹ���� 1��ʾѹ��������ѹ�澯
	uint16_t tem_sensor_fault;
	uint16_t hum_sensor_fault;
 	uint32_t fan_timer; // 1second++
	uint32_t compressor1_timer;//1second++
	uint32_t compressor2_timer;//1second++
	alarm_lock_st  alarm_lock[MAX_LOCK_CNT];
/*
	{0xffffffff,0xffffffff,0xffffffff},//ACL_HI_LOCK1                  0
	{0xffffffff,0xffffffff,0xffffffff},//ACL_HI_LOCK2                 1
	{0xffffffff,0xffffffff,0xffffffff},//ACL_LO_LOCK1                  2
	{0xffffffff,0xffffffff,0xffffffff},//ACL_LO_LOCK2                  3
	{0xffffffff,0xffffffff,0xffffffff},//ACL_EXTMP_LOCK1               4
	{0xffffffff,0xffffffff,0xffffffff},//ACL_EXTMP_LOCK2               5

*/
	compress_cycle_alarm_st cmpress_cycle_alarm[2];
	alarm_acl_status_st alarm_sts[ACL_TOTAL_NUM];
	
}alarm_st;



static alarm_st alarm_inst;
extern	sys_reg_st		g_sys; 

extern local_reg_st 				l_sys;	
//static uint16_t io_calc(uint8_t data,uint8_t refer);
//static uint16_t compare_calc(int16_t meter,int16_t min,int16_t max,Compare_type_st type);

//static uint16_t alarm_lock(uint16_t alarm_id);

static void   alarm_status_bitmap_op (uint8_t alarm_id,uint8_t option);


//��⺯��
static  uint16_t acl00(alarm_acl_status_st* acl_ptr);
static	uint16_t acl01(alarm_acl_status_st* acl_ptr);
static	uint16_t acl02(alarm_acl_status_st* acl_ptr);
static	uint16_t acl03(alarm_acl_status_st* acl_ptr);
static	uint16_t acl04(alarm_acl_status_st* acl_ptr);
static	uint16_t acl05(alarm_acl_status_st* acl_ptr);
static	uint16_t acl06(alarm_acl_status_st* acl_ptr);
static	uint16_t acl07(alarm_acl_status_st* acl_ptr);
//modebus slave hardware fault                   
static 	uint16_t acl08(alarm_acl_status_st* acl_ptr);
//modebus slave communication loss                     
static 	uint16_t acl09(alarm_acl_status_st* acl_ptr);
//NTC  invalid
static 	uint16_t acl10(alarm_acl_status_st* acl_ptr);
//��������                               
static	uint16_t acl11(alarm_acl_status_st* acl_ptr);
static	uint16_t acl12(alarm_acl_status_st* acl_ptr);
static	uint16_t acl13(alarm_acl_status_st* acl_ptr);
static	uint16_t acl14(alarm_acl_status_st* acl_ptr);                       
static	uint16_t acl15(alarm_acl_status_st* acl_ptr);
static	uint16_t acl16(alarm_acl_status_st* acl_ptr);
static	uint16_t acl17(alarm_acl_status_st* acl_ptr);
static	uint16_t acl18(alarm_acl_status_st* acl_ptr);
static	uint16_t acl19(alarm_acl_status_st* acl_ptr);
static	uint16_t acl20(alarm_acl_status_st* acl_ptr);                                    
static	uint16_t acl21(alarm_acl_status_st* acl_ptr);
static	uint16_t acl22(alarm_acl_status_st* acl_ptr);


static	uint16_t acl23(alarm_acl_status_st* acl_ptr);
static	uint16_t acl24(alarm_acl_status_st* acl_ptr);
                                            
                                            
//ѹ����1������                             
static	uint16_t acl25(alarm_acl_status_st* acl_ptr);
                                            
//ѹ����1��ʱ����                           
static	uint16_t acl26(alarm_acl_status_st* acl_ptr);
                                            
                                           
//ѹ����2                                   
                                            
static	uint16_t acl27(alarm_acl_status_st* acl_ptr);
static	uint16_t acl28(alarm_acl_status_st* acl_ptr);
static	uint16_t acl29(alarm_acl_status_st* acl_ptr);
static	uint16_t acl30(alarm_acl_status_st* acl_ptr);
//�̹�2                                    
static	uint16_t acl31(alarm_acl_status_st* acl_ptr);
static	uint16_t acl32(alarm_acl_status_st* acl_ptr);
static	uint16_t acl33(alarm_acl_status_st* acl_ptr);
static	uint16_t acl34(alarm_acl_status_st* acl_ptr);
//                                         
static	uint16_t acl35(alarm_acl_status_st* acl_ptr);
static	uint16_t acl36(alarm_acl_status_st* acl_ptr);
static	uint16_t acl37(alarm_acl_status_st* acl_ptr);
static	uint16_t acl38(alarm_acl_status_st* acl_ptr);
static	uint16_t acl39(alarm_acl_status_st* acl_ptr);
static  uint16_t acl40(alarm_acl_status_st* acl_ptr);
static	uint16_t acl41(alarm_acl_status_st* acl_ptr);
static	uint16_t acl42(alarm_acl_status_st* acl_ptr);
static	uint16_t acl43(alarm_acl_status_st* acl_ptr);
static	uint16_t acl44(alarm_acl_status_st* acl_ptr);
static	uint16_t acl45(alarm_acl_status_st* acl_ptr);
static	uint16_t acl46(alarm_acl_status_st* acl_ptr);
static	uint16_t acl47(alarm_acl_status_st* acl_ptr);
static	uint16_t acl48(alarm_acl_status_st* acl_ptr);
static	uint16_t acl49(alarm_acl_status_st* acl_ptr);
static	uint16_t acl50(alarm_acl_status_st* acl_ptr);
static	uint16_t acl51(alarm_acl_status_st* acl_ptr);
static	uint16_t acl52(alarm_acl_status_st* acl_ptr);
static	uint16_t acl53(alarm_acl_status_st* acl_ptr);
static	uint16_t acl54(alarm_acl_status_st* acl_ptr);
static	uint16_t acl55(alarm_acl_status_st* acl_ptr);
static	uint16_t acl56(alarm_acl_status_st* acl_ptr);
static	uint16_t acl57(alarm_acl_status_st* acl_ptr);
static	uint16_t acl58(alarm_acl_status_st* acl_ptr);
static	uint16_t acl59(alarm_acl_status_st* acl_ptr);
static	uint16_t acl60(alarm_acl_status_st* acl_ptr);
static	uint16_t acl61(alarm_acl_status_st* acl_ptr);
static	uint16_t acl62(alarm_acl_status_st* acl_ptr);
static	uint16_t acl63(alarm_acl_status_st* acl_ptr);
static	uint16_t acl64(alarm_acl_status_st* acl_ptr);
static	uint16_t acl65(alarm_acl_status_st* acl_ptr);
static	uint16_t acl66(alarm_acl_status_st* acl_ptr);
static	uint16_t acl67(alarm_acl_status_st* acl_ptr);
static	uint16_t acl68(alarm_acl_status_st* acl_ptr);
static	uint16_t acl69(alarm_acl_status_st* acl_ptr);
static	uint16_t acl70(alarm_acl_status_st* acl_ptr);
static	uint16_t acl71(alarm_acl_status_st* acl_ptr);
static	uint16_t acl72(alarm_acl_status_st* acl_ptr);
static	uint16_t acl73(alarm_acl_status_st* acl_ptr);
static	uint16_t acl74(alarm_acl_status_st* acl_ptr);
static	uint16_t acl75(alarm_acl_status_st* acl_ptr);
static	uint16_t acl76(alarm_acl_status_st* acl_ptr);
static	uint16_t acl77(alarm_acl_status_st* acl_ptr);
static	uint16_t acl78(alarm_acl_status_st* acl_ptr);
static	uint16_t acl79(alarm_acl_status_st* acl_ptr);
static	uint16_t acl80(alarm_acl_status_st* acl_ptr);
static  uint16_t acl81 (alarm_acl_status_st* acl_ptr);
static  uint16_t acl82(alarm_acl_status_st* acl_ptr);
static  uint16_t acl83(alarm_acl_status_st* acl_ptr);
static  uint16_t acl84(alarm_acl_status_st* acl_ptr);

//static  uint16_t acl85 (alarm_acl_status_st* acl_ptr);
//static  uint16_t acl86(alarm_acl_status_st* acl_ptr);
//static  uint16_t acl87(alarm_acl_status_st* acl_ptr);
//static  uint16_t acl88(alarm_acl_status_st* acl_ptr);
//static  uint16_t acl88(alarm_acl_status_st* acl_ptr);
//static  uint16_t acl89(alarm_acl_status_st* acl_ptr);

//�澯����ٲ�
//static void alarm_arbiration(void);
static void alarm_bitmap_op(uint8_t component_bpos, uint8_t action);
static void alarm_bitmap_mask_op(uint8_t component_bpos, uint8_t action);

enum
{
		ALARM_ACL_ID_POS=			0,//�澯���
		ALARM_ACL_EN_MASK_POS		,	//�̵�������
		ALARM_ACL_RESET_POS     ,//�����ʽ
		AlARM_ACL_LEVEL_POS     ,//�澯�ȼ�
		ALARM_ACL_DEV_POS      ,//�澯����
		ALARM_ACL_MAX
};


#define ALARM_FSM_INACTIVE			0x0001	
#define ALARM_FSM_PREACTIVE			0x0002
#define ALARM_FSM_ACTIVE				0x0003	
#define ALARM_FSM_POSTACTIVE		0x0004	
#define ALARM_FSM_ERROR					0x0005	

#define ALARM_ACL_TRIGGERED			0x0001
#define ALARM_ACL_CLEARED			0x0000
#define ALARM_ACL_HOLD              0x0002





//uint16_t alarm_tem_erro,alarm_hum_erro;

static uint16_t (* acl[ACL_TOTAL_NUM])(alarm_acl_status_st*) = 
{
	
//�ط���ͷ籨��(�¶Ⱥ�ʪ��)
		acl00,//		ACL_HI_TEMP_RETURN	
		acl01,//		ACL_LO_TEMP_RETURN
		acl02,//		ACL_HI_HUM_RETURN	
		acl03,//		ACL_LO_HUM_RETURN	
		acl04,//		ACL_HI_TEMP_SUPPLY	
		acl05,//		ACL_LO_TEMP_SUPPLY	
		acl06,//		ACL_HI_HUM_SUPPLY
		acl07,//		ACL_LO_HUM_SUPPLY	 

////��ʪ�ȴ���������ֵ���ޱ���
//		acl08,//		ACL_MBM_HARD_FAULT  
//		acl09,//		
		acl08,//		�ͷ���ʪ�ȴ���������,Alair,20161125
		acl09,//		�ط���ʪ�ȴ���������
		acl10,//		ACL_NTC_INVALIDE
////��������ʵ�ַ������ܻ��޸�
////�������ϱ���
		acl11,//ACL_FAN_OVERLOAD1		
		acl12,//ACL_FAN_OVERLOAD2		
		acl13,//ACL_FAN_OVERLOAD3		
		acl14,//ACL_FAN_OVERLOAD4		       
		acl15,//ACL_FAN_OVERLOAD5		
		acl16,//ACL_EX_FAN_CURRENT�����
		acl17,//ACL_FAN_OT1					
		acl18,//ACL_FAN_OT2					
		acl19,//ACL_FAN_OT3					
		acl20,//ACL_FAN_OT4					
		acl21,//ACL_FAN_OT5					          
		acl22,//ACL_EX_FAN2_CURRENT 
		
//ѹ���������
//ѹ����1
		acl23,//ACL_HI_PRESS1                  
		acl24,//ACL_HI_LOCK1			             
		acl25,//ACL_LO_PRESS1				
		acl26,//ACL_LO_LOCK1				
		acl27,//ACL_EXTMP1				
	  acl28,//ACL_EXTMP_LOCK1		
		acl29,//ACL_SHORT_TERM1		
		acl30,//ACL_COMPRESSOR_OT1
//ѹ����2
		acl31,//ACL_HI_PRESS2			
		acl32,//ACL_HI_LOCK2			
		acl33,//ACL_LO_PRESS2			
		acl34,//ACL_LO_LOCK2			
		acl35,//ACL_EXTMP2				
		acl36,//ACL_EXTMP_LOCK2		
		acl37,//ACL_SHORT_TERM2		
		acl38,//ACL_COMPRESSOR_OT2
////��ʪ����ر���
		acl39,//ACL_HUM_OCURRENT
////��ʪ����ˮλ����()
	  acl40,//ACL_HUM_HI_LEVEL
////��ʪ����ˮλ����
		acl41,//ACL_HUM_LO_LEVEL
		acl42,//ACL_HUM_OT				,
		
////���������б���1,2,3��ʱ����
		acl43,//ACL_HEATER_OD
		acl44,//ACL_HEATER_OT1			,
		acl45,//ACL_HEATER_OT2			,
////��Դ�౨��
		acl46,//ACL_POWER_LOSS				
		acl47,//ACL_POWER_EP					
		acl48,//ACL_POWER_HI_FD					
		acl49,//ACL_POWER_LO_FD				
		acl50,//ACL_POWER_A_HIGH
		acl51,//ACL_POWER_B_HIGH
		acl52,//ACL_POWER_C_HIGH
		acl53,//ACL_POWER_A_LOW	
		acl54,//ACL_POWER_B_LOW	
		acl55,//ACL_POWER_C_LOW	
		acl56,//ACL_POWER_A_OP	
		acl57,//ACL_POWER_B_OP	
    acl58,//ACL_POWER_C_OP	
////������ʧ ��������ʵ��
    acl59,// ACL_AIR_LOSS
//��������ʱ����
		acl60,//ACL_FILTER_OT					
////��������						
    acl61,//ACL_FILTER_CLOG				
//// Զ�̹ػ� ����
    acl62,//ACL_REMOTE_SHUT				
////�ذ��ˮ
    acl63,//ACL_WATER_OVERFLOW		
////Ⱥ��ʧ�ܸ澯(ר�ú���ʵ��)
    acl64,//ACL_GROUP_CONTROL_FAIL
//��Ƶ������
		acl65,//ACL_INV_FAULT
		acl66,//ACL_WATER_ELEC_HI
		acl67,//ACL_WATER_ELEC_LO
		acl68,//ACL_SMOKE_ALARM
		acl69,//ACL_USR_ALARM1
		acl70,//ACL_BACK_POWER
//�̹�1�澯
    acl71,//ACL_COIL_HI_TEM1      
		acl72,//ACL_COIL_VALVE_DEFAULT 
		acl73,//ACL_COIL_BLOCKING      
		acl74,//ACL_FAN_OVERLOAD6 

		acl75,//ACL_FAN_OVERLOAD7     
		acl76,//ACL_FAN_OVERLOAD8
		acl77,//ACL_FAN_OT6     
		acl78,//ACL_FAN_OT7

		acl79,//ACL_FAN_OT8
		acl80,//ACL_WATER_PUMP_HI_LIVEL
		acl81,//ACL_HUM_DEFAULT
		acl82,//ACL_FLOW_DIFF
		acl83,//ACL_HUM_OD
		acl84,//ACL_INV_HIPRESS
//		acl85,//ACL_WATERPAN
//		acl86,//ACL_INV_HI_TEMP
//		acl87,//ACL_EX_FAN_CURRENT
//		acl88,//ACL_EX_FAN2_CURRENT
        
//		acl89,//ACL_INV_HI_TEMP
//�û���չ����Ԥ������ʹ��		
        
//		ACL_USR_ALARM1					,
//		ACL_USR_ALARM2					,
//		ACL_USR_ALARM3           ,
};

#define DEV_TYPE_COMPRESSOR 0x0000
#define DEV_TYPE_FAN        0x0400
#define DEV_TYPE_OUT_FAN    0x0800
#define DEV_TYPE_HEATER     0x0c00
#define DEV_TYPE_HUM        0x1000
#define DEV_TYPE_POWER      0x1400
#define DEV_TYPE_TEM_SENSOR 0x1800
#define DEV_TYPE_WATER_PUMP 0x1c00
#define DEV_TYPE_OTHER      0x3c00


const uint16_t ACL_CONF[ACL_TOTAL_NUM][ALARM_ACL_MAX]=
//	id ,en_mask,reless_mask,DEV_type
{		
	  0,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_HI_TEMP_RETURN						
//		1,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_LO_TEMP_RETURN						
//		2,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_HI_HUM_RETURN						
//		3,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_LO_HUM_RETURN							
//		4,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_HI_TEMP_SUPPLY						
//		5,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_LO_TEMP_SUPPLY						
//		6,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_HI_HUM_SUPPLY						
//		7,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_LO_HUM_SUPPLY			
		1,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_LO_TEMP_RETURN						
		2,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_HI_HUM_RETURN						
		3,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_LO_HUM_RETURN							
		4,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_HI_TEMP_SUPPLY						
		5,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_LO_TEMP_SUPPLY						
		6,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_HI_HUM_SUPPLY						
		7,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_TEM_SENSOR,           //ACL_LO_HUM_SUPPLY			
		8,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_OTHER,                //ACL_MBM_HARD_FAULT						
		9,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_OTHER,                //ACL_MBM_COM_LOSS
		10,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_OTHER,                //ACL_NTC_INVALIDE		
	//fan							  	
		11,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_FAN,                 //ACL_FAN_OVERLOAD1					
		12,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_FAN,                 //ACL_FAN_OVERLOAD2					
		13,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_FAN,                 //ACL_FAN_OVERLOAD3		
		14,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_FAN,                 //ACL_FAN_OVERLOAD4		
		15,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_FAN,                 //ACL_FAN_OVERLOAD5		
		16,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_OTHER,               //ACL_EX_FAN_CURRENT
		17,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_FAN,                 //ACL_FAN_OT1					
		18,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_FAN,                 //ACL_FAN_OT2					
		19,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_FAN,                 //ACL_FAN_OT3					
		20,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_FAN,                 //ACL_FAN_OT4					
		21,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_FAN,                 //ACL_FAN_OT5				
		22,			3,		4,	CRITICAL_ALARM_lEVEL,		 DEV_TYPE_OTHER,             //ACL_EX_FAN_CURRENT2 
//compressor1	                               
		23,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_HI_PRESS1     
		24,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_HI_LOCK1			
		25,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_LO_PRESS1			
		26,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_LO_LOCK1			
		27,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_EXTMP1				
		28,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_EXTMP_LOCK1			
		29,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_SHORT_TERM1		
		30,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_COMPRESSOR,          //ACL_COMPRESSOR_OT1
//compressor2	        	
		31,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_HI_PRESS2							
		32,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_HI_LOCK2								
		33,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_LO_PRESS2						
		34,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_LO_LOCK2				
		35,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_EXTMP2					
		36,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_EXTMP_LOCK2				
		37,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_COMPRESSOR,          //ACL_SHORT_TERM2			
		38,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_COMPRESSOR,          //ACL_COMPRESSOR_OT2	
//hum                 
		39,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_HUM,                 //ACL_HUM_OCURRENT				
		40,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_HUM,                //ACL_HUM_HI_LEVEL	
		41,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_HUM,                   //ACL_HUM_LO_LEVEL	
		42,			3,    4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_HUM,                //ACL_HUM_OT		
//heater				      	
		43,			3,    4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_HEATER,            //ACL_HEATER_OD					
		44,			3,    4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_HEATER,            //ACL_HEATER_OT1					
		45,			3,    4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_HEATER,						//ACL_HEATER_OT2
//��Դ�౨��          
		                  
		46,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_POWER,//ACL_POWER_LOSS						
		47,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_EP					
		48,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_HI_FD	
		49,     3,    4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_LO_FD	
		50,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_A_HIGH	
		51,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_B_HIGH
		52,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_C_HIGH
		53,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_A_LOW					
		54,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_B_LOW					
		55,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_C_LOW		
		56,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_A_OP
		57,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_B_OP
    58,     3,    4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_POWER,//ACL_POWER_C_OP
		                  
		59,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_OTHER,//ACL_AIR_LOSS	                  
		60,			3,		0,  MIOOR_ALARM_LEVEL,     DEV_TYPE_OTHER,//ACL_FILTER_OT
//		61,     3,    4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_OTHER,//ACL_FILTER_CLOG
		61,     3,    4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_OTHER,//ACL_FILTER_CLOG//��������
		                  
		62,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_OTHER,//ACL_REMOTE_SHUT
		                  
		63,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_OTHER,//ACL_WATER_OVERFLOW
		64,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_OTHER,//ACL_GROUP_CONTROL_FAIL	
		                  
		                  
    65,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_OTHER,//ACL_INV_FAULT 
    66,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_HUM,//ACL_WATER_ELEC_HI
    67,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_HUM,//ACL_WATER_ELEC_LO
    68,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_OTHER,//ACL_SMOKE_ALARM
    69,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_OTHER,//ACL_USR_ALARM1
//    70,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_OTHER,//ACL_BACK_POWER
    70,			3,		4,  CRITICAL_ALARM_lEVEL,   DEV_TYPE_OTHER,//ACL_BACK_POWER
		//�̹ܸ澯       
		71,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_OTHER,//ACL_COIL_HI_TEM1     
		72,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_OTHER,//ACL_COIL_VALVE_DEFAULT
    73,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_OTHER,//ACL_COIL_BLOCKING     
    74,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_FAN,//ACL_FAN_OVERLOAD6
    75,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_FAN,//ACL_FAN_OVERLOAD7     
    76,			3,		4,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_FAN, //ACL_FAN_OVERLOAD8
    77,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_FAN, //ACL_FAN_OT6     
    78,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_FAN, //ACL_FAN_OT7
		79,     3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_FAN, //ACL_FAN_OT8
    //ˮ�̸�ˮλ�澯  	
		80,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_OTHER,//ACL_WATER_PUMP_HI_LIVEL
		81,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_HUM,//ACL_HUM_DEFAULT		
		//ACL_FLOW_DIFF
		82,			3,		4,  MIOOR_ALARM_LEVEL,     DEV_TYPE_FAN,//ACL_FLOW_DIFF
		83,			3,		4,  MAJOR_ALARM_LEVEL,     DEV_TYPE_HUM,//ACL_HUM_OD
		84,			3,		4,	CRITICAL_ALARM_lEVEL,		 DEV_TYPE_COMPRESSOR, //ACL_INV_HIPRESS 
//		85,			3,		0,  CRITICAL_ALARM_lEVEL,  DEV_TYPE_OTHER,//ACL_WATERPAN
		
};
/*
  * @brief  alarm data structure initialization
	* @param  none
  * @retval none
  */

static void init_alarm(alarm_st* alarm_spr) 
{
		uint16 i;
		
		//��ʼACL 
		for(i=0;i<ACL_TOTAL_NUM;i++)
		{
			
			alarm_spr->alarm_sts[i].timeout =  0;
			alarm_spr->alarm_sts[i].state = ALARM_FSM_INACTIVE;
			alarm_spr->alarm_sts[i].id = ACL_CONF[i][ALARM_ACL_ID_POS];
			alarm_spr->alarm_sts[i].enable_mask = ACL_CONF[i][ALARM_ACL_EN_MASK_POS];
			alarm_spr->alarm_sts[i].reset_mask = ACL_CONF[i][ALARM_ACL_RESET_POS];
			alarm_spr->alarm_sts[i].alarm_level = ACL_CONF[i][AlARM_ACL_LEVEL_POS];
			alarm_spr->alarm_sts[i].dev_type = ACL_CONF[i][ALARM_ACL_DEV_POS];
			alarm_spr->alarm_sts[i].alarm_proc = acl[i];
			alarm_spr->alarm_sts[i].alram_value=0xffff;
		}
		
		//��ʼ��lock�����
		for(i=0;i<MAX_LOCK_CNT;i++)
		{
			alarm_spr->alarm_lock[i].last_state = ALARM_FSM_INACTIVE;
			alarm_spr->alarm_lock[i].lock_flag = 0;
			alarm_spr->alarm_lock[i].lock_time[0] = 0xffffffff;
			alarm_spr->alarm_lock[i].lock_time[1] = 0xffffffff;
			alarm_spr->alarm_lock[i].lock_time[2] = 0xffffffff;
		}
		//��ʼ��ѹ������ ������
		//alarm_spr->cmpress_cycle_alarm[0].alarm_timer
		alarm_spr->cmpress_cycle_alarm[0].cycle_flag = 0;
		alarm_spr->cmpress_cycle_alarm[1].cycle_flag = 0;

		alarm_spr->cmpress_cycle_alarm[0].compress_state = COMPRESSOR_FSM_STATE_IDLE;
		alarm_spr->cmpress_cycle_alarm[1].compress_state = COMPRESSOR_FSM_STATE_IDLE;
		
		for(i=0;i<10;i++)
		{
				alarm_spr->cmpress_cycle_alarm[0].start_time[i] = 0xffffffff; 
		    alarm_spr->cmpress_cycle_alarm[1].start_time[i] = 0xffffffff; 
		}
		//��ѹ����������ʼ��
		alarm_spr->compress_high_flag[0] = 0;
		alarm_spr->compress_high_flag[1] = 0;
//		alarm_spr->inv_compress_alarm[0] = 0;
		//��ʼ��������ʱ��
		alarm_spr->fan_timer = 0;
		alarm_spr->compressor1_timer = 0;
		alarm_spr->compressor2_timer = 0;

		
}


void alarm_acl_init(void)
{
	uint8_t i;
	// ��ʼ����̬�ڴ����ռ�
	chain_init();
	init_alarm(&alarm_inst);
	//��ʼ���ֶ��������
	for(i=0;i<ALARM_TOTAL_WORD;i++)
	{
			g_sys.config.general.alarm_remove_bitmap[i] = 0;
	}
} 

uint8_t clear_alarm(uint8_t alarm_id)
{
		uint8_t byte_offset,bit_offset,i;

		if(alarm_id == 0xFF)
		{
					for(i=0;i<ALARM_TOTAL_WORD;i++)
					{
						g_sys.config.general.alarm_remove_bitmap[i] = 0xffff;
					}

				return(1);
		}
		else
		if(alarm_id < ACL_TOTAL_NUM)
		{
				byte_offset = alarm_id >> 4;
				bit_offset = alarm_id & 0x0f;
				g_sys.config.general.alarm_remove_bitmap[byte_offset] |= 0x0001<< bit_offset;
			  return(1);
		}
		else
		{
				return(0);
		}
}
 
//uint8_t clear_alarm(void)
//{
//		uint8_t i;	

//		for(i=0;i<ALARM_TOTAL_WORD;i++)
//		{
//			g_sys.config.general.alarm_remove_bitmap[i] = 0xffff;
//			
//		}
//		//Alair,20170104,���ά���澯
//		if(g_sys.status.alarm_bitmap[5]&WORK_LIMIT_CATION)
//		{
//			g_sys.status.alarm_bitmap[5]&=~WORK_LIMIT_CATION;
//		}
//		g_sys.status.ControlPassword.Run_State&=~WORK_PASS_ALL;//���ά���澯
//		
//		return 1;		
//}

static uint8_t get_alarm_remove_bitmap(uint8_t alarm_id)
{
	uint8_t byte_offset,bit_offset;
	if(alarm_id < ACL_TOTAL_NUM)
	{
		
			byte_offset = alarm_id >> 4;
			bit_offset = alarm_id & 0x0f;
			if((g_sys.config.general.alarm_remove_bitmap[byte_offset] >> bit_offset) & 0x0001 )
			{
					return(1);
			}
			else
			{
					return(0);
			}
	}
	else
	{
			return(0);
	}
	
}

static void clear_alarm_remove_bitmap(uint8_t alarm_id)
{
	uint8_t byte_offset,bit_offset;
	
		if(alarm_id < ACL_TOTAL_NUM)
		{
				byte_offset = alarm_id >> 4;
				bit_offset = alarm_id & 0x0f;
				g_sys.config.general.alarm_remove_bitmap[byte_offset] = g_sys.config.general.alarm_remove_bitmap[byte_offset] & (~(0x0001 << bit_offset));
		}
	
}

/**
  * @brief  alarm check list function execution
	* @param  none
  * @retval none
  */
 static void acl_power_on_timer(void)
{
		extern local_reg_st		l_sys;
  
		if(g_sys.status.dout_bitmap & (0x0001 << DO_COMP1_BPOS))
		{
			 alarm_inst.compressor1_timer++;	
		}
		else
		{
			alarm_inst.compressor1_timer = 0;	
		}
		
		if(g_sys.status.dout_bitmap & (0x0001 << DO_COMP2_BPOS))
		{
			 alarm_inst.compressor2_timer++;	
		}
		else
		{
			alarm_inst.compressor2_timer = 0;	
		}
		
		if(g_sys.status.dout_bitmap & (0x0001 << DO_FAN_BPOS))
		{
			 alarm_inst.fan_timer ++;	
		}
		else
		{
			alarm_inst.fan_timer = 0;	
		}
}


uint16_t Alarm_acl_delay(uint8_t ACL_Num)
{
	uint16_t ACL_Delay;
	
	switch (ACL_Num)
	{
			case(ACL_FAN_OT2):
			case(ACL_FAN_OT3):
			case(ACL_FAN_OT4):
			case(ACL_FAN_OT5):
			case(ACL_FAN_OT6):
			case(ACL_FAN_OT7):
			case(ACL_FAN_OT8):
			{
					ACL_Delay = g_sys.config.alarm[ACL_FAN_OT1].delay;
					break;
			}
			case(ACL_FAN_OVERLOAD2):
			case(ACL_FAN_OVERLOAD3):
			case(ACL_FAN_OVERLOAD4):
			case(ACL_FAN_OVERLOAD5):
			case(ACL_FAN_OVERLOAD6):
			case(ACL_FAN_OVERLOAD7):
			case(ACL_FAN_OVERLOAD8):
			{
					ACL_Delay = g_sys.config.alarm[ACL_FAN_OVERLOAD1].delay;
					break;
			}	
			default:
			{
					ACL_Delay = g_sys.config.alarm[ACL_Num].delay;
					break;	
			}				
	}
	return ACL_Delay;
}


	
void alarm_acl_exe(void)
{
		extern	sys_reg_st		g_sys;
		uint16_t acl_trigger_state;	
		uint16_t i;
		uint16_t c_state;
		uint16_t log_id;
		
		
		acl_power_on_timer();
		
    for(i=0;i<ACL_TOTAL_NUM;i++)	
		{
				//if acl disabled, continue loop
			
				if(((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)&&(alarm_inst.alarm_sts[i].state == ALARM_FSM_INACTIVE))
				{
						continue;				
				}
			
				acl_trigger_state = acl[i](&alarm_inst.alarm_sts[i]);
				c_state = alarm_inst.alarm_sts[i].state;	
				log_id = alarm_inst.alarm_sts[i].id|(alarm_inst.alarm_sts[i].alarm_level<<8)|alarm_inst.alarm_sts[i].dev_type;
				switch (c_state)
				{
						case(ALARM_FSM_INACTIVE):
						{
							  if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{
//										alarm_inst.alarm_sts[i].timeout = g_sys.config.alarm[i].delay;
										alarm_inst.alarm_sts[i].timeout = Alarm_acl_delay(i);
										alarm_inst.alarm_sts[i].state = ALARM_FSM_PREACTIVE;
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;
								}
								else
								{
										;
								}
								
								break;
						}
						case(ALARM_FSM_PREACTIVE):
						{
									 //״̬���ص� ALARM_FSM_INACTIVE ״̬
								if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)
								{
										alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;		
								}
								else if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{
										
									
									 if(alarm_inst.alarm_sts[i].timeout > 0)
										{
												alarm_inst.alarm_sts[i].timeout --;
												alarm_inst.alarm_sts[i].state = ALARM_FSM_PREACTIVE;
										}
										else
										{
												alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
												//yxq
												add_alarmlog_fifo(log_id,ALARM_TRIGER,alarm_inst.alarm_sts[i].alram_value);
												alarm_status_bitmap_op(alarm_inst.alarm_sts[i].id&0x00ff,1);
												node_append(log_id,alarm_inst.alarm_sts[i].alram_value);
											
										}
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		;
								}
								else
								{
										alarm_inst.alarm_sts[i].timeout = 0;
										alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;		
								}							
								break;
						}
						case(ALARM_FSM_ACTIVE):
						{
							 //״̬���ص� ALARM_FSM_INACTIVE ״̬
								if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)
								{
										alarm_inst.alarm_sts[i].timeout = 0;
									  alarm_inst.alarm_sts[i].state = ALARM_FSM_POSTACTIVE;		
								}
								else if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{					
										alarm_inst.alarm_sts[i].timeout = 0;
										alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
								}
								else if(acl_trigger_state == ALARM_ACL_CLEARED)
								{	
									//�Զ��������
									if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].reset_mask) == ACL_ENMODE_AUTO_RESET_ALARM)
									{
//										alarm_inst.alarm_sts[i].timeout = g_sys.config.alarm[i].delay;
										alarm_inst.alarm_sts[i].timeout = Alarm_acl_delay(i);
										
										alarm_inst.alarm_sts[i].state = ALARM_FSM_POSTACTIVE;	
									}
									else
									{
										alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
									}
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		;
								}
								else
								{
									;
								}
								break;
						}
						case(ALARM_FSM_POSTACTIVE):
						{
								 //״̬���ص� ALARM_FSM_INACTIVE ״̬
								if((g_sys.config.alarm[i].enable_mode & alarm_inst.alarm_sts[i].enable_mask) == ACL_ENMODE_DISABLE)
								{
											add_alarmlog_fifo(log_id,ALARM_END,alarm_inst.alarm_sts[i].alram_value);												
									     //ɾ��״̬�ڵ�
											node_delete(log_id);
											alarm_status_bitmap_op(alarm_inst.alarm_sts[i].id&0x00ff,0);
												
											alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;
											alarm_inst.alarm_sts[i].timeout = 0;
								}
								else if(acl_trigger_state == ALARM_ACL_CLEARED)//yxq
								{
										
										if(alarm_inst.alarm_sts[i].timeout > 0)
										{
												alarm_inst.alarm_sts[i].timeout --;
												alarm_inst.alarm_sts[i].state = ALARM_FSM_POSTACTIVE;
										}
										else
										{
												//yxq
												add_alarmlog_fifo(log_id,ALARM_END,alarm_inst.alarm_sts[i].alram_value);
//												
//												//ɾ��״̬�ڵ�
												alarm_status_bitmap_op(i,0);
												node_delete(log_id);
											
												
												alarm_inst.alarm_sts[i].state = ALARM_FSM_INACTIVE;
										}
												
								}
								else if(acl_trigger_state == ALARM_ACL_TRIGGERED)
								{
										alarm_inst.alarm_sts[i].timeout = 0;
										alarm_inst.alarm_sts[i].state = ALARM_FSM_ACTIVE;
								}
								else if(acl_trigger_state == ALARM_ACL_HOLD)
								{
								 		;
								}
								else
								{
									;
								}
								break;
						}
						default://yxq
							{
								alarm_inst.alarm_sts[i].state=ALARM_FSM_INACTIVE;
								
								break;
							}
				}
		}
	
								
//		alarm_arbiration();
		
		//LED�ƴ�

}
//��ȡ����λ
uint8_t get_alarm_bitmap(uint8_t alarm_id)
{
	uint8_t byte_offset,bit_offset;

	byte_offset = alarm_id>>4;
	bit_offset = alarm_id&0x0f;
	
	if((g_sys.status.alarm_bitmap[byte_offset] >>bit_offset) & (0x0001))
	{
		return(1);
	}
	else
	{
		return(0);
	}
	
}

//�澯����
  void alarm_arbiration(void)
{
	uint8_t compress1_alarm=0, compress2_alarm=0,close_dev=0;
	uint8_t index;
	uint8_t fan_default_cnt = 0;
	
	
//��ѹ������ر���

	//Alair,20170103
  if((alarm_inst.compress_high_flag[0])||(get_alarm_bitmap(ACL_HI_PRESS1))||(get_alarm_bitmap(ACL_HI_LOCK1))||
	(get_alarm_bitmap(ACL_LO_PRESS1))||(get_alarm_bitmap(ACL_LO_LOCK1))||
	(get_alarm_bitmap(ACL_EXTMP1))||(get_alarm_bitmap(ACL_EXTMP_LOCK1))||
//	(get_alarm_bitmap(ACL_SHORT_TERM1))||(get_alarm_bitmap(ACL_INV_FAULT))||
	(get_alarm_bitmap(ACL_SHORT_TERM1))||(get_alarm_bitmap(ACL_POWER_ERROR))||
  ((g_sys.config.general.cool_type == COOL_TYPE_MODULE_WIND)&&(get_alarm_bitmap(ACL_FAN_OVERLOAD4)))||
  ((get_alarm_bitmap(ACL_EX_FAN_CURRENT)))||	//��������
	(get_alarm_bitmap(ACL_INV_HIPRESS)))//��ѹ
	{
		compress1_alarm=1;
	}
	else
	{
		compress1_alarm=0;
	}

//ѹ����2
  if((alarm_inst.compress_high_flag[1])||(get_alarm_bitmap(ACL_HI_PRESS2))||(get_alarm_bitmap(ACL_HI_LOCK2))||
	(get_alarm_bitmap(ACL_LO_PRESS2))||(get_alarm_bitmap(ACL_LO_LOCK2))||
	(get_alarm_bitmap(ACL_EXTMP2))||(get_alarm_bitmap(ACL_EXTMP_LOCK2))||
	(get_alarm_bitmap(ACL_SHORT_TERM2))||(get_alarm_bitmap(ACL_POWER_ERROR))||
  ((g_sys.config.general.cool_type == COOL_TYPE_MODULE_WIND)&&(get_alarm_bitmap(ACL_FAN_OVERLOAD5)))||
  ((get_alarm_bitmap(ACL_EX_FAN2_CURRENT))))
	{
		compress2_alarm=1;
	}
	else
	{
		compress2_alarm=0;
	}

	
//�ر��豸

			////��Դ�౨��

			//			ACL_POWER_LOSS					,//ABC???????????
			//			ACL_POWER_FD						,	//freqency deviation
			//			ACL_POWER_A_HIGH				,
			//			ACL_POWER_B_HIGH				,
			//			ACL_POWER_C_HIGH				,
			//			ACL_POWER_A_LOW					,
			//			ACL_POWER_B_LOW					,
			//			ACL_POWER_C_LOW					,
			//			ACL_POWER_A_OP					,	//open phase
			//			ACL_POWER_B_OP					,	//open phase
			//			ACL_POWER_C_OP					,	//open phase

			//������ʧ ��������ʵ��
			//				ACL_AIR_LOSS				   ,

				// Զ�̹ػ� ����
			//				ACL_REMOTE_SHUT 			 ,
				//�ذ��ˮ
			//				ACL_WATER_OVERFLOW		 ,	
			//ACL_FAN_OVERLOAD1
			//ACL_FAN_OVERLOAD2
			//ACL_FAN_OVERLOAD3
			//����澯
			
		
		switch(g_sys.config.general.cool_type)
		{
				case COOL_TYPE_MODULE_WIND:
						if((get_alarm_bitmap(ACL_POWER_HI_FD))||(get_alarm_bitmap(ACL_POWER_LO_FD))||(get_alarm_bitmap(ACL_POWER_EP))||
						(get_alarm_bitmap(ACL_POWER_A_HIGH))||(get_alarm_bitmap(ACL_POWER_B_HIGH))||(get_alarm_bitmap(ACL_POWER_C_HIGH))||
						(get_alarm_bitmap(ACL_POWER_A_LOW))||(get_alarm_bitmap(ACL_POWER_B_LOW))||(get_alarm_bitmap(ACL_POWER_C_LOW))||
						(get_alarm_bitmap(ACL_POWER_A_OP))||(get_alarm_bitmap(ACL_POWER_B_OP))||(get_alarm_bitmap(ACL_POWER_C_OP))||
						(get_alarm_bitmap(ACL_AIR_LOSS))||(get_alarm_bitmap(ACL_REMOTE_SHUT))||((get_alarm_bitmap(ACL_WATER_OVERFLOW) == 1)&&(g_sys.config.alarm[ACL_WATER_OVERFLOW].alarm_param == 0))||
						(get_alarm_bitmap(ACL_FAN_OVERLOAD1))||(get_alarm_bitmap(ACL_FAN_OVERLOAD2))||(get_alarm_bitmap(ACL_FAN_OVERLOAD3))||
						/*(get_alarm_bitmap(ACL_FAN_OVERLOAD4))||(get_alarm_bitmap(ACL_FAN_OVERLOAD5))||*/(get_alarm_bitmap(ACL_SMOKE_ALARM))||(get_alarm_bitmap(ACL_USR_ALARM1)))
						{
							close_dev=1;	
						}
						else
						{
							close_dev=0;
						}				
				break;
				case COOL_TYPE_MODULE_WATER:
						if((get_alarm_bitmap(ACL_POWER_HI_FD))||(get_alarm_bitmap(ACL_POWER_LO_FD))||(get_alarm_bitmap(ACL_POWER_EP))||
							(get_alarm_bitmap(ACL_POWER_A_HIGH))||(get_alarm_bitmap(ACL_POWER_B_HIGH))||(get_alarm_bitmap(ACL_POWER_C_HIGH))||
							(get_alarm_bitmap(ACL_POWER_A_LOW))||(get_alarm_bitmap(ACL_POWER_B_LOW))||(get_alarm_bitmap(ACL_POWER_C_LOW))||
							(get_alarm_bitmap(ACL_POWER_A_OP))||(get_alarm_bitmap(ACL_POWER_B_OP))||(get_alarm_bitmap(ACL_POWER_C_OP))||
							(get_alarm_bitmap(ACL_AIR_LOSS))||(get_alarm_bitmap(ACL_REMOTE_SHUT))||((get_alarm_bitmap(ACL_WATER_OVERFLOW) == 1)&&(g_sys.config.alarm[ACL_WATER_OVERFLOW].alarm_param == 0))||
							(get_alarm_bitmap(ACL_FAN_OVERLOAD1))||(get_alarm_bitmap(ACL_FAN_OVERLOAD2))||(get_alarm_bitmap(ACL_FAN_OVERLOAD3))||
							/*(get_alarm_bitmap(ACL_FAN_OVERLOAD4))||(get_alarm_bitmap(ACL_FAN_OVERLOAD5))||*/(get_alarm_bitmap(ACL_SMOKE_ALARM))||(get_alarm_bitmap(ACL_USR_ALARM1)))
							{
								close_dev=1;	
							}
							else
							{
								close_dev=0;
							}		
				break;	
				case COOL_TYPE_MODULE_MIX:
						if((get_alarm_bitmap(ACL_POWER_HI_FD))||(get_alarm_bitmap(ACL_POWER_LO_FD))||(get_alarm_bitmap(ACL_POWER_EP))||
						(get_alarm_bitmap(ACL_POWER_A_HIGH))||(get_alarm_bitmap(ACL_POWER_B_HIGH))||(get_alarm_bitmap(ACL_POWER_C_HIGH))||
						(get_alarm_bitmap(ACL_POWER_A_LOW))||(get_alarm_bitmap(ACL_POWER_B_LOW))||(get_alarm_bitmap(ACL_POWER_C_LOW))||
						(get_alarm_bitmap(ACL_POWER_A_OP))||(get_alarm_bitmap(ACL_POWER_B_OP))||(get_alarm_bitmap(ACL_POWER_C_OP))||
						(get_alarm_bitmap(ACL_AIR_LOSS))||(get_alarm_bitmap(ACL_REMOTE_SHUT))||((get_alarm_bitmap(ACL_WATER_OVERFLOW) == 1)&&(g_sys.config.alarm[ACL_WATER_OVERFLOW].alarm_param == 0))||
						(get_alarm_bitmap(ACL_FAN_OVERLOAD1))||(get_alarm_bitmap(ACL_FAN_OVERLOAD2))||(get_alarm_bitmap(ACL_FAN_OVERLOAD3))||
						/*(get_alarm_bitmap(ACL_FAN_OVERLOAD4))||(get_alarm_bitmap(ACL_FAN_OVERLOAD5))||*/(get_alarm_bitmap(ACL_SMOKE_ALARM))||(get_alarm_bitmap(ACL_USR_ALARM1)))
						{
							close_dev=1;	
						}
						else
						{
							close_dev=0;
						}	
				break;		
				case COOL_TYPE_COLUMN_WIND:
				case COOL_TYPE_COLUMN_WATER:					
						fan_default_cnt = get_alarm_bitmap(ACL_FAN_OVERLOAD1);
						fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD2);
						fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD3);
						fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD4);
						fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD5);
						fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD6);
						fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD7);
						fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD8);
		//				if((get_alarm_bitmap(ACL_POWER_HI_FD))||(get_alarm_bitmap(ACL_POWER_LO_FD))||(get_alarm_bitmap(ACL_POWER_EP))||
		//				(get_alarm_bitmap(ACL_POWER_A_HIGH))||(get_alarm_bitmap(ACL_POWER_B_HIGH))||(get_alarm_bitmap(ACL_POWER_C_HIGH))||
		//				(get_alarm_bitmap(ACL_POWER_A_LOW))||(get_alarm_bitmap(ACL_POWER_B_LOW))||(get_alarm_bitmap(ACL_POWER_C_LOW))||
		//				(get_alarm_bitmap(ACL_POWER_A_OP))||(get_alarm_bitmap(ACL_POWER_B_OP))||(get_alarm_bitmap(ACL_POWER_C_OP))||
		//				(get_alarm_bitmap(ACL_AIR_LOSS))||(get_alarm_bitmap(ACL_REMOTE_SHUT))||((get_alarm_bitmap(ACL_WATER_OVERFLOW) == 1)&&(g_sys.config.alarm[ACL_WATER_OVERFLOW].alarm_param == 0))||
		//				(get_alarm_bitmap(ACL_SMOKE_ALARM))||(fan_default_cnt >=g_sys.config.fan.num)||(fan_default_cnt >1)||(get_alarm_bitmap(ACL_USR_ALARM1)))
						//Alair,20170227
						if((get_alarm_bitmap(ACL_POWER_HI_FD))||(get_alarm_bitmap(ACL_POWER_LO_FD))||(get_alarm_bitmap(ACL_POWER_EP))||
						(get_alarm_bitmap(ACL_POWER_A_HIGH))||(get_alarm_bitmap(ACL_POWER_B_HIGH))||(get_alarm_bitmap(ACL_POWER_C_HIGH))||
						(get_alarm_bitmap(ACL_POWER_A_LOW))||(get_alarm_bitmap(ACL_POWER_B_LOW))||(get_alarm_bitmap(ACL_POWER_C_LOW))||
						(get_alarm_bitmap(ACL_POWER_A_OP))||(get_alarm_bitmap(ACL_POWER_B_OP))||(get_alarm_bitmap(ACL_POWER_C_OP))||
						(get_alarm_bitmap(ACL_AIR_LOSS))||(get_alarm_bitmap(ACL_REMOTE_SHUT))||((get_alarm_bitmap(ACL_WATER_OVERFLOW) == 1)&&(g_sys.config.alarm[ACL_WATER_OVERFLOW].alarm_param == 0))||
						(get_alarm_bitmap(ACL_SMOKE_ALARM))||((fan_default_cnt >=g_sys.config.alarm[ACL_FAN_OVERLOAD1].alarm_param))||
						(get_alarm_bitmap(ACL_USR_ALARM1)))				
						{
							close_dev=1;	
						}
						else
						{
							close_dev=0;
						}
				break;	
				default:
				break;						
		}
		
		//ѹ����1�Ŀ���
		if((close_dev)||(compress1_alarm))
		{
			l_sys.Comp_State=0x5A;
			alarm_bitmap_op(DO_COMP1_BPOS,0);
			alarm_bitmap_mask_op(DO_COMP1_BPOS,1);
		}
		else
		{
			l_sys.Comp_State=0x00;
			alarm_bitmap_op(DO_COMP1_BPOS,1);
			alarm_bitmap_mask_op(DO_COMP1_BPOS,0);
		}

		//ѹ����2�Ŀ���
		if((close_dev)||(compress2_alarm))
		{
			alarm_bitmap_op(DO_COMP2_BPOS,0);
			alarm_bitmap_mask_op(DO_COMP2_BPOS,1);
		}
		else
		{
			alarm_bitmap_op(DO_COMP2_BPOS,1);
			alarm_bitmap_mask_op(DO_COMP2_BPOS,0);
		}
		//�������
		l_sys.u8AlarmClose_dev=close_dev;		
		if((close_dev))
		{
			alarm_bitmap_op(DO_FAN_BPOS,0);
			alarm_bitmap_mask_op(DO_FAN_BPOS,1);
		}
		else
		{
			alarm_bitmap_op(DO_FAN_BPOS,1);
			alarm_bitmap_mask_op(DO_FAN_BPOS,0);
		}
		
		{
			//עˮ������ ��ˮλ�澯�ر�עˮ��
			if((close_dev)||(get_alarm_bitmap(ACL_HUM_HI_LEVEL))||(get_alarm_bitmap(ACL_HUM_LO_LEVEL)))						
			{
				alarm_bitmap_op(DO_FILL_BPOS,0);
				alarm_bitmap_mask_op(DO_FILL_BPOS,1);
			}
			else
			{
				alarm_bitmap_op(DO_FILL_BPOS,1);
				alarm_bitmap_mask_op(DO_FILL_BPOS,0);
			}
				
			//��ʪ�����ƣ���ˮλ�澯�رռ�ʪ��,��ʪ����������,��ʪ������
			
			if((close_dev)||(get_alarm_bitmap(ACL_HUM_LO_LEVEL))||(get_alarm_bitmap(ACL_HUM_OCURRENT))||
				(get_alarm_bitmap(ACL_HUM_DEFAULT))||(get_alarm_bitmap(ACL_HUM_OD)))
			{
					alarm_bitmap_op(DO_HUM_BPOS,0);
					alarm_bitmap_mask_op(DO_HUM_BPOS,1);
					alarm_bitmap_op(DO_FILL_BPOS,0);
					alarm_bitmap_mask_op(DO_FILL_BPOS,1);
					alarm_bitmap_op(DO_DRAIN_BPOS,0);
					alarm_bitmap_mask_op(DO_DRAIN_BPOS,1);
			}
			else
			{
					alarm_bitmap_op(DO_HUM_BPOS,1);
					alarm_bitmap_mask_op(DO_HUM_BPOS,0);
					alarm_bitmap_op(DO_FILL_BPOS,1);
					alarm_bitmap_mask_op(DO_FILL_BPOS,0);
					alarm_bitmap_op(DO_DRAIN_BPOS,1);
					alarm_bitmap_mask_op(DO_DRAIN_BPOS,0);
			}			
		}

		//�������� ����������
		if((close_dev)||(get_alarm_bitmap(ACL_HEATER_OD)))
		{
			
			alarm_bitmap_op(DO_RH1_BPOS,0);
			alarm_bitmap_mask_op(DO_RH1_BPOS,1);

//			alarm_bitmap_op(DO_RH2_BPOS,0);
//			alarm_bitmap_mask_op(DO_RH2_BPOS,1);

		}
		else
		{
			alarm_bitmap_op(DO_RH1_BPOS,1);
			alarm_bitmap_mask_op(DO_RH1_BPOS,0);

//			alarm_bitmap_op(DO_RH2_BPOS,1);
//			alarm_bitmap_mask_op(DO_RH2_BPOS,0);

		}
		
		// ״̬����
				//Ⱥ��ʧ��
		if(get_alarm_bitmap(ACL_GROUP_CONTROL_FAIL))
		{
				sys_set_remap_status(WORK_MODE_STS_REG_NO,TEAM_STS_BPOS, 1);
		}
		else
		{
				sys_set_remap_status(WORK_MODE_STS_REG_NO,TEAM_STS_BPOS, 0);
		}
    //phase protect
//		if( get_alarm_bitmap(ACL_POWER_EP))
//		{
//				req_bitmap_op(DO_PHASE_P_BPOS,0);
//				req_bitmap_op(DO_PHASE_N_BPOS,1);
//		}
//		else
//		{
//				req_bitmap_op(DO_PHASE_P_BPOS,1);
//				req_bitmap_op(DO_PHASE_N_BPOS,0);
//		}
		 //�رչ����澯
		alarm_bitmap_op(DO_ALARM_BPOS,0);
		alarm_bitmap_mask_op(DO_ALARM_BPOS,1);
			//���� ������������
		for(index=0;index<ACL_TOTAL_NUM;index++)
		{
      
				if((g_sys.config.alarm[index].enable_mode & alarm_inst.alarm_sts[index].enable_mask) == ACL_ENMODE_ENABLE)
				{
					if(get_alarm_bitmap(index))//��������	
					{
						//������������
						
						alarm_bitmap_op(DO_ALARM_BPOS,1);
						alarm_bitmap_mask_op(DO_ALARM_BPOS,1);
						break;
					}
					
				}
		}
		
	
}

//����ʱ����� 
//����ֵʱ�䵥λ����
  uint16_t dev_runingtime(uint16_t low,uint16_t high)
{
		uint16_t runing_day;
		uint32_t run_time;
		
		run_time = high;
		run_time = (run_time<<16) + low;
		run_time = run_time >>12;
		runing_day = run_time/24;
	
		return(runing_day);
}



//
uint8_t get_alarm_bitmap_op(uint8_t component_bpos)
{
	
		if((l_sys.bitmap[BITMAP_ALARM]>>component_bpos)&0x01)
		{
				return(1);
		}
		else
		{
				return(0);
		}
			
}

uint8_t get_alarm_bitmap_mask(uint8_t component_bpos)
{
	
		if((l_sys.bitmap[BITMAP_MASK] >>component_bpos)&0x01)
		{
				return(1);
		}
		else
		{
				return(0);
		}
			
}











//DO_FAN_BPOS//offset
//����λ����

static void alarm_bitmap_op(uint8_t component_bpos, uint8_t action)
{
		extern local_reg_st l_sys;
		if(action == 0)
		{
				l_sys.bitmap[BITMAP_ALARM] &= ~(0x0001<<component_bpos);
		}
		else
		{
				l_sys.bitmap[BITMAP_ALARM] |= (0x0001<<component_bpos);
		}
}

static void alarm_bitmap_mask_op(uint8_t component_bpos, uint8_t action)
{
		extern local_reg_st l_sys;
		if(action == 0)
		{
				l_sys.bitmap[BITMAP_MASK] &= ~(0x0001<<component_bpos);
		}
		else
		{
				l_sys.bitmap[BITMAP_MASK] |= (0x0001<<component_bpos);
		}
}

//�澯״̬λ
static void   alarm_status_bitmap_op (uint8_t alarm_id,uint8_t option)
{
		uint8_t byte_offset,bit_offset;
		
		byte_offset = alarm_id >> 4;
		bit_offset = alarm_id &0x0f;
		if(option == 1)
		{
				g_sys.status.alarm_bitmap[byte_offset] |= (0x0001<<bit_offset);
		}
		else
		{
				g_sys.status.alarm_bitmap[byte_offset] &= ~(0x0001<<bit_offset);
		}
		
}

/**
  * @brief  alarm 0 rule check function
	* @param  str_ptr: alarm_acl_st* structure pointer 
  * @retval none
  */



  uint16_t io_calc(uint8_t data,uint8_t refer)
{
	
	if(data==refer)
	{
			
		return 1;
	}
	else
	{
		
		return 0;
	}
}

/**
  * @brief  alarm 1 rule check function
	* @param  str_ptr: alarm_acl_st* structure pointer 
  * @retval none
  */

//ģ������������

//const compare_st  compare_table[]=
//{  //���ݲ���ֵ ���������޵ͣ� ���������޸ߣ��жϷ���
//		{&g_sys.config.algorithm.supply_air_temp,NULL,&g_sys.config.alarm[ACL_HI_TEMP_RETURN].alarm_param,COMPARE_MAX}//supply_air_temp alarm
//		
//};

// ģ����������⺯��
  uint16_t compare_calc(int16_t meter,int16_t min,int16_t max,Compare_type_st type)
{
	
		if(type==THR_MAX_TYPE)//����������޴���
		{
			if(meter > max)
			{
				return(1);
			}
			else
			{
				return(0);
			}
		}
		else if(type==SML_MIN_TYPE)//С����С���޴���
		{
			if(meter<min)
			{
				return(1);
			}
			else
			{
				return(0);
			}
		}
		else if(type==IN_MIN_MAX_TYPE)//�������ڱ���
		{
			if((meter > min)&&(meter < max))
			{
				return(1);
			}
			else
			{
				return(0);
			}
			
		}
		else//���������ⱨ��
		{
			if((meter<min)||(meter>max))
			{
				return(1);
			}
			else
			{
				
				return(0);
			}
			
		}
	
}

/**
  * @brief  alarm 2 rule check function
	* @param  str_ptr: alarm_acl_st* structure pointer 
  * @retval none
  */
  //�����౨��


  uint16_t alarm_lock(uint16_t alarm_id)
{
	uint8_t index=0xff;
	
	switch(alarm_id)
	{
		case ACL_HI_LOCK1://��ѹ1����
			index = 0;
		break;
		case  ACL_HI_LOCK2://��ѹ2����
			index = 1;
		break;
		case ACL_LO_LOCK1://��ѹ1����
			index = 2;
		break;
		case ACL_LO_LOCK2://��ѹ2����
			index=3;
		break;
		case ACL_EXTMP_LOCK1://������1�¶ȱ���
			index = 4;
		break;
		case ACL_EXTMP_LOCK2://������2�¶ȱ���
			index = 5;
		break;
		default:
			index = 0xff;
		break;
			
	}
	
	if(index != 0xff)
	{
			//�����������
			if(get_alarm_remove_bitmap(alarm_id) == 1)
			{
					alarm_inst.alarm_lock[index].lock_time[0] = 0xffffffff;
					alarm_inst.alarm_lock[index].lock_time[1] = 0xffffffff;
					alarm_inst.alarm_lock[index].lock_time[2] = 0xffffffff;
					alarm_inst.alarm_lock[index].lock_flag =0;
					
				//	clear_alarm_remove_bitmap(alarm_id);
			}
			
			if(alarm_inst.alarm_lock[index].lock_flag)
			{
					return(ALARM_ACL_TRIGGERED);
			}
			if(alarm_inst.alarm_lock[index].lock_time[0] != 0xffffffff)
			{
					if((alarm_inst.alarm_lock[index].lock_time[2] - alarm_inst.alarm_lock[index].lock_time[0]) <= 3600)
					{
							alarm_inst.alarm_lock[index].lock_flag = 1;
							return(ALARM_ACL_TRIGGERED);
					}	
			}
   }
	return(ALARM_ACL_CLEARED);
}




   uint8_t acl_clear(alarm_acl_status_st* acl_ptr)
{
		if((get_alarm_remove_bitmap(acl_ptr->id) == 1)&&(acl_ptr->state > ALARM_FSM_PREACTIVE))
		{
				acl_ptr->state = ALARM_FSM_POSTACTIVE;
				acl_ptr->timeout = 0;
				clear_alarm_remove_bitmap(acl_ptr->id);
				return(1);
		}
		clear_alarm_remove_bitmap(acl_ptr->id);
		return(0);
}
  uint16_t acl_get_pwr_sts(void)
{
			return(1);
}
//�ط磬�ͷ���ʪ�ȱ���



//ACL_HI_TEMP_RETURN

static  uint16_t acl00(alarm_acl_status_st* acl_ptr)
{
//	extern team_local_st team_local_inst;
//	int16_t max;
//	int16_t meter;
//	
//	//����ȷ��
//	// ��������ʱ
//	// ��� ����
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(acl_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
////	if(alarm_inst.power_on_timer < g_sys.config.alarm[ACL_HI_TEMP_RETURN].alarm_param2)
////	{
////			acl_ptr->state = ALARM_FSM_INACTIVE;
////			return(ALARM_ACL_HOLD);
////	}

//	//modbus_dev
//	if(((g_sys.config.dev_mask.mb_comp&(0X01<<MBM_DEV_A1_ADDR))==0)&&
//    ((g_sys.config.dev_mask.ain&((0x01<<AI_RETURN_NTC1)))==0))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	meter = g_sys.status.sys_tem_hum.return_air_temp;
//	if(meter == 0x7fff)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	
//	if(((team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STATUS]&(0x0001<<TEAM_STS_CONFD_BPOS))==0)||
//		(g_sys.config.team.team_en == 0)||(g_sys.config.team.addr >(team_local_inst.team_config[TEAM_CONF_CNT]&0x00ff)))
//	{
//		max = g_sys.config.alarm[ACL_HI_TEMP_RETURN].alarm_param;
//	}
//	else
//	{
//		max = team_local_inst.team_config[TEAM_CONF_ALARM_HI_TEMP_PARAM] ;
//	}
//	
//	alarm_inst.alarm_sts[acl_ptr->id].alram_value=meter;
//	
//	return(compare_calc( meter,0,max,THR_MAX_TYPE));
	return(ALARM_ACL_CLEARED);
}


//ACL_LO_TEMP_RETURN
static  uint16_t acl01(alarm_acl_status_st* acl_ptr)
{
//	extern team_local_st team_local_inst;
//	int16_t meter;
//	int16_t min;
//	//����ȷ��
//		// ��� ����
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	// ��������ʱ
//	
//	if(acl_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
////	if(alarm_inst.power_on_timer < g_sys.config.alarm[ACL_LO_TEMP_RETURN].alarm_param2)
////	{
////			acl_ptr->state = ALARM_FSM_INACTIVE;
////			return(ALARM_ACL_HOLD);
////	}
//	//modbus_dev
//	if(((g_sys.config.dev_mask.mb_comp&(0X01<<MBM_DEV_A1_ADDR))==0)&&
//    ((g_sys.config.dev_mask.ain&((0x01<<AI_RETURN_NTC1)))==0))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	meter = g_sys.status.sys_tem_hum.return_air_temp;
//	if(meter == ABNORMAL_VALUE)
//	{
//			return(ALARM_ACL_CLEARED);
//	}	
//	if(((team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STATUS]&(0x0001<<TEAM_STS_CONFD_BPOS))==0)||
//	(g_sys.config.team.team_en == 0)||(g_sys.config.team.addr >(team_local_inst.team_config[TEAM_CONF_CNT]&0x00ff)))
//	{
//			min = g_sys.config.alarm[ACL_LO_TEMP_RETURN].alarm_param;
//	}
//	else
//	{
//		 min = team_local_inst.team_config[TEAM_CONF_ALARM_LO_TEMP_PARAM] ;
//	}

//	
//	alarm_inst.alarm_sts[acl_ptr->id].alram_value=meter;

//	return(compare_calc( meter,min,0,SML_MIN_TYPE));
			return(ALARM_ACL_CLEARED);		
}

//ACL_HI_HUM_RETURN
static  uint16_t acl02(alarm_acl_status_st*acl_ptr)
{
//	extern team_local_st team_local_inst;
//	uint16_t meter;
//	uint16_t max;
//	//����ȷ��
//	// ��� ����
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	// ��������ʱ
//	if(acl_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
////	if(alarm_inst.power_on_timer < g_sys.config.alarm[ACL_HI_HUM_RETURN].alarm_param2)
////	{
////			acl_ptr->state = ALARM_FSM_INACTIVE;
////			return(ALARM_ACL_HOLD);
////	}
//  //modbus
//	if(((g_sys.config.dev_mask.mb_comp&(0X01<<MBM_DEV_A1_ADDR))==0))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	meter = g_sys.status.sys_tem_hum.return_air_hum;
//	if(meter == 0x7fff)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(((team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STATUS]&(0x0001<<TEAM_STS_CONFD_BPOS))==0)||
//	(g_sys.config.team.team_en == 0)||(g_sys.config.team.addr >(team_local_inst.team_config[TEAM_CONF_CNT]&0x00ff)))
//	{
//			max = g_sys.config.alarm[ACL_HI_HUM_RETURN].alarm_param;;
//	}
//	else
//	{
//			max = team_local_inst.team_config[TEAM_CONF_ALARM_HI_HUM_PARAM] ;
//	}
//	
//	alarm_inst.alarm_sts[acl_ptr->id].alram_value=meter;
//	return(compare_calc( meter,0,max,THR_MAX_TYPE));
			return(ALARM_ACL_CLEARED);		
}

//ACL_LO_HUM_RETURN

static  uint16_t acl03(alarm_acl_status_st*acl_ptr)
{
//	extern team_local_st team_local_inst;
//	uint16_t meter;
//	uint16_t min;
//	//����ȷ��
//	// ��� ����
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	// ��������ʱ
//	if(acl_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	
////	if(alarm_inst.power_on_timer < g_sys.config.alarm[ACL_LO_HUM_RETURN].alarm_param2)
////	{
////		acl_ptr->state = ALARM_FSM_INACTIVE;
////		return(ALARM_ACL_HOLD);
////	}
//	if(((g_sys.config.dev_mask.mb_comp&(0X01<<MBM_DEV_A1_ADDR))==0))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	meter = g_sys.status.sys_tem_hum.return_air_hum;	
//	if(meter == ABNORMAL_VALUE)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(((team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STATUS]&(0x0001<<TEAM_STS_CONFD_BPOS))==0)||
//	(g_sys.config.team.team_en == 0)||(g_sys.config.team.addr >(team_local_inst.team_config[TEAM_CONF_CNT]&0x00ff)))
//	{
//			min = g_sys.config.alarm[ACL_LO_HUM_RETURN].alarm_param;
//	}
//	else
//	{
//			min = team_local_inst.team_config[TEAM_CONF_ALARM_LO_HUM_PARAM] ;
//	}
//	
//	alarm_inst.alarm_sts[acl_ptr->id].alram_value=meter;
//	return(compare_calc( meter,min,0,SML_MIN_TYPE));
			return(ALARM_ACL_CLEARED);		
}

//ACL_HI_TEMP_SUPPLY

static  uint16_t acl04(alarm_acl_status_st* acl_ptr)
{
//	extern team_local_st team_local_inst;
//	int16_t max;
//	int16_t meter;
//	//����ȷ��
//	// ��� ����
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	// ��������ʱ
//	if(acl_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
////	if(alarm_inst.power_on_timer < g_sys.config.alarm[ACL_HI_TEMP_SUPPLY].alarm_param2)
////	{
////	
////		acl_ptr->state = ALARM_FSM_INACTIVE;
////		return(ALARM_ACL_HOLD);
////	}
//	if(((g_sys.config.dev_mask.mb_comp&(0x01<<MBM_DEV_A2_ADDR))==0)&&
//    ((g_sys.config.dev_mask.ain&((0x01<<AI_SUPPLY_NTC1)))==0))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	meter = g_sys.status.sys_tem_hum.supply_air_temp;
//	if(meter == ABNORMAL_VALUE)
//	{
//			return(ALARM_ACL_CLEARED);
//	}		
//	if(((team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STATUS]&(0x0001<<TEAM_STS_CONFD_BPOS))==0)||
//		(g_sys.config.team.team_en == 0)||(g_sys.config.team.addr >(team_local_inst.team_config[TEAM_CONF_CNT]&0x00ff)))
//	{
//		max = g_sys.config.alarm[ACL_HI_TEMP_SUPPLY].alarm_param;
//	}
//	else
//	{
//		max = team_local_inst.team_config[TEAM_CONF_ALARM_HI_TEMP_PARAM] ;
//	}	
//	alarm_inst.alarm_sts[acl_ptr->id].alram_value=meter;
//	return(compare_calc( meter,0,max,THR_MAX_TYPE));
			return(ALARM_ACL_CLEARED);		
}

//ACL_LO_TEMP_SUPPLY
static  uint16_t acl05(alarm_acl_status_st* acl_ptr)
{
//	extern team_local_st team_local_inst;
//	int16_t meter;
//	int16_t min;
//	//����ȷ��
//	// ��������ʱ
//	if(acl_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
////	if(alarm_inst.power_on_timer < g_sys.config.alarm[ACL_LO_TEMP_SUPPLY].alarm_param2)
////	{
////		acl_ptr->state = ALARM_FSM_INACTIVE;
////		return(ALARM_ACL_HOLD);
////	}
//	// ��� ����
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}	
//	//dev
//	if(((g_sys.config.dev_mask.mb_comp&(0x01<<MBM_DEV_A2_ADDR))==0)&&
//    ((g_sys.config.dev_mask.ain&((0x01<<AI_SUPPLY_NTC1)))==0))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	meter = g_sys.status.sys_tem_hum.supply_air_temp;
//	if(meter == ABNORMAL_VALUE)
//	{
//			return(ALARM_ACL_CLEARED);
//	}		
//	if(((team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STATUS]&(0x0001<<TEAM_STS_CONFD_BPOS))==0)||
//	(g_sys.config.team.team_en == 0)||(g_sys.config.team.addr >(team_local_inst.team_config[TEAM_CONF_CNT]&0x00ff)))
//	{
//			min = g_sys.config.alarm[ACL_LO_TEMP_SUPPLY].alarm_param;
//	}
//	else
//	{
//		 min = team_local_inst.team_config[TEAM_CONF_ALARM_LO_TEMP_PARAM] ;
//	}
//	
//	alarm_inst.alarm_sts[acl_ptr->id].alram_value=meter;
//	return(compare_calc( meter,min,0,SML_MIN_TYPE));
			return(ALARM_ACL_CLEARED);		
}



//ACL_HI_HUM_SUPPLY
static  uint16_t acl06(alarm_acl_status_st* acl_ptr)
{
//	extern team_local_st team_local_inst;
//	uint16_t meter;
//	uint16_t max;
//	//����ȷ��
//	// ��� ����
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(acl_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
////	if(alarm_inst.power_on_timer < g_sys.config.alarm[ACL_HI_HUM_SUPPLY].alarm_param2)
////	{
////		acl_ptr->state = ALARM_FSM_INACTIVE;
////		return(ALARM_ACL_HOLD);
////	}
//	//de
//  if(((g_sys.config.dev_mask.mb_comp&(0x01<<MBM_DEV_A2_ADDR))==0))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	meter = g_sys.status.sys_tem_hum.supply_air_hum;
//	if(meter == ABNORMAL_VALUE)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(((team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STATUS]&(0x0001<<TEAM_STS_CONFD_BPOS))==0)||
//	(g_sys.config.team.team_en == 0)||(g_sys.config.team.addr >(team_local_inst.team_config[TEAM_CONF_CNT]&0x00ff)))
//	{
//			max = g_sys.config.alarm[ACL_HI_HUM_SUPPLY].alarm_param;
//	}
//	else
//	{
//			max = team_local_inst.team_config[TEAM_CONF_ALARM_HI_HUM_PARAM] ;
//	}
//	
//	alarm_inst.alarm_sts[acl_ptr->id].alram_value=meter;
//	return(compare_calc( meter,0,max,THR_MAX_TYPE));
			return(ALARM_ACL_CLEARED);		
}
////ACL_LO_HUM_SUPPLY
static  uint16_t acl07(alarm_acl_status_st*acl_ptr)
{
//	extern team_local_st team_local_inst;
//	uint16_t meter;
//	uint16_t min;
//	
//	// ��� ����
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(acl_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
////	if(alarm_inst.power_on_timer < g_sys.config.alarm[ACL_LO_HUM_SUPPLY].alarm_param2)
////	{
////		acl_ptr->state = ALARM_FSM_INACTIVE;
////		return(ALARM_ACL_HOLD);
////	}
//	if(((g_sys.config.dev_mask.mb_comp&(0x01<<MBM_DEV_A2_ADDR))==0))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	meter = g_sys.status.sys_tem_hum.supply_air_hum;	
//	if(meter == ABNORMAL_VALUE)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(((team_local_inst.team_table[g_sys.config.team.addr-1][TEAM_TAB_STATUS]&(0x0001<<TEAM_STS_CONFD_BPOS))==0)||
//	(g_sys.config.team.team_en == 0)||(g_sys.config.team.addr >(team_local_inst.team_config[TEAM_CONF_CNT]&0x00ff)))
//	{
//			min = g_sys.config.alarm[ACL_LO_HUM_SUPPLY].alarm_param;
//	}
//	else
//	{
//			min = team_local_inst.team_config[TEAM_CONF_ALARM_LO_HUM_PARAM] ;
//	}
//	
//	alarm_inst.alarm_sts[acl_ptr->id].alram_value=meter;
//	return(compare_calc( meter,min,0,SML_MIN_TYPE));
			return(ALARM_ACL_CLEARED);		
}
// ModbusӲ������
static  uint16_t acl08(alarm_acl_status_st*acl_ptr)
{
			return(ALARM_ACL_CLEARED);	
//		uint8_t req,index;
//		
//		req =0;
//		// ��� ����
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		//modbus_dev_default
//		for(index=MBM_DEV_A1_ADDR; index<=POWER_MODULE_BPOS;index++)
//		{
//					if(((g_sys.config.dev_mask.mb_comp) & (0X01<<index))&&(sys_get_mbm_online(index) == 1))
//					{		
//									if(g_sys.status.mbm.tnh[index].dev_sts & MBM_DEV_STS_DEFAULT)
//									{
//												sys_set_remap_status(SENSOR_STS_REG_NO,index,1);
//											
//												req =1;
//									}
//									else
//									{
//												sys_set_remap_status(SENSOR_STS_REG_NO,index,0);;
//									}   	
//									
//					}
//			}
//				
//		
//		acl_ptr->alram_value = g_sys.status.status_remap[SENSOR_STS_REG_NO];
//	  return(req);	

////		uint8_t req;
////		
////		//uint16_t HUM_erro=0;
////		req = 0;
////		// ��� ����
////		if(acl_clear(acl_ptr))
////		{
////				return(ALARM_ACL_CLEARED);
////		}
//////		if(g_sys.status.status_remap[MBM_COM_STS_REG_NO] != g_sys.config.dev_mask.mb_comp)
//////		{
//////				req = 1;
//////		}
//////		acl_ptr->alram_value = g_sys.status.status_remap[MBM_COM_STS_REG_NO]; 
//////		return(req);		
////		if((g_sys.config.dev_mask.mb_comp&0x02)== 0)
////		{
////				return(ALARM_ACL_CLEARED);
////		}
////		else
////		{
////			if(sys_get_mbm_online(MBM_DEV_A2_ADDR) == 0)
////			{
////				req = 1;				
////			}
////			else
////			{
////				return(ALARM_ACL_CLEARED);				
////			}			
////		}
////		acl_ptr->alram_value = g_sys.status.status_remap[MBM_COM_STS_REG_NO]; 
////		return(req);
}

// Modbusͨ���쳣
static  uint16_t acl09(alarm_acl_status_st* acl_ptr)
{
//		uint8_t req;
//		uint16_t u16mb_comp;
//		
//		//uint16_t HUM_erro=0;
//		req = 0;
//		// ��� ����
//		if(acl_clear(acl_ptr))
//		{
//				g_sys.status.Alarm_COM_NTC_BIT[ALARM_COM]=0;
//				return(ALARM_ACL_CLEARED);
//		}
//		
//		u16mb_comp=g_sys.config.dev_mask.mb_comp&0x7FFF;
//		
//		if(g_sys.status.status_remap[MBM_COM_STS_REG_NO] != u16mb_comp)
//		{
//				g_sys.status.Alarm_COM_NTC_BIT[ALARM_COM]=(g_sys.status.status_remap[MBM_COM_STS_REG_NO])^(u16mb_comp);
//				req = 1;
//		}
//		else
//		{
//				g_sys.status.Alarm_COM_NTC_BIT[ALARM_COM]=0;
//				req = 0;			
//		}
//		acl_ptr->alram_value = g_sys.status.status_remap[MBM_COM_STS_REG_NO]; 
////		rt_kprintf("status_remap[MBM_COM_STS_REG_NO] = %x,mb_comp = %x,req = %x\n",g_sys.status.status_remap[MBM_COM_STS_REG_NO],g_sys.config.dev_mask.mb_comp,req);
//		return(req);		
////		if((g_sys.config.dev_mask.mb_comp&0x01) == 0)
////		{
////				return(ALARM_ACL_CLEARED);
////		}
////		else
////		{
////			if(sys_get_mbm_online(MBM_DEV_A1_ADDR) == 0)
////			{
////				req = 1;				
////			}
////			else
////			{
////				return(ALARM_ACL_CLEARED);				
////			}			
////		}
////		acl_ptr->alram_value = g_sys.status.status_remap[MBM_COM_STS_REG_NO]; 
////		return(req);
			return(ALARM_ACL_CLEARED);	
}

// ACL_NTC_INVALID
static  uint16_t acl10(alarm_acl_status_st* acl_ptr)
{
//		int16_t min;
//		int16_t max;
//		uint8_t req;
//		int16_t meter;
//		uint8_t i;
//		
//		// ��� ����
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		
//		max = ACL_TEM_MAX;
//		min = ACL_TEM_MIN;
//		
//		req =0;
//	//TEM_SENSOR
//		alarm_inst.tem_sensor_fault = 0;	
//		
//		 //NTC
//		for(i=AI_NTC1;i<AI_MAX_CNT;i++)
//		{
//				if((g_sys.config.dev_mask.ain>>i)&0x0001)
//		 {
//				meter =   g_sys.status.ain[i];
//				if(compare_calc( meter,min,max,OUT_MIN_MAX_TYPE))
//				{
//						sys_set_remap_status(SENSOR_STS_REG_NO,i,1);
//				    req =1;
//				}
//				else
//				{
//				    sys_set_remap_status(SENSOR_STS_REG_NO,i,0);
//				}
//			}
//		}	 	 
////		rt_kprintf("g_sys.status.status_remap[SENSOR_STS_REG_NO] = %X\n",g_sys.status.status_remap[SENSOR_STS_REG_NO]);	
//		 acl_ptr->alram_value = g_sys.status.status_remap[SENSOR_STS_REG_NO]; 
//		return(req);
			return(ALARM_ACL_CLEARED);	
}
//ACL_FAN_OVERLOAD1

static	uint16_t acl11(alarm_acl_status_st* acl_ptr)
{
//		uint8_t data;

//	
////			rt_kprintf("alarm_inst.alarm_sts[i].state = %d\n",alarm_inst.alarm_sts[ACL_FAN_OVERLOAD1].state);	
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_HOLD);
//		}
//		if(sys_get_do_sts(DO_FAN_BPOS)==0)
//		{
//				return(ALARM_ACL_HOLD);
//		}			
//		data = sys_get_di_sts(DI_FAN01_OD_BPOS);
////					rt_kprintf("data = %d\n",data);	
//		return(io_calc( data,IO_CLOSE));
			return(ALARM_ACL_CLEARED);	
}
//ACL_FAN_OVERLOAD2

static	uint16_t acl12(alarm_acl_status_st* acl_ptr)
{
////		return(ALARM_ACL_CLEARED);
//	uint8_t data;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(g_sys.config.fan.num < 2)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(sys_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_HOLD);
//	}
//	if(sys_get_do_sts(DO_FAN_BPOS)==0)
//	{
//			return(ALARM_ACL_HOLD);
//	}	
//	if(g_sys.config.fan.num>=2)//DI����
//	{
//		data = sys_get_di_sts(DI_FAN02_ERR_BPOS);			
//	}
//	else
//	{
////		data = sys_get_di_sts(DI_FAN02_OD_BPOS);	
//			return(ALARM_ACL_CLEARED);		
//	}
//	return(io_calc( data,IO_CLOSE));
			return(ALARM_ACL_CLEARED);	
}

//ACL_FAN_OVERLOAD3

static	uint16_t acl13(alarm_acl_status_st*acl_ptr)
{
		return(ALARM_ACL_CLEARED);
//	uint8_t data;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(g_sys.config.fan.num < 3)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(sys_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_HOLD);
//	}
//	if(sys_get_do_sts(DO_FAN_BPOS)==0)
//	{
//			return(ALARM_ACL_HOLD);
//	}	

//	data = sys_get_di_sts(DI_FAN03_OD_BPOS);	
//		
//	return(io_calc( data,IO_CLOSE));	
}

//ACL_FAN_OVERLOAD4

static	uint16_t acl14(alarm_acl_status_st*acl_ptr)
{
		return(ALARM_ACL_CLEARED);
//	uint8_t data;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(sys_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_HOLD);
//	}

//		{
//				if(g_sys.config.fan.num < 4)
//				{
//						return(ALARM_ACL_CLEARED);
//				}
//				if(sys_get_do_sts(DO_FAN_BPOS)==0)
//				{
//						return(ALARM_ACL_HOLD);
//				}

//				data = sys_get_di_sts(DI_FAN04_OD_BPOS);				
//		}	
//		
//	return(io_calc( data,IO_CLOSE));	
}
	//ACL_FAN_OVERLOAD5

static	uint16_t acl15(alarm_acl_status_st*acl_ptr)
{
			return(ALARM_ACL_CLEARED);
//		uint8_t data;
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
////		//Alair,20170224
////		if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND)&&(g_sys.config.compressor.type == COMP_QABP))//�м��Ƶ�������������ͷ�����
////		{
////				return(ALARM_ACL_CLEARED);		
////		}
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_HOLD);
//		}
//		{
//				if(g_sys.config.fan.num < 5)
//				{
//						return(ALARM_ACL_CLEARED);
//				}
//				if(sys_get_do_sts(DO_FAN_BPOS)==0)
//				{
//						return(ALARM_ACL_HOLD);
//				}

//				data = sys_get_di_sts(DI_FAN05_OD_BPOS);				
//		}	
//		return(io_calc( data,IO_CLOSE));	
}

//ACL_EX_FAN_CURRENT				,
static uint16_t acl16(alarm_acl_status_st* acl_ptr)
{
//			return(ALARM_ACL_CLEARED);
	
//		int16_t meter;
//		int16_t max;
//		uint8_t data;
//	
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(acl_get_pwr_sts() == 0)
//		{
//			 return(ALARM_ACL_CLEARED);
//		}
//			
//		if(g_sys.config.Ex_Fan.FAN_TYPE)
//		{
//			if(sys_get_mbm_online(EX_FAN_ADDR) == 0)
//			{
//					return(ALARM_ACL_CLEARED);
//			}

//			meter = g_sys.status.mbm.EX_FAN[0].P_Current[0];
//			if(meter<g_sys.status.mbm.EX_FAN[0].P_Current[1])	
//			{
//					meter = g_sys.status.mbm.EX_FAN[0].P_Current[1];			
//			}
//			if(meter<g_sys.status.mbm.EX_FAN[0].P_Current[2])	
//			{
//					meter = g_sys.status.mbm.EX_FAN[0].P_Current[2];			
//			}		
//					
//			max = g_sys.config.alarm[ACL_EX_FAN_CURRENT].alarm_param ;
//			
//			alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//			return(compare_calc( meter,0,max,THR_MAX_TYPE));
//		}
//		else
//		{
//			if(g_sys.status.Ex_Fan_Speed[0] >= (g_sys.config.Ex_Fan.MINSPEEDID+5 ))
//			{
//					data = sys_get_di_sts(DI_EX_FAN_BPOS);
//					return(io_calc( data,IO_CLOSE));
//			}
//			else
//			{
//					return(ALARM_ACL_CLEARED);				
//			}
//		}

return(ALARM_ACL_CLEARED);

//	uint8_t data;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(sys_get_pwr_sts() == 0)
//	{
//		 return(ALARM_ACL_CLEARED);
//	}
//		//Alair,20180310
//	if(g_sys.config.compressor.type == COMP_QABP)//��Ƶ
//	{
//				if(!(g_sys.config.dev_mask.ain&0x0004))
//				{
//					return(ALARM_ACL_CLEARED);
//				}
//				if(((int16)g_sys.status.ain[AI_SENSOR3]>ALARM_THRESHOLD)&&(g_sys.status.ain[AI_SENSOR3]!=ABNORMAL_VALUE))
//				{
//						if(sys_get_do_sts(DO_COMP1_BPOS) == 0)
//						{
//							return(ALARM_ACL_HOLD);
//						}
//						data =IO_CLOSE;	
//				}
//				else
//				{
//						data =IO_OPEN;						
//				}
//	}		
//	return(io_calc( data,IO_CLOSE));	
}

////ACL_EXIT_UNIT
//static	uint16_t acl16(alarm_acl_status_st*acl_ptr)
//{
//		uint8_t data;
//	
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(acl_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//				
//		//Alair,20170318��AI����
//		if(g_sys.config.dev_mask.ain&0x8000)
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		else
//		{
//				if(!(g_sys.config.dev_mask.ain&0x0001))
//				{
//					return(ALARM_ACL_CLEARED);
//				}
//				if((g_sys.status.ain[AI_SENSOR1]>ALARM_THRESHOLD)&&(g_sys.status.ain[AI_SENSOR1]!=ABNORMAL_VALUE))
//				{
//						data =IO_OPEN;	
//				}
//				else
//				{
//						data =IO_CLOSE;						
//				}		
//		}	
//		return(data);	
//}
//������г�ʱ
//ACL_FAN_OT1
static	uint16_t acl17(alarm_acl_status_st*acl_ptr)
{
//		uint32_t run_time;
//		int16_t max;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FAN_BPOS].low,g_sys.status.run_time[DO_FAN_BPOS].high);
//		max = g_sys.config.alarm[ACL_FAN_OT1].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));
			return(ALARM_ACL_CLEARED);	
}
//ACL_FAN_OT2
static	uint16_t acl18(alarm_acl_status_st*acl_ptr)
{
			return(ALARM_ACL_CLEARED);
//		uint32_t run_time;
//		int16_t max;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.fan.num < 2)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FAN2_DUMMY_BPOS].low,g_sys.status.run_time[DO_FAN2_DUMMY_BPOS].high);
//		max = g_sys.config.alarm[ACL_FAN_OT1].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));
}
//ACL_FAN_OT3
static	uint16_t acl19(alarm_acl_status_st*acl_ptr)
{
			return(ALARM_ACL_CLEARED);
//		uint32_t run_time;
//		int16_t max;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.fan.num < 3)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FAN3_DUMMY_BPOS].low,g_sys.status.run_time[DO_FAN3_DUMMY_BPOS].high);
//		max = g_sys.config.alarm[ACL_FAN_OT1].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));
}
//ACL_FAN_OT4
static	uint16_t acl20(alarm_acl_status_st*acl_ptr)
{
			return(ALARM_ACL_CLEARED);
//		uint32_t run_time;
//		int16_t max;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.fan.num < 4)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FAN4_DUMMY_BPOS].low,g_sys.status.run_time[DO_FAN4_DUMMY_BPOS].high);
//		max = g_sys.config.alarm[ACL_FAN_OT1].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));
}

//ACL_FAN_OT5
static	uint16_t acl21(alarm_acl_status_st*acl_ptr)
{
			return(ALARM_ACL_CLEARED);
//		uint32_t run_time;
//		int16_t max;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.fan.num < 5)
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FAN5_DUMMY_BPOS].low,g_sys.status.run_time[DO_FAN5_DUMMY_BPOS].high);
//		max = g_sys.config.alarm[ACL_FAN_OT1].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));
}

//ACL_EX_FAN2_CURRENT				,
static uint16_t acl22(alarm_acl_status_st* acl_ptr)
{
			return(ALARM_ACL_CLEARED);
//		extern sys_reg_st   g_sys;
//		int16_t meter;
//		int16_t max;
//		uint8_t data;
//	
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(acl_get_pwr_sts() == 0)
//		{
//			 return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.Ex_Fan.FAN_TYPE)
//		{
//			if(sys_get_mbm_online(EX_FAN_ADDR) == 0)
//			{
//					return(ALARM_ACL_CLEARED);
//			}

//			meter = g_sys.status.mbm.EX_FAN[1].P_Current[0];
//			if(meter<g_sys.status.mbm.EX_FAN[1].P_Current[1])	
//			{
//					meter = g_sys.status.mbm.EX_FAN[1].P_Current[1];			
//			}
//			if(meter<g_sys.status.mbm.EX_FAN[1].P_Current[2])	
//			{
//					meter = g_sys.status.mbm.EX_FAN[1].P_Current[2];			
//			}		
//					
//			max = g_sys.config.alarm[ACL_EX_FAN2_CURRENT].alarm_param ;
//			
//			alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//			return(compare_calc( meter,0,max,THR_MAX_TYPE));
//		}
//		else
//		{
//			if(g_sys.status.Ex_Fan_Speed[1] >= (g_sys.config.Ex_Fan.MINSPEEDID+5 ))
//			{
//					data = sys_get_di_sts(DI_EX_FAN2_BPOS);
//					return(io_calc( data,IO_CLOSE));
//			}
//			else
//			{
//					return(ALARM_ACL_CLEARED);				
//			}
//		}
}




//		alarm_io, //ACL_HI_PRESS1
static	uint16_t acl23(alarm_acl_status_st* acl_ptr)
{
//		uint8_t data;
//		time_t now;
//		uint8_t index;

//		index=0;
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
////		if(acl_get_pwr_sts() == 0)
////		{
////				return(ALARM_ACL_CLEARED);
////		}
//		if(devinfo_get_compressor_cnt() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		
////		data = sys_get_di_sts(DI_HI_PRESS1_BPOS);	
////		data = io_calc( data,IO_CLOSE);
//		
//		//Alair,20170318��AI����
//		if(g_sys.config.dev_mask.ain&0x8000)
//		{
//				if(!(g_sys.config.dev_mask.ain&0x0001))
//				{
//					return(ALARM_ACL_CLEARED);
//				}
//				if((g_sys.status.ain[AI_SENSOR1]>ALARM_THRESHOLD)&&(g_sys.status.ain[AI_SENSOR1]!=ABNORMAL_VALUE))
//				{
//						data =IO_OPEN;	
//				}
//				else
//				{
//						data =IO_CLOSE;						
//				}
//		}	
//		else
//		{
//				data = sys_get_di_sts(DI_HI_PRESS1_BPOS);	
//				data = io_calc( data,IO_CLOSE);			
//		}
////		rt_kprintf("data=%d\n",data);
//		if(alarm_inst.alarm_lock[index].last_state != acl_ptr->state)
//		{
//			if((alarm_inst.alarm_lock[index].last_state == ALARM_FSM_PREACTIVE)
//				 && (acl_ptr->state == ALARM_FSM_ACTIVE))//��Ԥ��������
//				{
//					get_local_time(&now);
//					alarm_inst.alarm_lock[index].lock_time[0] = alarm_inst.alarm_lock[index].lock_time[1];
//					alarm_inst.alarm_lock[index].lock_time[1] = alarm_inst.alarm_lock[index].lock_time[2];
//					alarm_inst.alarm_lock[index].lock_time[2] = now;
//				}
//			alarm_inst.alarm_lock[index].last_state = acl_ptr->state;
//		}

//		if(data)
//		{
//			alarm_inst.compress_high_flag[index] = 1;
//		}
//		else
//		{
//			alarm_inst.compress_high_flag[index] = 0;
//		}
//		
//		return(data);	
			return(ALARM_ACL_CLEARED);	
}
//		alarm_lock,//ACL_HI_LOCK1
static	uint16_t acl24(alarm_acl_status_st* acl_ptr)
{	
//		uint8_t req;
//	
//		//remove loc
//		req = alarm_lock(ACL_HI_LOCK1);	
//		acl_clear(acl_ptr);
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(devinfo_get_compressor_cnt() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}		
//		return (req);
	
		return(ALARM_ACL_CLEARED);
	
}
//		alarm_io,//ACL_LO_PRESS1
static	uint16_t acl25(alarm_acl_status_st* acl_ptr)
{
//	extern local_reg_st 	l_sys;
//	uint8_t data=0;
//	time_t now;
//	uint8_t index;
//	index = 2;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	
////	if(acl_get_pwr_sts() == 0)
////	{
////			return(ALARM_ACL_CLEARED);
////	}
//	if(devinfo_get_compressor_cnt() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(alarm_inst.alarm_lock[index].last_state != acl_ptr->state)
//	{
//		if((alarm_inst.alarm_lock[index].last_state == ALARM_FSM_PREACTIVE)
//			 && (acl_ptr->state == ALARM_FSM_ACTIVE))//��Ԥ��������
//			{
//				get_local_time(&now);
//				alarm_inst.alarm_lock[index].lock_time[0] = alarm_inst.alarm_lock[index].lock_time[1];
//				alarm_inst.alarm_lock[index].lock_time[1] = alarm_inst.alarm_lock[index].lock_time[2];
//				alarm_inst.alarm_lock[index].lock_time[2] = now;
//			}
//		alarm_inst.alarm_lock[index].last_state = acl_ptr->state;
//	}

//	
//		data = sys_get_di_sts(DI_LO_PRESS1_BPOS);	
//	
////				rt_kprintf("data = %d\n",data);	  
////				rt_kprintf("alarm_inst.compressor1_timer = %d\n",alarm_inst.compressor1_timer);	 
////				rt_kprintf("g_sys.config.compressor.startup_lowpress_shield = %d\n",g_sys.config.compressor.startup_lowpress_shield);	 
//  if(alarm_inst.compressor1_timer == 0)
//  {
//      if(data == IO_CLOSE)
//      {
//          data = ALARM_ACL_HOLD;
//      }
//      else
//      {
//          data = ALARM_ACL_CLEARED;
//      }
//  }
//  else if(alarm_inst.compressor1_timer < g_sys.config.compressor.startup_lowpress_shield)
//  {
//      data = ALARM_ACL_HOLD;
//  }
//  else
//  {
//      if(data == IO_CLOSE)
//      {
//          data = ALARM_ACL_TRIGGERED;
//      }
//      else
//      {
//          data = ALARM_ACL_CLEARED;
//      }
//  }
////					rt_kprintf("data = %d\n",data);	 
//	return(data);		
			return(ALARM_ACL_CLEARED);	
}
//		alarm_lock,//ACL_LO_LOCK1
static	uint16_t acl26(alarm_acl_status_st* acl_ptr)
{
//		uint8_t req;
//		
//		//remove loc
//		req = alarm_lock(ACL_LO_LOCK1);	
//		
//		acl_clear(acl_ptr);
//	
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(devinfo_get_compressor_cnt() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}		
//		
//		return (req);
	
			return(ALARM_ACL_CLEARED);
	
}
////		alarm_io,//ACL_EXTMP1
static	uint16_t acl27(alarm_acl_status_st* acl_ptr)
{
////	uint8_t data;
////	time_t now;
////	uint8_t index;
////	
////	index = 4;
////	if(acl_clear(acl_ptr))
////	{
////			return(ALARM_ACL_CLEARED);
////	}
////	if(sys_get_pwr_sts() == 0)
////	{
////			return(ALARM_ACL_CLEARED);
////	}
////	if(devinfo_get_compressor_cnt() == 0)
////	{
////			return(ALARM_ACL_CLEARED);
////	}
////	data = sys_get_di_sts(DI_EXT_TEMP1_BPOS);
////	data = io_calc( data,IO_CLOSE);
////	
////	if(alarm_inst.alarm_lock[index].last_state != acl_ptr->state)
////	{
////			if((alarm_inst.alarm_lock[index].last_state == ALARM_FSM_PREACTIVE)
////				 && (acl_ptr->state == ALARM_FSM_ACTIVE))//��Ԥ��������
////				{
////					get_local_time(&now);
////					alarm_inst.alarm_lock[index].lock_time[0] = alarm_inst.alarm_lock[index].lock_time[1];
////					alarm_inst.alarm_lock[index].lock_time[1] = alarm_inst.alarm_lock[index].lock_time[2];
////					alarm_inst.alarm_lock[index].lock_time[2] = now;
////				}
////			alarm_inst.alarm_lock[index].last_state = acl_ptr->state;
////	}
////	
////	return(data);	

//	extern local_reg_st 	l_sys;
////	time_t now;
//	
//	if(acl_clear(acl_ptr))
//	{
//			memset(g_sys.status.inv_compress_alarm.Inv_hi_temp_Starttime,0xFFFFFFFF,sizeof(g_sys.status.inv_compress_alarm.Inv_hi_temp_Starttime));
//			memset(g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime,0xFFFFFFFF,sizeof(g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime));
//			g_sys.status.inv_compress_alarm.Inv_hi_temp_Stop = 0;
//			g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag  = 0;
//			return(ALARM_ACL_CLEARED);
//	}

////	if(acl_get_pwr_sts() == 0)
////	{
////			return(ALARM_ACL_CLEARED);
////	}
//		//Alair,20170301
//	if(g_sys.config.compressor.type == COMP_QABP)//��Ƶ
//	{
//			
////			rt_kprintf("g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_AVG]: %d\n",g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_AVG]);
////			rt_kprintf("g_sys.config.alarm[ACL_INV_HI_TEMP].alarm_param: %d\n",g_sys.config.alarm[ACL_INV_HI_TEMP].alarm_param);
//			if(get_inv_comp_freq_down_signal(EXHAUST_TEMP) >0)
//			{			
//					if(g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_AVG]>=g_sys.config.alarm[ACL_EXTMP1].alarm_param)//  �����¶ȴ�����ֵ
//					{
//							return(1);				
//					}
//			}
//	}	
//	return(0);		
			return(ALARM_ACL_CLEARED);	
}
////		alarm_lock,//ACL_EXTMP_LOCK1
static	uint16_t acl28(alarm_acl_status_st* acl_ptr)
{
	
			return(ALARM_ACL_CLEARED);	
//		uint8_t req;
//		//remove loc
//		req = alarm_lock(ACL_EXTMP_LOCK1);	
//		
//		acl_clear(acl_ptr);
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(devinfo_get_compressor_cnt() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}		
//		
//		return (req);
}





////ѹ����1������
////ACL_SHORT_TERM1	
static	uint16_t acl29(alarm_acl_status_st* acl_ptr)
{
//	extern local_reg_st 	l_sys;
//	uint8_t index=0,i;
//	time_t now;
//	
//	
//	if(acl_clear(acl_ptr))
//	{
//			for(i=0;i<10;i++)
//			{
//					alarm_inst.cmpress_cycle_alarm[index].start_time[i] = 0xffffffff;
//			}
//			alarm_inst.cmpress_cycle_alarm[index].cycle_flag = 0;
//			alarm_inst.cmpress_cycle_alarm[index].alarm_timer = 0;
//			return(ALARM_ACL_CLEARED);
//	}
//	
//	if(sys_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(devinfo_get_compressor_cnt() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}	
//	
//	if(alarm_inst.cmpress_cycle_alarm[index].compress_state != ((g_sys.status.dout_bitmap >> DO_COMP1_BPOS)&0x0001))
//	{
//		if((alarm_inst.cmpress_cycle_alarm[index].compress_state == 0)&& //��ѹ�����ж�
//			(((g_sys.status.dout_bitmap >> DO_COMP1_BPOS)&0x0001) == 1))	
//		{
//			get_local_time(&now);
//			alarm_inst.cmpress_cycle_alarm[index].start_time[0] = alarm_inst.cmpress_cycle_alarm[index].start_time[1];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[1] = alarm_inst.cmpress_cycle_alarm[index].start_time[2];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[2] = alarm_inst.cmpress_cycle_alarm[index].start_time[3];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[3] = alarm_inst.cmpress_cycle_alarm[index].start_time[4];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[4] = alarm_inst.cmpress_cycle_alarm[index].start_time[5];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[5] = alarm_inst.cmpress_cycle_alarm[index].start_time[6];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[6] = alarm_inst.cmpress_cycle_alarm[index].start_time[7];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[7] = alarm_inst.cmpress_cycle_alarm[index].start_time[8];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[8] = alarm_inst.cmpress_cycle_alarm[index].start_time[9];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[9] = now;

//			if((alarm_inst.cmpress_cycle_alarm[index].start_time[0] != 0xffffffff)&&
//				((now-alarm_inst.cmpress_cycle_alarm[index].start_time[0]) <= 3600))//1��Сʱ�ڷ���10��ѹ������������
//			{
//				alarm_inst.cmpress_cycle_alarm[index].alarm_timer = 1800;
//				alarm_inst.cmpress_cycle_alarm[index].cycle_flag = 1;
//			}
//				
//		}
//		//״̬����
//		alarm_inst.cmpress_cycle_alarm[index].compress_state = ((g_sys.status.dout_bitmap >> DO_COMP1_BPOS)&0x0001);
//		
//	}
//	if(alarm_inst.cmpress_cycle_alarm[index].cycle_flag)
//	{
//		if(alarm_inst.cmpress_cycle_alarm[index].alarm_timer > 0 )
//		{
//			alarm_inst.cmpress_cycle_alarm[index].alarm_timer--;
//			if((get_alarm_bitmap(ACL_HI_TEMP_RETURN)) || (get_alarm_bitmap(ACL_HI_TEMP_SUPPLY)))//���±������Զ�����
//			{
//					alarm_inst.cmpress_cycle_alarm[index].cycle_flag = 0;
//					alarm_inst.cmpress_cycle_alarm[index].alarm_timer = 0;
//				  return(0);
//			}
//			else
//			{
//				  return(1);
//			}
//		}
//		else//��ʱ�����Զ�����
//		{
//				alarm_inst.cmpress_cycle_alarm[index].cycle_flag = 0;
//				return(0);
//		}
//	}
//	else
//	{
//		   return(0);
//	}
			return(ALARM_ACL_CLEARED);	
}

//ACL_COMPRESSOR_OT1			,

static	uint16_t acl30(alarm_acl_status_st* acl_ptr)
{	
//		uint32_t run_time;
//		int16_t max;
//		//����ȷ��
//	
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(devinfo_get_compressor_cnt() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//	
//		run_time = dev_runingtime(g_sys.status.run_time[DO_COMP1_BPOS].low,g_sys.status.run_time[DO_COMP1_BPOS].high);
//		max = g_sys.config.alarm[ACL_COMPRESSOR_OT1].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
			return(ALARM_ACL_CLEARED);	
}

//ѹ����2




//		alarm_io, //ACL_HI_PRESS2
static	uint16_t acl31(alarm_acl_status_st* acl_ptr)
{
//	uint8_t data;
//	time_t now;
//	uint8_t index;

//	index=1;
//	
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
////	if(acl_get_pwr_sts() == 0)
////	{
////				return(ALARM_ACL_CLEARED);
////	}
//	if(devinfo_get_compressor_cnt() != 2)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	data = sys_get_di_sts(DI_HI_PRESS2_BPOS);
//	data = io_calc( data,IO_CLOSE);
//	
//	if(alarm_inst.alarm_lock[index].last_state != acl_ptr->state)
//	{
//		if((alarm_inst.alarm_lock[index].last_state == ALARM_FSM_PREACTIVE)
//			 && (acl_ptr->state == ALARM_FSM_ACTIVE))//????,????
//			{
//				get_local_time(&now);
//				alarm_inst.alarm_lock[index].lock_time[0] = alarm_inst.alarm_lock[index].lock_time[1];
//				alarm_inst.alarm_lock[index].lock_time[1] = alarm_inst.alarm_lock[index].lock_time[2];
//				alarm_inst.alarm_lock[index].lock_time[2] = now;
//			}
//		alarm_inst.alarm_lock[index].last_state = acl_ptr->state;
//	}

//	if(data)
//	{
//		alarm_inst.compress_high_flag[index] = 1;
//	}
//	else
//	{
//		alarm_inst.compress_high_flag[index] = 0;
//	}
//	return(data);	
			return(ALARM_ACL_CLEARED);	
}
//		alarm_lock,//ACL_HI_LOCK2
static	uint16_t acl32(alarm_acl_status_st* acl_ptr)
{
		
//		uint8_t req;
//			//remove loc
//		req = alarm_lock(ACL_HI_LOCK2);	
//		
//		acl_clear(acl_ptr);
//		if(acl_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(devinfo_get_compressor_cnt() != 2)
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		
//		return (req);
			return(ALARM_ACL_CLEARED);		
}
//		alarm_io,//ACL_LO_PRESS2
static	uint16_t acl33(alarm_acl_status_st* acl_ptr)
{
//	uint8_t data=0;
//	time_t now;
//	uint8_t index;
//	extern local_reg_st  l_sys;	

//	// ��������ʱ
//	index = 3;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
////	if(acl_get_pwr_sts() == 0)
////	{
////				return(ALARM_ACL_CLEARED);
////	}
//	if(devinfo_get_compressor_cnt() != 2)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(alarm_inst.alarm_lock[index].last_state != acl_ptr->state)
//	{
//			if((alarm_inst.alarm_lock[index].last_state == ALARM_FSM_PREACTIVE)
//				 && (acl_ptr->state == ALARM_FSM_ACTIVE))//????,????
//			{
//					get_local_time(&now);
//					alarm_inst.alarm_lock[index].lock_time[0] = alarm_inst.alarm_lock[index].lock_time[1];
//					alarm_inst.alarm_lock[index].lock_time[1] = alarm_inst.alarm_lock[index].lock_time[2];
//					alarm_inst.alarm_lock[index].lock_time[2] = now;
//			}
//			alarm_inst.alarm_lock[index].last_state = acl_ptr->state;
//	}

//	data = sys_get_di_sts(DI_LO_PRESS2_BPOS);	
//  
//  if(alarm_inst.compressor2_timer == 0)
//  {
//      if(data == IO_CLOSE)
//      {
//          data = ALARM_ACL_HOLD;
//      }
//      else
//      {
//          data = ALARM_ACL_CLEARED;
//      }
//  }
//  else if(alarm_inst.compressor2_timer < g_sys.config.compressor.startup_lowpress_shield)
//  {
//      data = ALARM_ACL_HOLD;
//  }
//  else
//  {
//      if(data == IO_CLOSE)
//      {
//          data = ALARM_ACL_TRIGGERED;
//      }
//      else
//      {
//          data = ALARM_ACL_CLEARED;
//      }
//  }
//	return(data);		
			return(ALARM_ACL_CLEARED);	  
}
//		alarm_lock,//ACL_LO_LOCK2
static	uint16_t acl34(alarm_acl_status_st* acl_ptr)
{
//		uint8_t req;
//		//remove loc
//		req = alarm_lock(ACL_LO_LOCK2);	
//		
//		acl_clear(acl_ptr);
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(devinfo_get_compressor_cnt() != 2)
//		{
//				return(ALARM_ACL_CLEARED);
//		}		
//		
//		return (req);
			return(ALARM_ACL_CLEARED);	
}
//		alarm_io,//ACL_EXTMP2
static	uint16_t acl35(alarm_acl_status_st* acl_ptr)
{
			return(ALARM_ACL_CLEARED);
//	uint8_t data;
//	time_t now;
//	uint8_t index;

//	index = 5;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(acl_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(devinfo_get_compressor_cnt() != 2)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	data = sys_get_di_sts(DI_EXT_TEMP2_BPOS);
//	data = io_calc( data,IO_CLOSE);
//	
//	
//	if(alarm_inst.alarm_lock[index].last_state != acl_ptr->state)
//	{
//			if((alarm_inst.alarm_lock[index].last_state == ALARM_FSM_PREACTIVE)
//			&& (acl_ptr->state == ALARM_FSM_ACTIVE))//????,????
//			{
//					get_local_time(&now);
//					alarm_inst.alarm_lock[index].lock_time[0] = alarm_inst.alarm_lock[index].lock_time[1];
//					alarm_inst.alarm_lock[index].lock_time[1] = alarm_inst.alarm_lock[index].lock_time[2];
//					alarm_inst.alarm_lock[index].lock_time[2] = now;
//			}
//			alarm_inst.alarm_lock[index].last_state = acl_ptr->state;
//	}

//	return(data);	
}
//		alarm_lock,//ACL_EXTMP_LOCK2
static	uint16_t acl36(alarm_acl_status_st* acl_ptr)
{
			return(ALARM_ACL_CLEARED);
//		uint8_t req;
//		//remove loc
//		req = alarm_lock(ACL_EXTMP_LOCK2);	
//		
//		acl_clear(acl_ptr);
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(devinfo_get_compressor_cnt() != 2)
//		{
//				return(ALARM_ACL_CLEARED);
//		}		
//		
//		
//		return (req);

}



//ѹ����������

static	uint16_t acl37(alarm_acl_status_st* acl_ptr)
{
//	extern local_reg_st 	l_sys;
//	uint8_t index=1,i;
//	time_t now;
//	if(acl_clear(acl_ptr))
//	{
//			for(i=0;i<10;i++)
//			{
//					alarm_inst.cmpress_cycle_alarm[index].start_time[i] = 0xffffffff;
//			}
//			alarm_inst.cmpress_cycle_alarm[index].alarm_timer = 0;
//			alarm_inst.cmpress_cycle_alarm[index].cycle_flag = 0;
//			return(ALARM_ACL_CLEARED);
//	}
//	if(sys_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(devinfo_get_compressor_cnt() != 2)
//	{
//			return(ALARM_ACL_CLEARED);
//	}	
//	if(alarm_inst.cmpress_cycle_alarm[index].compress_state != ((g_sys.status.dout_bitmap >> DO_COMP2_BPOS)&0x0001))
//	{
//		if((alarm_inst.cmpress_cycle_alarm[index].compress_state == 0)&& //��ѹ�����ж�
//			(((g_sys.status.dout_bitmap >> DO_COMP2_BPOS)&0x0001) == 1))	
//		{
//			get_local_time(&now);
//			alarm_inst.cmpress_cycle_alarm[index].start_time[0] = alarm_inst.cmpress_cycle_alarm[index].start_time[1];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[1] = alarm_inst.cmpress_cycle_alarm[index].start_time[2];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[2] = alarm_inst.cmpress_cycle_alarm[index].start_time[3];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[3] = alarm_inst.cmpress_cycle_alarm[index].start_time[4];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[4] = alarm_inst.cmpress_cycle_alarm[index].start_time[5];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[5] = alarm_inst.cmpress_cycle_alarm[index].start_time[6];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[6] = alarm_inst.cmpress_cycle_alarm[index].start_time[7];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[7] = alarm_inst.cmpress_cycle_alarm[index].start_time[8];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[8] = alarm_inst.cmpress_cycle_alarm[index].start_time[9];
//			alarm_inst.cmpress_cycle_alarm[index].start_time[9] = now; 
//			
//			if((alarm_inst.cmpress_cycle_alarm[index].start_time[0] != 0xffffffff)&&
//				((now-alarm_inst.cmpress_cycle_alarm[index].start_time[0] ) <= 3600))//1??????10????????
//			{
//				alarm_inst.cmpress_cycle_alarm[index].alarm_timer = 1800;
//				alarm_inst.cmpress_cycle_alarm[index].cycle_flag = 1;
//			}
//				
//		}

//		alarm_inst.cmpress_cycle_alarm[index].compress_state = ((g_sys.status.dout_bitmap >> DO_COMP2_BPOS)&0x0001);
//		
//	}
//	if(alarm_inst.cmpress_cycle_alarm[index].cycle_flag)
//	{
//		if(alarm_inst.cmpress_cycle_alarm[index].alarm_timer > 0 )
//		{
//			alarm_inst.cmpress_cycle_alarm[index].alarm_timer--;
//			if((get_alarm_bitmap(ACL_HI_TEMP_RETURN)) || (get_alarm_bitmap(ACL_HI_TEMP_SUPPLY)))//���±������Զ�����
//			{
//					alarm_inst.cmpress_cycle_alarm[index].cycle_flag = 0;
//					alarm_inst.cmpress_cycle_alarm[index].alarm_timer = 0;
//				  return(0);
//			}
//			else
//			{
//			return(1);
//			}
//		}
//		else
//		{
//			alarm_inst.cmpress_cycle_alarm[index].cycle_flag = 0;
//			return(0);
//		}
//	}
//	else
//	{
//		return(0);
//	}
			return(ALARM_ACL_CLEARED);		

}

//ACL_COMPRESSOR_OT2	

static	uint16_t acl38(alarm_acl_status_st* acl_ptr)
{
//		uint32_t run_time;
//		int16_t max;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(devinfo_get_compressor_cnt() != 2)
//		{
//				return(ALARM_ACL_CLEARED);
//		}		
//	
//		run_time = dev_runingtime(g_sys.status.run_time[DO_COMP2_BPOS].low,g_sys.status.run_time[DO_COMP2_BPOS].high);
//		max = g_sys.config.alarm[ACL_COMPRESSOR_OT2].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
			return(ALARM_ACL_CLEARED);	
}


//ACL_HUM_OCURRENT           

static	uint16_t acl39(alarm_acl_status_st* acl_ptr)
{
//	uint16_t meter;
//	uint16_t max;
//	//����ȷ��
//	//���������²ű�����û�����б���

//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(sys_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(g_sys.config.ac_power_supply.PH_Ver==0)
//	{
//		if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.humidifier.hum_type == HUM_TYPE_INFRARED))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//	}
//	else
//	{
//		if((sys_get_mbm_online(HUM_MODULE_BPOS) == 0)||(g_sys.config.humidifier.hum_type == HUM_TYPE_INFRARED))
//		{
//				return(ALARM_ACL_CLEARED);
//		}		
//	}
//	if((g_sys.status.dout_bitmap & (0x0001 << DO_HUM_BPOS)) == 0)
//	{
//			return(ALARM_ACL_HOLD);
//	}
//	
//	meter = g_sys.status.mbm.hum.hum_current;		
//	max = HUM_CURRENT_UNIT*g_sys.config.humidifier.hum_real_cap*150;//g_sys.config.alarm[ACL_HUM_OCURRENT].alarm_param;
//	
//	alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//	
//	return(compare_calc( meter,0,max,THR_MAX_TYPE));		
			return(ALARM_ACL_CLEARED);	
}

//			ACL_HUM_HI_LEVEL			  ,
uint16_t acl40(alarm_acl_status_st* acl_ptr)
{
//	uint8_t data;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(acl_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(g_sys.config.ac_power_supply.PH_Ver==0)
//	{
//			if(sys_get_mbm_online(POWER_MODULE_BPOS) == 0)
//			{
//					return(ALARM_ACL_CLEARED);
//			}
//			data = g_sys.status.mbm.hum.water_level;
//	}
//	else
//	{
//			if(sys_get_mbm_online(HUM_MODULE_BPOS) == 0)
//			{
//					return(ALARM_ACL_CLEARED);
//			}
//			data = g_sys.status.mbm.hum.water_level;	
//	}	
//	
//	return(io_calc( data,IO_CLOSE));
			return(ALARM_ACL_CLEARED);	
}
//			ACL_HUM_LO_LEVEL        ,
static	uint16_t acl41(alarm_acl_status_st* acl_ptr)
{
//		extern local_reg_st		l_sys;
//		uint16_t min, meter;
//		uint8_t data;
//	
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(acl_get_pwr_sts() == 0)
//		{
//			 return(ALARM_ACL_CLEARED);
//		}

////		if(g_sys.config.humidifier.hum_type == HUM_TYPE_INFRARED)
////		{
////					data = sys_get_di_sts(DI_RED_LOW_LEVEL);	
////					if((g_sys.status.dout_bitmap & (0x0001 << DO_HUM_BPOS)) == 0)
////					{
////								return(ALARM_ACL_HOLD);
////					}
////					
////					return(io_calc( data,IO_CLOSE));
////		}
////		else
//		{

//					if(g_sys.config.ac_power_supply.PH_Ver==0)
//					{
//						if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.humidifier.hum_type == HUM_TYPE_INFRARED))
//						{
//							return(ALARM_ACL_CLEARED);
//						}
//					}
//					else
//					{
//						if((sys_get_mbm_online(HUM_MODULE_BPOS) == 0)||(g_sys.config.humidifier.hum_type == HUM_TYPE_INFRARED))
//						{
//							return(ALARM_ACL_CLEARED);
//						}
//					}	
//					if(g_sys.status.mbm.hum.water_level == 1)
//					{
//								return(ALARM_ACL_CLEARED);
//					}
//					
//					if(((g_sys.status.dout_bitmap & (0x0001 << DO_HUM_BPOS)) == 0)||(l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE]==HUM_FSM_STATE_WARM))
//					{
//							return(ALARM_ACL_HOLD);
//					}
//				
//					min =g_sys.config.alarm[ACL_HUM_LO_LEVEL].alarm_param*HUM_CURRENT_UNIT*g_sys.config.humidifier.hum_real_cap;
//					meter = g_sys.status.mbm.hum.hum_current;		
//					
//				//û�и�ˮλ����������£����Ҽ�ʪ������С
//					if((compare_calc( meter,min,0,SML_MIN_TYPE) == 1)&&
//						(io_calc( g_sys.status.mbm.hum.water_level,IO_CLOSE) == 0))
//					{
//							return(1);
//					}
//					else
//					{
//							return(0);
//					}				
//		}
			return(ALARM_ACL_CLEARED);	
}
//			ACL_HUM_OT					    ,
static	uint16_t acl42(alarm_acl_status_st* acl_ptr)
{		
//		uint32_t run_time;
//		int16_t max;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(sys_get_pwr_sts() == 0)
//		{
//			 return(ALARM_ACL_CLEARED);
//		}

//		if(g_sys.config.ac_power_supply.PH_Ver==0)
//		{
//			if(sys_get_mbm_online(POWER_MODULE_BPOS) == 0)
//			{
//					return(ALARM_ACL_CLEARED);
//			}
//		}
//		else
//		{
//			if(sys_get_mbm_online(HUM_MODULE_BPOS) == 0)
//			{
//					return(ALARM_ACL_CLEARED);
//			}
//		}		
//		run_time = dev_runingtime(g_sys.status.run_time[DO_HUM_BPOS].low,g_sys.status.run_time[DO_HUM_BPOS].high);
//		max = g_sys.config.alarm[ACL_HUM_OT].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));		
			return(ALARM_ACL_CLEARED);	
}
//ACL_HEATER_OD

 static uint16_t acl43(alarm_acl_status_st* acl_ptr)
{
//		uint8_t data;
//	
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(sys_get_pwr_sts() == 0)
//		{
//			return(ALARM_ACL_CLEARED);
//		}
//		//�޼���
//		if(devinfo_get_heater_level()== 0)
//		{
//			return(ALARM_ACL_CLEARED);
//		}
//		
//		data = sys_get_di_sts(DI_HEATER_OD_BPOS);	
//		//����ʱ��¼��������
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = 0; 	

//		if(g_sys.status.dout_bitmap &(0x0001<<DO_RH1_BPOS))
//		{
//				alarm_inst.alarm_sts[acl_ptr->id].alram_value |=0x0001;
//		}

//		if(g_sys.status.dout_bitmap &(0x0001<<DO_RH2_BPOS))
//		{
//				alarm_inst.alarm_sts[acl_ptr->id].alram_value |=0x0002;
//		}

////		if(g_sys.status.dout_bitmap &(0x0001<<DO_RH3_BPOS))
////		{
////				alarm_inst.alarm_sts[acl_ptr->id].alram_value |=0x0004;
////		}
//		return(io_calc( data,IO_CLOSE));
			return(ALARM_ACL_CLEARED);	
}

////������ ���г�ʱ
//			ACL_HEATER_OT1			    ,
static	uint16_t acl44(alarm_acl_status_st* acl_ptr)
{
//		uint32_t run_time;
//		int16_t max;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(sys_get_pwr_sts() == 0)
//		{
//			return(ALARM_ACL_CLEARED);
//		}
//	
//		run_time = dev_runingtime(g_sys.status.run_time[DO_RH1_BPOS].low,g_sys.status.run_time[DO_RH1_BPOS].high);
//		max = g_sys.config.alarm[ACL_HEATER_OT1].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));		
			return(ALARM_ACL_CLEARED);		
}
//			ACL_HEATER_OT2				  ,
static	uint16_t acl45(alarm_acl_status_st* acl_ptr)
{
			return(ALARM_ACL_CLEARED);
//		uint32_t run_time;
//		int16_t max;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(sys_get_pwr_sts() == 0)
//		{
//			return(ALARM_ACL_CLEARED);
//		}
//	
//		run_time = dev_runingtime(g_sys.status.run_time[DO_RH2_BPOS].low,g_sys.status.run_time[DO_RH2_BPOS].high);
//		max = g_sys.config.alarm[ACL_HEATER_OT2].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
}

//    alarm_compare,//			ACL_POWER_LOSS					,//ABC������ѹ�������޵�ѹ
static uint16_t acl46(alarm_acl_status_st* acl_ptr)
{
//	  int8_t data;

//		if(acl_clear(acl_ptr))
//		{
//			  sys_option_di_sts(DI_POWER_LOSS_BPOS,0);
//				return(ALARM_ACL_CLEARED);
//		}
//		data = sys_get_di_sts(DI_POWER_LOSS_BPOS);		
//		return(io_calc( data,IO_CLOSE));
			return(ALARM_ACL_CLEARED);	
}
//    alarm_io,//			ACL_POWER_EP						,	//reverse phase
static uint16_t acl47(alarm_acl_status_st* acl_ptr)
{
			return(ALARM_ACL_CLEARED);
//	int8_t data;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//	{
//				return(ALARM_ACL_CLEARED);
//	}
//	if(((g_sys.config.dev_mask.dout & (0x01<<DO_PHASE_P_BPOS))==(0x01<<DO_PHASE_P_BPOS))||((g_sys.config.dev_mask.dout & (0x01<<DO_PHASE_N_BPOS))==(0x01<<DO_PHASE_N_BPOS)))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(g_sys.config.ac_power_supply.power_mode == POWER_MODE_THREBLE)//�����
//	{
//			data = g_sys.status.mbm.pwr.dev_sts&0x0001;
//			return(io_calc( data,IO_CLOSE));
//	}
//	else
//	{
//			return(ALARM_ACL_CLEARED);
//	}
}

//				ACL_POWER_HI_FD						,	//freqency deviation
static uint16_t acl48(alarm_acl_status_st* acl_ptr)
{
			return(ALARM_ACL_CLEARED);	
//	int16_t meter;
//	int16_t max ;
//	
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//	{
//				return(ALARM_ACL_CLEARED);
//	}
//	meter=g_sys.status.mbm.pwr.freq;

//	max = g_sys.config.alarm[ACL_POWER_HI_FD].alarm_param;
//	

//	alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;

//	return(compare_calc( meter,0,max,THR_MAX_TYPE));
	
}
//ACL_POWER_LO_FD						,	//freqency deviation
static uint16_t acl49(alarm_acl_status_st* acl_ptr)
{
			return(ALARM_ACL_CLEARED);		
//	int16_t meter;
//	int16_t min ;
//	
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//	{
//				return(ALARM_ACL_CLEARED);
//	}
//	meter=g_sys.status.mbm.pwr.freq;

//	min = g_sys.config.alarm[ACL_POWER_LO_FD].alarm_param;
//	
//	alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;

//	return(compare_calc( meter,min,0,SML_MIN_TYPE));
	
}

//		alarm_compare,//			ACL_POWER_A_HIGH				,
static uint16_t acl50(alarm_acl_status_st* acl_ptr)
{
				return(ALARM_ACL_CLEARED);	
//		int16_t meter;
//		int16_t max;

//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//	if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//	{
//				return(ALARM_ACL_CLEARED);
//	}
//		meter = g_sys.status.mbm.pwr.pa_volt;
//		
//		max = g_sys.config.alarm[ACL_POWER_A_HIGH].alarm_param;
//		
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//		
//		return(compare_calc( meter,0,max,THR_MAX_TYPE));
}

//		alarm_compare,//			ACL_POWER_B_HIGH				,
static uint16_t acl51(alarm_acl_status_st* acl_ptr)
{
			return(ALARM_ACL_CLEARED);	
//		int16_t meter;
//		int16_t max;
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//	if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//	{
//				return(ALARM_ACL_CLEARED);
//	}
//		if(g_sys.config.ac_power_supply.power_mode == POWER_MODE_THREBLE)//�����
//		{
//				
//				meter = g_sys.status.mbm.pwr.pb_volt;
//						
//				max = g_sys.config.alarm[ACL_POWER_A_HIGH].alarm_param ;
//				
//				alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//				return(compare_calc( meter,0,max,THR_MAX_TYPE));
//		}
//		else
//		{
//				return(ALARM_ACL_CLEARED);
//		}
}

//		alarm_compare,//			ACL_POWER_C_HIGH				,
static uint16_t acl52(alarm_acl_status_st* acl_ptr)
{
//		int16_t meter;
//		int16_t max;
//	
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//	if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//	{
//				return(ALARM_ACL_CLEARED);
//	}
//		if(g_sys.config.ac_power_supply.power_mode == POWER_MODE_THREBLE)//�����
//		{
//				
//				meter = g_sys.status.mbm.pwr.pc_volt;
//				max = g_sys.config.alarm[ACL_POWER_A_HIGH].alarm_param;
//				alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;	
//				return(compare_calc( meter,0,max,THR_MAX_TYPE));
//		}
//		else
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		
			return(ALARM_ACL_CLEARED);	
}

//		ACL_POWER_A_LOW					
static uint16_t acl53(alarm_acl_status_st* acl_ptr)
{
//		int16_t meter;
//		int16_t min;

//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//	if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//	{
//				return(ALARM_ACL_CLEARED);
//	}
//		meter = g_sys.status.mbm.pwr.pa_volt;
//				
//		min = g_sys.config.alarm[ACL_POWER_A_LOW].alarm_param; 
//	
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//		return(compare_calc( meter,min,0,SML_MIN_TYPE));
				return(ALARM_ACL_CLEARED);	
}


//		alarm_compare,//			ACL_POWER_B_LOW					,
static uint16_t acl54(alarm_acl_status_st* acl_ptr)
{
//		int16_t meter;
//		int16_t min;

//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//	if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//	{
//				return(ALARM_ACL_CLEARED);
//	}
//		if(g_sys.config.ac_power_supply.power_mode == POWER_MODE_THREBLE)//�����
//		{
//				
//				meter = g_sys.status.mbm.pwr.pb_volt;		
//		    min = g_sys.config.alarm[ACL_POWER_A_LOW].alarm_param;
//				alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//				return(compare_calc( meter,min,0,SML_MIN_TYPE));
//		}
//		else
//		{
//				return(ALARM_ACL_CLEARED);
//		}
			return(ALARM_ACL_CLEARED);			
}

//		alarm_compare,//			ACL_POWER_C_LOW					,
static uint16_t acl55(alarm_acl_status_st* acl_ptr)
{
//		int16_t meter;
//		int16_t min;
//	
//	
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//	if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//	{
//				return(ALARM_ACL_CLEARED);
//	}
//		if(g_sys.config.ac_power_supply.power_mode == POWER_MODE_THREBLE)//�����
//		{
//				meter = g_sys.status.mbm.pwr.pc_volt;	
//				min = g_sys.config.alarm[ACL_POWER_A_LOW].alarm_param ;
//				//min = min >>POWER_DOT_BIT;
//				//min=((g_sys.config.alarm[ACL_POWER_C_LOW].alarm_param & POWER_DOT)/100)*min;
//				alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//				return(compare_calc( meter,min,0,SML_MIN_TYPE));
//		}
//		else
//		{
//				return(ALARM_ACL_CLEARED);
//		}
			return(ALARM_ACL_CLEARED);	
}

//		alarm_io,//			ACL_POWER_A_OP					,	//open phase
static uint16_t acl56(alarm_acl_status_st* acl_ptr)
{
//		int16_t meter;
//		int16_t min;

//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//	if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//	{
//				return(ALARM_ACL_CLEARED);
//	}
//		if(g_sys.config.ac_power_supply.power_mode == POWER_MODE_THREBLE)//�����
//		{
//					meter = g_sys.status.mbm.pwr.pa_volt;		
//					min = g_sys.config.alarm[ACL_POWER_A_OP].alarm_param ;
//					//	min = min >>POWER_DOT_BIT;
//					//	min=((g_sys.config.alarm[ACL_POWER_A_OP].alarm_param & POWER_DOT)/100)*min;
//					alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//					return(compare_calc( meter,min,0,SML_MIN_TYPE));
//		}
//		else
//		{
//					return(ALARM_ACL_CLEARED);
//		}
			return(ALARM_ACL_CLEARED);			
}

//		alarm_io,//			ACL_POWER_B_OP					,	//open phase
static uint16_t acl57(alarm_acl_status_st* acl_ptr)
{
//		int16_t meter;
//		int16_t min;

//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//	if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//	{
//				return(ALARM_ACL_CLEARED);
//	}
//		if(g_sys.config.ac_power_supply.power_mode == POWER_MODE_THREBLE)//�����
//		{
//				meter = g_sys.status.mbm.pwr.pb_volt;
//				min = g_sys.config.alarm[ACL_POWER_A_OP].alarm_param ;
//				//min = min >>POWER_DOT_BIT;
//				//min=((g_sys.config.alarm[ACL_POWER_B_OP].alarm_param & POWER_DOT)/100)*min;
//				alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//				return(compare_calc( meter,min,0,SML_MIN_TYPE));
//		}
//		else
//		{
//				return(ALARM_ACL_CLEARED);
//		}
			return(ALARM_ACL_CLEARED);			
}


//		alarm_io,//			ACL_POWER_C_OP					,	//open phase
static uint16_t acl58(alarm_acl_status_st* acl_ptr)
{
//		int16_t meter;
//		int16_t min;

//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if((sys_get_mbm_online(POWER_MODULE_BPOS) == 0)||(g_sys.config.ac_power_supply.power_mode==POWER_MODE_NONE))
//		{
//					return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.ac_power_supply.power_mode == POWER_MODE_THREBLE)//�����
//		{
//				
//				meter = g_sys.status.mbm.pwr.pc_volt;	
//				min = g_sys.config.alarm[ACL_POWER_A_OP].alarm_param ;
//				//	min = min >>POWER_DOT_BIT;
//				//min=((g_sys.config.alarm[ACL_POWER_C_OP].alarm_param & POWER_DOT)/100)*min;
//				alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//				return(compare_calc( meter,min,0,SML_MIN_TYPE));
//		}
//		else
//		{
//				return(ALARM_ACL_CLEARED);
//		}
			return(ALARM_ACL_CLEARED);			
}


////������ʧ ��������ʵ��
//			ACL_AIR_LOSS				   
static	uint16_t acl59(alarm_acl_status_st* acl_ptr)
{
//	uint8_t data;
//	
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(sys_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_HOLD);
//	}
//	//Alair,20170327
////  if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND)||(g_sys.config.general.cool_type ==COOL_TYPE_COLUMN_WATER))
////  {
////      return(ALARM_ACL_CLEARED);
////  }
//	//Alair,20170331,��������ʱ����������ʧ�澯
//	if(sys_get_do_sts(DO_FAN_BPOS)==0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}	
//	if(g_sys.config.fan.num>=2)//DI����
//	{
//			return(ALARM_ACL_CLEARED);	
//	}
//	if((alarm_inst.fan_timer < g_sys.config.fan.cold_start_delay)&&(alarm_inst.fan_timer != 0))
//	{
//			return(ALARM_ACL_CLEARED);
//	}	
//	data = sys_get_di_sts(DI_AIR_FLOW_BPOS);	
//	return(io_calc( data,IO_CLOSE));
			return(ALARM_ACL_CLEARED);		
}
//��������ʱ����
//ACL_FILTER_OT
static	uint16_t acl60(alarm_acl_status_st* acl_ptr)
{
//		uint32_t run_time;
//		int16_t max;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(sys_get_pwr_sts() == 0)
//		{
//			return(ALARM_ACL_CLEARED);
//		}
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].low,g_sys.status.run_time[DO_FILLTER_DUMMY_BPOS].high);
//		max = g_sys.config.alarm[ACL_FILTER_OT].alarm_param;
//		
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));	
				return(ALARM_ACL_CLEARED);	
}

////��������
//			ACL_FILTER_CLOG        ,
static	uint16_t acl61(alarm_acl_status_st* acl_ptr)
{
//	uint8_t data;
//	
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
////	if(acl_get_pwr_sts() == 0)
////	{
////			return(ALARM_ACL_CLEARED);
////	}
//	data = sys_get_di_sts(DI_FILTER_BPOS);
//	return(io_calc( data,IO_CLOSE));
				return(ALARM_ACL_CLEARED);	
}

//// Զ�̹ػ� ����
//			ACL_REMOTE_SHUT				 
static	uint16_t acl62(alarm_acl_status_st* acl_ptr)
{
//	uint8_t data; 
//	
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	
//	data = sys_get_di_sts(DI_REMOTE_BPOS);
//	return(io_calc( data,IO_CLOSE));
			return(ALARM_ACL_CLEARED);	
}
////�ذ��ˮ
//			ACL_WATER_OVERFLOW		 ,	

static	uint16_t acl63(alarm_acl_status_st* acl_ptr)
{
//	uint8_t data;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	data = sys_get_di_sts(DI_WATER_OVERFLOW_BPOS);
//	return(io_calc( data,IO_CLOSE));
			return(ALARM_ACL_CLEARED);	
}
///
//Ⱥ��ʧ�ܸ澯
//ACL_GROUP_CONTROL_FAIL 
static	uint16_t acl64(alarm_acl_status_st* acl_ptr)
{
//	uint8_t data;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
////	if(acl_get_pwr_sts() == 0)
////	{
////			return(ALARM_ACL_CLEARED);
////	}
//	if(g_sys.config.team.team_en == 1)
//	{
//		  data = sys_get_di_sts(DI_GROUP_DEFAULT_BPOS);	
//			return(io_calc( data,IO_CLOSE));
//			
//	}
//	else
//	{
//			return(ALARM_ACL_CLEARED);
//	}
			return(ALARM_ACL_CLEARED);	
}

//��Ƶ���澯
//ACL_INV_FAULT
static	uint16_t acl65(alarm_acl_status_st* acl_ptr)
{
	
//	uint8_t data;
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(sys_get_pwr_sts() == 0)
//	{
//		 return(ALARM_ACL_CLEARED);
//	}
//		//Alair,20180310
//	if(g_sys.config.compressor.type == COMP_QABP)//��Ƶ
//	{
//				if(!(g_sys.config.dev_mask.ain&0x0004))
//				{
//					return(ALARM_ACL_CLEARED);
//				}
//				if(((int16)g_sys.status.ain[AI_SENSOR3]>ALARM_THRESHOLD)&&(g_sys.status.ain[AI_SENSOR3]!=ABNORMAL_VALUE))
//				{
//						if(sys_get_do_sts(DO_COMP1_BPOS) == 0)
//						{
//							return(ALARM_ACL_HOLD);
//						}
//						data =IO_CLOSE;	
//				}
//				else
//				{
//						data =IO_OPEN;						
//				}
//	}		
//	return(io_calc( data,IO_CLOSE));	
			return(ALARM_ACL_CLEARED);	
}

//עˮ�絼�ʸ߸澯
//ACL_WATER_ELEC_HI
static	uint16_t acl66(alarm_acl_status_st* acl_ptr)
{
//		uint16_t max;
//		uint16_t meter;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(acl_get_pwr_sts() == 0)
//		{
//			 return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.ac_power_supply.PH_Ver==0)
//		{
//			if(sys_get_mbm_online(POWER_MODULE_BPOS) == 0)
//			{
//					return(ALARM_ACL_CLEARED);
//			}
//		}
//		else
//		{
//			if(sys_get_mbm_online(HUM_MODULE_BPOS) == 0)
//			{
//					return(ALARM_ACL_CLEARED);
//			}
//		}	
//	
//		meter = g_sys.status.mbm.hum.conductivity;
//		max = g_sys.config.alarm[ACL_WATER_ELEC_HI].alarm_param;
//		
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//		return(compare_calc( meter,0,max,THR_MAX_TYPE));	
			return(ALARM_ACL_CLEARED);		
}
//עˮ�絼�ʵ͸澯
//ACL_WATER_ELEC_LO
static	uint16_t acl67(alarm_acl_status_st* acl_ptr)
{
//		int16_t min;
//		uint16_t meter;
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(acl_get_pwr_sts() == 0)
//		{
//			return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.ac_power_supply.PH_Ver==0)
//		{
//			if(sys_get_mbm_online(POWER_MODULE_BPOS) == 0)
//			{
//					return(ALARM_ACL_CLEARED);
//			}
//		}
//		else
//		{
//			if(sys_get_mbm_online(HUM_MODULE_BPOS) == 0)
//			{
//					return(ALARM_ACL_CLEARED);
//			}
//		}	
//		meter = g_sys.status.mbm.hum.conductivity;
//		min = g_sys.config.alarm[ACL_WATER_ELEC_LO].alarm_param;
//		
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = meter;
//		return(compare_calc( meter,min,0,SML_MIN_TYPE));	
			return(ALARM_ACL_CLEARED);		
}

//������
//DI_SMOKE_BPOS
static	uint16_t acl68(alarm_acl_status_st* acl_ptr)
{
//	uint8_t data; 
//	
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	
//	data = sys_get_di_sts(DI_SMOKE_BPOS);
//	return(io_calc( data,IO_CLOSE));
			return(ALARM_ACL_CLEARED);	
}

//			ACL_USER_DEFINE,    �û��Զ���澯
static	uint16_t acl69(alarm_acl_status_st* acl_ptr)
{
//	uint8_t data; 
//	
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	
//	data = sys_get_di_sts(DI_USR_BPOS);
//	return(io_calc( data,IO_CLOSE));
			return(ALARM_ACL_CLEARED);	
}
//���õ�Դ�л��澯
//ACL_BACK_POWER,��·��Դ
static	uint16_t acl70(alarm_acl_status_st* acl_ptr)
{
		return(ALARM_ACL_CLEARED);	
//	uint8_t data; 
//	
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	
//	data = sys_get_di_sts(DI_BK_POWER_BPOS);
//	return(io_calc( data,IO_CLOSE));
}

//ACL_COIL_HI_TEM1                  
static	uint16_t acl71(alarm_acl_status_st* acl_ptr)
{
//	int16_t meter;
//	int16_t max;

//	//�����ж�
//	if((g_sys.config.general.cool_type == COOL_TYPE_MODULE_WIND)||
//			(g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	//�ػ����Զ������
//	if(sys_get_pwr_sts() == 0)
//	{
//			return(ALARM_ACL_HOLD);
//	}
//		//ˮ���ص�����·��ر���
//	if(g_sys.status.aout[AO_WATER_VALVE] == 0)
//	{
//		return(ALARM_ACL_HOLD);
//	}
//// ��������ʱ
////	if(alarm_inst.power_on_timer < g_sys.config.alarm[ACL_COIL_HI_TEM1].alarm_param2)
////	{
////		acl_ptr->state = ALARM_FSM_INACTIVE;
////		return(ALARM_ACL_HOLD);
////	}
////����ȷ��
//	if(((g_sys.config.dev_mask.ain) & (0x01<<AI_COOL_NTC1)) == 0)
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	meter = (int16_t)(g_sys.status.ain[AI_COOL_NTC1]);
//	max = (int16_t)(g_sys.config.alarm[ACL_COIL_HI_TEM1].alarm_param);
//	
//	alarm_inst.alarm_sts[acl_ptr->id].alram_value=meter;
//	
//	return(compare_calc( meter,0,max,THR_MAX_TYPE));
			return(ALARM_ACL_CLEARED);	
}

//ACL_COIL_VALVE_DEFAULT  

static	uint16_t acl72(alarm_acl_status_st* acl_ptr)
{
//		int16_t meter;
//		int16_t max;
//		//�����ж�
//		if((g_sys.config.general.cool_type == COOL_TYPE_MODULE_WIND)||
//				(g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//    if((g_sys.config.dev_mask.ain&(0x01<<AI_WATER_VALVE_FB))==0)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		//Alair,20170318��AI����
//		if(g_sys.config.dev_mask.ain&0x8000)
//		{
//				return(ALARM_ACL_CLEARED);
//		}			
//    if((sys_get_pwr_sts() == 0)||((sys_get_remap_status(WORK_MODE_STS_REG_NO, COOLING_STS_BPOS) == 0)&&(sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) == 0)))
//    {
//        return(ALARM_ACL_HOLD);
//    }
//		
//		if(g_sys.status.aout[AO_WATER_VALVE] > g_sys.status.ain[AI_WATER_VALVE_FB])
//		{
//				meter = g_sys.status.aout[AO_WATER_VALVE] - g_sys.status.ain[AI_WATER_VALVE_FB]; 
//		}
//		else
//		{
//				meter =  g_sys.status.ain[AI_WATER_VALVE_FB] - g_sys.status.aout[AO_WATER_VALVE];
//		}
//		
//	
//		max = (int16_t)(g_sys.config.alarm[ACL_COIL_VALVE_DEFAULT].alarm_param);
//		return(compare_calc( meter,0,max,THR_MAX_TYPE));
			return(ALARM_ACL_CLEARED);	
}

//ACL_COIL_BLOCKING  
static	uint16_t acl73(alarm_acl_status_st* acl_ptr)
{
//	
//	extern local_reg_st l_sys;
//	int16_t meter;
//	int16_t max;

//	//	//�����ж�
//	if((g_sys.config.general.cool_type == COOL_TYPE_MODULE_WIND)||
//		(g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND))
//	{
//			return(ALARM_ACL_CLEARED);
//	} 
//	
//	if(acl_clear(acl_ptr))
//	{
//			return(ALARM_ACL_CLEARED);
//	}
//	
//	if((sys_get_pwr_sts() == 0)||(l_sys.l_fsm_state[WATERVALVE_FSM_STATE]!= WATERVALVE_FSM_STATE_NORMAL))
//	{
//			return(ALARM_ACL_HOLD);
//	}
////	// �ж������ص�����£�������������
////	if(g_sys.config.dev_mask.din[0]&(0x01<<DI_BLOCKING_BPOS))
////	{
////			data = sys_get_di_sts(DI_BLOCKING_BPOS);	
////		  return(io_calc( data,IO_CLOSE));
////	}
////	else
//	{
//			//�䱸�ط紫����
//			if(g_sys.status.sys_tem_hum.supply_air_temp == ABNORMAL_VALUE)
//			{
//					return(ALARM_ACL_CLEARED);
//			}
//			//ˮ���ص�����·��ر���
//			if(g_sys.status.aout[AO_WATER_VALVE] == 0)
//			{
//				return(ALARM_ACL_HOLD);
//			}
//			meter = (int16_t) g_sys.status.sys_tem_hum.supply_air_temp ;
//			
//			max = (int16_t)(g_sys.config.alarm[ACL_COIL_BLOCKING].alarm_param);
//			
//			alarm_inst.alarm_sts[acl_ptr->id].alram_value=meter;
//			
//			return(compare_calc( meter,0,max,THR_MAX_TYPE));	
//	}
			return(ALARM_ACL_CLEARED);	
}
//ACL_FAN_OVERLOAD6  
static	uint16_t acl74(alarm_acl_status_st* acl_ptr)
{	
			return(ALARM_ACL_CLEARED);
//		uint8_t data;
//		//�����ж�
//		if((g_sys.config.general.cool_type != COOL_TYPE_COLUMN_WATER)&&(g_sys.config.general.cool_type != COOL_TYPE_COLUMN_WIND) && (devinfo_get_compressor_cnt() != 1) )
//		{
//				return(ALARM_ACL_CLEARED);
//		} 
//		//remove loc
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.fan.num < 6)
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_HOLD);
//		}
//		if(sys_get_do_sts(DO_FAN_BPOS)==0)
//		{
//				return(ALARM_ACL_HOLD);
//		}		
//		data = sys_get_di_sts(DI_FAN06_OD_BPOS);
//		return(io_calc( data,IO_CLOSE));
}
//ACL_FAN_OVERLOAD7                 
static	uint16_t acl75(alarm_acl_status_st* acl_ptr)
{
				return(ALARM_ACL_CLEARED);
//	uint8_t data;
//		//�����ж�
////		if(g_sys.config.general.cool_type != COOL_TYPE_COLUMN_WATER)
//		if((g_sys.config.general.cool_type != COOL_TYPE_COLUMN_WATER)&&(g_sys.config.general.cool_type != COOL_TYPE_COLUMN_WIND))
//		{
//				return(ALARM_ACL_CLEARED);
//		} 
//		//Alair,20170318��AI����
//		if(!(g_sys.config.dev_mask.ain&0x8000))
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		//remove loc
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.fan.num < 7)
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_HOLD);
//		}
//		if(sys_get_do_sts(DO_FAN_BPOS)==0)
//		{
//				return(ALARM_ACL_HOLD);
//		}
//		data = sys_get_di_sts(DI_FAN07_OD_BPOS);
//		return(io_calc( data,IO_CLOSE));
	
}
//ACL_FAN_OVERLOAD8             
static	uint16_t acl76(alarm_acl_status_st* acl_ptr)
{
			return(ALARM_ACL_CLEARED);	
//		uint8_t data;
//		//�����ж�
////		if(g_sys.config.general.cool_type != COOL_TYPE_COLUMN_WATER)
//		if((g_sys.config.general.cool_type != COOL_TYPE_COLUMN_WATER)&&(g_sys.config.general.cool_type != COOL_TYPE_COLUMN_WIND))
//		{
//				return(ALARM_ACL_CLEARED);
//		} 
//		//Alair,20170318��AI����
//		if(!(g_sys.config.dev_mask.ain&0x8000))
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		//remove loc
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.fan.num < 8)
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		if(sys_get_pwr_sts() == 0)
//		{
//				return(ALARM_ACL_HOLD);
//		}
//		if(sys_get_do_sts(DO_FAN_BPOS)==0)
//		{
//				return(ALARM_ACL_HOLD);
//		}
//		data = sys_get_di_sts(DI_FAN08_OD_BPOS);
//		return(io_calc( data,IO_CLOSE));
}

//ACL_FAN_OT6 
static	uint16_t acl77(alarm_acl_status_st* acl_ptr)
{
				return(ALARM_ACL_CLEARED);
//		uint32_t run_time;
//		int16_t max;
//		//�����ж�
//		if((g_sys.config.general.cool_type != COOL_TYPE_COLUMN_WATER)||((g_sys.config.dev_mask.din[0]&(0x01<<DI_FAN06_OD_BPOS)) == 0))
//		{
//				return(ALARM_ACL_CLEARED);
//		} 
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.fan.num < 6)
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FAN6_DUMMY_BPOS].low,g_sys.status.run_time[DO_FAN6_DUMMY_BPOS].high);
//		max = g_sys.config.alarm[ACL_FAN_OT1].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));
}

//ACL_FAN_OT7  
static	uint16_t acl78(alarm_acl_status_st* acl_ptr)
{
				return(ALARM_ACL_CLEARED);
//		uint32_t run_time;
//		int16_t max;
//		//�����ж�
//		if((g_sys.config.general.cool_type != COOL_TYPE_COLUMN_WATER)||((g_sys.config.dev_mask.din[0]&(0x01<<DI_FAN07_OD_BPOS)) == 0))
//		{
//				return(ALARM_ACL_CLEARED);
//		} 
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.fan.num < 7)
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FAN7_DUMMY_BPOS].low,g_sys.status.run_time[DO_FAN7_DUMMY_BPOS].high);
//		max = g_sys.config.alarm[ACL_FAN_OT1].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));
}
//ACL_FAN_OT8  
static	uint16_t acl79(alarm_acl_status_st* acl_ptr)
{
				return(ALARM_ACL_CLEARED);
//		uint32_t run_time;
//		int16_t max;
//		//�����ж�
//		if((g_sys.config.general.cool_type != COOL_TYPE_COLUMN_WATER)||((g_sys.config.dev_mask.din[0]&(0x01<<DI_FAN08_OD_BPOS)) == 0))
//		{
//				return(ALARM_ACL_CLEARED);
//		} 
//		//����ȷ��
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(g_sys.config.fan.num < 8)
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		run_time = dev_runingtime(g_sys.status.run_time[DO_FAN8_DUMMY_BPOS].low,g_sys.status.run_time[DO_FAN8_DUMMY_BPOS].high);
//		max = g_sys.config.alarm[ACL_FAN_OT1].alarm_param;
//		alarm_inst.alarm_sts[acl_ptr->id].alram_value = run_time;
//		return(compare_calc( run_time,0,max,THR_MAX_TYPE));
}
//ACL_WATER_PUMP_HI_LIVEL
static	uint16_t acl80(alarm_acl_status_st* acl_ptr)
{
				return(ALARM_ACL_CLEARED);
//		uint8_t data;
//	
//	
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		
//		}
//		//�����ж�
//		if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER)||
//			((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND) && (g_sys.config.compressor.type != COMP_QABP)) )
//		{
//				
//				if(g_sys.status.dout_bitmap & (0x0001 << DO_PUMMP_BPOS))
//				{
//						data = sys_get_di_sts(DI_COND_HI_LEVEL_BPOS);	
//						return(io_calc( data,IO_CLOSE));
//				}
//				else
//				{
//						return(ALARM_ACL_HOLD);
//				}
//			
//		} 
//		else
//		{
//					return(ALARM_ACL_CLEARED);
//		}
//		
//		
}

//ACL_HUM_DEFAULT
static uint16_t acl81(alarm_acl_status_st* acl_ptr)
{
//		uint8_t data;
//	
//		{
//			if(acl_clear(acl_ptr))
//			{
//					sys_option_di_sts(DI_HUM_DEFAULT_BPOS,0);
//					return(ALARM_ACL_CLEARED);
//			}
//			if(sys_get_pwr_sts() == 0)
//			{
//				 sys_option_di_sts(DI_HUM_DEFAULT_BPOS,0);
//				 return(ALARM_ACL_CLEARED);
//			}
//			
//			data = sys_get_di_sts(DI_HUM_DEFAULT_BPOS);	
//		}
//		return(io_calc( data,IO_CLOSE));			
			return(ALARM_ACL_CLEARED);	
}

//ACL_FLOW_DIFF
static  uint16_t acl82(alarm_acl_status_st* acl_ptr)
{
//		extern local_reg_st l_sys;
//	
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(sys_get_pwr_sts() == 0)
//		{
//			  return(ALARM_ACL_CLEARED);
//		}
//		// ���ģʽ�ж�
//		if(g_sys.config.fan.mode != FAN_MODE_PRESS_DIFF)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//				//Alair,20170318��AI����
//		if(g_sys.config.dev_mask.ain&0x8000)
//		{
//				return(ALARM_ACL_CLEARED);
//		}	
//		//fan_speed == max_speed
//		if( l_sys.ao_list[AO_EC_FAN][BITMAP_REQ] >= g_sys.config.fan.max_speed )
//		{
//				if(g_sys.status.ain[AI_SENSOR1] < (g_sys.config.fan.set_flow_diff - g_sys.config.fan.flow_diff_deadzone))
//				{
//							return(ALARM_ACL_TRIGGERED);
//				}
//				else
//				{
//						 return(ALARM_ACL_CLEARED);
//				}
//		}
//		else
//		{
//				 return(ALARM_ACL_HOLD);
//		}
			return(ALARM_ACL_CLEARED);	
}
//ACL_HUM_OD
static  uint16_t acl83(alarm_acl_status_st* acl_ptr)
{
			return(ALARM_ACL_CLEARED);
//		uint8_t data;
//		if(acl_clear(acl_ptr))
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(sys_get_pwr_sts() == 0)
//		{
//			  return(ALARM_ACL_CLEARED);
//		}	
//		if(g_sys.config.humidifier.hum_type != HUM_TYPE_INFRARED)
//		{
//				return(ALARM_ACL_CLEARED);
//		}
//		if(((g_sys.status.dout_bitmap & (0x0001 << DO_HUM_BPOS)) == 0)||(l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE]==HUM_FSM_STATE_WARM))
//		{
//					return(ALARM_ACL_HOLD);
//		}
//		data = sys_get_di_sts(DI_RED_HIGH_TEMP_BPOS);	
//		data = io_calc( data,IO_CLOSE);
//		
//		
//		return(data);			
}

//ACL_INV_HIPRESS
static  uint16_t acl84(alarm_acl_status_st* acl_ptr)
{
//	extern local_reg_st 	l_sys;
////	time_t now;
//	if(acl_clear(acl_ptr))
//	{
//			g_sys.status.inv_compress_alarm.inv_start_time[0] = 0xffffffff;
//			g_sys.status.inv_compress_alarm.inv_start_time[1] = 0xffffffff;
//			g_sys.status.inv_compress_alarm.inv_start_time[2] = 0xffffffff;
//			g_sys.status.inv_compress_alarm.inv_stop_time[0] = 0xffffffff;
//			g_sys.status.inv_compress_alarm.inv_stop_time[1] = 0xffffffff;
//			g_sys.status.inv_compress_alarm.inv_stop_time[2] = 0xffffffff;
//			g_sys.status.inv_compress_alarm.inv_hipress_flag = 0;
//			g_sys.status.inv_compress_alarm.inv_hipress_tmp  = 0;
//			return(ALARM_ACL_CLEARED);
//	}

//		//Alair,20170301
//	if(g_sys.config.compressor.type == COMP_QABP)//��Ƶ
//	{
//			if(g_sys.status.inv_compress_alarm.avg_hi_press[GET_AVG]>g_sys.config.alarm[ACL_INV_HIPRESS].alarm_param)//��ѹѹ��������ֵ
//			{
//					return(1);				
//			}
//			//Alair,20170311,ȡ�����θ�ѹͣѹ��
///*			
//			//���θ�ѹִ���������
//			if(	g_sys.status.inv_compress_alarm.inv_hipress_flag == 0)
//			{
//					return(ALARM_ACL_HOLD);
//			}	
//			if(get_inv_comp_freq_down_signal(HIGH_PRESS) == 1)
//			{
//				get_local_time(&now);
//		//		rt_kprintf("time: %d\n",(now-g_sys.status.inv_compress_alarm.inv_start_time[0]));
//				if((g_sys.status.inv_compress_alarm.inv_hipress_flag == 1) && (g_sys.status.inv_compress_alarm.inv_start_time[0] != 0xffffffff)
//					&& ((now-g_sys.status.inv_compress_alarm.inv_start_time[0]) <= 7200))
//				{
//		//			rt_kprintf("ok2\n");
//					return (g_sys.status.inv_compress_alarm.inv_hipress_flag);
//				}
//				else if((g_sys.status.inv_compress_alarm.inv_alarm_counter <= 2) && (g_sys.status.inv_compress_alarm.inv_start_time[0] != 0xffffffff)
//					&& ((now-g_sys.status.inv_compress_alarm.inv_start_time[0]) > 7200))
//				{
//					g_sys.status.inv_compress_alarm.inv_start_time[0] = 0xffffffff;
//					g_sys.status.inv_compress_alarm.inv_start_time[1] = 0xffffffff;
//					g_sys.status.inv_compress_alarm.inv_start_time[2] = 0xffffffff;
//					g_sys.status.inv_compress_alarm.inv_stop_time[0] = 0xffffffff;
//					g_sys.status.inv_compress_alarm.inv_stop_time[1] = 0xffffffff;
//					g_sys.status.inv_compress_alarm.inv_stop_time[2] = 0xffffffff;
//					g_sys.status.inv_compress_alarm.inv_hipress_flag = 0;
//					g_sys.status.inv_compress_alarm.inv_hipress_tmp  = 0;
//					g_sys.status.inv_compress_alarm.inv_alarm_counter = 0;
//				}
//				return(g_sys.status.inv_compress_alarm.inv_hipress_flag);
//			}
//			return(g_sys.status.inv_compress_alarm.inv_hipress_flag);	
//*/			
//	}	
			return(ALARM_ACL_CLEARED);
}
//ACL_WATERPAN
//static  uint16_t acl85(alarm_acl_status_st* acl_ptr)
//{
//		return(ALARM_ACL_CLEARED);
////	uint8_t data;
////	if(acl_clear(acl_ptr))
////	{
////			return(ALARM_ACL_CLEARED);
////	}
////	data = sys_get_di_sts(DI_WATER_PAN_BPOS);
////	return(io_calc( data,IO_CLOSE));
//}



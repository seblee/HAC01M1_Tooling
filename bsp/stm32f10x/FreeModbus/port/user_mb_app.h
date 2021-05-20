#ifndef	USER_APP
#define USER_APP
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbutils.h"
#include "sys_conf.h"

/* -----------------------Slave Defines -------------------------------------*/
#define 				 S_REG_HOLDING_START           0
#define 				 S_REG_HOLDING_NREGS           112
#define 				 REG_HOLDING_WRITE_NREGS       5//可写范围+
//从机模式：在保持寄存器中，各个地址对应的功能定义
#define          S_HD_RESERVE                     0		  //保留
#define          S_HD_CPU_USAGE_MAJOR             1         //当前CPU利用率的整数位
#define          S_HD_CPU_USAGE_MINOR             2         //当前CPU利用率的小数位

//从机模式：在输入寄存器中，各个地址对应的功能定义
#define          S_IN_RESERVE                     0		  //保留


/* -----------------------Master Defines -------------------------------------*/
#define 				 M_REG_HOLDING_START           0
#define 				 M_REG_HOLDING_NREGS           45
#define 				 M_REG_HOLDING_USR_START       20
#define 				 M_REG_HOLDING_USR_END       		23

//主机模式：在保持寄存器中，各个地址对应的功能定义
#define          M_HD_RESERVE                     0		  //保留

//主机模式：在输入寄存器中，各个地址对应的功能定义
#define          M_IN_RESERVE                     0		  //保留

//主机模式：在线圈中，各个地址对应的功能定义
#define          M_CO_RESERVE                     2		  //保留

//主机模式：在离散输入中，各个地址对应的功能定义
#define          M_DI_RESERVE                     1		  //保留

//#define          MBM_DEV_PM1_ADDR                 10        // power mode 1
//#define          MBM_DEV_PM2_ADDR                 11        // power mode 1

#define   ERROR_CNT_MAX   100		 //异常次数
enum
{
    MBM_DEV_A1_ADDR	=0,					//return air temp&hum board 1 modbus address
    MBM_DEV_A2_ADDR	, 					//supply air temp&hum board 1 modbus address
    MBM_DEV_A3_ADDR	,					  //return air temp&hum board 2 modbus address
    MBM_DEV_A4_ADDR	, 					//supply air temp&hum board 2 modbus address
    MBM_DEV_A5_ADDR	,					  //return air temp&hum board 3 modbus address
    MBM_DEV_A6_ADDR	,           //supply air temp&hum board 3 modbus address
    MBM_DEV_A7_ADDR	,					  //return air temp&hum board 4 modbus address
    MBM_DEV_A8_ADDR	,					  //supply air temp&hum board 4 modbus address
    MBM_DEV_H_ADDR,					  //humidifier board modbus address
    MBM_DEV_P_ADDR	, 				//power board modbus address
    MBM_DEV_EX_FAN_ADDR	, 				//
    MBM_DEV_EEV_ADDR	, 				//
		MBM_DEV_PT_ADDR,
//    MBM_DEV_EX_FAN2_ADDR	, 				//
//    MBM_DEV_INV_ADDR	, 				//
    MB_MASTER_TOTAL_SLAVE_NUM 		//Total
};

#define MB_DEV_AEC01F1_ADDR		MBM_DEV_H_ADDR					  //AEC01F1 board modbus address
#define MB_MASTER_DISCRETE_OFFSET    MBM_DEV_EX_FAN_ADDR  //

#define MB_DEV_TH_ADDR_START    MBM_DEV_A1_ADDR  //1
#define MB_DEV_TH_ADDR_END    	MBM_DEV_A8_ADDR  //8

#define 				 MBM_REG_ADDR_CNT_ADDR						0
#define 				 MBM_REG_DEV_TPYE_ADDR						1
#define 				 MBM_REG_SOFT_VER_ADDR						3
#define 				 MBM_REG_HARD_VER_ADDR						4
#define 				 MBM_REG_SERIAL_ADDR							5
#define 				 MBM_REG_MAN_DATE_ADDR						9
#define 				 MBM_REG_COMM_AD_ADDR							11
#define 				 MBM_REG_BAUDRATE_ADDR						12
#define 				 MBM_REG_STATUS_ADDR							18
#define 				 MBM_REG_CONF_ADDR								19

//AEC01F1 board reg definition
#define					 MB_CFG_FANOUT											12
#define					 MB_AO															15
#define					 MB_DI_Bitmap												16

//humidifier control board reg definition
#define					 MBM_DEV_H_REG_STATUS_ADDR					20
#define					 MBM_DEV_H_REG_CONDUCT_ADDR					21
#define					 MBM_DEV_H_REG_HUMCUR_ADDR					22
#define					 MBM_DEV_H_REG_WT_LV_ADDR					23

//hum&temp control board reg definition
#define          MBM_DEV_H_REG_HT_STATUS_ADDR                       20
#define					 MBM_DEV_H_REG_TEMP_ADDR					21
#define					 MBM_DEV_H_REG_HUM_ADDR						22
//power control board reg definition
#define          MBM_DEV_H_REG_P0WER_STATUS_ADDR  					20
#define					 MBM_DEV_H_REG_PA_VOLT_ADDR					21
#define					 MBM_DEV_H_REG_PB_VOLT_ADDR					22
#define					 MBM_DEV_H_REG_PC_VOLT_ADDR					23
#define					 MBM_DEV_H_REG_FREQ_ADDR					24
#define					 MBM_DEV_H_REG_PE_ADDR						25//相序
#define					 MBM_DEV_H_REG_PA_CUR_ADDR				26//
#define					 MBM_DEV_H_REG_PB_CUR_ADDR				27//
#define					 MBM_DEV_H_REG_PC_CUR_ADDR				28//
//outside control bord reg definition
#define          MBM_DEV_H_REG_OSC_STATUS_ADDR		    20
#define          MBM_DEV_H_REG_OSC_DI_BIT_MAP_ADDR		21
#define          MBM_DEV_H_REG_OSC_PRESSOR_ADDR		    22
#define          MBM_DEV_H_REG_OSC_TEMP_ADDR		    23
#define          MBM_DEV_H_REG_OSC_FAN_SPD_ADDR		    24
#define          MBM_DEV_H_REG_OSC_DO_ADDR		        25
#define          MBM_DEV_H_REG_OSC_CMP_SPD_ADDR		    26


// POWER reg definition\

// 0x0002
// 0x1002
// 0x1003
// 0x2000
// 0x1000
// 0x1001

#define         MBM_POWER_STATUS_ADDR           0X01        
#define         MBM_POWER_SET_V_ADDR            0X06        
#define         MBM_POWER_SET_I_ADDR            0X08        
#define         MBM_POWER_READ_ERRO             0X03        
#define         MBM_POWER_OUTPUT_V_ADDR         0X12        
#define         MBM_POWER_OUTPUT_I_ADDR         0X15   

//#define         MBM_POWER_STATUS_ADDR         0x0002
//#define         MBM_POWER_OUTPUT_V_ADDR       0x1000
//#define         MBM_POWER_OUTPUT_I_ADDR       0x1001
//#define         MBM_POWER_SET_V_ADDR          0x1002
//#define         MBM_POWER_SET_I_ADDR          0x1003
//#define         MBM_POWER_READ_ERRO           0x2000


#define         MBM_DEV_PWR_STATUS_ADDR         0x0000//0x8000
#define         MBM_DEV_PWR_SET_V_ADDR          0x0001// 0x1001  
#define         MBM_DEV_PWR_SET_I_ADDR          0x0002// 0x1001
#define         MBM_DEV_PWR_ERO_ADDR            0x0003// 0x1001    
#define         MBM_DEV_PWR_V_ADDR              0x0004// 0x1001    
#define         MBM_DEV_PWR_I_ADDR              0x0005// 0x1001    


//modbus master FSM definition
#define					MBM_FSM_IDLE			0x01
#define					MBM_FSM_SYNC			0x02
#define					MBM_FSM_UPDATE		0x04
#define					MBM_FSM_SEND      0x05


//
#define					MBM_POLL_TIMEOUT_THRESHOLD 				5
#define					MBM_INIT_TIMEOUT_THRESHOLD 				5
#define					MBM_UPDATE_TIMEOUT_THRESHOLD 			5

typedef struct
{
		uint16_t poll;
		uint16_t init;
		uint16_t update;
	  uint16_t discrete;
}mbm_bitmap_st;

typedef struct
{
		uint16_t poll;
		uint16_t init;
		uint16_t update;
}mbm_timeout_st;

typedef struct
{
		uint16_t poll;
		uint16_t init;
		uint16_t update;
}mbm_errcnt_st;

/****************************************************************
mem_dev_reg数据说明：
dev_reg_addr 设备寄存器实际地址
mem_addr 设备对应的内存映射地址
mem_addr & dev_reg_addr 必须一一对应
***************************************************************/
typedef struct
{
	uint16_t dev_reg_addr;//通信读取地址
	uint16_t NRegs;
	uint16_t* data_ptr;
	uint16_t 	J_Offset_Addr;//跳变内存地址
}mbm_read_dev_reg;

typedef struct
{
	uint16_t dev_reg_addr;
	uint16_t Conf_addr;
	uint16_t* conf_ptr;
	uint8_t* conf_Flag;
	uint16_t* status_ptr;
}mbm_write_dev_reg;


typedef struct
{
		uint8_t  mbm_addr;
		uint8_t reg_rd_cnt;
		mbm_read_dev_reg *rd_pt;
		uint8_t reg_w_cnt;
	 mbm_write_dev_reg *w_pt;
}mbm_read_st;

typedef struct
{
		mbm_bitmap_st bitmap;
		mbm_errcnt_st errcnt;
		mbm_timeout_st timeout;
		uint16_t mbm_fsm;
		uint8_t Write_Enable;
}mbm_dev_st;

//typedef struct
//{
//		uint8_t  mbm_addr;
//		uint8_t  mbm_fun_code;
//		uint16_t reg_addr;
//		uint16_t reg_value;	
////		uint8_t  mbm_NRegs;
////		uint8_t  mbm_WriteType;
//}mbm_data_st;

typedef struct
{
		uint8_t  mbm_addr;
		uint8_t  mbm_fun_code;
		uint16_t reg_addr;
		uint16_t reg_value;	
		uint8_t  mbm_NRegs;
}mbm_data_st;

typedef enum
{
		MB_WRITE_SINGLE, //写单个寄存器                  
		MB_WRITE_MULITE, //写多个寄存器               
}eMBWriteMode;



void mbm_sts_update(sys_reg_st* gds_ptr);
void mbs_sts_update(void);// 更新本地变量到协议栈寄存器中
#endif

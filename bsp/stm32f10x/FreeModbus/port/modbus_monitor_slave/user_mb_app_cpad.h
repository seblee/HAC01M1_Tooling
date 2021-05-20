#ifndef	USER_APP
#define USER_APP
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb_monitor.h"
#include "mb_m.h"
#include "mbconfig.h"
#include "mbframe.h"
#include "mbutils.h"
#include "sys_conf.h"
/* -----------------------Slave Defines -------------------------------------*/
#define 				 MONITOR_REG_HOLDING_START           0
#define 				 MONITOR_REG_HOLDING_NREGS           37
#define 				 REG_HOLDING_WRITE_NREGS       5//��д��Χ+
//�ӻ�ģʽ���ڱ��ּĴ����У�������ַ��Ӧ�Ĺ��ܶ���
#define          S_HD_RESERVE                     0		  //����
#define          S_HD_CPU_USAGE_MAJOR             1         //��ǰCPU�����ʵ�����λ
#define          S_HD_CPU_USAGE_MINOR             2         //��ǰCPU�����ʵ�С��λ

//�ӻ�ģʽ��������Ĵ����У�������ַ��Ӧ�Ĺ��ܶ���
#define          S_IN_RESERVE                     0		  //����




void mbm_sts_update(sys_reg_st* gds_ptr);
void mbs_sts_update(void);// ���±��ر�����Э��ջ�Ĵ�����
#endif

#ifndef _PASSWORD_H
#define _PASSWORD_H

#include "sys_conf.h"
enum
{
	WORK_MODE_FSM_OPEN = 0xE1,//��ȫ��Ȩ
	WORK_MODE_FSM_MANAGE = 0xD2,//��ʱ�ܿ�
	WORK_MODE_FSM_LOCK = 0xC3,//�����ܿ�
	WORK_MODE_FSM_LIMIT = 0xB4,//��ʱ����
};

enum//ʣ��ʱ��
{
	WORK_REMAIN_0=0x00,//0��
	WORK_REMAIN_1=0x01,//1��
	WORK_REMAIN_3=0x03,//3��
	WORK_REMAIN_7=0x07,//7��
	WORK_REMAIN_15=0x0F,//15��
	WORK_REMAIN_30=0x1E,//30��
};

enum//��ʱʣ��ʱ��
{
	WORK_LIMIT_ZERO=0x01,//0��
	WORK_LIMIT_DAY_1=0x02,//1��
	WORK_LIMIT_DAY_3=0x04,//3��
	WORK_LIMIT_DAY_7=0x08,//7��
	WORK_LIMIT_DAY_15=0x10,//15��
	WORK_LIMIT_DAY_30=0x20,//30��
	WORK_LIMIT_LOCK=0x100,//����
	WORK_LIMIT_CATION=0x400,//�澯����,90�Ÿ澯״̬��
	WORK_PASS_CLEAR=0x3F,//����澯
	WORK_PASS_ALL=0x13F,//����澯
	WORK_PASS_SET=0x1000,//
};
#define LIMIT_DAY_MAX 1000
//5�����ܹܿ�
enum
{
	GRADE_POWERON = 0xA1,//�׶�0������
	GRADE_1 = 0xB2,//�׶�1
	GRADE_2 = 0xC3,//�׶�2
	GRADE_3 = 0xD4,//�׶�3
	GRADE_4 = 0xE5,//�׶�4
	GRADE_LOCK = 0xF6,//�����ػ�
	GRADE_OPEN = 0x6F,//��ȫ��Ȩ
};

void init_work_mode(void);

void work_mode_manage(void);

uint8_t passward_compare(uint8_t *st1,uint8_t *st2, uint8_t len);
uint8_t cpad_work_mode(uint8_t work_mode,uint16_t day_limit);

uint8_t  write_passward (uint8_t*password, uint8_t work_mode,uint16 limit_time);
uint8_t get_work_mode_power_state(void);

uint8_t  Write_Controlpassward (uint16_t*MBBuffer);
uint8_t  Passward_Verify(uint16_t MBBuff);
#endif


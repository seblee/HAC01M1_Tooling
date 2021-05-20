#include <rtthread.h>
#include "team.h"
#include "calc.h"
#include "sys_conf.h"
#include "local_status.h"
#include "req_execution.h"
#include "alarm_acl_funcs.h"
#include "sys_status.h"
#include "dio_bsp.h"
#include "rtc_bsp.h"
#include <stdlib.h>
#include <math.h>
#include "user_mb_app.h"
#include "fifo.h"
#include "global_var.h"
#include "event_record.h"
#include <stdio.h>

enum
{
    PPE_FSM_POWER_ON,
    PPE_FSM_PN_SWITCH,
    PPE_FSM_NO_CONF,
    PPE_FSM_NORMAL,
    PPE_FSM_REVERSE
};

#define INIT_DELAY 10

#define COMPRESSOR_BITMAP_MASK_POS 0x0006
#define HEATER_BITMAP_MASK_POS 0x0418
#define HUMIDIFIER_BITMAP_MASK_POS 0x0020
#define DEHUMER_BITMAP_MASK_POS 0x0300

#define HUM_CHECKE_INTERVAL 12 * 3600

enum
{
    RUNING_STATUS_COOLING_BPOS = 0,
    RUNING_STATUS_HEATING_BPOS,
    RUNING_STATUS_HUMIDIFYING_BPOS,
    RUNING_STATUS_DEHUMING_BPOS,
};

enum
{
    COMPRESSOR_SIG_HOLD = 0,
    COMPRESSOR_SIG_ON,
    COMPRESSOR_SIG_OFF,
    COMPRESSOR_SIG_ERR,
};

enum
{
    WATER_COOLED_SIG_HOLD = 0,
    WATER_COOLED_SIG_ON,
    WATER_COOLED_SIG_OFF,
    WATER_COOLED_SIG_ERR,
};

enum
{
    HEATER_SIG_IDLE = 0,
    HEATER_SIG_L1,
    HEATER_SIG_L2,
    HEATER_SIG_L3,
    HEATER_SIG_ERR,
};

enum
{
    HEATER_FSM_IDLE = 0,
    HEATER_FSM_L1,
    HEATER_FSM_L2,
    HEATER_FSM_L3,
    HEATER_FSM_ERR,
};

enum
{
    FAN_SIG_IDLE = 0,
    FAN_SIG_START,
    FAN_SIG_STOP
};

typedef struct
{
    uint16_t time_out;
    uint16_t flush_delay_timer;
    uint16_t hum_fill_cnt;
    uint32_t hum_timer;
    uint32_t check_timer;
    uint8_t check_fill_flag;
    uint8_t check_drain_flag;
    uint8_t check_flag;
    uint16_t warm_time;
} hum_timer;

static hum_timer hum_delay_timer;

/************************外风机**************************/

typedef enum _P_RANGE_POINT_
{
    P_SET_POINT,    //???????
    P_OFF_POINT,    //?????????,??????1BAR
    P_FLAT_POINT,   //????????
    P_FLAT2_POINT,  //?????????????,????3BAR
    P_STEP_POINT,   //????????
    MAXPOINTNUM
} P_RANGE_POINT;

#define P_INVALID_VAL 8888
#define P_ADJ_TIMES 250
#define P_ADJ_ST 30

//扩大10倍，单位BAR
#define MAXDELTAP1 1
#define MAXDELTAP2 5

#define EX_FAN_NUM 2
#define SPEED_DIF 5

static int16_t g_i16PDeltaRef[EX_FAN_NUM];  //压力变化参考值
// static uint16_t  g_u16SamDlyFlag[EX_FAN_NUM];	//
static int16_t g_i16Ex_Fan_Speed[EX_FAN_NUM];  //

//需求比特位操作函数
void req_bitmap_op(uint8_t component_bpos, uint8_t action)
{
    extern local_reg_st l_sys;
    if (action == 0)
    {
        l_sys.bitmap[BITMAP_REQ] &= ~(0x0001 << component_bpos);
    }
    else
    {
        l_sys.bitmap[BITMAP_REQ] |= (0x0001 << component_bpos);
    }
}

//需求模拟输出操作函数
static void req_ao_op(uint8_t component_bpos, int16_t value, uint8_t u8Sig)
{
    extern local_reg_st l_sys;
    extern sys_reg_st g_sys;
    uint16_t u16Step;
    int16_t i16Ao_list;
    // Alair,20170329
    if (value <= 0)
    {
        value = 0;
    }
    if ((g_sys.config.general.diagnose_mode_en == 0) && (g_sys.config.general.testing_mode_en == 0))
    {  // normal mode
        switch (component_bpos)
        {
            case (AO_EC_FAN): {
                if (u8Sig == FAN_SIG_STOP)
                {
                    value = 0;
                }
                {
                    i16Ao_list = l_sys.ao_list[component_bpos][BITMAP_REQ];
                    u16Step    = g_sys.config.fan.adjust_step;
                    if (abs(value - i16Ao_list) > u16Step)
                    {
                        if (value > i16Ao_list)
                        {
                            i16Ao_list += u16Step;
                        }
                        else
                        {
                            i16Ao_list -= u16Step;
                        }
                    }
                    else
                    {
                        i16Ao_list = value;
                    }
                }

                if ((u8Sig == FAN_SIG_STOP) || l_sys.Fan.Fan_Start == FALSE)  // close
                {
                    l_sys.ao_list[component_bpos][BITMAP_REQ] = i16Ao_list;
                }
                else
                {
                    l_sys.ao_list[component_bpos][BITMAP_REQ] =
                        lim_min_max(g_sys.config.fan.min_speed, g_sys.config.fan.max_speed, i16Ao_list);
                }
                //								rt_kprintf("value = %d, BITMAP_REQ = %d, i16Ao_list = %d, u16Step =
                //%d\n", value, l_sys.ao_list[component_bpos][BITMAP_REQ],i16Ao_list,u16Step);
                break;
            }
            case (AO_EC_COMPRESSOR): {
                if (1)
                {
                    l_sys.ao_list[component_bpos][BITMAP_REQ] = value;
                }
                break;
            }
            case (AO_WATER_VALVE): {
                if (!g_sys.config.water_valve.auto_mode_en)
                {
                    l_sys.ao_list[component_bpos][BITMAP_REQ] = value;
                }
                // calc step value
                else if (abs(value - l_sys.ao_list[component_bpos][BITMAP_REQ]) >
                         g_sys.config.water_valve.act_threashold)
                {
                    if (value > l_sys.ao_list[component_bpos][BITMAP_REQ])
                    {
                        l_sys.ao_list[component_bpos][BITMAP_REQ] += g_sys.config.water_valve.act_threashold;
                    }
                    else
                    {
                        l_sys.ao_list[component_bpos][BITMAP_REQ] -= g_sys.config.water_valve.act_threashold;
                    }
                }
                else
                {
                    l_sys.ao_list[component_bpos][BITMAP_REQ] = value;
                }
                //										rt_kprintf("temp = %d, ao = %d\n", temp,
                // l_sys.ao_list[component_bpos][BITMAP_REQ]);
                break;
            }
            default: {
                l_sys.ao_list[component_bpos][BITMAP_REQ] = value;
                break;
            }
        }
    }
    else
    {  // dianose or test mode, output directly
        l_sys.ao_list[component_bpos][BITMAP_REQ] = value;
    }
}

// static void req_pwm_op(uint8_t component_bpos, int16_t value)
//{
//		extern local_reg_st l_sys;

//		l_sys.pwm_list[component_bpos][BITMAP_REQ] = value;
//}

/**************************************
COMPRESSOR requirement execution function
**************************************/

static uint16_t compressor_signal_gen(int16_t req_temp, int16_t req_hum, uint8_t* comp1_sig, uint8_t* comp2_sig)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    uint8_t comp1_alarm_flag, comp2_alarm_flag;
    uint16_t compressor_count;

    uint32_t comp1_runtime, comp2_runtime;

    if ((g_sys.config.general.alarm_bypass_en == 0) && ((l_sys.bitmap[BITMAP_REQ] & (0x0001 << DO_COMP1_BPOS)) != 0) &&
        ((l_sys.bitmap[BITMAP_ALARM] & (0x0001 << DO_COMP1_BPOS)) == 0) &&
        ((l_sys.bitmap[BITMAP_MASK] & (0x0001 << DO_COMP1_BPOS)) != 0))
        comp1_alarm_flag = 1;
    else
        comp1_alarm_flag = 0;

    if ((g_sys.config.general.alarm_bypass_en == 0) && ((l_sys.bitmap[BITMAP_REQ] & (0x0001 << DO_COMP2_BPOS)) != 0) &&
        ((l_sys.bitmap[BITMAP_ALARM] & (0x0001 << DO_COMP2_BPOS)) == 0) &&
        ((l_sys.bitmap[BITMAP_MASK] & (0x0001 << DO_COMP2_BPOS)) != 0))
        comp2_alarm_flag = 1;
    else
        comp2_alarm_flag = 0;

    compressor_count = devinfo_get_compressor_cnt();

    comp1_runtime = g_sys.status.run_time[DO_COMP1_BPOS].high;
    comp1_runtime = comp1_runtime << 4 | (g_sys.status.run_time[DO_COMP1_BPOS].low >> 12);

    comp2_runtime = g_sys.status.run_time[DO_COMP2_BPOS].high;
    comp2_runtime = comp2_runtime << 4 | (g_sys.status.run_time[DO_COMP2_BPOS].low >> 12);

    if (sys_get_do_sts(DO_FAN_BPOS) == 0)  // fan disabled, emergency shutdown
    {
        *comp1_sig                                = COMPRESSOR_SIG_ERR;
        *comp2_sig                                = COMPRESSOR_SIG_ERR;
        l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
        return 0;
    }

    if ((sys_get_pwr_sts() == 0) || (g_sys.config.general.cool_type == COOL_TYPE_MODULE_WATER))
    {
        *comp1_sig                                = COMPRESSOR_SIG_OFF;
        *comp2_sig                                = COMPRESSOR_SIG_OFF;
        l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
        return 0;
    }
    if ((sys_get_remap_status(WORK_MODE_STS_REG_NO, FAN_STS_BPOS) != 0) &&
        (l_sys.l_fsm_state[FAN_FSM_STATE] == FSM_FAN_NORM))
    {
        if (compressor_count == 1)  // one compressor configured
        {
            if (g_sys.config.compressor.type == COMP_QABP)  // if it is ec compressor
            {
                if ((req_temp >= g_sys.config.compressor.ec_comp_start_req) ||
                    (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) != 0))
                {
                    *comp1_sig = COMPRESSOR_SIG_ON;
                    *comp2_sig = COMPRESSOR_SIG_OFF;
                }
                else if (req_temp < 0)
                //								//Alair,20170228
                //								else if((req_temp <=
                // 0)&&(sys_get_remap_status(WORK_MODE_STS_REG_NO,DEMHUM_STS_BPOS)
                //!= 1))
                {
                    *comp1_sig = COMPRESSOR_SIG_OFF;
                    *comp2_sig = COMPRESSOR_SIG_OFF;
                }
                else
                {
                    *comp1_sig = COMPRESSOR_SIG_HOLD;
                    *comp2_sig = COMPRESSOR_SIG_HOLD;
                }
            }
            else
            {
                if ((req_temp >= 100) || (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) == 1))
                {
                    *comp1_sig = COMPRESSOR_SIG_ON;
                    *comp2_sig = COMPRESSOR_SIG_OFF;
                }
                //								else if((req_temp <=
                // 0)&&(sys_get_remap_status(WORK_MODE_STS_REG_NO,DEMHUM_STS_BPOS)
                //!= 1)) 需求小于0关闭制冷,Alair,2018.01.27
                else if ((req_temp < 0) && (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) != 1))
                {
                    *comp1_sig = COMPRESSOR_SIG_OFF;
                    *comp2_sig = COMPRESSOR_SIG_OFF;
                }
                else
                {
                    *comp1_sig = COMPRESSOR_SIG_HOLD;
                    *comp2_sig = COMPRESSOR_SIG_OFF;
                }
            }
            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
        }
        else if (compressor_count == 2)  // two compressors configured
        {
            if (g_sys.config.compressor.type == COMP_QABP)  // if it is ec compressor
            {
                switch (l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE])
                {
                    case (0): {
                        *comp1_sig = COMPRESSOR_SIG_OFF;
                        *comp2_sig = COMPRESSOR_SIG_OFF;
                        if ((req_temp >= g_sys.config.compressor.ec_comp_start_req) ||
                            (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) != 0))
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
                        }
                        else
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
                        }
                        break;
                    }
                    case (1): {
                        *comp1_sig = COMPRESSOR_SIG_ON;
                        *comp2_sig = COMPRESSOR_SIG_OFF;
                        if (((req_temp < 0) && (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) == 0)) ||
                            ((comp1_alarm_flag & comp2_alarm_flag) != 0))
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
                        }
                        else if (req_temp >= 100)
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 3;
                        }
                        else if ((sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) != 0) &&
                                 (comp1_alarm_flag == 1) && (comp2_alarm_flag == 0))  // alarm alternation
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 2;
                        }
                        else
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
                        }
                        break;
                    }
                    case (2): {
                        *comp1_sig = COMPRESSOR_SIG_OFF;
                        *comp2_sig = COMPRESSOR_SIG_ON;
                        //												if(((req_temp <=
                        // 0)&&(sys_get_remap_status(WORK_MODE_STS_REG_NO,DEMHUM_STS_BPOS) == 0))||
                        //														((comp1_alarm_flag&comp2_alarm_flag) !=
                        // 0)) 需求小于0关闭制冷,Alair,2018.01.27
                        if (((req_temp < 0) && (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) == 0)) ||
                            ((comp1_alarm_flag & comp2_alarm_flag) != 0))
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
                        }
                        else if (req_temp >= 100)
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 3;
                        }
                        else if ((sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) != 0) &&
                                 (comp2_alarm_flag == 1) && (comp1_alarm_flag == 0))  // alarm alternation
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
                        }
                        else
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 2;
                        }
                        break;
                    }
                    case (3): {
                        *comp1_sig = COMPRESSOR_SIG_ON;
                        *comp2_sig = COMPRESSOR_SIG_ON;
                        if (req_temp < 50)
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
                        }
                        else
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 3;
                        }
                        break;
                    }
                    default: {
                        *comp1_sig                                = COMPRESSOR_SIG_OFF;
                        *comp2_sig                                = COMPRESSOR_SIG_OFF;
                        l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
                        break;
                    }
                }
            }
            else  //定频
            {
                switch (l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE])
                {
                    case (0): {
                        *comp1_sig = COMPRESSOR_SIG_OFF;
                        *comp2_sig = COMPRESSOR_SIG_OFF;
                        if ((req_temp >= 50) || (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) != 0))
                        {
                            if (comp1_runtime < comp2_runtime)
                            {
                                l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
                            }
                            else
                            {
                                l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 2;
                            }
                        }
                        else
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
                        }
                        break;
                    }
                    case (1): {
                        *comp1_sig = COMPRESSOR_SIG_ON;
                        *comp2_sig = COMPRESSOR_SIG_OFF;
                        //												if(((req_temp <=
                        // 0)&&(sys_get_remap_status(WORK_MODE_STS_REG_NO,DEMHUM_STS_BPOS) == 0))||
                        //														((comp1_alarm_flag&comp2_alarm_flag) !=
                        // 0)) 需求小于0关闭制冷,Alair,2018.01.27
                        if (((req_temp < 0) && (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) == 0)) ||
                            ((comp1_alarm_flag & comp2_alarm_flag) != 0))
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
                        }
                        else if (req_temp >= 100)
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 3;
                        }
                        else if (((comp1_runtime > comp2_runtime + g_sys.config.compressor.alter_time) &&
                                  (g_sys.config.compressor.alter_mode == 1)) ||  // timing alternation
                                 (comp1_alarm_flag == 1))                        // alarm alternation
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 2;
                        }
                        else
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
                        }
                        break;
                    }
                    case (2): {
                        *comp1_sig = COMPRESSOR_SIG_OFF;
                        *comp2_sig = COMPRESSOR_SIG_ON;
                        //												if(((req_temp <=
                        // 0)&&(sys_get_remap_status(WORK_MODE_STS_REG_NO,DEMHUM_STS_BPOS) == 0))||
                        //														((comp1_alarm_flag&comp2_alarm_flag) !=
                        // 0)) 需求小于0关闭制冷,Alair,2018.01.27
                        if (((req_temp < 0) && (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) == 0)) ||
                            ((comp1_alarm_flag & comp2_alarm_flag) != 0))
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
                        }
                        else if (req_temp >= 100)
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 3;
                        }
                        else if (((comp2_runtime > comp1_runtime + g_sys.config.compressor.alter_time) &&
                                  (g_sys.config.compressor.alter_mode == 1)) ||  // timing alternation
                                 (comp2_alarm_flag == 1))                        // alarm alternation
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
                        }
                        else
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 2;
                        }
                        break;
                    }
                    case (3): {
                        *comp1_sig = COMPRESSOR_SIG_ON;
                        *comp2_sig = COMPRESSOR_SIG_ON;
                        if (req_temp < 50)
                        {
                            if (comp2_runtime > comp1_runtime)
                            {
                                l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 1;
                            }
                            else
                            {
                                l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 2;
                            }
                        }
                        else
                        {
                            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 3;
                        }
                        break;
                    }
                    default: {
                        *comp1_sig                                = COMPRESSOR_SIG_OFF;
                        *comp2_sig                                = COMPRESSOR_SIG_OFF;
                        l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
                        break;
                    }
                }
            }
        }
        else
        {
            *comp1_sig                                = COMPRESSOR_SIG_OFF;
            *comp2_sig                                = COMPRESSOR_SIG_OFF;
            l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
        }
    }
    else
    {
        *comp1_sig                                = COMPRESSOR_SIG_OFF;
        *comp2_sig                                = COMPRESSOR_SIG_OFF;
        l_sys.l_fsm_state[COMPRESS_SIG_FSM_STATE] = 0;
    }
    return 1;
}

//压缩机状态机函数
static void compressor_fsm(uint8_t compressor_id, uint8_t signal)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    extern team_local_st team_local_inst;

    uint16_t compress_fsm_state;

    uint8_t l_fsm_state_id;
    uint8_t do_bpos;
    static uint8_t u8Cnt;
    //		uint8_t DO_EV_bpos;

    if (compressor_id == 0)
    {
        l_fsm_state_id = COMPRESS1_FSM_STATE;
        do_bpos        = DO_COMP1_BPOS;
        //				DO_EV_bpos =DO_EV1_BPOS;
    }
    else
    {
        l_fsm_state_id = COMPRESS2_FSM_STATE;
        do_bpos        = DO_COMP2_BPOS;
        //				DO_EV_bpos =DO_EV2_BPOS;
    }

    compress_fsm_state = l_sys.l_fsm_state[l_fsm_state_id];
    // TEST
    if (do_bpos == DO_COMP1_BPOS)
    {
        if (signal == COMPRESSOR_SIG_ON)
        {
            u8Cnt++;
        }
        //			rt_kprintf("signal = %d,compress_fsm_state= %d,,l_sys.comp_timeout[do_bpos]= %d,u8Cnt=
        //%d\n",signal,compress_fsm_state,l_sys.comp_timeout[do_bpos],u8Cnt);
    }
    //		rt_kprintf("compress_fsm_state = %d\n",compress_fsm_state);
    //		rt_kprintf("l_sys.comp_timeout[do_bpos] = %d\n",l_sys.comp_timeout[do_bpos]);
    //		rt_kprintf("l_sys.Comp_Delay = %d\n",l_sys.Comp_Delay);
    switch (compress_fsm_state)
    {
        case (COMPRESSOR_FSM_STATE_IDLE): {
            if ((signal == COMPRESSOR_SIG_ON) && (l_sys.comp_timeout[do_bpos] == 0))
            {
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_INIT;
                // not in teammode
                if (((team_local_inst.team_table[g_sys.config.team.addr - 1][TEAM_TAB_STATUS] &
                      (0x0001 << TEAM_STS_CONFD_BPOS)) == 0) ||
                    (g_sys.config.team.team_en == 0) ||
                    (g_sys.config.team.addr > (team_local_inst.team_config[TEAM_CONF_CNT] & 0x00ff)))
                {
                    l_sys.comp_timeout[do_bpos] = g_sys.config.compressor.startup_delay;
                }
                else
                {
                    l_sys.comp_timeout[do_bpos] =
                        g_sys.config.compressor.startup_delay + ((g_sys.config.team.addr - 1) & (0x000f));
                }
                req_bitmap_op(do_bpos, 0);
            }
            else
            {
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_IDLE;
                l_sys.comp_timeout[do_bpos]       = l_sys.comp_timeout[do_bpos];
                req_bitmap_op(do_bpos, 0);
            }
            break;
        }
        case (COMPRESSOR_FSM_STATE_INIT): {
            if (signal != COMPRESSOR_SIG_ON)
            {
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_IDLE;
                l_sys.comp_timeout[do_bpos]       = 0;
                req_bitmap_op(do_bpos, 0);
            }
            else if ((signal == COMPRESSOR_SIG_ON) && (l_sys.comp_timeout[do_bpos] == 0))
            {
                if (l_sys.comp_startup_interval == 0)
                {
                    l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STARTUP;
                    l_sys.comp_timeout[do_bpos]       = g_sys.config.compressor.startup_lowpress_shield;
                    req_bitmap_op(do_bpos, 1);
                    l_sys.comp_startup_interval = g_sys.config.compressor.start_interval;
                }
                else
                {
                    l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_INIT;
                    l_sys.comp_timeout[do_bpos]       = l_sys.comp_timeout[do_bpos];
                    req_bitmap_op(do_bpos, 0);
                }
            }
            else
            {
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_INIT;
                l_sys.comp_timeout[do_bpos]       = l_sys.comp_timeout[do_bpos];
                req_bitmap_op(do_bpos, 0);
            }
            break;
        }
        case (COMPRESSOR_FSM_STATE_STARTUP): {
            if (signal == COMPRESSOR_SIG_ERR)
            {
                //								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_IDLE;
                //								l_sys.comp_timeout[do_bpos] = g_sys.config.compressor.min_stoptime;
                //								req_bitmap_op(do_bpos,0);
                // Alair,20170313
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STOP;
            }
            else if (l_sys.comp_timeout[do_bpos] == 0)
            {
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_NORMAL;
                l_sys.comp_timeout[do_bpos] =
                    (g_sys.config.compressor.min_runtime > g_sys.config.compressor.startup_lowpress_shield)
                        ? (g_sys.config.compressor.min_runtime - g_sys.config.compressor.startup_lowpress_shield)
                        : 0;
                req_bitmap_op(do_bpos, 1);
            }
            else
            {
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STARTUP;
                l_sys.comp_timeout[do_bpos]       = l_sys.comp_timeout[do_bpos];
                req_bitmap_op(do_bpos, 1);
            }
            break;
        }
        case (COMPRESSOR_FSM_STATE_NORMAL): {
            if (signal == COMPRESSOR_SIG_ERR)
            {
                //								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_IDLE;
                //								l_sys.comp_timeout[do_bpos] = g_sys.config.compressor.min_stoptime;
                //								req_bitmap_op(do_bpos,0);
                // Alair,20170313
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STOP;
            }
            else if ((signal == COMPRESSOR_SIG_OFF) && (l_sys.comp_timeout[do_bpos] == 0))
            {
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_SHUTING;
                l_sys.comp_timeout[do_bpos]       = g_sys.config.compressor.stop_delay;
                req_bitmap_op(do_bpos, 1);
            }
            else
            {
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_NORMAL;
                l_sys.comp_timeout[do_bpos]       = l_sys.comp_timeout[do_bpos];
                req_bitmap_op(do_bpos, 1);
            }
            break;
        }
        case (COMPRESSOR_FSM_STATE_SHUTING): {
            if (signal == COMPRESSOR_SIG_ERR)
            {
                //								l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_IDLE;
                //								l_sys.comp_timeout[do_bpos] = g_sys.config.compressor.min_stoptime;
                //								req_bitmap_op(do_bpos,0);
                // Alair,20170313
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STOP;
            }
            else if ((signal == COMPRESSOR_SIG_OFF) && (l_sys.comp_timeout[do_bpos] == 0))
            {
                // Alair,20170225
                if (compressor_id == 0)  //压机1
                {
                    if (g_sys.config.compressor.type == COMP_QABP)
                    {
                        if (g_sys.status.aout[AO_EC_COMPRESSOR] > 0x00)
                        {
                            l_sys.Comp_Delay = 10;  //延时10S关闭
                            break;
                        }
                        else
                        {
                            if (l_sys.Comp_Delay == 0x00)
                            {
                                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STOP;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                    else
                    {
                        l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STOP;
                    }
                }
                else  //压机2
                {
                    l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_STOP;
                }
            }
            else if (signal == COMPRESSOR_SIG_ON)
            {
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_NORMAL;
                l_sys.comp_timeout[do_bpos]       = 0;
                req_bitmap_op(do_bpos, 1);
            }
            else
            {
                l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_SHUTING;
                l_sys.comp_timeout[do_bpos]       = l_sys.comp_timeout[do_bpos];
                req_bitmap_op(do_bpos, 1);
            }
            break;
        }
        case (COMPRESSOR_FSM_STATE_STOP): {
            l_sys.l_fsm_state[l_fsm_state_id] = COMPRESSOR_FSM_STATE_IDLE;
            l_sys.comp_timeout[do_bpos]       = g_sys.config.compressor.min_stoptime;
            req_bitmap_op(do_bpos, 0);
            //						l_sys.comp_timeout[DO_EV_bpos]=g_sys.config.compressor.ev_Delay_shut_time;//电子膨胀阀滞后时间
            break;
        }
    }
}

//模拟输出跟踪函数，向设置目标，按照步长参数进行变化；
static int16_t analog_step_follower(int16_t target, uint16_t dev_type)
{
    extern sys_reg_st g_sys;
    uint16_t current_output;
    int16_t ret_val;
    int16_t delta;
    uint16_t min_fan, max_fan, min_com, max_com, aid, kid, calc_target;

    static uint16_t compressor_timer = 0;
    switch (dev_type)
    {
        case (AO_EC_FAN): {
            current_output = g_sys.status.aout[AO_EC_FAN];
            delta          = target - current_output;

            if (abs(delta) >= g_sys.config.fan.adjust_step)
            {
                if (delta > 0)
                    ret_val = current_output + g_sys.config.fan.adjust_step;
                else
                    ret_val = current_output - g_sys.config.fan.adjust_step;
            }
            else if (delta == 0)
            {
                ret_val = current_output;
            }
            else
            {
                ret_val = current_output + delta;
            }
            rt_kprintf("current_output:%d,target:%d,ret_val:%d \n", current_output, target, ret_val);
            break;
        }
        case (AO_INV_FAN): {
            min_fan = g_sys.config.fan.min_speed;
            max_fan = g_sys.config.fan.max_speed;
            min_com = g_sys.config.compressor.speed_lower_lim;
            max_com = g_sys.config.compressor.speed_upper_lim;

            kid = ((max_fan - min_fan) * 100) / (max_com - min_com);
            aid = min_fan * 100 - (kid * min_com);

            current_output = g_sys.status.aout[AO_EC_COMPRESSOR];
            calc_target    = (kid * current_output + aid) / 100;
            if (calc_target < min_fan)
                ret_val = min_fan;
            else if (calc_target > max_fan)
                ret_val = max_fan;
            else
            {
                delta = calc_target - g_sys.status.aout[AO_EC_FAN];

                if (abs(delta) >= g_sys.config.fan.inv_step)
                {
                    ret_val = calc_target;
                }
                else if (delta == 0)
                {
                    ret_val = g_sys.status.aout[AO_EC_FAN];
                }
                else
                {
                    ret_val = g_sys.status.aout[AO_EC_FAN];
                }
            }
            //						rt_kprintf("ret_val1:%d \n",ret_val);
            if (target == 0)
            {
                ret_val = ret_val;
            }
            if (target == 1)
            {
                ret_val = ret_val * g_sys.config.fan.dehum_ratio / 100;
            }
            //						rt_kprintf("ret_val2:%d \n",ret_val);
            break;
        }
        case (AO_EC_COMPRESSOR): {
            current_output = g_sys.status.aout[AO_EC_COMPRESSOR];
            delta          = target - current_output;
            if (compressor_timer >= (g_sys.config.compressor.step_period - 1))
            {
                if (abs(delta) > g_sys.config.compressor.step)
                {
                    if (delta > 0)
                    {
                        ret_val = current_output + g_sys.config.compressor.step;
                    }
                    else
                    {
                        ret_val = current_output - g_sys.config.compressor.step;
                    }
                }
                else if (delta == 0)
                {
                    ret_val = current_output;
                }
                else
                {
                    ret_val = current_output + delta;
                }
                compressor_timer = 0;
            }
            else
            {
                ret_val = current_output;
                compressor_timer++;
            }
            break;
        }
        case (AO_WATER_VALVE): {
            ret_val = target;
            break;
        }
        default: {
            ret_val = target;
            break;
        }
    }
    return ret_val;
}

//变频压缩机停止信号发生器
int16_t get_inv_comp_freq_down_signal(uint8_t Type)
{
    extern sys_reg_st g_sys;
    int16_t ret, Buffer;

    ret = 0;
    switch (Type)
    {
        case HIGH_PRESS:
            Buffer = g_sys.config.compressor.high_press_threshold - g_sys.config.compressor.high_press_hyst;
            if (g_sys.status.inv_compress_alarm.avg_hi_press[GET_AVG] >= g_sys.config.compressor.high_press_threshold)
            {
                ret = 1;
            }
            else if (g_sys.status.inv_compress_alarm.avg_hi_press[GET_AVG] <= Buffer)
            {
                ret = -1;
            }
            else
            {
                ret = 0;
            }
            break;
        case EXHAUST_TEMP:
            Buffer =
                g_sys.config.compressor.Exhaust_temperature - g_sys.config.compressor.Exhaust_temperature_hystersis;
            if (g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_AVG] >=
                g_sys.config.compressor.Exhaust_temperature)
            {
                ret = 1;
            }
            else if (g_sys.status.inv_compress_alarm.Inv_hi_Temperature[GET_AVG] <= Buffer)
            {
                ret = -1;
            }
            else
            {
                ret = 0;
            }
            break;
        default:
            ret = 0;
            break;
    }
    return ret;
}

//变频压缩机状态机标志
enum
{
    INV_COMP_IDLE,
    INV_COMP_STARTUP,
    INV_COMP_NORMAL,
    INV_COMP_RETOIL
};

//变频压缩机模拟输出控制函数
static void ec_compressor_output(int16_t req_temp, uint8_t comp_sig)
{
    extern sys_reg_st g_sys;
    static int16_t local_compress_max_speed = 0;
    static int16_t local_Exhaust_max_speed  = 0;
    static uint16_t ro_timeout              = 0;
    static uint8_t inv_comp_fsm             = INV_COMP_IDLE;
    int16_t require;

    time_t now;

    require = 0;
    if (g_sys.config.compressor.type == COMP_CONSTANT_FRE)  // if it is not inv compressor
    {
        if ((g_sys.status.dout_bitmap & (0x0001 << DO_COMP1_BPOS)) == 0)
        {
            require = 0;
        }
        else
        {
            require = 100;
        }
        ro_timeout               = 0;
        local_compress_max_speed = g_sys.config.compressor.speed_upper_lim;
        inv_comp_fsm             = INV_COMP_IDLE;
    }
    else
    {
        //		rt_kprintf("req_temp = %d,DEMHUM_STS_BPOS = %d,inv_comp_fsm =
        //%d\n",req_temp,sys_get_remap_status(WORK_MODE_STS_REG_NO,DEMHUM_STS_BPOS),inv_comp_fsm);
        switch (inv_comp_fsm)
        {
            case (INV_COMP_IDLE): {
                if ((g_sys.status.dout_bitmap & (0x0001 << DO_COMP1_BPOS)) != 0)
                {
                    ro_timeout               = g_sys.config.compressor.startup_lowpress_shield;
                    local_compress_max_speed = g_sys.config.compressor.speed_upper_lim;
                    require      = lim_min_max(g_sys.config.compressor.speed_lower_lim, local_compress_max_speed,
                                          g_sys.config.compressor.startup_freq);
                    inv_comp_fsm = INV_COMP_STARTUP;
                }
                else
                {
                    ro_timeout               = 0;
                    require                  = 0;
                    local_compress_max_speed = g_sys.config.compressor.speed_upper_lim;
                    inv_comp_fsm             = INV_COMP_IDLE;
                }
                break;
            }
            case (INV_COMP_STARTUP): {
                if ((g_sys.status.dout_bitmap & (0x0001 << DO_COMP1_BPOS)) != 0)
                {
                    local_compress_max_speed = g_sys.config.compressor.speed_upper_lim;
                    if (ro_timeout > 0)
                    {
                        ro_timeout--;
                        //                        require =
                        //                        lim_min_max(g_sys.config.compressor.speed_lower_lim,local_compress_max_speed,g_sys.config.compressor.startup_freq);
                        // Alair,20170227
                        require      = g_sys.config.compressor.startup_freq;
                        inv_comp_fsm = INV_COMP_STARTUP;
                    }
                    else
                    {
                        //                        require =
                        //                        lim_min_max(g_sys.config.compressor.speed_lower_lim,local_compress_max_speed,req_temp);
                        // Alair,20170227
                        require      = g_sys.config.compressor.startup_freq;
                        inv_comp_fsm = INV_COMP_NORMAL;
                        ro_timeout   = 0;
                    }
                }
                else
                {
                    ro_timeout               = 0;
                    require                  = 0;
                    local_compress_max_speed = g_sys.config.compressor.speed_upper_lim;
                    inv_comp_fsm             = INV_COMP_IDLE;
                }
                break;
            }

            case (INV_COMP_NORMAL): {
                if ((g_sys.status.dout_bitmap & (0x0001 << DO_COMP1_BPOS)) == 0)
                {
                    ro_timeout               = 0;
                    require                  = 0;
                    local_compress_max_speed = g_sys.config.compressor.speed_upper_lim;
                    inv_comp_fsm             = INV_COMP_IDLE;
                }
                else
                {
                    if (ro_timeout >= (g_sys.config.compressor.low_freq_switch_period * 60))
                    {
                        ro_timeout = 0;
                        require =
                            lim_min_max(g_sys.config.compressor.speed_lower_lim,
                                        g_sys.config.compressor.speed_upper_lim, g_sys.config.compressor.ret_oil_freq);
                        local_compress_max_speed = local_compress_max_speed;
                        inv_comp_fsm             = INV_COMP_RETOIL;
                    }
                    else
                    {
                        if (g_sys.status.aout[AO_EC_COMPRESSOR] > g_sys.config.compressor.low_freq_threshold)
                        {
                            ro_timeout = 0;
                        }
                        else if (g_sys.status.aout[AO_EC_COMPRESSOR] <= g_sys.config.compressor.low_freq_threshold)
                        {
                            ro_timeout++;
                        }
                        //除湿状态
                        // Alair,20170817
                        if (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) == 1)  //除湿
                        {
                            req_temp = g_sys.config.compressor.Dehumidity_Freq;
                        }
                        /*
                                                                    //Alair,20170228,排气温度降频,2小时内3次关机
                                                                        if(get_inv_comp_freq_down_signal(EXHAUST_TEMP) >
                        0)
                                                {
                                                                                if(g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag==0)
                                                                                {
                                                                                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag=1;
                                                                                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Count++;
                                                                                        if(g_sys.status.inv_compress_alarm.Inv_hi_temp_Count>2)//超过2次停机
                                                                                        {
                                                                                                g_sys.status.inv_compress_alarm.Inv_hi_temp_Count=0;
                                                                                                g_sys.status.inv_compress_alarm.Inv_hi_temp_Stop=1;
                                                                                        }
                                                                                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Starttime[0]=g_sys.status.inv_compress_alarm.Inv_hi_temp_Starttime[1];
                                                                                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Starttime[1]=g_sys.status.inv_compress_alarm.Inv_hi_temp_Starttime[2];
                                                                                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Starttime[2]
                        = now;
                                                                                }
                                                    local_compress_max_speed = g_sys.status.aout[AO_EC_COMPRESSOR]-1;
                                                }
                                                                        else if
                        (get_inv_comp_freq_down_signal(EXHAUST_TEMP) < 0)
                                                                        {
                                                                                g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag=0;
                                                                                memset(g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime,0x00,sizeof(g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime));

                                                                                local_compress_max_speed =
                        g_sys.config.compressor.speed_upper_lim; //恢复正常输出
                        //														local_compress_max_speed =
                        lim_min_max(g_sys.config.compressor.speed_lower_lim,local_compress_max_speed,analog_step_follower(req_temp,AO_EC_COMPRESSOR));
                                                                        }
                                                                        else
                                                                        {
                                                                                if(g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag==1)
                                                                                {
                                                                                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag=0;
                                                                                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[0]=g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[1];
                                                                                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[1]=g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[2];
                                                                                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[2]
                        = now;
                                                                                }
                        //														get_local_time(&now);
                                                                                if ((req_temp >=
                        g_sys.status.aout[AO_EC_COMPRESSOR]) && ((now -
                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[0]) < 3600)
                                                                                    &&
                        (g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[0] != 0xffffffff))
                                                                                {
                                                                                        local_compress_max_speed =
                        local_compress_max_speed; //保持
                                                                                }
                                                                                else if ((req_temp >=
                        g_sys.status.aout[AO_EC_COMPRESSOR]) && ((now -
                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[1]) < 3600)
                                                                                    &&
                        (g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[1] != 0xffffffff))
                                                                                {
                                                                                        local_compress_max_speed =
                        local_compress_max_speed; //保持
                                                                                }
                                                                                else if (req_temp <
                        g_sys.status.aout[AO_EC_COMPRESSOR] - 2)
                                                                                {
                                                                                        local_compress_max_speed =
                        analog_step_follower(req_temp,AO_EC_COMPRESSOR);
                                                                                }

                                                                                if((g_sys.status.inv_compress_alarm.Inv_hi_temp_Count
                        == 1)
                                                                                    && ((now -
                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[2]) >= 3600)//1小时
                                                                                    &&
                        (g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[2] != 0xffffffff))
                                                                                {
                                                                                    local_compress_max_speed =
                        g_sys.config.compressor.speed_upper_lim; //解除高压降频
                        //															rt_kprintf("triggle1\n");
                                                                                }
                                                                                else
                        if((g_sys.status.inv_compress_alarm.inv_alarm_counter == 2)
                                                                                    && ((now -
                        g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[2]) >= 3600)
                                                                                &&
                        (g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[2] != 0xffffffff))
                                                                                {
                                                                                    local_compress_max_speed =
                        g_sys.config.compressor.speed_upper_lim; //解除高压降频
                        //															rt_kprintf("triggle2\n");
                                                                                }
                                                }
                        */
                        get_local_time(&now);
                        // Alair,20170303,排气温度降频
                        local_Exhaust_max_speed = local_compress_max_speed;
                        if (get_inv_comp_freq_down_signal(EXHAUST_TEMP) > 0)
                        {
                            if (g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag == 0)
                            {
                                g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag = 1;
                                g_sys.status.inv_compress_alarm.Inv_hi_temp_Count++;
                            }
                            local_Exhaust_max_speed = g_sys.status.aout[AO_EC_COMPRESSOR] - 1;
                        }
                        else if (get_inv_comp_freq_down_signal(EXHAUST_TEMP) < 0)
                        {
                            g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag = 0;
                            memset(g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime, 0x00,
                                   sizeof(g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime));

                            local_Exhaust_max_speed = g_sys.config.compressor.speed_upper_lim;  //恢复正常输出
                            //														local_compress_max_speed =
                            // lim_min_max(g_sys.config.compressor.speed_lower_lim,local_compress_max_speed,analog_step_follower(req_temp,AO_EC_COMPRESSOR));
                        }
                        else
                        {
                            if (g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag == 1)
                            {
                                g_sys.status.inv_compress_alarm.Inv_hi_temp_Flag        = 0;
                                g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[0] = now;  //结束时间
                            }
                            //														get_local_time(&now);
                            if ((req_temp >= g_sys.status.aout[AO_EC_COMPRESSOR]) &&
                                ((now - g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[0]) < 3600) &&
                                (g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[0] != 0xffffffff))
                            {
                                local_Exhaust_max_speed = local_Exhaust_max_speed;  //保持
                                //																rt_kprintf("triggle1\n");
                            }
                            else if (req_temp < (g_sys.status.aout[AO_EC_COMPRESSOR] - 2))
                            {
                                local_Exhaust_max_speed = analog_step_follower(req_temp, AO_EC_COMPRESSOR);
                            }

                            if ((g_sys.status.inv_compress_alarm.Inv_hi_temp_Count != 0x00) &&
                                ((now - g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[0]) >= 3600)  // 1小时
                                && (g_sys.status.inv_compress_alarm.Inv_hi_temp_Stoptime[0] != 0xffffffff))
                            {
                                g_sys.status.inv_compress_alarm.Inv_hi_temp_Count = 0;
                                local_Exhaust_max_speed = g_sys.config.compressor.speed_upper_lim;  //解除高温降频
                                //															rt_kprintf("triggle1\n");
                            }
                        }
                        //												rt_kprintf("local_Exhaust_max_speed:
                        //%d\n",local_Exhaust_max_speed); Alair,20170311,取消三次高压停压机
                        /*
                                                                        //高压降频
                                                if(get_inv_comp_freq_down_signal(HIGH_PRESS) > 0)
                                                {
                                                                                g_sys.status.inv_compress_alarm.inv_stop_time[0]
                        = 0xffffffff; g_sys.status.inv_compress_alarm.inv_stop_time[1] = 0xffffffff;
                                                                                g_sys.status.inv_compress_alarm.inv_stop_time[2]
                        = 0xffffffff; local_compress_max_speed = g_sys.status.aout[AO_EC_COMPRESSOR]-1;
                                                }
                                                                        else if
                        (get_inv_comp_freq_down_signal(HIGH_PRESS) < 0)
                                                                        {
                                                                                g_sys.status.inv_compress_alarm.inv_stop_time[0]
                        = 0xffffffff; g_sys.status.inv_compress_alarm.inv_stop_time[1] = 0xffffffff;
                                                                                g_sys.status.inv_compress_alarm.inv_stop_time[2]
                        = 0xffffffff;

                                                                                local_compress_max_speed =
                        g_sys.config.compressor.speed_upper_lim; //恢复正常输出
                        //														local_compress_max_speed =
                        lim_min_max(g_sys.config.compressor.speed_lower_lim,local_compress_max_speed,analog_step_follower(req_temp,AO_EC_COMPRESSOR));
                                                                        }
                                                                        else
                                                                        {
                                                                                if ((req_temp >=
                        g_sys.status.aout[AO_EC_COMPRESSOR]) && ((now -
                        g_sys.status.inv_compress_alarm.inv_stop_time[0]) < 3600) &&
                        (g_sys.status.inv_compress_alarm.inv_stop_time[0] != 0xffffffff))
                                                                                {
                                                                                        local_compress_max_speed =
                        local_compress_max_speed; //保持
                        //																local_compress_max_speed_tmp
                        =local_compress_max_speed;
                                                                                }
                                                                                else if ((req_temp >=
                        g_sys.status.aout[AO_EC_COMPRESSOR]) && ((now -
                        g_sys.status.inv_compress_alarm.inv_stop_time[1]) < 3600) &&
                        (g_sys.status.inv_compress_alarm.inv_stop_time[1] != 0xffffffff))
                                                                                {
                                                                                        local_compress_max_speed =
                        local_compress_max_speed; //保持
                        //																local_compress_max_speed_tmp
                        =local_compress_max_speed;
                                                                                }
                                                                                else if (req_temp <
                        g_sys.status.aout[AO_EC_COMPRESSOR] - 2)
                                                                                {
                                                                                        local_compress_max_speed =
                        analog_step_follower(req_temp,AO_EC_COMPRESSOR);
                                                                                }
                        //
                        rt_kprintf("g_sys.status.inv_compress_alarm.inv_stop_time[2]:%d
                        \n",g_sys.status.inv_compress_alarm.inv_stop_time[2]);
                                                                                if((g_sys.status.inv_compress_alarm.inv_alarm_counter
                        == 1) && ((now - g_sys.status.inv_compress_alarm.inv_stop_time[2]) >= 3600) &&
                        (g_sys.status.inv_compress_alarm.inv_stop_time[2] != 0xffffffff))
                                                                                {
                                                                                    local_compress_max_speed =
                        g_sys.config.compressor.speed_upper_lim; //解除高压降频
                        //															rt_kprintf("triggle1\n");
                                                                                }
                                                                                else
                        if((g_sys.status.inv_compress_alarm.inv_alarm_counter == 2) && ((now -
                        g_sys.status.inv_compress_alarm.inv_stop_time[2]) >= 3600) &&
                        (g_sys.status.inv_compress_alarm.inv_stop_time[2] != 0xffffffff))
                                                                                {
                                                                                    local_compress_max_speed =
                        g_sys.config.compressor.speed_upper_lim; //解除高压降频
                        //															rt_kprintf("triggle2\n");
                                                                                }
                                                }
                        */
                        //高压降频
                        if (get_inv_comp_freq_down_signal(HIGH_PRESS) > 0)
                        {
                            if (g_sys.status.inv_compress_alarm.inv_hipress_tmp == 0)
                            {
                                g_sys.status.inv_compress_alarm.inv_hipress_tmp = 1;
                                g_sys.status.inv_compress_alarm.inv_alarm_counter++;
                            }
                            local_compress_max_speed = g_sys.status.aout[AO_EC_COMPRESSOR] - 1;
                        }
                        else if (get_inv_comp_freq_down_signal(HIGH_PRESS) < 0)
                        {
                            g_sys.status.inv_compress_alarm.inv_hipress_tmp = 0;
                            memset(g_sys.status.inv_compress_alarm.inv_stop_time, 0x00,
                                   sizeof(g_sys.status.inv_compress_alarm.inv_stop_time));
                            local_compress_max_speed = g_sys.config.compressor.speed_upper_lim;  //恢复正常输出
                        }
                        else
                        {
                            if (g_sys.status.inv_compress_alarm.inv_hipress_tmp == 1)
                            {
                                g_sys.status.inv_compress_alarm.inv_hipress_tmp  = 0;
                                g_sys.status.inv_compress_alarm.inv_stop_time[0] = now;  //结束时间
                            }
                            //														get_local_time(&now);
                            if ((req_temp >= g_sys.status.aout[AO_EC_COMPRESSOR]) &&
                                ((now - g_sys.status.inv_compress_alarm.inv_stop_time[0]) < 3600) &&
                                (g_sys.status.inv_compress_alarm.inv_stop_time[0] != 0xffffffff))
                            {
                                local_compress_max_speed = local_compress_max_speed;  //保持
                                //																rt_kprintf("triggle1\n");
                            }
                            else if (req_temp < (g_sys.status.aout[AO_EC_COMPRESSOR] - 2))
                            {
                                local_compress_max_speed = analog_step_follower(req_temp, AO_EC_COMPRESSOR);
                            }

                            if ((g_sys.status.inv_compress_alarm.inv_alarm_counter != 0x00) &&
                                ((now - g_sys.status.inv_compress_alarm.inv_stop_time[0]) >= 3600)  // 1小时
                                && (g_sys.status.inv_compress_alarm.inv_stop_time[0] != 0xffffffff))
                            {
                                g_sys.status.inv_compress_alarm.inv_alarm_counter = 0;
                                local_compress_max_speed = g_sys.config.compressor.speed_upper_lim;  //解除高压降频
                                //															rt_kprintf("triggle1\n");
                            }
                        }

                        if (local_Exhaust_max_speed < local_compress_max_speed)
                        {
                            local_compress_max_speed = local_Exhaust_max_speed;
                        }

                        // Alair,20170228
                        if (comp_sig == COMPRESSOR_SIG_OFF)  //关机
                        {
                            require = analog_step_follower(req_temp, AO_EC_COMPRESSOR);
                        }
                        else
                        {
                            require = lim_min_max(g_sys.config.compressor.speed_lower_lim, local_compress_max_speed,
                                                  analog_step_follower(req_temp, AO_EC_COMPRESSOR));
                            if (require >= g_sys.config.compressor.speed_upper_lim)
                            {
                                require = g_sys.config.compressor.speed_upper_lim;
                            }
                            if (require <= g_sys.config.compressor.speed_lower_lim)
                            {
                                require = g_sys.config.compressor.speed_lower_lim;
                            }
                        }
                        inv_comp_fsm = INV_COMP_NORMAL;
                    }
                }
                break;
            }
            case (INV_COMP_RETOIL): {
                if ((g_sys.status.dout_bitmap & (0x0001 << DO_COMP1_BPOS)) == 0)
                {
                    ro_timeout               = 0;
                    require                  = 0;
                    local_compress_max_speed = g_sys.config.compressor.speed_upper_lim;
                    inv_comp_fsm             = INV_COMP_IDLE;
                }
                else
                {
                    if (ro_timeout >= g_sys.config.compressor.ret_oil_period)
                    {
                        ro_timeout               = 0;
                        local_compress_max_speed = local_compress_max_speed;
                        require                  = lim_min_max(g_sys.config.compressor.speed_lower_lim,
                                              g_sys.config.compressor.speed_upper_lim, req_temp);
                        inv_comp_fsm             = INV_COMP_NORMAL;
                    }
                    else
                    {
                        ro_timeout++;
                        local_compress_max_speed = local_compress_max_speed;
                        require =
                            lim_min_max(g_sys.config.compressor.speed_lower_lim,
                                        g_sys.config.compressor.speed_upper_lim, g_sys.config.compressor.ret_oil_freq);
                        inv_comp_fsm = INV_COMP_RETOIL;
                    }
                }
                break;
            }
            default: {
                ro_timeout               = 0;
                require                  = 0;
                local_compress_max_speed = local_compress_max_speed;
                inv_comp_fsm             = INV_COMP_IDLE;
                break;
            }
        }
    }
    //机柜空调变频器告警特殊处理
    if ((sys_get_do_sts(DO_FAN_BPOS) == 1) && (get_alarm_bitmap(ACL_INV_FAULT) == 1))
    {
        require = g_sys.config.compressor.speed_lower_lim;
    }
    //		rt_kprintf("require = %d,speed_lower_lim = %d\n",require,g_sys.config.compressor.speed_lower_lim);
    req_ao_op(AO_EC_COMPRESSOR, require, 1);
}

// liquid solenod valve control logic
// static void liq_sole_valve(void)
//{
//    extern sys_reg_st		g_sys;
//    extern local_reg_st l_sys;
//
//    uint8_t lsv_value = 0;
//
//    if(((g_sys.config.dev_mask.dout & (0x0001<<DO_LIQ_SOLE_VALVE)) != 0)&&(g_sys.config.compressor.type == 1))
//    {
//        switch(l_sys.l_fsm_state[COMPRESS1_FSM_STATE])
//        {
//            case (COMPRESSOR_FSM_STATE_STARTUP):
//            {
//                lsv_value = 1;
//                break;
//            }
//            case (COMPRESSOR_FSM_STATE_NORMAL):
//            {
//                lsv_value = 1;
//                break;
//            }
//            case (COMPRESSOR_FSM_STATE_SHUTING):
//            {
//                if(l_sys.comp_timeout[DO_COMP1_BPOS] <= g_sys.config.compressor.liq_sole_delay )
//                {
//                    lsv_value = 0;
//                }
//                else
//                {
//                    lsv_value = 1;
//                }
//                break;
//            }
//            default:
//            {
//                lsv_value = 0;
//                break;
//            }
//        }
//    }
//    else
//    {
//        lsv_value = 0;
//    }
//    req_bitmap_op(DO_LIQ_SOLE_VALVE,lsv_value);
//}

////电子膨胀阀控制逻辑
// static void ev_ctrl_exe(uint8_t compressor_id)
//{
//    extern sys_reg_st		g_sys;
//    extern local_reg_st l_sys;
//    uint8_t ev_value = 0;
//
//    uint8_t DO_EV_bpos=0;
//    uint8_t DO_COMP_BPOS=0;
//    uint8_t COMP_FSM=0;
//
//		if(compressor_id==0)
//		{
//				DO_EV_bpos=DO_EV1_BPOS;
//				DO_COMP_BPOS=DO_COMP1_BPOS;
//				COMP_FSM=COMPRESS1_FSM_STATE;
//		}
//		else if(compressor_id==1)
//		{
//				DO_EV_bpos=DO_EV2_BPOS;
//				DO_COMP_BPOS=DO_COMP2_BPOS;
//				COMP_FSM=COMPRESS2_FSM_STATE;
//		}
//
////    if(((g_sys.config.dev_mask.dout & (0x0001<<DO_EV1_BPOS)) != 0)&&(g_sys.config.compressor.type == COMP_QABP))
//    if((g_sys.config.dev_mask.dout & (0x0001<<DO_EV_bpos)) != 0)
//    {
//        switch(l_sys.l_fsm_state[COMP_FSM])
//        {
//
//            case (COMPRESSOR_FSM_STATE_INIT):
//            {
//                if(l_sys.comp_timeout[DO_COMP_BPOS] <= g_sys.config.compressor.ev_ahead_start_time)
//                    ev_value = 1;
//                else
//                    ev_value = 0;
//                break;
//            }
//            case (COMPRESSOR_FSM_STATE_STARTUP):
//            {
//                ev_value = 1;
//                break;
//            }
//            case (COMPRESSOR_FSM_STATE_NORMAL):
//            {
//                ev_value = 1;
//                break;
//            }
//            case (COMPRESSOR_FSM_STATE_SHUTING):
//            {
//									ev_value = 1;
//									break;
//            }
//						//Alair,20170228
//            default:
//            {
//								if((l_sys.comp_timeout[DO_EV_bpos] == 0)&&((g_sys.status.dout_bitmap
//&(0x0001<<DO_COMP_BPOS))==0))
//								{
//										ev_value = 0;
//								}
//								else
//								{
//										if(g_sys.status.dout_bitmap &(0x0001<<DO_FAN_BPOS))
//										{
//												ev_value = 1;
//										}
//										else
//										{
//												ev_value = 0;
//										}
//								}
//                break;
//            }
//        }
//    }
//    else
//    {
//        ev_value = 0;
//    }
//    req_bitmap_op(DO_EV_bpos,ev_value);
//}

void compressor_alarm_signal_gen(uint8_t* comp1_sig, uint8_t* comp2_sig)
{
    extern local_reg_st l_sys;

    if ((get_alarm_bitmap_mask(DO_COMP1_BPOS) == 1) && (get_alarm_bitmap_op(DO_COMP1_BPOS) == 0))
    {
        *comp1_sig = COMPRESSOR_SIG_OFF;
        if ((l_sys.l_fsm_state[COMPRESS1_FSM_STATE] != COMPRESSOR_FSM_STATE_STOP) &&
            (l_sys.l_fsm_state[COMPRESS1_FSM_STATE] != COMPRESSOR_FSM_STATE_IDLE))
        {
            l_sys.l_fsm_state[COMPRESS1_FSM_STATE] = COMPRESSOR_FSM_STATE_STOP;
        }
    }

    if ((get_alarm_bitmap_mask(DO_COMP2_BPOS) == 1) && (get_alarm_bitmap_op(DO_COMP2_BPOS) == 0))
    {
        *comp2_sig = COMPRESSOR_SIG_OFF;
        if ((l_sys.l_fsm_state[COMPRESS2_FSM_STATE] != COMPRESSOR_FSM_STATE_STOP) &&
            (l_sys.l_fsm_state[COMPRESS2_FSM_STATE] != COMPRESSOR_FSM_STATE_IDLE))
        {
            l_sys.l_fsm_state[COMPRESS2_FSM_STATE] = COMPRESSOR_FSM_STATE_STOP;
        }
    }
}

/*********************************************************************
外风机输出控制
*********************************************************************/

void CalculateFout(uint8_t u8Exfan_ID, int16_t i16Pressure)
{
    extern sys_reg_st g_sys;
    static int16_t s_i16PRefAverage[EX_FAN_NUM] = {0}, s_i16PDeltaRef[EX_FAN_NUM] = {0},
                   s_i16AdjTimes[EX_FAN_NUM] = {0};
    uint16_t i16Temp                         = 0;

    uint16_t u16PressurePoint[MAXPOINTNUM];  //压力参数

    if (u8Exfan_ID >= EX_FAN_NUM)
    {
        return;
    }
    //压力参数
    u16PressurePoint[P_SET_POINT]  = g_sys.config.Ex_Fan.PSETID;
    u16PressurePoint[P_OFF_POINT]  = u16PressurePoint[P_SET_POINT] - g_sys.config.Ex_Fan.PHYSTID;
    u16PressurePoint[P_FLAT_POINT] = (u16PressurePoint[P_SET_POINT] + g_sys.config.Ex_Fan.PBANDID);

    // R22,R407C
    u16PressurePoint[P_STEP_POINT]  = u16PressurePoint[P_FLAT_POINT] + g_sys.config.Ex_Fan.PSETID;
    u16PressurePoint[P_FLAT2_POINT] = u16PressurePoint[P_STEP_POINT] - g_sys.config.Ex_Fan.PHYSTID;
    //		else
    //		{
    //			//R410A
    //			u16PressurePoint[P_STEP_POINT] = i16PressurePoint[P_FLAT_POINT] + g_u16CfgPara[PSTEPID];
    //			u16PressurePoint[P_FLAT2_POINT] = i16PressurePoint[P_STEP_POINT] - g_u16CfgPara[PHYSTID];
    //		}

    if ((i16Pressure <= u16PressurePoint[P_SET_POINT]) || (i16Pressure >= u16PressurePoint[P_FLAT_POINT]))
    {
        g_i16PDeltaRef[u8Exfan_ID] = i16Pressure;
        s_i16PDeltaRef[u8Exfan_ID] = u16PressurePoint[P_SET_POINT];
        s_i16AdjTimes[u8Exfan_ID]  = 0;
    }

    //	if(g_u16SamDlyFlag[u8Exfan_ID] == FALSE)
    //	{
    //		return;
    //	}
    //	rt_kprintf("u8Exfan_ID= %d,i16Pressure= %d,P_SET_POINT= %d,P_FLAT_POINT= %d,P_FLAT2_POINT=
    //%d\n",u8Exfan_ID,i16Pressure,u16PressurePoint[P_SET_POINT],u16PressurePoint[P_OFF_POINT],
    //	u16PressurePoint[P_FLAT_POINT],u16PressurePoint[P_FLAT2_POINT]);
    //????1?2?????????1
    if (i16Pressure < u16PressurePoint[P_OFF_POINT])
    {
        g_sys.status.Ex_Fan_Speed[u8Exfan_ID] = 0;
    }
    else if (i16Pressure < u16PressurePoint[P_SET_POINT])
    {  //????1??2???????????????1
        if (g_sys.status.Ex_Fan_Speed[u8Exfan_ID] >= (g_sys.config.Ex_Fan.MINSPEEDID))
        {
            g_sys.status.Ex_Fan_Speed[u8Exfan_ID] = g_sys.config.Ex_Fan.MINSPEEDID;
        }
        else
        {
            g_sys.status.Ex_Fan_Speed[u8Exfan_ID] = 0;
        }
    }
    else if (i16Pressure <= u16PressurePoint[P_FLAT_POINT])
    {  //???????
        if (s_i16PDeltaRef[u8Exfan_ID] > (u16PressurePoint[P_FLAT_POINT] - MAXDELTAP2))
        {  //?????????????????u????
            s_i16PDeltaRef[u8Exfan_ID] = u16PressurePoint[P_FLAT_POINT] - MAXDELTAP2;
        }
        if (abs(g_i16PDeltaRef[u8Exfan_ID] - i16Pressure) > MAXDELTAP1)
        {  //??????????????????
            //?????????
            if (s_i16AdjTimes == 0)
            {  //??????????
                s_i16PRefAverage[u8Exfan_ID] = i16Pressure;
            }
            else
            {  //?????????
                s_i16PRefAverage[u8Exfan_ID] = (s_i16PRefAverage[u8Exfan_ID] + i16Pressure) / 2;
            }
            if (abs(s_i16PDeltaRef - s_i16PRefAverage) > MAXDELTAP2)
            {  //????????????,?????S?????
                s_i16PDeltaRef[u8Exfan_ID] = (s_i16PDeltaRef[u8Exfan_ID] + s_i16PRefAverage[u8Exfan_ID]) / 2;
                s_i16AdjTimes[u8Exfan_ID]  = 1;
            }
            if (s_i16AdjTimes[u8Exfan_ID] < P_ADJ_TIMES)
            {  //????????????????,??????????,???????????????
                s_i16AdjTimes[u8Exfan_ID]++;
                if (s_i16AdjTimes[u8Exfan_ID] > P_ADJ_ST)
                {  //?????????????
                    if (abs(i16Pressure - s_i16PDeltaRef[u8Exfan_ID]) < MAXDELTAP1)
                    {  //??30????????????????????MAXDELTAP1
                        s_i16AdjTimes[u8Exfan_ID] = P_ADJ_TIMES;
                    }
                }
                //??????????????????F=Fmin+(Pnow-Pset)*(Fmax-Fmin)/Pband
                //???????????????????????
                i16Temp =
                    g_sys.config.Ex_Fan.MINSPEEDID +
                    ((long)(i16Pressure - u16PressurePoint[P_SET_POINT]) *
                     (g_sys.config.Ex_Fan.MAXSPEEDID - g_sys.config.Ex_Fan.MINSPEEDID) / (g_sys.config.Ex_Fan.PBANDID));
                g_sys.status.Ex_Fan_Speed[u8Exfan_ID] = i16Temp;
            }
            //?????????
            g_i16PDeltaRef[u8Exfan_ID] = i16Pressure;
        }
    }
    else if (i16Pressure <= u16PressurePoint[P_FLAT2_POINT])
    {  //??????100%???????,???????100%??00%?????????
        g_sys.status.Ex_Fan_Speed[u8Exfan_ID] = g_sys.config.Ex_Fan.MAXSPEEDID;
    }
    else if (i16Pressure <= u16PressurePoint[P_STEP_POINT])
    {  //?????????,???????100%??????????
        if (g_sys.status.Ex_Fan_Speed[u8Exfan_ID] <= (g_sys.config.Ex_Fan.MAXSPEEDID))
        {
            g_sys.status.Ex_Fan_Speed[u8Exfan_ID] = g_sys.config.Ex_Fan.MAXSPEEDID;
        }
    }
    else
    {  //???????????,???????100%?????? 50Hz,????100?
        //		g_sys.status.Ex_Fan_Speed[0] = 100 ;
        g_sys.status.Ex_Fan_Speed[u8Exfan_ID] = g_sys.config.Ex_Fan.MAXSPEEDID;
    }
}

//外风机控制
static void Ex_Fan_Ctrl(void)
{
    extern sys_reg_st g_sys;
    extern fifo8_cb_td mbm_data_fifo;
    mbm_data_st mbm_send_data[2];
    int16_t i16Pressure_Buff[2];
    static uint8_t u8Cnt = 0;

    u8Cnt++;
    if (u8Cnt >= 250)
    {
        u8Cnt = 0;
    }
    if (u8Cnt % 2 == 0)
    {
        if (((g_sys.status.dout_bitmap & (0x0001 << DO_COMP1_BPOS)) != 0))
        {
            i16Pressure_Buff[0] = (int16_t)g_sys.status.ain[3];
            CalculateFout(0, i16Pressure_Buff[0]);
            if ((abs)(g_i16Ex_Fan_Speed[0] - (int16_t)g_sys.status.Ex_Fan_Speed[0]) >= SPEED_DIF)
            {
                g_i16Ex_Fan_Speed[0] = (int16_t)g_sys.status.Ex_Fan_Speed[0];

                if ((((g_sys.config.dev_mask.mb_comp >> MBM_DEV_EX_FAN_ADDR) & 0x0001) == 1) &&
                    (g_sys.config.Ex_Fan.FAN_TYPE))
                {
                    if (g_sys.status.mbm.EX_FAN[0].Set_Speed_Mode == 1)
                    {
                        mbm_send_data[0].mbm_fun_code = 6;
                        mbm_send_data[0].mbm_addr     = MBM_DEV_EX_FAN_ADDR + 1;
                        mbm_send_data[0].reg_addr     = 0x1038;
                        mbm_send_data[0].reg_value    = g_i16Ex_Fan_Speed[0];
                        //压入FIFO;
                        if (fifo8_push(&mbm_data_fifo, (uint8_t*)&mbm_send_data[1]) == 0)
                        {
                            rt_kprintf("\n mbm_data_fifo ERRO\n");
                        }
                    }
                }
                req_ao_op(AO_EX_FAN, g_i16Ex_Fan_Speed[0], 1);
            }
        }
        else
        {
            g_sys.status.Ex_Fan_Speed[0] = 0;
            g_i16Ex_Fan_Speed[0]         = (int16_t)g_sys.status.Ex_Fan_Speed[0];
            if (u8Cnt % 3 == 0)
            {
                //							if((((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EX_FAN_ADDR)&0x0001) ==
                // 1)&&(g_sys.config.Ex_Fan.FAN_TYPE))
                //							{
                //								g_i16Ex_Fan_Speed[0]=0x00;
                //								if(g_sys.status.mbm.EX_FAN[0].Set_Speed_Mode == 1)
                //								{
                //										mbm_send_data[0].mbm_fun_code = 6;
                //										mbm_send_data[0].mbm_addr = MBM_DEV_EX_FAN_ADDR + 1;
                //										mbm_send_data[0].reg_addr =0x1038;
                //										mbm_send_data[0].reg_value =g_i16Ex_Fan_Speed[0];
                //										//压入FIFO;
                //										if(fifo8_push(&mbm_data_fifo,(uint8_t*)&mbm_send_data[1]) == 0)
                //										{
                //												rt_kprintf("\n mbm_data_fifo ERRO\n");
                //										}
                //								}
                //							}
            }
            req_ao_op(AO_EX_FAN, g_i16Ex_Fan_Speed[0], 1);
        }
    }
    //		else
    //		{
    //			if(((g_sys.status.dout_bitmap &(0x0001<<DO_COMP2_BPOS))!=0))
    //			{
    //					i16Pressure_Buff[1]=(int16_t)g_sys.status.ain[4];
    //					CalculateFout(1,i16Pressure_Buff[1]);
    //					if((abs)(g_i16Ex_Fan_Speed[1]-(int16_t)g_sys.status.Ex_Fan_Speed[1])>=SPEED_DIF)
    //					{
    //							g_i16Ex_Fan_Speed[1]=(int16_t)g_sys.status.Ex_Fan_Speed[1];
    //							if((((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EX_FAN_ADDR)&0x0001) ==
    // 1)&&(g_sys.config.Ex_Fan.FAN_TYPE))
    //							{
    //								if(g_sys.status.mbm.EX_FAN[1].Set_Speed_Mode == 1)
    //								{
    //										mbm_send_data[1].mbm_fun_code = 6;
    //										mbm_send_data[1].mbm_addr = MBM_DEV_EX_FAN2_ADDR + 1;
    //										mbm_send_data[1].reg_addr =0x1038;
    //										mbm_send_data[1].reg_value =g_i16Ex_Fan_Speed[1];
    //										//压入FIFO;
    //										if(fifo8_push(&mbm_data_fifo,(uint8_t*)&mbm_send_data[1]) == 0)
    //										{
    //												rt_kprintf("\n mbm_data_fifo ERRO\n");
    //										}
    //								}
    //							}
    //							req_ao_op(AO_EX_FAN2,g_i16Ex_Fan_Speed[1]);
    //					}
    //			}
    //			else
    //			{
    //						g_sys.status.Ex_Fan_Speed[1]=0;
    //						g_i16Ex_Fan_Speed[1]=(int16_t)g_sys.status.Ex_Fan_Speed[1];
    //						if(u8Cnt%7==0)
    //						{
    ////							if((((g_sys.config.dev_mask.mb_comp>>MBM_DEV_EX_FAN_ADDR)&0x0001) ==
    /// 1)&&(g_sys.config.Ex_Fan.FAN_TYPE)) /							{ /
    /// if(g_sys.status.mbm.EX_FAN[1].Set_Speed_Mode == 1) /								{ /
    /// mbm_send_data[1].mbm_fun_code = 6;
    ////										mbm_send_data[1].mbm_addr = MBM_DEV_EX_FAN2_ADDR + 1;
    ////										mbm_send_data[1].reg_addr =0x1038;
    ////										mbm_send_data[1].reg_value =g_i16Ex_Fan_Speed[1];
    ////										//压入FIFO;
    ////										if(fifo8_push(&mbm_data_fifo,(uint8_t*)&mbm_send_data[1]) == 0)
    ////										{
    ////												rt_kprintf("\n mbm_data_fifo ERRO\n");
    ////										}
    ////								}
    ////							}
    //						}
    //						req_ao_op(AO_EX_FAN2,g_i16Ex_Fan_Speed[1]);
    //				}
    //		}
    //		rt_kprintf("u8Cnt= %x,Ex_Fan_Speed[0]= %d,Ex_Fan_Speed[1]=
    //%d\n",u8Cnt,g_sys.status.Ex_Fan_Speed[0],g_sys.status.Ex_Fan_Speed[1]);
}

//外风机温度控制
void Ex_Fan_Ctrl_Temp(void)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    uint16_t u16Condenser_Temp;

    u16Condenser_Temp = g_sys.status.ain[AI_NTC6];
    //		rt_kprintf("u16Condenser_Temp= %x,Ex_Fan_Speed[0]= %d,Ex_Fan_Speed[1]=
    //%d\n",u16Condenser_Temp,g_sys.status.Ex_Fan_Speed[0],g_sys.status.Ex_Fan_Speed[1]);
    if (sys_get_do_sts(DO_COMP1_BPOS) == 0)
    {
        l_sys.u16EF_Flag                     = FALSE;
        l_sys.comp_timeout[DO_CONDENSE_BPOS] = 0;
    }
    if (l_sys.u16EF_Flag == FALSE)
    {
        if (sys_get_do_sts(DO_COMP1_BPOS))
        {
            req_bitmap_op(DO_CONDENSE_BPOS, 1);
            l_sys.u16EF_Flag                     = TRUE;
            l_sys.comp_timeout[DO_CONDENSE_BPOS] = 30;
        }
    }
    if (l_sys.comp_timeout[DO_CONDENSE_BPOS])
    {
        return;
    }
    if ((sys_get_do_sts(DO_COMP1_BPOS)) && (u16Condenser_Temp >= g_sys.config.Ex_Fan.Start_Temp))
    {
        req_bitmap_op(DO_CONDENSE_BPOS, 1);
    }
    else if ((sys_get_do_sts(DO_COMP1_BPOS) == 0) ||
             (u16Condenser_Temp < (g_sys.config.Ex_Fan.Start_Temp - g_sys.config.Ex_Fan.Hyster_Temp)))
    {
        req_bitmap_op(DO_CONDENSE_BPOS, 0);
    }
    //		if((u16Condenser_Temp>=g_sys.config.Ex_Fan.Start_Temp))
    //		{
    //				req_bitmap_op(DO_CONDENSE_BPOS,1);
    //		}
    //		else
    //		if((u16Condenser_Temp<(g_sys.config.Ex_Fan.Start_Temp-g_sys.config.Ex_Fan.Hyster_Temp)))
    //		{
    //				req_bitmap_op(DO_CONDENSE_BPOS,0);
    //		}
    return;
}
// compressor requirement execution
void compressor_req_exe(int16_t req_temp, int16_t req_hum)
{
    uint8_t comp1_sig, comp2_sig;

    compressor_signal_gen(req_temp, req_hum, &comp1_sig, &comp2_sig);  // FSM signal generation
    compressor_alarm_signal_gen(&comp1_sig, &comp2_sig);               // compressors alarm
    //		rt_kprintf("req_temp = %d,DEMHUM_STS_BPOS = %d,comp1_sig =
    //%d\n",req_temp,sys_get_remap_status(WORK_MODE_STS_REG_NO,DEMHUM_STS_BPOS),comp1_sig);
    compressor_fsm(0, comp1_sig);  // compressor 1 FSM execution
    compressor_fsm(1, comp2_sig);  // compressor 2 FSM execution
    //    liq_sole_valve();
    //    ev_ctrl_exe(0);                                                    //EEV control logic
    //    ev_ctrl_exe(1);                                                    //EEV control logic
    ec_compressor_output(req_temp, comp1_sig);  // invertor compressor analog output control
    Ex_Fan_Ctrl();                              //外风机控制
                                                //		Ex_Fan_Ctrl_Temp();//外风机温度控制
    return;
}

static uint16_t watervalve_signal_gen(int16_t req_temp, uint8_t* watervalve_sig)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;

    if (sys_get_do_sts(DO_FAN_BPOS) == 0)  // fan disabled, emergency shutdown
    {
        *watervalve_sig = WATER_COOLED_SIG_ERR;
        return 0;
    }

    if ((sys_get_pwr_sts() == 0) || (g_sys.config.general.cool_type == COOL_TYPE_MODULE_WIND) ||
        (g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND) ||
        (l_sys.l_fsm_state[FAN_FSM_STATE] != FSM_FAN_NORM) ||
        (sys_get_remap_status(WORK_MODE_STS_REG_NO, FAN_STS_BPOS) == 0))
    {
        *watervalve_sig = WATER_COOLED_SIG_OFF;
        return 0;
    }

    if ((req_temp > g_sys.config.water_valve.start_req) ||
        (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) != 0))
    {
        *watervalve_sig = WATER_COOLED_SIG_ON;
    }
    //    else if(req_temp <= 0)
    //需求小于0关闭制冷,Alair,2018.01.27
    else if (req_temp < 0)
    {
        *watervalve_sig = WATER_COOLED_SIG_OFF;
    }
    else
    {
        *watervalve_sig = WATER_COOLED_SIG_HOLD;
    }

    return 1;
}

static void watervalve_fsm(uint8_t signal)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;

    uint16_t watervalve_fsm_state;

    watervalve_fsm_state = l_sys.l_fsm_state[WATERVALVE_FSM_STATE];

    switch (watervalve_fsm_state)
    {
        case (WATERVALVE_FSM_STATE_STOP): {
            if (signal == WATER_COOLED_SIG_ON)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_STARTUP;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = g_sys.config.water_valve.action_delay;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 0);
            }
            else
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_STOP;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS];
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 0);
            }
            break;
        }
        case (WATERVALVE_FSM_STATE_STARTUP): {
            if (signal != WATER_COOLED_SIG_ON)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_STOP;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = 0;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 0);
            }
            else if (l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] == 0)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE] = WATERVALVE_FSM_STATE_WARMUP;
                l_sys.watervalve_warmup_delay           = g_sys.config.water_valve.temp_act_delay;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 1);
            }
            else
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_STARTUP;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS];
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 0);
            }
            break;
        }
        case (WATERVALVE_FSM_STATE_WARMUP): {
            if (signal == WATER_COOLED_SIG_ERR)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_STOP;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = 0;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 0);
            }
            else if (signal == WATER_COOLED_SIG_OFF)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_WARMUP_SHUTING;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = g_sys.config.water_valve.action_delay;
                l_sys.watervalve_warmup_delay                 = l_sys.watervalve_warmup_delay;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 1);
            }
            else if (l_sys.watervalve_warmup_delay == 0)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_NORMAL;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS];
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 1);
            }
            else
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE] = WATERVALVE_FSM_STATE_WARMUP;
                l_sys.watervalve_warmup_delay           = l_sys.watervalve_warmup_delay;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 1);
            }
            break;
        }
        case (WATERVALVE_FSM_STATE_WARMUP_SHUTING): {
            if (signal == WATER_COOLED_SIG_ERR)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_STOP;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = 0;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 0);
            }
            else if (signal == WATER_COOLED_SIG_ON)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_WARMUP;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = 0;
                l_sys.watervalve_warmup_delay                 = l_sys.watervalve_warmup_delay;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 1);
            }
            else if (l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] == 0)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_STOP;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = 0;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 0);
            }
            else
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_WARMUP_SHUTING;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS];
                l_sys.watervalve_warmup_delay                 = l_sys.watervalve_warmup_delay;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 1);
            }
            break;
        }
        case (WATERVALVE_FSM_STATE_NORMAL): {
            if (signal == WATER_COOLED_SIG_ERR)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_STOP;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = 0;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 0);
            }
            else if (signal == WATER_COOLED_SIG_OFF)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_SHUTING;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = g_sys.config.water_valve.action_delay;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 1);
            }
            else
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_NORMAL;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS];
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 1);
            }
            break;
        }
        case (WATERVALVE_FSM_STATE_SHUTING): {
            if (signal == WATER_COOLED_SIG_ERR)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_STOP;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = 0;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 0);
            }
            else if (signal == WATER_COOLED_SIG_ON)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_NORMAL;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS];
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 1);
            }
            else if (l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] == 0)
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_STOP;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = 0;
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 0);
            }
            else
            {
                l_sys.l_fsm_state[WATERVALVE_FSM_STATE]       = WATERVALVE_FSM_STATE_SHUTING;
                l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS] = l_sys.comp_timeout[DO_WATER_VALVE_DUMMY_BPOS];
                req_bitmap_op(DO_WATER_VALVE_DUMMY_BPOS, 1);
            }
            break;
        }
    }
}

static void watervalve_output(int16_t req_temp)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    uint16_t watervalve_opening;
    uint16_t max, min;

    // calc watervalve_opening
    max = g_sys.config.water_valve.max_opening;
    min = g_sys.config.water_valve.min_opening;
    if (req_temp >= 100)
    {
        watervalve_opening = max;
    }
    else if (req_temp <= 0)
    {
        watervalve_opening = min;
    }
    else
    {
        watervalve_opening = req_temp;
    }
    if (watervalve_opening > max)
    {
        watervalve_opening = max;
    }
    if (watervalve_opening < min)
    {
        watervalve_opening = min;
    }
    //手动调节
    if (!g_sys.config.water_valve.auto_mode_en)
    {
        watervalve_opening = g_sys.config.water_valve.set_opening;
    }
    //除湿状态位
    if (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) != 0)
    {
        watervalve_opening = max;
    }
    else if (sys_get_remap_status(WORK_MODE_STS_REG_NO, HEATING_STS_BPOS) != 0)
    {  //未除湿加热下关闭水阀
        watervalve_opening = 0;
    }

    // output
    //    if(g_sys.config.water_valve.auto_mode_en)
    //    {
    if ((g_sys.status.dout_bitmap & (0x0001 << DO_WATER_VALVE_DUMMY_BPOS)))
    //				if(l_sys.bitmap[BITMAP_REQ] & (0x0001 << DO_WATER_VALVE_DUMMY_BPOS))
    {
        // auto calc value
        req_ao_op(AO_WATER_VALVE, watervalve_opening, 1);
    }
    else
    {
        req_ao_op(AO_WATER_VALVE, 0, 1);
    }
    //    }
    //    else
    //    {
    //        if((g_sys.status.dout_bitmap & (0x0001 << DO_WATER_VALVE_DUMMY_BPOS)))
    ////				if(l_sys.bitmap[BITMAP_REQ] & (0x0001 << DO_WATER_VALVE_DUMMY_BPOS))
    //        {
    //            //set value
    //            req_ao_op(AO_WATER_VALVE, g_sys.config.water_valve.set_opening);
    //        }
    //        else
    //        {
    //            req_ao_op(AO_WATER_VALVE, 0);
    //        }
    //    }
}

// water valve requirement execution
void watervalve_req_exe(int16_t req_temp, int16_t req_hum)
{
    uint8_t watervalve_sig;
    watervalve_signal_gen(req_temp, &watervalve_sig);
    watervalve_fsm(watervalve_sig);
    watervalve_output(req_temp);
}

/**************************************
HEATER requirement execution function
**************************************/
//电加热需求执行函数
void heater_req_exe(int16_t req_temp, int16_t req_hum)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    uint16_t rh_sts;
    uint16_t heater_level;
    //    int16_t require;
    //
    //    require = 0;
    heater_level = devinfo_get_heater_level();

    rh_sts = sys_get_do_sts(DO_RH1_BPOS) | (sys_get_do_sts(DO_RH2_BPOS) << 1);

    if ((l_sys.l_fsm_state[FAN_FSM_STATE] != FSM_FAN_NORM) || (sys_get_pwr_sts() == 0) ||
        (sys_get_remap_status(WORK_MODE_STS_REG_NO, FAN_STS_BPOS) ==
         0))  // fan disabled or power off, emergency shutdown
    {
        req_bitmap_op(DO_RH1_BPOS, 0);
        req_bitmap_op(DO_RH2_BPOS, 0);
        //        req_pwm_op(PWM_OUT0,0);
        sys_set_remap_status(WORK_MODE_STS_REG_NO, HEATING_STS_BPOS, 0);
        return;
    }
    if (heater_level != 0)
    {
        // M3只有一级加热
        if (heater_level >= 1)
        {
            heater_level = 1;
        }
        switch (heater_level)
        {
            case (0): {
                req_bitmap_op(DO_RH1_BPOS, 0);
                break;
            }
            case (1): {
                if (req_temp <= -100)
                {
                    req_bitmap_op(DO_RH1_BPOS, 1);
                    req_bitmap_op(DO_RH2_BPOS, 0);
                }
                //                else if(req_temp >= 0)
                //需求大于0关闭制热,Alair,2018.01.27
                else if (req_temp > 0)
                {
                    req_bitmap_op(DO_RH1_BPOS, 0);
                    req_bitmap_op(DO_RH2_BPOS, 0);
                }
                else
                {
                    ;
                }
                break;
            }
            case (3): {
                switch (rh_sts)
                {
                    case (HEATER_SIG_IDLE): {
                        if (req_temp < -33)
                        {
                            req_bitmap_op(DO_RH1_BPOS, 1);
                            req_bitmap_op(DO_RH2_BPOS, 0);
                        }
                        else
                        {
                            ;
                        }
                        break;
                    }
                    case (HEATER_SIG_L1): {
                        if (req_temp < -66)
                        {
                            req_bitmap_op(DO_RH1_BPOS, 0);
                            req_bitmap_op(DO_RH2_BPOS, 1);
                        }
                        else if (req_temp >= 0)
                        {
                            req_bitmap_op(DO_RH1_BPOS, 0);
                            req_bitmap_op(DO_RH2_BPOS, 0);
                        }
                        else
                        {
                            ;
                        }
                        break;
                    }
                    case (HEATER_SIG_L2): {
                        if (req_temp <= -100)
                        {
                            req_bitmap_op(DO_RH1_BPOS, 1);
                            req_bitmap_op(DO_RH2_BPOS, 1);
                        }
                        else if (req_temp >= -33)
                        {
                            req_bitmap_op(DO_RH1_BPOS, 1);
                            req_bitmap_op(DO_RH2_BPOS, 0);
                        }
                        else
                        {
                            ;
                        }
                        break;
                    }
                    case (HEATER_SIG_L3): {
                        if (req_temp >= -66)
                        {
                            req_bitmap_op(DO_RH1_BPOS, 0);
                            req_bitmap_op(DO_RH2_BPOS, 1);
                        }
                        else
                        {
                            ;
                        }
                        break;
                    }
                    default: {
                        req_bitmap_op(DO_RH1_BPOS, 0);
                        req_bitmap_op(DO_RH2_BPOS, 0);
                        break;
                    }
                }
                break;
            }
            default: {
                req_bitmap_op(DO_RH1_BPOS, 0);
                req_bitmap_op(DO_RH2_BPOS, 0);
                break;
            }
        }
        //        req_pwm_op(PWM_OUT0,0);
    }
    //    else if((g_sys.config.dev_mask.pwm_out&(0x0001<<PWM_OUT0)) != 0)
    //    {
    //        if(req_temp >= 0)
    //        {
    //            require = 0 ;
    //        }
    //        else if(req_temp < (-g_sys.config.heater.start_req))
    //        {
    //            require = abs(req_temp);
    //        }
    //        else
    //        {
    //            if(g_sys.status.pwmout[PWM_OUT0] != 0)
    //            {
    //                require = abs(req_temp);
    //            }
    //            else
    //            {
    //                require = 0;
    //            }
    //        }
    //        require = lim_min_max(0,100,require);
    //        req_bitmap_op(DO_RH1_BPOS,0);
    //				req_bitmap_op(DO_RH2_BPOS,0);
    //        req_pwm_op(PWM_OUT0,require);
    //    }
    else
    {
        req_bitmap_op(DO_RH1_BPOS, 0);
        req_bitmap_op(DO_RH2_BPOS, 0);
        //        req_pwm_op(PWM_OUT0,0);
    }

    return;
}

/**************************************
HUMIDIFIER requirement execution function
**************************************/
static uint16_t hum_signal_gen(int16_t req_hum)
{
    static uint8_t hum_sig = 0;
    if ((req_hum >= 0) || (sys_get_pwr_sts() == 0) || (sys_get_do_sts(DO_FAN_BPOS) == 0))
    {
        hum_sig = 0;
    }
    else if ((req_hum <= -100) && ((sys_get_remap_status(WORK_MODE_STS_REG_NO, FAN_STS_BPOS) == 1)))  //加湿
    {
        hum_sig = 1;
    }
    else
    {
        hum_sig = hum_sig;
    }
    return hum_sig;
}

//加湿器高水位检测
static uint16_t hum_high_level(void)
{
    uint8_t data;
    static uint32_t hi_level = 0;
    extern sys_reg_st g_sys;

    data = g_sys.status.mbm.hum.water_level;

    {
        if (data == 1)
        {
            hi_level = (hi_level | 0x01) << 1;
        }
        else
        {
            hi_level = hi_level << 1;
        }
        if (hi_level)
        {
            return (1);
        }
        else
        {
            return (0);
        }
    }
}
void hum_capacity_calc(void)
{
    extern sys_reg_st g_sys;
    float fBuff = 0;

    //		switch(g_sys.config.humidifier.hum_cap_type)
    //		{
    //			case 0:// 3KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 30;
    //					break;
    //			}
    //			case 1:// 5 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 50;
    //					break;
    //			}
    //			case 2:// 8 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 80;
    //					break;
    //			}
    //			case 3:// 10 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 100;
    //					break;
    //			}
    //			case 4:// 13 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 130;
    //					break;
    //			}
    //			case 5:// 15 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 150;
    //					break;
    //			}
    //			case 6:// 23KG
    ////			{
    ////					g_sys.config.humidifier.hum_real_cap = 230;
    ////					break;
    ////			}
    ////			case 7:// 42 KG
    ////			{
    ////					g_sys.config.humidifier.hum_real_cap = 420;
    ////					break;
    ////			}
    //			case 0:// 1KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 10;
    //					break;
    //			}
    //			case 1:// 1.5 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 15;
    //					break;
    //			}
    //			case 2:// 2 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 20;
    //					break;
    //			}
    //			case 3:// 3KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 30;
    //					break;
    //			}
    //			case 4:// 5 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 50;
    //					break;
    //			}
    //			case 5:// 8 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 80;
    //					break;
    //			}
    //			case 6:// 10 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 100;
    //					break;
    //			}
    //			case 7:// 13 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 130;
    //					break;
    //			}
    //			case 8:// 15 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 150;
    //					break;
    //			}
    //			case 9:// 23KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 230;
    //					break;
    //			}
    //			case 10:// 42 KG
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 420;
    //					break;
    //			}
    //			default:
    //			{
    //					g_sys.config.humidifier.hum_real_cap = 50;
    ////					g_sys.config.humidifier.hum_real_cap =
    /// g_sys.config.humidifier.hum_capacity*g_sys.config.humidifier.water_conductivity/100;
    //					break;
    //			}
    //		}
    // alair,20170112
    //		g_sys.config.humidifier.hum_real_cap *= g_sys.config.humidifier.water_conductivity/100;
    // Alair,20200506
    g_sys.config.humidifier.hum_real_cap = g_sys.config.humidifier.hum_cap_type;
    fBuff                                = g_sys.config.humidifier.water_conductivity;
    fBuff /= 100;
    fBuff *= g_sys.config.humidifier.hum_real_cap;
    g_sys.config.humidifier.hum_real_cap = fBuff;
    //		rt_kprintf("g_sys.config.humidifier.hum_real_cap = %d\n",g_sys.config.humidifier.hum_real_cap);
}
static void hum_checke_timer(void)
{
    if (hum_delay_timer.check_timer > 0)
    {
        hum_delay_timer.check_timer--;
    }
}
static void hum_time_out(void)
{
    if (hum_delay_timer.time_out > 0)
    {
        hum_delay_timer.time_out--;
    }
}
//红外加湿
static void infrared_hum_fsm(uint8_t hum_sig)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    static uint8_t drain_abs_timer = 0;

    // infrared hum
    if (g_sys.config.ac_power_supply.PH_Ver == 0)
    {
        if ((sys_get_pwr_sts() == 0) || (g_sys.config.humidifier.hum_en == 0) ||
            (sys_get_mbm_online(POWER_MODULE_BPOS) == 0) || (l_sys.l_fsm_state[FAN_FSM_STATE] != FSM_FAN_NORM) ||
            (sys_get_remap_status(WORK_MODE_STS_REG_NO, FAN_STS_BPOS) == 0))  // fan disabled, emergency shutdown
        {
            l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
            sys_set_remap_status(WORK_MODE_STS_REG_NO, HUMING_STS_BPOS, 0);
            hum_delay_timer.check_timer = 0;
            req_bitmap_op(DO_DRAIN_BPOS, 0);
            req_bitmap_op(DO_FILL_BPOS, 0);
            req_bitmap_op(DO_HUM_BPOS, 0);
            return;
        }
    }
    else
    {
        if ((sys_get_pwr_sts() == 0) || (g_sys.config.humidifier.hum_en == 0) ||
            (sys_get_mbm_online(HUM_MODULE_BPOS) == 0) || (l_sys.l_fsm_state[FAN_FSM_STATE] != FSM_FAN_NORM) ||
            (sys_get_remap_status(WORK_MODE_STS_REG_NO, FAN_STS_BPOS) == 0))  // fan disabled, emergency shutdown
        {
            l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
            sys_set_remap_status(WORK_MODE_STS_REG_NO, HUMING_STS_BPOS, 0);
            hum_delay_timer.check_timer = 0;
            req_bitmap_op(DO_DRAIN_BPOS, 0);
            req_bitmap_op(DO_FILL_BPOS, 0);
            req_bitmap_op(DO_HUM_BPOS, 0);
            return;
        }
    }

    //进入加湿条件
    switch (l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE])
    {
        case HUM_FSM_STATE_IDLE: {
            // go to check
            if ((hum_sig == 1) && (get_alarm_bitmap(ACL_HUM_LO_LEVEL) == 0) &&
                (get_alarm_bitmap(ACL_HUM_DEFAULT) == 0) && (get_alarm_bitmap(ACL_HUM_OD) == 0) &&
                (get_alarm_bitmap(ACL_HUM_OCURRENT) == 0) && (hum_delay_timer.check_timer == 0))  //启动check 模式
            {
                hum_delay_timer.check_fill_flag = 1;
                hum_delay_timer.time_out        = g_sys.config.alarm[ACL_HUM_DEFAULT].alarm_param;

                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_CHECK;
                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 1);
                //							rt_kprintf("go to HUM_FSM_STATE_CHECK = %d\n",now);
            }

            // go to warm
            else if ((hum_sig == 1) && (get_alarm_bitmap(ACL_HUM_LO_LEVEL) == 0) &&
                     (get_alarm_bitmap(ACL_HUM_DEFAULT) == 0) && (get_alarm_bitmap(ACL_HUM_OD) == 0) &&
                     (get_alarm_bitmap(ACL_HUM_OCURRENT) == 0) && (hum_delay_timer.check_timer > 0))
            {
                hum_delay_timer.flush_delay_timer = 0;
                hum_delay_timer.hum_fill_cnt      = 0;
                hum_delay_timer.hum_timer         = 0;
                hum_delay_timer.time_out          = g_sys.config.humidifier.drain_timer;
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 1);
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_WARM;
                //										rt_kprintf("go to HUM_FSM_STATE_WARM = %d\n",now);
            }
            else
            {
                hum_checke_timer();
                req_bitmap_op(DO_FILL_BPOS, 0);
                req_bitmap_op(DO_HUM_BPOS, 0);
                if (hum_high_level() == 1)  //开启排水阀
                {
                    req_bitmap_op(DO_DRAIN_BPOS, 1);
                }
                else  //关闭排水阀
                {
                    req_bitmap_op(DO_DRAIN_BPOS, 0);
                }
            }
            break;
        }
        case HUM_FSM_STATE_CHECK: {
            if (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1)
            {
                sys_option_di_sts(DI_HUM_DEFAULT_BPOS, 1);
                hum_delay_timer.check_timer             = 0;
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
                //									rt_kprintf("go to HUM_FSM_STATE_IDLE= %d\n",now);
            }
            else
            {
                hum_time_out();
                if ((hum_delay_timer.check_fill_flag == 1) &&
                    (hum_delay_timer.time_out > 0))  // g_sys.config.alarm[ACL_HUM_DEFAULT].alarm_param))
                {
                    req_bitmap_op(DO_HUM_BPOS, 0);
                    req_bitmap_op(DO_DRAIN_BPOS, 0);
                    req_bitmap_op(DO_FILL_BPOS, 1);
                    if (hum_high_level() == 1)
                    {
                        hum_delay_timer.check_fill_flag = 0;
                        hum_delay_timer.time_out        = g_sys.config.humidifier.drain_timer;
                    }
                }
                else
                {
                    if (hum_delay_timer.check_fill_flag == 0)
                    {
                        if (hum_delay_timer.time_out >
                            0)  // if(hum_delay_timer.time_out < g_sys.config.humidifier.drain_timer)
                        {
                            drain_abs_timer++;
                            req_bitmap_op(DO_HUM_BPOS, 0);
                            // open 5s close 3s
                            if (drain_abs_timer < 5)
                            {
                                req_bitmap_op(DO_DRAIN_BPOS, 1);
                            }
                            else
                            {
                                req_bitmap_op(DO_DRAIN_BPOS, 0);
                                if (drain_abs_timer > 8)
                                {
                                    drain_abs_timer = 0;
                                }
                            }

                            req_bitmap_op(DO_FILL_BPOS, 0);
                            hum_high_level();
                        }
                        else
                        {
                            if (hum_high_level() == 0)
                            {
                                hum_delay_timer.time_out    = g_sys.config.alarm[ACL_HUM_DEFAULT].alarm_param;
                                hum_delay_timer.check_timer = HUM_CHECKE_INTERVAL;

                                hum_delay_timer.flush_delay_timer = 0;
                                hum_delay_timer.hum_fill_cnt      = 0;
                                hum_delay_timer.hum_timer         = 0;

                                sys_option_di_sts(DI_HUM_DEFAULT_BPOS, 0);
                                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_WARM;
                                req_bitmap_op(DO_HUM_BPOS, 1);
                                req_bitmap_op(DO_DRAIN_BPOS, 0);
                                req_bitmap_op(DO_FILL_BPOS, 1);

                                //																	 rt_kprintf("go to
                                // HUM_FSM_STATE_WARM= %d\n",now);
                            }
                            else
                            {
                                sys_option_di_sts(DI_HUM_DEFAULT_BPOS, 1);
                                hum_delay_timer.check_timer             = 0;
                                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
                                req_bitmap_op(DO_HUM_BPOS, 0);
                                req_bitmap_op(DO_DRAIN_BPOS, 0);
                                req_bitmap_op(DO_FILL_BPOS, 0);
                                //																	rt_kprintf("go to
                                // HUM_FSM_STATE_IDLE= %d\n",now);
                            }
                        }
                    }
                    else
                    {
                        sys_option_di_sts(DI_HUM_DEFAULT_BPOS, 1);
                        hum_delay_timer.check_timer             = 0;
                        l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
                        req_bitmap_op(DO_HUM_BPOS, 0);
                        req_bitmap_op(DO_DRAIN_BPOS, 0);
                        req_bitmap_op(DO_FILL_BPOS, 0);
                        //												rt_kprintf("go to HUM_FSM_STATE_IDLE=
                        //%d\n",now);
                    }
                }
            }
            break;
        }
        case HUM_FSM_STATE_WARM: {
            if ((hum_sig == 0) || (hum_high_level() == 1) || (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1) ||
                (hum_delay_timer.time_out == 0))
            {
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_HUM;
                // rt_kprintf("go to HUM_FSM_STATE_HUM= %d\n",now);
            }
            else
            {
                hum_delay_timer.hum_timer++;
                hum_checke_timer();
                hum_time_out();
                //	no hum wait high level
                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 1);
            }

            break;
        }

        case HUM_FSM_STATE_HUM: {
            // flash_current mA

            //	 go to HUM_STATE_IDLE
            if ((hum_sig == 0) || (get_alarm_bitmap(ACL_HUM_LO_LEVEL) == 1) || (get_alarm_bitmap(ACL_HUM_OD) == 1) ||
                (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1) || (get_alarm_bitmap(ACL_HUM_OCURRENT) == 1))
            {
                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
                //								rt_kprintf("go to HUM_FSM_STATE_IDLE= %d\n",now);
            }
            // go to HUM_STATE_FILL
            else if (hum_high_level() == 0)
            {
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 1);
                hum_delay_timer.hum_fill_cnt++;
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_FILL;
                //								rt_kprintf("go to HUM_FSM_STATE_FILL= %d\n",now);
            }
            //    go to HUM_STATE_FLUSH
            else if ((hum_delay_timer.flush_delay_timer > g_sys.config.humidifier.flush_interval) &&
                     ((hum_delay_timer.hum_fill_cnt > g_sys.config.humidifier.fill_cnt) ||
                      (hum_delay_timer.hum_timer > g_sys.config.humidifier.hum_time * 60)))
            {
                hum_delay_timer.time_out          = g_sys.config.humidifier.flush_time;
                hum_delay_timer.hum_timer         = 0;
                hum_delay_timer.hum_fill_cnt      = 0;
                hum_delay_timer.flush_delay_timer = 0;

                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 1);
                req_bitmap_op(DO_FILL_BPOS, 1);

                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_FLUSH;
                //								rt_kprintf("go to HUM_FSM_STATE_FLUSH= %d\n",now);
            }
            else  // hum
            {
                //开启加湿器
                hum_delay_timer.flush_delay_timer++;
                hum_delay_timer.hum_timer++;
                hum_checke_timer();
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
            }
            break;
        }
        case HUM_FSM_STATE_FILL: {
            // go to  HUM_STATE_HUM
            if ((hum_sig == 0) || (get_alarm_bitmap(ACL_HUM_LO_LEVEL) == 1) ||
                (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1) || (hum_high_level() == 1) ||
                (get_alarm_bitmap(ACL_HUM_OD) == 1))
            {
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);

                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_HUM;
                //							rt_kprintf("go to HUM_FSM_STATE_HUM= %d\n",now);
            }
            else
            {
                hum_delay_timer.flush_delay_timer++;
                hum_delay_timer.hum_timer++;
                hum_checke_timer();
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 1);
            }
            break;
        }

        case HUM_FSM_STATE_FLUSH: {
            // time out goto HUM_STATE_HUM
            if ((hum_sig == 0) || (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1) || (hum_delay_timer.time_out == 0) ||
                (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1) || (get_alarm_bitmap(ACL_HUM_OD) == 1))
            {
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_HUM;
                //								rt_kprintf("go to HUM_FSM_STATE_ADJUST= %d\n",now);
            }
            else
            {
                hum_time_out();
                hum_checke_timer();
                // open flush
                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 1);
                req_bitmap_op(DO_FILL_BPOS, 1);
                //水位高
                if (hum_high_level() == 1)
                {
                    req_bitmap_op(DO_FILL_BPOS, 0);  //关闭注水阀
                }
                else
                {
                    req_bitmap_op(DO_FILL_BPOS, 1);  //开启注水阀
                }
            }
            break;
        }
        default: {
            l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
            hum_delay_timer.check_timer             = 0;
            req_bitmap_op(DO_DRAIN_BPOS, 0);
            req_bitmap_op(DO_FILL_BPOS, 0);
            //关闭加湿器
            req_bitmap_op(DO_HUM_BPOS, 0);
            break;
        }
    }
}

static void humidifier_fsm(uint8_t hum_sig, int16_t target_req_hum)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    static uint8_t drain_abs_timer = 0;
    uint16_t req_hum;
    uint16_t flash_current, fill_current, full_current;
    uint8_t u8HumStart = FALSE;
    // test
    //		time_t now;
    //		get_local_time(&now);

    //无加湿板
    if (g_sys.config.ac_power_supply.PH_Ver == 0)
    {
        if ((sys_get_mbm_online(POWER_MODULE_BPOS) == 1) && (g_sys.config.humidifier.hum_en == 1))
        {
            u8HumStart = TRUE;
        }
        else
        {
            u8HumStart = FALSE;
        }
    }
    else
    {
        if ((sys_get_mbm_online(HUM_MODULE_BPOS) == 1) && (g_sys.config.humidifier.hum_en == 1))
        {
            u8HumStart = TRUE;
        }
        else
        {
            u8HumStart = FALSE;
        }
    }
    //上电自检，快速加湿
    if ((g_sys.config.humidifier.HumStart) && (u8HumStart == TRUE))
    {
        if (l_sys.HumCheck == 0)
        {
            l_sys.HumCheck                          = 1;
            l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STARTCHECK_FILL;
            l_sys.Humer_Delay                       = 10;
            return;
        }
        else if (l_sys.HumCheck == 1)
        {
            switch (l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE])
            {
                case HUM_FSM_STARTCHECK_FILL:  //注水
                {
                    if (l_sys.Humer_Delay)
                    {
                        req_bitmap_op(DO_FILL_BPOS, 1);
                        req_bitmap_op(DO_DRAIN_BPOS, 1);
                    }
                    else
                    {
                        req_bitmap_op(DO_FILL_BPOS, 0);
                        req_bitmap_op(DO_DRAIN_BPOS, 0);
                        l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
                        l_sys.HumCheck                          = 2;
                    }
                    break;
                }
                default: {
                    l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
                    break;
                }
            }
            return;
        }
        else
        {
        }
    }
    // calc_hum_capacity
    hum_capacity_calc();

    if (g_sys.config.ac_power_supply.PH_Ver == 0)
    {
        if ((sys_get_pwr_sts() == 0) || (sys_get_mbm_online(POWER_MODULE_BPOS) == 0) ||
            (g_sys.config.humidifier.hum_en == 0) || (l_sys.l_fsm_state[FAN_FSM_STATE] != FSM_FAN_NORM) ||
            (sys_get_remap_status(WORK_MODE_STS_REG_NO, FAN_STS_BPOS) == 0))  // fan disabled, emergency shutdown
        {
            l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
            sys_set_remap_status(WORK_MODE_STS_REG_NO, HUMING_STS_BPOS, 0);
            hum_delay_timer.check_timer = 0;
            req_bitmap_op(DO_DRAIN_BPOS, 0);
            req_bitmap_op(DO_FILL_BPOS, 0);
            req_bitmap_op(DO_HUM_BPOS, 0);
            return;
        }
    }
    else
    {
        if ((sys_get_pwr_sts() == 0) || (sys_get_mbm_online(HUM_MODULE_BPOS) == 0) ||
            (g_sys.config.humidifier.hum_en == 0) || (l_sys.l_fsm_state[FAN_FSM_STATE] != FSM_FAN_NORM) ||
            (sys_get_remap_status(WORK_MODE_STS_REG_NO, FAN_STS_BPOS) == 0))  // fan disabled, emergency shutdown
        {
            l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
            sys_set_remap_status(WORK_MODE_STS_REG_NO, HUMING_STS_BPOS, 0);
            hum_delay_timer.check_timer = 0;
            req_bitmap_op(DO_DRAIN_BPOS, 0);
            req_bitmap_op(DO_FILL_BPOS, 0);
            req_bitmap_op(DO_HUM_BPOS, 0);
            return;
        }
    }

    // K
    if (g_sys.config.humidifier.hum_type == HUM_TYPE_P)
    {
        req_hum = (0 - target_req_hum) / 10;
        req_hum = req_hum * 10;
        if (req_hum < g_sys.config.humidifier.min_percentage)
        {
            req_hum = g_sys.config.humidifier.min_percentage;
        }
        if (req_hum > 100)
        {
            req_hum = 100;
        }
    }
    else
    {
        req_hum = 100;
    }

    //进入加湿条件
    switch (l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE])
    {
        case HUM_FSM_STATE_IDLE: {
            // go to check
            if ((hum_sig == 1) && (get_alarm_bitmap(ACL_HUM_LO_LEVEL) == 0) &&
                (get_alarm_bitmap(ACL_HUM_DEFAULT) == 0) && (get_alarm_bitmap(ACL_HUM_OCURRENT) == 0) &&
                (hum_delay_timer.check_timer == 0))  //启动check 模式
            {
                if (g_sys.config.humidifier.HumStart)  //快速启动加湿
                {
                    l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_HUM;
                }
                else
                {
                    hum_delay_timer.check_fill_flag = 1;
                    hum_delay_timer.time_out        = g_sys.config.alarm[ACL_HUM_DEFAULT].alarm_param;

                    l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_CHECK;
                    req_bitmap_op(DO_HUM_BPOS, 0);
                    req_bitmap_op(DO_DRAIN_BPOS, 0);
                    req_bitmap_op(DO_FILL_BPOS, 1);
                }
            }

            // go to warm
            else if ((hum_sig == 1) && (get_alarm_bitmap(ACL_HUM_LO_LEVEL) == 0) &&
                     (get_alarm_bitmap(ACL_HUM_DEFAULT) == 0) && (get_alarm_bitmap(ACL_HUM_OCURRENT) == 0) &&
                     (hum_delay_timer.check_timer > 0))
            {
                hum_delay_timer.flush_delay_timer = 0;
                hum_delay_timer.hum_fill_cnt      = 0;
                hum_delay_timer.hum_timer         = 0;
                hum_delay_timer.time_out          = g_sys.config.humidifier.drain_timer;
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 1);
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_WARM;
                //										rt_kprintf("go to HUM_FSM_STATE_WARM = %d\n",now);
            }
            else
            {
                hum_checke_timer();
                req_bitmap_op(DO_FILL_BPOS, 0);
                req_bitmap_op(DO_HUM_BPOS, 0);
                if (hum_high_level() == 1)  //开启排水阀
                {
                    req_bitmap_op(DO_DRAIN_BPOS, 1);
                }
                else  //关闭排水阀
                {
                    req_bitmap_op(DO_DRAIN_BPOS, 0);
                }
            }
            break;
        }
        case HUM_FSM_STATE_CHECK: {
            if (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1)
            {
                sys_option_di_sts(DI_HUM_DEFAULT_BPOS, 1);
                hum_delay_timer.check_timer             = 0;
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
                //									rt_kprintf("go to HUM_FSM_STATE_IDLE= %d\n",now);
            }
            else
            {
                hum_time_out();
                if ((hum_delay_timer.check_fill_flag == 1) &&
                    (hum_delay_timer.time_out > 0))  // g_sys.config.alarm[ACL_HUM_DEFAULT].alarm_param))
                {
                    req_bitmap_op(DO_HUM_BPOS, 0);
                    req_bitmap_op(DO_DRAIN_BPOS, 0);
                    req_bitmap_op(DO_FILL_BPOS, 1);
                    if (hum_high_level() == 1)
                    {
                        hum_delay_timer.check_fill_flag = 0;
                        hum_delay_timer.time_out        = g_sys.config.humidifier.drain_timer;
                    }
                }
                else
                {
                    if (hum_delay_timer.check_fill_flag == 0)
                    {
                        if (hum_delay_timer.time_out >
                            0)  // if(hum_delay_timer.time_out < g_sys.config.humidifier.drain_timer)
                        {
                            drain_abs_timer++;
                            req_bitmap_op(DO_HUM_BPOS, 0);
                            // open 5s close 3s
                            if (drain_abs_timer < 5)
                            {
                                req_bitmap_op(DO_DRAIN_BPOS, 1);
                            }
                            else
                            {
                                req_bitmap_op(DO_DRAIN_BPOS, 0);
                                if (drain_abs_timer > 8)
                                {
                                    drain_abs_timer = 0;
                                }
                            }

                            req_bitmap_op(DO_FILL_BPOS, 0);
                            hum_high_level();
                        }
                        else
                        {
                            if (hum_high_level() == 0)
                            {
                                hum_delay_timer.time_out    = g_sys.config.alarm[ACL_HUM_DEFAULT].alarm_param;
                                hum_delay_timer.check_timer = HUM_CHECKE_INTERVAL;

                                hum_delay_timer.flush_delay_timer = 0;
                                hum_delay_timer.hum_fill_cnt      = 0;
                                hum_delay_timer.hum_timer         = 0;

                                sys_option_di_sts(DI_HUM_DEFAULT_BPOS, 0);
                                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_WARM;
                                req_bitmap_op(DO_HUM_BPOS, 1);
                                req_bitmap_op(DO_DRAIN_BPOS, 0);
                                req_bitmap_op(DO_FILL_BPOS, 1);

                                //																	 rt_kprintf("go to
                                // HUM_FSM_STATE_WARM= %d\n",now);
                            }
                            else
                            {
                                sys_option_di_sts(DI_HUM_DEFAULT_BPOS, 1);
                                hum_delay_timer.check_timer             = 0;
                                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
                                req_bitmap_op(DO_HUM_BPOS, 0);
                                req_bitmap_op(DO_DRAIN_BPOS, 0);
                                req_bitmap_op(DO_FILL_BPOS, 0);
                                //																	rt_kprintf("go to
                                // HUM_FSM_STATE_IDLE= %d\n",now);
                            }
                        }
                    }
                    else
                    {
                        sys_option_di_sts(DI_HUM_DEFAULT_BPOS, 1);
                        hum_delay_timer.check_timer             = 0;
                        l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
                        req_bitmap_op(DO_HUM_BPOS, 0);
                        req_bitmap_op(DO_DRAIN_BPOS, 0);
                        req_bitmap_op(DO_FILL_BPOS, 0);
                        //												rt_kprintf("go to HUM_FSM_STATE_IDLE=
                        //%d\n",now);
                    }
                }
            }
            break;
        }
        case HUM_FSM_STATE_WARM: {
            full_current = g_sys.config.alarm[ACL_HUM_LO_LEVEL].alarm_param * HUM_CURRENT_UNIT *
                           g_sys.config.humidifier.hum_real_cap;
            // go to  HUM_STATE_HUM
            if ((hum_sig == 0) || (hum_high_level() == 1) || (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1) ||
                (hum_delay_timer.time_out == 0) || (g_sys.status.mbm.hum.hum_current > full_current))
            {
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_HUM;
                // rt_kprintf("go to HUM_FSM_STATE_HUM= %d\n",now);
            }
            else
            {
                hum_delay_timer.hum_timer++;
                hum_checke_timer();
                hum_time_out();

                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 1);
            }

            break;
        }

        case HUM_FSM_STATE_HUM: {
            // flash_current mA
            flash_current = (20 + 100) * HUM_CURRENT_UNIT * g_sys.config.humidifier.hum_real_cap;
            fill_current  = g_sys.config.humidifier.current_percentage * HUM_CURRENT_UNIT *
                           g_sys.config.humidifier.hum_real_cap * req_hum / 100;
            //	 go to HUM_STATE_IDLE
            if ((hum_sig == 0) || (get_alarm_bitmap(ACL_HUM_LO_LEVEL) == 1) ||
                (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1) || (get_alarm_bitmap(ACL_HUM_OCURRENT) == 1))
            {
                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
                //								rt_kprintf("go to HUM_FSM_STATE_IDLE= %d\n",now);
            }
            // go to HUM_STATE_FILL
            else if ((g_sys.status.mbm.hum.hum_current < fill_current) && (hum_high_level() == 0))
            {
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 1);
                hum_delay_timer.hum_fill_cnt++;
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_FILL;
                //								rt_kprintf("go to HUM_FSM_STATE_FILL= %d\n",now);
            }
            //    go to HUM_STATE_FLUSH
            else if ((hum_delay_timer.flush_delay_timer > g_sys.config.humidifier.flush_interval) &&
                     ((hum_delay_timer.hum_fill_cnt > g_sys.config.humidifier.fill_cnt) ||
                      (hum_delay_timer.hum_timer > g_sys.config.humidifier.hum_time * 60)))
            {
                hum_delay_timer.time_out          = g_sys.config.humidifier.flush_time;
                hum_delay_timer.hum_timer         = 0;
                hum_delay_timer.hum_fill_cnt      = 0;
                hum_delay_timer.flush_delay_timer = 0;

                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 1);
                req_bitmap_op(DO_FILL_BPOS, 1);

                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_FLUSH;
                //								rt_kprintf("go to HUM_FSM_STATE_FLUSH= %d\n",now);
            }
            else if ((g_sys.status.mbm.hum.hum_current >= flash_current))  // go to drain
            {
                hum_delay_timer.time_out = 3;
                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 1);
                req_bitmap_op(DO_FILL_BPOS, 0);

                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_DRAIN;
            }
            else  // hum
            {
                //开启加湿器
                hum_delay_timer.flush_delay_timer++;
                hum_delay_timer.hum_timer++;
                hum_checke_timer();
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
            }
            break;
        }
        case HUM_FSM_STATE_FILL: {
            full_current = HUM_CURRENT_UNIT * g_sys.config.humidifier.hum_real_cap * req_hum;
            // go to  HUM_STATE_HUM
            if ((hum_sig == 0) || (get_alarm_bitmap(ACL_HUM_LO_LEVEL) == 1) ||
                (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1) || (hum_high_level() == 1) ||
                (g_sys.status.mbm.hum.hum_current > full_current))
            {
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);

                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_HUM;

                if ((hum_high_level() == 1) && (g_sys.status.mbm.hum.hum_current < full_current))
                {
                    hum_delay_timer.hum_fill_cnt = 0;
                }
                //									rt_kprintf("go to HUM_FSM_STATE_HUM= %d\n",now);
            }
            else
            {
                hum_delay_timer.flush_delay_timer++;
                hum_delay_timer.hum_timer++;
                hum_checke_timer();
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 1);
            }
            break;
        }

        case HUM_FSM_STATE_DRAIN: {
            if ((hum_sig == 0) || (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1) || (hum_delay_timer.time_out == 0))
            {
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_HUM;
            }
            else  // drain
            {
                hum_time_out();
                hum_checke_timer();
                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 1);
                req_bitmap_op(DO_FILL_BPOS, 0);
                //								rt_kprintf("go to HUM_FSM_STATE_FILL= %d\n",now);
            }
            break;
        }
        case HUM_FSM_STATE_FLUSH: {
            // time out goto HUM_STATE_HUM
            if ((hum_sig == 0) || (get_alarm_bitmap(ACL_HUM_DEFAULT) == 1) || (hum_delay_timer.time_out == 0))
            {
                req_bitmap_op(DO_HUM_BPOS, 1);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
                l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_HUM;
                //								rt_kprintf("go to HUM_FSM_STATE_ADJUST= %d\n",now);
            }
            else
            {
                hum_time_out();
                hum_checke_timer();
                // open flush
                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 1);
                req_bitmap_op(DO_FILL_BPOS, 1);
                //水位高
                if (hum_high_level() == 1)
                {
                    req_bitmap_op(DO_FILL_BPOS, 0);  //关闭注水阀
                }
                else
                {
                    req_bitmap_op(DO_FILL_BPOS, 1);  //开启注水阀
                }
            }
            break;
        }
        default: {
            l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
            hum_delay_timer.check_timer             = 0;
            req_bitmap_op(DO_DRAIN_BPOS, 0);
            req_bitmap_op(DO_FILL_BPOS, 0);
            //关闭加湿器
            req_bitmap_op(DO_HUM_BPOS, 0);
            break;
        }
    }
}
//其他加湿板加湿
static void Other_Hum_Fsm(uint8_t Hum_Sig)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;

    // infrared hum

    if ((sys_get_pwr_sts() == 0) || (g_sys.config.humidifier.hum_en == 0) ||
        (l_sys.l_fsm_state[FAN_FSM_STATE] != FSM_FAN_NORM) ||
        (sys_get_remap_status(WORK_MODE_STS_REG_NO, FAN_STS_BPOS) == 0))  // fan disabled, emergency shutdown
    {
        l_sys.l_fsm_state[HUMIDIFIER_FSM_STATE] = HUM_FSM_STATE_IDLE;
        sys_set_remap_status(WORK_MODE_STS_REG_NO, HUMING_STS_BPOS, 0);
        hum_delay_timer.check_timer = 0;
        req_bitmap_op(DO_DRAIN_BPOS, 0);
        req_bitmap_op(DO_FILL_BPOS, 0);
        req_bitmap_op(DO_HUM_BPOS, 0);
        return;
    }
    //进入加湿条件
    if (Hum_Sig)
    {
        req_bitmap_op(DO_HUM_BPOS, 1);
    }
    else
    {
        req_bitmap_op(DO_HUM_BPOS, 0);
    }

    return;
}

void humidifier_req_exe(int16_t target_req_hum)
{
    extern sys_reg_st g_sys;
    uint8_t hum_sig;
    hum_sig = hum_signal_gen(target_req_hum);
    {
        if (g_sys.config.humidifier.hum_type == HUM_TYPE_INFRARED)
        {
            infrared_hum_fsm(hum_sig);  //红外加湿
        }
        else if (g_sys.config.humidifier.hum_type == HUM_TYPE_OTHER)  //通达达加湿板
        {
            Other_Hum_Fsm(hum_sig);  //
        }
        else
        {
            humidifier_fsm(hum_sig, target_req_hum);
        }
    }
}

static void MBM_Send(uint16_t Addr, uint16_t Value)
{
    extern sys_reg_st g_sys;
    extern fifo8_cb_td mbm_data_fifo;
    mbm_data_st mbm_send_data[1];

    mbm_send_data[0].mbm_addr     = MBM_DEV_EEV_ADDR + 1;
    mbm_send_data[0].reg_addr     = Addr;
    mbm_send_data[0].reg_value    = Value;
    mbm_send_data[0].mbm_NRegs    = 1;
    mbm_send_data[0].mbm_fun_code = MB_WRITE_SINGLE;

    //压入FIFO;
    if (fifo8_push(&mbm_data_fifo, (uint8_t*)&mbm_send_data[0]) == 0)
    {
        rt_kprintf("\n mbm_data_fifo ERRO\n");
    }
    rt_thread_delay(1000);
}
//除湿阀输出反向
void DO_Bitmap_Rev(uint8_t component_bpos, uint8_t u8Rev)
{
    if (u8Rev)
    {
        req_bitmap_op(component_bpos, 0);
    }
    else
    {
        req_bitmap_op(component_bpos, 1);
    }
}

/**************************************
DEHUMER requirement execution function
**************************************/
//除湿逻辑执行函数
void dehumer_req_exe(int16_t req_temp, int16_t req_hum)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    static uint8_t u8DehumTemp = FALSE;

    if (sys_get_pwr_sts() == 0)
    {
        req_bitmap_op(DO_DEHUM1_BPOS, 0);
        sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 0);
        return;
    }
    {
        if ((l_sys.l_fsm_state[FAN_FSM_STATE] != FSM_FAN_NORM) ||
            (sys_get_remap_status(WORK_MODE_STS_REG_NO, FAN_STS_BPOS) == 0))
        {
            req_bitmap_op(DO_DEHUM1_BPOS, 1);
            sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 0);

            return;
        }
        // Alair,20200602
        if (g_sys.config.dehumer.DehumPriority)
        {
            if ((g_sys.status.status_remap[WORK_MODE_STS_REG_NO] & (0x0001 << DEMHUM_STS_BPOS)) != 0)  // in dehum mode
            {
                if (req_hum <= 0)
                {
                    u8DehumTemp = FALSE;
                    req_bitmap_op(DO_DEHUM1_BPOS, 1);
                    sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 0);
                }
                else if (g_sys.status.sys_tem_hum.return_air_temp <= g_sys.config.dehumer.stop_dehum_temp)
                {
                    u8DehumTemp = TRUE;
                    req_bitmap_op(DO_DEHUM1_BPOS, 1);
                    sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 0);
                }
                else
                {
                    if ((l_sys.l_fsm_state[COMPRESS1_FSM_STATE] == COMPRESSOR_FSM_STATE_NORMAL) ||
                        (l_sys.l_fsm_state[COMPRESS2_FSM_STATE] == COMPRESSOR_FSM_STATE_NORMAL))
                    {
                        req_bitmap_op(DO_DEHUM1_BPOS, 0);
                    }
                    else
                    {
                        req_bitmap_op(DO_DEHUM1_BPOS, 1);
                    }
                    sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 1);
                }
            }
            else
            {
                //进入除湿
                if (g_sys.config.dehumer.stop_dehum_diff)  //
                {
                    if (u8DehumTemp == TRUE)
                    {
                        if ((req_hum >= 100) &&
                            ((g_sys.status.sys_tem_hum.return_air_temp >
                              (g_sys.config.dehumer.stop_dehum_temp + g_sys.config.dehumer.stop_dehum_diff))))
                        {
                            u8DehumTemp = FALSE;
                            sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 1);
                            req_bitmap_op(DO_DEHUM1_BPOS, 0);
                        }
                    }
                    else
                    {
                        if ((req_hum >= 100) &&
                            ((g_sys.status.sys_tem_hum.return_air_temp > g_sys.config.dehumer.stop_dehum_temp)))
                        {
                            sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 1);
                            req_bitmap_op(DO_DEHUM1_BPOS, 0);
                        }
                    }
                }
                else
                {
                    if ((req_hum >= 100) &&
                        ((g_sys.status.sys_tem_hum.return_air_temp > g_sys.config.dehumer.stop_dehum_temp)))
                    {
                        sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 1);
                        req_bitmap_op(DO_DEHUM1_BPOS, 0);
                    }
                }
            }
        }
        else
        {
            if ((g_sys.status.status_remap[WORK_MODE_STS_REG_NO] & (0x0001 << DEMHUM_STS_BPOS)) != 0)  // in dehum mode
            {
                //退出除湿
                if ((req_hum <= 0) || (req_temp >= 150))
                {
                    u8DehumTemp = FALSE;
                    req_bitmap_op(DO_DEHUM1_BPOS, 1);
                    sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 0);
                }
                else if (g_sys.status.sys_tem_hum.return_air_temp <= g_sys.config.dehumer.stop_dehum_temp)
                {
                    u8DehumTemp = TRUE;
                    req_bitmap_op(DO_DEHUM1_BPOS, 1);
                    sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 0);
                }
                else
                {
                    if ((l_sys.l_fsm_state[COMPRESS1_FSM_STATE] == COMPRESSOR_FSM_STATE_NORMAL) ||
                        (l_sys.l_fsm_state[COMPRESS2_FSM_STATE] == COMPRESSOR_FSM_STATE_NORMAL))
                    {
                        req_bitmap_op(DO_DEHUM1_BPOS, 0);
                    }
                    else
                    {
                        req_bitmap_op(DO_DEHUM1_BPOS, 1);
                    }
                    sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 1);
                }
            }
            else
            {
                //进入除湿
                if (g_sys.config.dehumer.stop_dehum_diff)  //
                {
                    if (u8DehumTemp == TRUE)
                    {
                        if ((req_temp < 125) && (req_hum >= 100) &&
                            ((g_sys.status.sys_tem_hum.return_air_temp >
                              (g_sys.config.dehumer.stop_dehum_temp + g_sys.config.dehumer.stop_dehum_diff))))
                        {
                            u8DehumTemp = FALSE;
                            sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 1);
                            req_bitmap_op(DO_DEHUM1_BPOS, 0);
                        }
                    }
                    else
                    {
                        if ((req_temp < 125) && (req_hum >= 100) &&
                            ((g_sys.status.sys_tem_hum.return_air_temp > g_sys.config.dehumer.stop_dehum_temp)))
                        {
                            sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 1);
                            req_bitmap_op(DO_DEHUM1_BPOS, 0);
                        }
                    }
                }
                else
                {
                    if ((req_temp < 125) && (req_hum >= 100) &&
                        ((g_sys.status.sys_tem_hum.return_air_temp > g_sys.config.dehumer.stop_dehum_temp)))
                    {
                        sys_set_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS, 1);
                        req_bitmap_op(DO_DEHUM1_BPOS, 0);
                    }
                }
            }
        }
    }

    //			rt_kprintf("u16Temp = %x,WORK_MODE_STS_REG_NO = %x,dout_bitmap =
    //%x\n",u16Temp,g_sys.status.status_remap[WORK_MODE_STS_REG_NO],g_sys.status.dout_bitmap);
    //除湿模式，使用除湿过热度控制
    if (((g_sys.config.dev_mask.mb_comp >> MBM_DEV_EEV_ADDR) & 0x0001) == 1)
    {
        if (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) == 1)
        {
            if (g_sys.status.mbm.EEV.Set_Dehumidity_Supperheart != g_sys.config.dehumer.Dehumer_Superheat)
            {
                MBM_Send(19, g_sys.config.dehumer.Dehumer_Superheat);
            }
        }
        else
        {
            if (g_sys.status.mbm.EEV.Set_Dehumidity_Supperheart != 0x00)
            {
                MBM_Send(19, 0x00);
            }
        }
    }
    return;
}

/**************************************
FAN FSM logic process function
**************************************/
//风机状态机信号产生电路
static uint8_t fan_signal_gen(void)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    uint8_t fan_signal;
    int16_t ao_sig_flag;
    uint16_t i;

    ao_sig_flag = 0;
    if (g_sys.config.fan.mode == FAN_MODE_FIX)
    {
        ao_sig_flag = 0;
    }
    else
    {
        for (i = (AO_EC_FAN + 1); i < AO_MAX_CNT; i++)
        {
            if (l_sys.ao_list[i][BITMAP_REQ] != 0)
            {
                ao_sig_flag = 1;
                break;
            }
        }
    }
    fan_signal = 0;
    if (sys_get_pwr_sts() == 1)
    {
        fan_signal = FAN_SIG_START;
    }
    //		if(sys_get_pwr_sts() == 1)
    //		{
    //				if((get_alarm_bitmap_mask(DO_FAN_BPOS)==0))
    //				{
    //					fan_signal = FAN_SIG_START;
    //				}
    //		}
    //		else if((sys_get_pwr_sts() ==
    // 0)&&((g_sys.status.dout_bitmap&(~((0x0001<<DO_FAN_BPOS)|(0x0001<<DO_ALARM_BPOS)|(0x0001<<DO_PHASE_P_BPOS)|(0x0001<<DO_PHASE_N_BPOS)|(0x0001<<DO_DEHUM1_BPOS))))
    //== 0)&&(ao_sig_flag == 0)) Alair 20161113
    else if (sys_get_pwr_sts() == 0)
    {
        if (((g_sys.status.dout_bitmap &
              (~((0x0001 << DO_FAN_BPOS) | (0x0001 << DO_ALARM_BPOS) | (0x0001 << DO_DEHUM1_BPOS)))) == 0) &&
            (ao_sig_flag == 0))
        {
            fan_signal = FAN_SIG_STOP;
        }
    }
    else
    {
        fan_signal = FAN_SIG_IDLE;
    }
    //告警关机
    if (l_sys.u8AlarmClose_dev)
    {
        fan_signal = FAN_SIG_STOP;
    }
    return fan_signal;
}

//风机档位输出,Alair,20161113,除湿机
static void Fan_Fsm_Out(uint8_t Fan_Gear)
{
    extern sys_reg_st g_sys;

    {
        switch (Fan_Gear)
        {
            case FAN_GEAR_START:
                req_bitmap_op(DO_FAN_LOW_BPOS, 1);
                break;
            case FAN_GEAR_NO:
                req_bitmap_op(DO_FAN_LOW_BPOS, 0);
                break;
            default:
                req_bitmap_op(DO_FAN_LOW_BPOS, 0);
                break;
        }
    }
}

/**
 * @brief 	fan output control state FSM execution
 * @param  none
 * @retval none
 */
//风机状态机执行函数
static void fan_fsm_exe(uint8_t fan_signal)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;

    if ((get_alarm_bitmap_mask(DO_FAN_BPOS) == 1))
    {
        l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_IDLE;
    }
    //		rt_kprintf("l_sys.l_fsm_state[FAN_FSM_STATE] = %d\n",l_sys.l_fsm_state[FAN_FSM_STATE]);
    //		rt_kprintf("fan_signal = %d\n",fan_signal);
    //			rt_kprintf("l_sys.comp_timeout[DO_FAN_BPOS]  = %d\n",l_sys.comp_timeout[DO_FAN_BPOS] );
    //			rt_kprintf("l_sys.require[LOCAL_REQ][H_REQ] = %d\n",l_sys.require[LOCAL_REQ][H_REQ]);
    switch (l_sys.l_fsm_state[FAN_FSM_STATE])
    {
        case (FSM_FAN_IDLE): {
            if (fan_signal == FAN_SIG_START)
            {
                l_sys.Fan.Fan_Start              = TRUE;
                l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_INIT;
                l_sys.comp_timeout[DO_FAN_BPOS] =
                    g_sys.config.fan.startup_delay;  // assign startup delay to timeout counter
            }
            else
            {
                l_sys.Fan.Fan_Start              = FALSE;
                l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_IDLE;
                l_sys.comp_timeout[DO_FAN_BPOS]  = l_sys.comp_timeout[DO_FAN_BPOS];  // remains timeout counter
            }
            //						req_bitmap_op(DO_FAN_BPOS,0);
            l_sys.Fan.Fan_Gear = FAN_GEAR_NO;  //无输出						//disable fan output
            break;
        }
        case (FSM_FAN_INIT): {
            if (fan_signal != FAN_SIG_START)
            {
                l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_IDLE;
                //								req_bitmap_op(DO_FAN_BPOS,0);				//disable fan output
                l_sys.Fan.Fan_Gear              = FAN_GEAR_NO;                      //无输出
                l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];  // reset timeout counter
            }
            else
            {
                if (l_sys.comp_timeout[DO_FAN_BPOS] == 0)
                {
                    l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_START_UP;
                    l_sys.comp_timeout[DO_FAN_BPOS]  = g_sys.config.fan.cold_start_delay;
                    //										req_bitmap_op(DO_FAN_BPOS,1);				//enable fan
                    // output
                    l_sys.Fan.Fan_Gear = FAN_GEAR_START;  //
                }
                else  // wait until startup delay elapses
                {
                    l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_INIT;
                    //										req_bitmap_op(DO_FAN_BPOS,0);	//disable fan output
                    l_sys.Fan.Fan_Gear = FAN_GEAR_NO;  //无输出
                }
            }
            break;
        }
        case (FSM_FAN_START_UP): {
            if (l_sys.comp_timeout[DO_FAN_BPOS] == 0)
            {
                l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_NORM;
            }
            else
            {
                l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_START_UP;
            }
            //						req_bitmap_op(DO_FAN_BPOS,1);				//enable fan output
            l_sys.Fan.Fan_Gear              = FAN_GEAR_START;                   //
            l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];  // remain timeout counter
            break;
        }
        case (FSM_FAN_NORM): {
            if ((fan_signal == FAN_SIG_STOP) && (l_sys.comp_timeout[DO_FAN_BPOS] == 0))
            {
                l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_SHUT;
                l_sys.comp_timeout[DO_FAN_BPOS] =
                    g_sys.config.fan.stop_delay;  // assign startup delay to timeout counter
            }
            else
            {
                l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_NORM;
                l_sys.comp_timeout[DO_FAN_BPOS]  = l_sys.comp_timeout[DO_FAN_BPOS];  // reset timeout counter
            }
            //						req_bitmap_op(DO_FAN_BPOS,1);
            {
                //								req_bitmap_op(DO_FAN_BPOS,1);	//
                l_sys.Fan.Fan_Gear = FAN_GEAR_START;  //
            }

            // disable fan output
            break;
        }
        case (FSM_FAN_SHUT): {
            if (fan_signal == FAN_SIG_START)
            {
                l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_NORM;
                //								req_bitmap_op(DO_FAN_BPOS,1);
                ////disable fan output
                l_sys.Fan.Fan_Gear              = FAN_GEAR_START;                   //
                l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];  // reset timeout counter
            }
            else
            {
                if (l_sys.comp_timeout[DO_FAN_BPOS] == 0)
                {
                    l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_IDLE;
                    l_sys.Fan.Fan_Gear =
                        FAN_GEAR_NO;                                                    //																			//enable
                                                                                        // fan output
                    l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];  // reset timeout counter
                }
                else  // wait until startup delay elapses
                {
                    l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_SHUT;
                    //										req_bitmap_op(DO_FAN_BPOS,1);
                    ////enable fan output
                    l_sys.comp_timeout[DO_FAN_BPOS] = l_sys.comp_timeout[DO_FAN_BPOS];  // remain timeout counter
                }
            }
            break;
        }
        default: {
            l_sys.l_fsm_state[FAN_FSM_STATE] = FSM_FAN_IDLE;
            //						req_bitmap_op(DO_FAN_BPOS,0);
            ////enable fan output
            l_sys.Fan.Fan_Gear              = FAN_GEAR_NO;  //
            l_sys.comp_timeout[DO_FAN_BPOS] = 0;            // reset timeout counter
            break;
        }
    }
    //		rt_kprintf("l_sys.Fan_Gear = %d\n",l_sys.Fan_Gear);

    //		if(sys_get_pwr_sts() == 0)
    //		{
    //				l_sys.Fan_Gear=FAN_GEAR_NO;//风机无输出
    //		}
    if (g_sys.config.fan.type == FAN_TPYE_EC)
    {
        if ((sys_get_pwr_sts() != 0) && (l_sys.Fan.Fan_Gear != FAN_GEAR_NO))
        {
            l_sys.Fan.Fan_Gear = FAN_GEAR_LOW;  //风机低挡
        }
    }
    //		rt_kprintf("l_sys.Fan_Gear = %d\n",l_sys.Fan_Gear);
    Fan_Fsm_Out(l_sys.Fan.Fan_Gear);  //风机DO输出
}

static int16_t ecfan_default_cacl_output(int16_t req_temp)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    int8_t fan_default_cnt = 0;
    int16_t toatl_req;

    fan_default_cnt = get_alarm_bitmap(ACL_FAN_OVERLOAD1);
    fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD2);
    fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD3);
    fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD4);
    fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD5);
    fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD6);
    fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD7);
    fan_default_cnt += get_alarm_bitmap(ACL_FAN_OVERLOAD8);

    toatl_req = req_temp * g_sys.config.fan.num;

    req_temp = toatl_req / (g_sys.config.fan.num - fan_default_cnt);

    l_sys.Fan.Fan_default_cnt = fan_default_cnt;  //异常风机数量

    return (req_temp);
}
uint16_t ec_fan_adjust(uint16_t fan_require)
{
    extern sys_reg_st g_sys;

    static uint16_t rel_req = 50;

    if ((fan_require - rel_req) > 0)
    {
        if ((fan_require - rel_req) > g_sys.config.fan.adjust_step)
        {
            rel_req = rel_req + g_sys.config.fan.adjust_step;
        }
        else
        {
            rel_req = fan_require;
        }
    }
    else
    {
        if ((rel_req - fan_require) > g_sys.config.fan.adjust_step)
        {
            rel_req = rel_req - g_sys.config.fan.adjust_step;
        }
        else
        {
            rel_req = fan_require;
        }
    }
    rel_req = lim_min_max(g_sys.config.fan.min_speed, g_sys.config.fan.max_speed, rel_req);
    return (rel_req);
}

#define EC_FAN_NOLOAD_DELAY 20

static uint16_t fan_mode_mange(void)
{
    extern sys_reg_st g_sys;

    extern team_local_st team_local_inst;
    uint16_t fan_run_mode;

    if ((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER) && (g_sys.config.team.team_en == 1))
    {
        if (g_sys.config.fan.mode == FAN_MODE_PRESS_DIFF)
        {
            fan_run_mode = FAN_MODE_PRESS_DIFF;
        }
        //
        else if ((team_local_inst.team_fsm != TEAM_FSM_SLAVE) && (team_local_inst.team_fsm != TEAM_FSM_MASTER))
        {
            fan_run_mode = FAN_MODE_FIX;
        }
        else  //需求模式。
        {
            fan_run_mode = FAN_MODE_AVR_SUPPLY;
        }
    }
    else
    {
        fan_run_mode = g_sys.config.fan.mode;
    }

    return (fan_run_mode);
}

//风量计算
void CFM_Calute()
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    static uint32_t Single_CFM;
    static uint32_t Buff[2];
    static uint16_t Temp;

    if ((sys_get_pwr_sts() == 0) || (l_sys.l_fsm_state[FAN_FSM_STATE] != FSM_FAN_NORM))
    {
        g_sys.status.CFM = 0;
    }
    else
    {
        if (g_sys.config.fan.CFM_Para_A & 0x8000)
        {
            Temp    = ~(g_sys.config.fan.CFM_Para_A - 1);
            Buff[0] = pow(g_sys.status.aout[AO_EC_FAN], 2) * Temp / 10;
            Buff[0] = -Buff[0];
        }
        else
        {
            Buff[0] = pow(g_sys.status.aout[AO_EC_FAN], 2) * g_sys.config.fan.CFM_Para_A / 10;
        }
        //				rt_kprintf("Buff[0] = %X\n",Buff[0]);
        if (g_sys.config.fan.CFM_Para_B & 0x8000)
        {
            Temp    = ~(g_sys.config.fan.CFM_Para_B - 1);
            Buff[1] = g_sys.status.aout[AO_EC_FAN] * Temp / 10;
            Buff[1] = -Buff[1];
        }
        else
        {
            Buff[1] = g_sys.status.aout[AO_EC_FAN] * g_sys.config.fan.CFM_Para_B / 10;
        }
        //				rt_kprintf("Buff[1] = %X\n",Buff[1]);
        Single_CFM = (Buff[0] + Buff[1]) / 10 + g_sys.config.fan.CFM_Para_C;  //取1位小数
        //				rt_kprintf("Single_CFM = %d\n",Single_CFM);
        Single_CFM       = Single_CFM * (g_sys.config.fan.num - l_sys.Fan.Fan_default_cnt);
        g_sys.status.CFM = Single_CFM;
        ////				rt_kprintf("Single_CFM = %d\n",Single_CFM);
        //				rt_kprintf("g_sys.status.CFM = %d\n",g_sys.status.CFM);
    }
    return;
}
static void ec_fan_output(int16_t req_temp, int16_t req_hum, int16_t fan_req, uint8_t fan_sig)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    static uint16_t ec_fan_shut_delay = 0, temp_add_delay = 0;
    //		uint16_t bitmap_sense;
    uint16_t status_sense;
    uint16_t require;
    uint8_t fan_mode;
    uint8_t u8LineSpeed = FALSE;

    //		bitmap_sense = 0;
    //		status_sense = 0;
    //		bitmap_sense = g_sys.status.dout_bitmap & ((0x0001<<DO_COMP1_BPOS) | (0x0001<<DO_COMP2_BPOS) |
    //(0x0001<<DO_RH1_BPOS) | (0x0001<<DO_RH2_BPOS) | (0x0001<<DO_HUM_BPOS));

    fan_mode = (uint8_t)fan_mode_mange();
    // determine if any peripheral component is on, fan should not be shutted
    status_sense =
        g_sys.status.status_remap[WORK_MODE_STS_REG_NO] & ((0x0001 << HEATING_STS_BPOS) | (0x0001 << COOLING_STS_BPOS) |
                                                           (0x0001 << HUMING_STS_BPOS) | (0x0001 << DEMHUM_STS_BPOS));
    require = 0;
    //				rt_kprintf("status_sense = %d\n",status_sense);
    {
        if ((g_sys.status.dout_bitmap & (0x0001 << DO_FAN_BPOS)) == 0)
        {
            require = 0;
        }
        else if (g_sys.config.fan.type == FAN_TPYE_EC)
        {
            if (l_sys.l_fsm_state[FAN_FSM_STATE] == FSM_FAN_START_UP)
            {
                require = g_sys.config.fan.set_speed;
            }
            else
            {
                // if using invertor compressor//变频压机
                if ((g_sys.config.compressor.type == COMP_QABP) &&
                    ((g_sys.config.general.cool_type == COOL_TYPE_MODULE_WIND) ||
                     (g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND)))  // ec compressor
                {
                    // fan mode in invertor compressor mode
                    if (fan_mode == FAM_MODE_INV_COMP)  //变速
                    {
                        // dehuming mode
                        if (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) != 0)
                        {
                            //														require = analog_step_follower(1,
                            // AO_INV_FAN); 														ec_fan_shut_delay =
                            // EC_FAN_NOLOAD_DELAY; Alair，20170816
                            require = g_sys.config.fan.set_speed * g_sys.config.fan.dehum_ratio / 100;
                        }
                        else if ((sys_get_remap_status(WORK_MODE_STS_REG_NO, HEATING_STS_BPOS) != 0) ||
                                 (sys_get_remap_status(WORK_MODE_STS_REG_NO, HUMING_STS_BPOS) != 0))
                        {
                            require           = g_sys.config.fan.set_speed;
                            ec_fan_shut_delay = EC_FAN_NOLOAD_DELAY;
                            //												rt_kprintf("inv2\r\n");
                        }
                        else if ((g_sys.config.fan.noload_down != 0) && (status_sense == 0))
                        {
                            if (ec_fan_shut_delay <= 0)
                            {
                                require           = g_sys.config.fan.min_speed;
                                ec_fan_shut_delay = 0;
                            }
                            else
                            {
                                require = analog_step_follower(0, AO_INV_FAN);
                                //														rt_kprintf("inv3\r\n");
                                ec_fan_shut_delay--;
                            }
                        }
                        else
                        {
                            // Alair,20171011,修改压机1关机后，风机最低转速运行的bug
                            //													require =
                            // analog_step_follower(0,AO_INV_FAN);
                            if ((g_sys.status.dout_bitmap & (0x0001 << DO_COMP1_BPOS)) == 0)
                            {
                                require = analog_step_follower(fan_req, AO_EC_FAN);
                            }
                            else
                            {
                                require = analog_step_follower(0, AO_INV_FAN);
                            }
                            ec_fan_shut_delay = EC_FAN_NOLOAD_DELAY;
                            //												rt_kprintf("inv: %d \r\n",require);
                        }
                    }
                    else
                    {
                        // dehuming mode
                        if (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) != 0)
                        {
                            require = g_sys.config.fan.set_speed * g_sys.config.fan.dehum_ratio / 100;
                        }
                        else if ((g_sys.config.fan.noload_down != 0) && (status_sense == 0))
                        {
                            require = g_sys.config.fan.min_speed;
                        }
                        else
                        {
                            require = g_sys.config.fan.set_speed;
                        }
                    }
                }
                //            else if((fan_mode == FAN_MODE_FIX)||(g_sys.config.general.cool_type ==
                //            COOL_TYPE_MODULE_WIND)||(g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND))
                //            //fix mode
                // Alair 20161111
                else if (fan_mode == FAN_MODE_FIX)  //定速模式
                {
                    // dehuming mode
                    if (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) != 0)
                    {
                        require = g_sys.config.fan.set_speed * g_sys.config.fan.dehum_ratio / 100;
                    }
                    else if ((g_sys.config.fan.noload_down != 0) && (status_sense == 0))
                    {
                        require = g_sys.config.fan.min_speed;
                    }
                    else
                    {
                        require = g_sys.config.fan.set_speed;
                    }
                    // fan default req calc
                    if ((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND) ||
                        (g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER))
                    {
                        require = ecfan_default_cacl_output(require);
                    }
                }
                else if (fan_mode == FAN_MODE_PRESS_DIFF)  // flex mode, change fan speed accordingly with compressor or
                                                           // watervalve, to be accomplished
                {
                    require = g_sys.config.fan.set_speed + l_sys.ec_fan_diff_reg;
                }
                else  // require mode
                {
                    // dehuming mode
                    if (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) == 1)
                    {
                        require = g_sys.config.fan.set_speed * g_sys.config.fan.dehum_ratio / 100;
                    }
                    // huming heater
                    else if ((sys_get_remap_status(WORK_MODE_STS_REG_NO, HUMING_STS_BPOS) == 1) ||
                             (sys_get_remap_status(WORK_MODE_STS_REG_NO, HEATING_STS_BPOS) == 1))
                    {
                        require = g_sys.config.fan.set_speed;
                    }
                    // water cool
                    else if ((g_sys.config.general.cool_type == COOL_TYPE_MODULE_WATER) ||
                             (g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER) ||
                             (g_sys.config.general.cool_type == COOL_TYPE_MODULE_WIND))
                    {
                        //												if(fan_req < g_sys.config.fan.min_speed)
                        //												{
                        //														require = g_sys.config.fan.min_speed;
                        //												}
                        //												else
                        {
                            require = fan_req;  //风机需求、变速
                        }
                        // temp add
                        if ((req_temp > 100) && (g_sys.config.fan.temp_add_fan_en == 1))
                        {
                            if ((temp_add_delay++) >= g_sys.config.fan.tem_add_fan_delay)
                            {
                                temp_add_delay = g_sys.config.fan.tem_add_fan_delay;
                                require        = g_sys.config.fan.min_speed +
                                          require * (g_sys.config.fan.max_speed - g_sys.config.fan.min_speed) / 100 +
                                          (req_temp - 100) * g_sys.config.algorithm.temp_precision / 100;
                            }
                        }
                        else
                        {
                            temp_add_delay = 0;
                        }
                        u8LineSpeed = TRUE;
                    }
                    else
                    {
                        require     = fan_req;  // l_sys.require[LOCAL_REQ][F_REQ] ;
                        u8LineSpeed = TRUE;
                    }
                    //										 require = ec_fan_adjust(require);
                }
            }
            // fan default req calc
            if ((fan_mode != FAN_MODE_PRESS_DIFF) &&
                ((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND) ||
                 (g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER)) &&
                (fan_mode != FAM_MODE_INV_COMP))
            {
                require = ecfan_default_cacl_output(require);
            }
            //回气温度调节（在风压控制模式下，不进行回气温度调节）
            if (((sys_get_remap_status(WORK_MODE_STS_REG_NO, COOLING_STS_BPOS) == 1) ||
                 (sys_get_remap_status(WORK_MODE_STS_REG_NO, DEMHUM_STS_BPOS) == 1)) &&
                (fan_mode != FAN_MODE_PRESS_DIFF) && (fan_mode != FAM_MODE_INV_COMP) && (fan_mode != FAN_MODE_FIX))
            {
                require = require + l_sys.ec_fan_suc_temp;
            }
            //						rt_kprintf("require: %d \r\n",require);
            if (u8LineSpeed == TRUE)
            {
                require = line_min_max(g_sys.config.fan.min_speed, g_sys.config.fan.max_speed, require);
            }
        }
        else if (g_sys.config.fan.type == FAN_TPYE_AC)
        {
            require = 100;
        }
        else
        {
            require = 0;
        }
    }
    req_ao_op(AO_EC_FAN, require, fan_sig);
    //		CFM_Calute();
}

/**
 * @brief 	fan output control logic FSM
 * @param  none
 * @retval none
 */
//风机需求执行函数
void fan_req_exe(int16_t target_req_temp, int16_t target_req_hum, int16_t fan_req)
{
    uint8_t fan_signal;
    fan_signal = fan_signal_gen();                                        //风机状态机信号产生
    fan_fsm_exe(fan_signal);                                              //风机状态机执行
    ec_fan_output(target_req_temp, target_req_hum, fan_req, fan_signal);  //风机模拟输出控制
}

////相序控制逻辑
// static void power_phase_switch(void)
//{
//		extern sys_reg_st		g_sys;
//		static uint8_t power_phase_fsm_state = 0;
//		static uint16_t switch_delay = 0;
//		switch(power_phase_fsm_state)
//		{
//				case (PPE_FSM_POWER_ON):
//				{
//						if((((g_sys.config.dev_mask.dout>>DO_PHASE_P_BPOS) & 0x0001) &
//((g_sys.config.dev_mask.dout>>DO_PHASE_N_BPOS) & 0x0001)) == 0)
//						{
//								power_phase_fsm_state = PPE_FSM_NO_CONF;
//								switch_delay = 0;
//								req_bitmap_op(DO_PHASE_P_BPOS,0);
//								req_bitmap_op(DO_PHASE_N_BPOS,0);
//						}
//						else
//						{
//								power_phase_fsm_state = PPE_FSM_PN_SWITCH;
//								switch_delay = INIT_DELAY;
//								req_bitmap_op(DO_PHASE_P_BPOS,0);
//								req_bitmap_op(DO_PHASE_N_BPOS,0);
//						}
//						break;
//				}
//				case (PPE_FSM_PN_SWITCH):
//				{
//						if(switch_delay > 0)
//						{
//								switch_delay--;
//						}
//						else
//						{
//								switch_delay = 0;
//						}
//
//						if(switch_delay == 0)
//						{
//								if((g_sys.status.mbm.pwr.dev_sts&0x0001) == 0)//power phase error
//								{
//										power_phase_fsm_state = PPE_FSM_NORMAL;
//										req_bitmap_op(DO_PHASE_P_BPOS,0);
//										req_bitmap_op(DO_PHASE_N_BPOS,0);
//								}
//								else
//								{
//										power_phase_fsm_state = PPE_FSM_REVERSE;
//										req_bitmap_op(DO_PHASE_P_BPOS,0);
//										req_bitmap_op(DO_PHASE_N_BPOS,0);
//								}
//						}
//						break;
//				}
//				case (PPE_FSM_NO_CONF):
//				{
//						power_phase_fsm_state = PPE_FSM_NO_CONF;
//						switch_delay = 0;
//						req_bitmap_op(DO_PHASE_P_BPOS,0);
//						req_bitmap_op(DO_PHASE_N_BPOS,0);
//						break;
//				}
//				case (PPE_FSM_NORMAL)://相序正确
//				{
//						power_phase_fsm_state = PPE_FSM_NORMAL;
//						switch_delay = 0;
//						req_bitmap_op(DO_PHASE_P_BPOS,1);
//						req_bitmap_op(DO_PHASE_N_BPOS,0);
//						break;
//				}
//				case (PPE_FSM_REVERSE)://相序错
//				{
//						power_phase_fsm_state = PPE_FSM_REVERSE;
//						switch_delay = 0;
//						req_bitmap_op(DO_PHASE_P_BPOS,0);
//						req_bitmap_op(DO_PHASE_N_BPOS,1);
//						break;
//				}
//				default:
//				{
//						power_phase_fsm_state = PPE_FSM_NORMAL;
//						switch_delay = 0;
//						req_bitmap_op(DO_PHASE_P_BPOS,1);
//						req_bitmap_op(DO_PHASE_N_BPOS,0);
//						break;
//				}
//		}
//}

enum
{
    FSM_C_COMPRESSOR,
    FSM_C_WATERVALVE
};
////水泵控制逻辑
// static void pumb_execution(void)
//{
//		//机型判断
//		extern sys_reg_st		g_sys;
////		if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER)|| ((g_sys.config.general.cool_type ==
/// COOL_TYPE_COLUMN_WIND) && (g_sys.config.compressor.type != COMP_QABP)) )
//		//Alair,20170227
//		if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WATER)|| (g_sys.config.general.cool_type ==
// COOL_TYPE_COLUMN_WIND) )
//		{
//				// 高水位和低水位检测都触发
//				if((sys_get_di_sts(DI_COND_HI_LEVEL_BPOS)==1)&&(sys_get_di_sts(DI_COND_LO_LEVEL_BPOS)==1))
//				{
//							req_bitmap_op(DO_PUMMP_BPOS,1);
//				}
//				//高水位和低水位检测都解除
//				else if((sys_get_di_sts(DI_COND_HI_LEVEL_BPOS)==0)&&(sys_get_di_sts(DI_COND_LO_LEVEL_BPOS)==0))
//				{
//						 req_bitmap_op(DO_PUMMP_BPOS,0);
//				}
//				else
//				{
//						;
//				}
//		}
//}

enum
{
    FSM_HGBP_OFF,
    FSM_HGBP_ON,
};
////卸载电磁阀控制逻辑
// static void hgbp_exe(int16_t target_req_temp)
//{
//		extern sys_reg_st		g_sys;
//		static uint16_t			hgbp_cd=0;
//		static uint16_t fsm_hgbp_state = FSM_HGBP_OFF;
//
//		if(g_sys.config.hgbp.Hgbp_SkyLight!=0)//卸载电磁阀启用
//		{
//			if(((g_sys.config.dev_mask.dout & ((0x0001)<<DO_HGBP_BPOS)) != 0)&&
//					((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND)||(g_sys.config.general.cool_type ==
// COOL_TYPE_MODULE_WIND)))
//			{
//					switch(fsm_hgbp_state)
//					{
//							case FSM_HGBP_OFF:
//							{
//									if((sys_get_remap_status(WORK_MODE_STS_REG_NO,COOLING_STS_BPOS) != 0)&&
//											(target_req_temp < g_sys.config.hgbp.on_req)&&(target_req_temp>0)&&
//											(hgbp_cd == 0))
//									{
//											req_bitmap_op(DO_HGBP_BPOS,1);
//											hgbp_cd = g_sys.config.hgbp.max_on_time;
//											fsm_hgbp_state = FSM_HGBP_ON;
//									}
//									else
//									{
//											req_bitmap_op(DO_HGBP_BPOS,0);
//											if(hgbp_cd > 0)
//											{
//													hgbp_cd--;
//											}
//											else
//											{
//													hgbp_cd = 0;
//											}
//											fsm_hgbp_state = FSM_HGBP_OFF;
//									}
//									break;
//							}
//							case FSM_HGBP_ON:
//							{
//									if((sys_get_remap_status(WORK_MODE_STS_REG_NO,COOLING_STS_BPOS) == 0)||
//											(target_req_temp >
//(g_sys.config.hgbp.on_req+g_sys.config.hgbp.hysterisis))|| 											(target_req_temp
//<= 0)|| (hgbp_cd == 0))
//									{
//											req_bitmap_op(DO_HGBP_BPOS,0);
//											hgbp_cd = g_sys.config.hgbp.min_off_time;
//											fsm_hgbp_state = FSM_HGBP_OFF;
//									}
//									else
//									{
//											req_bitmap_op(DO_HGBP_BPOS,1);
//											if(hgbp_cd > 0)
//											{
//													hgbp_cd--;
//											}
//											else
//											{
//													hgbp_cd = 0;
//											}
//											fsm_hgbp_state = FSM_HGBP_ON;
//									}
//									break;
//							}
//							default:
//							{
//									req_bitmap_op(DO_HGBP_BPOS,0);
//									hgbp_cd = 0;
//									fsm_hgbp_state = FSM_HGBP_OFF;
//									break;
//							}
//					}
//			}
//			else
//			{
//					req_bitmap_op(DO_HGBP_BPOS,0);
//					hgbp_cd = 0;
//					fsm_hgbp_state = FSM_HGBP_OFF;
//					return;
//			}
//		}
//}

////冷凝风机输出
// static void analog_dummy_out(void)
//{
//		extern sys_reg_st		g_sys;

//		uint16_t k = 0,b = 0,ext_fan_out;
//		if(g_sys.config.ext_fan_inst.ext_fan_cnt==0)
//		{
//			return;
//		}
//		if((g_sys.config.general.cool_type == COOL_TYPE_COLUMN_WIND) && (sys_get_do_sts(DO_FAN_BPOS)==1)
//&&(sys_get_do_sts(DO_COMP1_BPOS) == 1)&&
//			((get_alarm_bitmap(ACL_FAN_OVERLOAD3) == 0) || (get_alarm_bitmap(ACL_FAN_OVERLOAD4) == 0) ||
//(get_alarm_bitmap(ACL_FAN_OVERLOAD5) == 0)))
//		{
//
////					rt_kprintf("g_sys.status.ain[AI_HI_PRESS_CONDENSATE]=%d
///\n",g_sys.status.ain[AI_HI_PRESS_CONDENSATE]);
//				if(g_sys.status.ain[AI_HI_PRESS_CONDENSATE] >= ABNORMAL_VALUE)
//				{
//						ext_fan_out=g_sys.config.ext_fan_inst.ext_fan1_set_speed;
//				}
//				else if(g_sys.status.ain[AI_HI_PRESS_CONDENSATE] > g_sys.config.ext_fan_inst.ext_fan_max_press)
//				{
//						ext_fan_out=g_sys.config.ext_fan_inst.ext_fan_max_speed;
//				}
//				else if((g_sys.status.ain[AI_HI_PRESS_CONDENSATE] <= g_sys.config.ext_fan_inst.ext_fan_max_press) &&
//(g_sys.status.ain[AI_HI_PRESS_CONDENSATE] >= g_sys.config.ext_fan_inst.ext_fan_min_press))
//				{
//					k = (g_sys.config.ext_fan_inst.ext_fan_max_press -
// g_sys.config.ext_fan_inst.ext_fan_min_press)/(g_sys.config.ext_fan_inst.ext_fan1_set_speed -
// g_sys.config.ext_fan_inst.ext_fan_min_speed); 					b = g_sys.config.ext_fan_inst.ext_fan_max_press -
// k*(g_sys.config.ext_fan_inst.ext_fan1_set_speed); 					ext_fan_out =
// (g_sys.status.ain[AI_HI_PRESS_CONDENSATE]
// - b)/k; 					if(ext_fan_out < g_sys.config.ext_fan_inst.ext_fan_min_speed)
//					{
//						ext_fan_out = g_sys.config.ext_fan_inst.ext_fan_min_speed;
//					}
//					if(ext_fan_out > g_sys.config.ext_fan_inst.ext_fan_max_speed)
//					{
//						ext_fan_out = g_sys.config.ext_fan_inst.ext_fan_max_speed;
//					}

//				}
//				else
//				{
//						ext_fan_out = g_sys.config.ext_fan_inst.ext_fan_min_speed;
//				}
//		}
//		else
//		{
//				ext_fan_out =0;
//		}
////		rt_kprintf("ext_fan_out =%d \n",ext_fan_out);
//		req_ao_op(AO_EX_FAN, ext_fan_out);
//}

////天窗控制
// static void Sky_Light_execution(void)
//{
//		extern sys_reg_st		g_sys;
//		if(((g_sys.config.dev_mask.dout & ((0x0001)<<DO_SKYLIGHT_BPOS)) != 0)&&(g_sys.config.hgbp.Hgbp_SkyLight==0))
//		{
//
//		//Alair,20171011,天窗控制，高温告警时打开
//			if((get_alarm_bitmap(ACL_HI_TEMP_RETURN))||(get_alarm_bitmap(ACL_HI_TEMP_SUPPLY))||(get_alarm_bitmap(ACL_SMOKE_ALARM)))//高温报警，烟雾
//			{
//					req_bitmap_op(DO_SKYLIGHT_BPOS,1);
//			}
//			else
//			{
//					req_bitmap_op(DO_SKYLIGHT_BPOS,0);
//			}
//		}
//		return;
//}

static void MB_ADJ(uint8_t MB_Addr, uint16_t u16CompData, uint16_t Reg_Addr, uint16_t u16Data)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    extern fifo8_cb_td mbm_data_fifo;

    mbm_data_st mbm_send_data;

    if ((g_sys.config.dev_mask.mb_comp & (0x01 << MB_Addr)) == 0)
    {
        return;
    }

    if (u16CompData != u16Data)
    {
        mbm_send_data.mbm_fun_code = MB_FUNC_WRITE_REGISTER;
        mbm_send_data.mbm_addr     = MB_Addr + 1;
        mbm_send_data.reg_addr     = Reg_Addr;
        mbm_send_data.reg_value    = u16Data;
        //压入FIFO;
        if (fifo8_push(&mbm_data_fifo, (uint8_t*)&mbm_send_data) == 0)
        {
            rt_kprintf("\n mbm_data_fifo ERRO\n");
        }
        rt_thread_delay(500);
    }

    //		rt_kprintf("EEV_Addr= %d,u16Mbm_EevMode= %x,EEV_Type=
    //%x\n",EEV_Addr,g_sys.config.Compressor.EEV[EEV_Addr].u16Mbm_EevMode,EEV_Type);
    return;
}

void GetPowerKeyDI(void)
{
    extern local_reg_st l_sys;
    extern sys_reg_st g_sys;
    static uint16_t u16ICTKey_Delay = 0;
    if (l_sys.u8ICT_PowerKey == FALSE)
    {
        //				if(sys_get_di_sts(DI_ICT_POWERON_BPOS) == 0)//按键按下
        //				{
        //					l_sys.u8ICT_PowerKey=TRUE;
        //				}
        if (!(g_sys.status.din_bitmap[1] & 0x01))
        {
            l_sys.u8ICT_PowerKey = TRUE;
        }
    }
    else
    {
        u16ICTKey_Delay++;
        if (u16ICTKey_Delay >= 10)
        {
            u16ICTKey_Delay = 10;
            //				if(sys_get_di_sts(DI_ICT_POWEROFF_BPOS) == 0)
            //				{
            //					l_sys.u8ICT_PowerKey=FALSE;
            //				}
            if (!(g_sys.status.din_bitmap[1] & 0x02))
            {
                l_sys.u8ICT_PowerKey = FALSE;
            }
        }
    }
}

//总体需求执行逻辑
void req_execution(int16_t target_req_temp, int16_t target_req_hum, int16_t target_req_fan)
{
    extern sys_reg_st g_sys;
    extern local_reg_st l_sys;
    uint8_t ICT_fsm_state;
    uint16_t ICT_test   = 0;
    uint16_t u16AI_V[2] = {0};

    GetPowerKeyDI();
    if (l_sys.u8ICT_PowerKey == FALSE)  //上电
    {
        g_sys.status.ICT.u16Status = ICT_IDLE;
    }
    ICT_fsm_state = g_sys.status.ICT.u16Status;

    switch (ICT_fsm_state)
    {
        case ICT_IDLE: {
            if (l_sys.u8ICT_PowerKey == TRUE)  //上电
            {
                g_sys.status.ICT.u16Status = ICT_START;
                g_sys.status.ICT.u16Fsm    = ICT_ST_START;
                g_sys.status.ICT.u16Test   = 0;
                l_sys.u8ICT_Delay          = 5;
                l_sys.u8ICT_Start          = TRUE;
                l_sys.u8ICT_Fsm            = 0;
                ICT_test                   = 0;
                req_bitmap_op(DO_POWER_ON_BPOS, 1);

                req_bitmap_op(DO_HUM_BPOS, 0);     // 1
                req_bitmap_op(DO_FILL_BPOS, 0);    // 2
                req_bitmap_op(DO_DRAIN_BPOS, 0);   // 3
                req_bitmap_op(DO_DEHUM1_BPOS, 0);  // 4
                req_bitmap_op(DO_RH1_BPOS, 0);     // 5
                req_bitmap_op(DO_COMP1_BPOS, 0);   // 6
                req_bitmap_op(DO_FAN_BPOS, 0);     // 7
                req_bitmap_op(DO_ALARM_BPOS, 1);   // 8
            }
            else
            {
                g_sys.status.ICT.u16Status = ICT_IDLE;
                req_bitmap_op(DO_POWER_ON_BPOS, 0);
                req_bitmap_op(DO_HUM_BPOS, 0);
                req_bitmap_op(DO_FILL_BPOS, 0);
                req_bitmap_op(DO_DRAIN_BPOS, 0);
                l_sys.u8ICT_Delay  = 0;
                l_sys.u8ICT_Cnt[0] = 0;
                l_sys.u8ICT_Cnt[1] = 0;
            }
        }
        break;
        case ICT_START: {
            if (l_sys.u8ICT_Delay)
            {
                g_sys.status.ICT.u16Vout[0] = 0;
                g_sys.status.ICT.u16Vout[1] = 0;
                g_sys.status.ICT.u16Vout[2] = 0;
                g_sys.status.ICT.u16Vout[3] = 0;
                g_sys.status.ICT.u16Vout[4] = 0;
                g_sys.status.ICT.u16Vout[5] = 0;
                g_sys.status.ICT.u16DI[0]   = 0;
                g_sys.status.ICT.u16DI[1]   = 0;
                l_sys.u8ICT_Delay--;
            }
            else
            {
                g_sys.status.ICT.u16Status = ICT_TEST;
            }
        }
        break;
        case ICT_TEST: {
            if (l_sys.u8ICT_Delay)
            {
                l_sys.u8ICT_Delay--;
                break;
            }
            ICT_test = g_sys.status.ICT.u16Test;
            req_bitmap_op(DO_POWER_ON_BPOS, 1);
            u16AI_V[0] = (float)(g_sys.status.ain[AI_SENSOR1] * 3.3 * 9.99 / 2.49 / 4095) * 100;
            u16AI_V[1] = (float)(g_sys.status.ain[AI_SENSOR2] * 3.3 * 9.99 / 2.49 / 4095) * 100;
            //					char u8Buff[128];
            //					sprintf(u8Buff,"AI_SENSOR1 = %d,V1 = %04f,AI_SENSOR2 =
            //%d,\n",g_sys.status.ain[AI_SENSOR1],(float)(g_sys.status.ain[AI_SENSOR1]*3.3*9.99/2.49/4095),g_sys.status.ain[AI_SENSOR2]);
            //					rt_kprintf("%s,\n",u8Buff);
            rt_kprintf("AI_SENSOR1 = %d,u16AI_V[0] = %d,AI_SENSOR2 = %d,u16AI_V[1] = %d,\n",
                       g_sys.status.ain[AI_SENSOR1], u16AI_V[0], g_sys.status.ain[AI_SENSOR2], u16AI_V[1]);

            switch (l_sys.u8ICT_Fsm)
            {
                case ICT_FSM_START: {
                    l_sys.u8ICT_Fsm = ICT_FSM_V12;
                    req_bitmap_op(DO_HUM_BPOS, 1);
                    l_sys.u8ICT_Delay = 4;
                    break;
                }
                case ICT_FSM_V12: {
                    if ((abs(u16AI_V[0] - 1200) > ICT_V_MAX) || (u16AI_V[0] < 100))
                    {
                        ICT_test |= 0x01;
                    }
                    g_sys.status.ICT.u16Vout[0] = u16AI_V[0];
                    l_sys.u8ICT_Fsm             = ICT_FSM_V10;
                    req_bitmap_op(DO_HUM_BPOS, 0);
                    req_bitmap_op(DO_FILL_BPOS, 1);
                    l_sys.u8ICT_Delay = 4;
                    break;
                }
                case ICT_FSM_V10: {
                    if ((abs(u16AI_V[0] - 1000) > ICT_V_MIN) || (u16AI_V[0] < 100))
                    {
                        ICT_test |= 0x02;
                    }
                    g_sys.status.ICT.u16Vout[1] = u16AI_V[0];
                    l_sys.u8ICT_Fsm             = ICT_FSM_V33;
                    req_bitmap_op(DO_FILL_BPOS, 0);
                    req_bitmap_op(DO_DRAIN_BPOS, 1);
                    l_sys.u8ICT_Delay = 4;
                    break;
                }
                case ICT_FSM_V33: {
                    if ((abs(u16AI_V[0] - 330) > ICT_I_MAX) || (u16AI_V[0] < 100))
                    {
                        ICT_test |= 0x04;
                    }
                    g_sys.status.ICT.u16Vout[2] = u16AI_V[0];
                    l_sys.u8ICT_Fsm             = ICT_FSM_VFG;
                    l_sys.u8ICT_Cnt[1]          = 25;
                    req_bitmap_op(DO_DRAIN_BPOS, 0);
                    break;
                }
                case ICT_FSM_VFG: {
                    if (l_sys.u8ICT_Cnt[1])
                    {
                        l_sys.u8ICT_Cnt[1]--;
                        MB_ADJ(MB_DEV_AEC01F1_ADDR, g_sys.status.ICT.u16Buff[0], MB_CFG_FANOUT, 20);
                    }
                    else
                    {
                        if (g_sys.status.ICT.u16Buff[2])
                        {
                            ICT_test |= 0x08;
                        }
                        g_sys.status.ICT.u16DI[0] = g_sys.status.ICT.u16Buff[2];
                        u16AI_V[1] += ICT_VI_OFFSET;
                        if ((abs(u16AI_V[1] - 200) > ICT_I_MAX) || (u16AI_V[1] < 100))
                        {
                            ICT_test |= 0x10;
                        }
                        g_sys.status.ICT.u16Vout[3] = u16AI_V[1];
                        l_sys.u8ICT_Fsm             = ICT_FSM_VAO;
                        l_sys.u8ICT_Cnt[1]          = 25;
                    }
                    break;
                }
                case ICT_FSM_VAO: {
                    if (l_sys.u8ICT_Cnt[1])
                    {
                        l_sys.u8ICT_Cnt[1]--;
                        MB_ADJ(MB_DEV_AEC01F1_ADDR, g_sys.status.ICT.u16Buff[0], MB_CFG_FANOUT, 50);
                    }
                    else
                    {
                        if (g_sys.status.ICT.u16Buff[2] != 0x3FFF)
                        {
                            ICT_test |= 0x20;
                        }
                        g_sys.status.ICT.u16DI[1] = g_sys.status.ICT.u16Buff[2];
                        u16AI_V[1] += ICT_VSP_OFFSET;
                        if ((abs(u16AI_V[1] - 500) > ICT_I_MAX) || (u16AI_V[1] < 100))
                        {
                            ICT_test |= 0x40;
                        }
                        g_sys.status.ICT.u16Vout[4] = u16AI_V[1];
                        g_sys.status.ICT.u16Status  = ICT_STOP;
                    }

                    break;
                }
                default: {
                    g_sys.status.ICT.u16Status = ICT_STOP;
                    break;
                }
            }

            g_sys.status.ICT.u16Test = ICT_test;
            //						rt_kprintf("pa_volt0=%d,pa_volt1=%d,p_cur0=%d,p_cur10=%d,p_cur11=%d,ICT_test=%x\n",g_sys.status.mbm.pwr[0].pa_volt,g_sys.status.mbm.pwr[1].pa_volt,g_sys.status.mbm.pwr[0].p_cur[0],g_sys.status.mbm.pwr[1].p_cur[0],g_sys.status.mbm.pwr[1].p_cur[1],ICT_test);
            rt_kprintf(
                "u16DI[1] =%x,u16Vout[3] =%d,u16Vout[4] =%d,l_sys.u8ICT_PowerKey =%x,u16Status =%x,u16Test =%x\n",
                g_sys.status.ICT.u16DI[1], g_sys.status.ICT.u16Vout[3], g_sys.status.ICT.u16Vout[4],
                l_sys.u8ICT_PowerKey, g_sys.status.ICT.u16Status, g_sys.status.ICT.u16Test);
        }
        break;
        case ICT_STOP: {
            if (g_sys.status.ICT.u16Test)
            {
                g_sys.status.ICT.u16Fsm = ICT_ST_ERR;
            }
            else
            {
                g_sys.status.ICT.u16Fsm = ICT_ST_OK;
            }
            g_sys.status.ICT.u16Status = ICT_IDLE;
            l_sys.u8ICT_PowerKey       = FALSE;
            req_bitmap_op(DO_POWER_ON_BPOS, 0);
            req_bitmap_op(DO_HUM_BPOS, 0);
            req_bitmap_op(DO_FILL_BPOS, 0);
            req_bitmap_op(DO_DRAIN_BPOS, 0);
            rt_kprintf("u16Test =%x,u16Vout[0] =%d,1 =%d,2 =%d,u16DI[0] =%x,u16DI[1] =%x,u16Vout[4] =%d,\n",
                       g_sys.status.ICT.u16Test, g_sys.status.ICT.u16Vout[0], g_sys.status.ICT.u16Vout[1],
                       g_sys.status.ICT.u16Vout[2], g_sys.status.ICT.u16DI[0], g_sys.status.ICT.u16DI[1],
                       g_sys.status.ICT.u16Vout[4]);
        }
        break;
        default: {
            g_sys.status.ICT.u16Status = ICT_IDLE;
            l_sys.u8ICT_PowerKey       = FALSE;
            req_bitmap_op(DO_POWER_ON_BPOS, 0);
            req_bitmap_op(DO_HUM_BPOS, 0);
            req_bitmap_op(DO_FILL_BPOS, 0);
            req_bitmap_op(DO_DRAIN_BPOS, 0);
        }
        break;
    }
    //		rt_kprintf("din_bitmap[0] =%x,din_bitmap[1] =%x,l_sys.u8ICT_PowerKey =%x,u16Status =%x,u16Test
    //=%x\n",g_sys.status.din_bitmap[0],g_sys.status.din_bitmap[1],l_sys.u8ICT_PowerKey,g_sys.status.ICT.u16Status,g_sys.status.ICT.u16Test);
    ////    //电源相序切换
    ////		power_phase_switch();
    //    //风机控制函数
    //		fan_req_exe(target_req_temp,target_req_hum,target_req_fan);
    //    //压缩机与冷冻水控制
    //		compressor_req_exe(target_req_temp,target_req_hum);
    //    //电加热控制
    //		heater_req_exe(target_req_temp,target_req_hum);
    //    //加湿器控制
    //		humidifier_req_exe(target_req_hum);
    //    //除湿控制
    //		dehumer_req_exe(target_req_temp,target_req_hum);
    return;
}

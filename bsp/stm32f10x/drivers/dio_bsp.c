#ifndef __DIO
#define __DIO
#include <components.h>
#include <rtthread.h>
#include "dio_bsp.h"
#include "led_bsp.h"
//#define	DI_MAX_CNT						18
#define DI_MAX_CNT 9
#define DIN_MASK_MASK 0x003f
#define DIN_MASK_MASK1 0xffff  //屏蔽
#define DIN_POLARITY_MASK 0xffc0
#define DI_BUF_DEPTH 50
#define DI_UPDATE_PERIOD 100
#define SAMPLE_INTERVAL 2

#define DIN_WORD2 16  // 2个字节

// RT_TIMER_TICK_PER_SECOND
typedef struct
{
    uint16_t bitmap[2];
    uint16_t reg_array[2][DI_BUF_DEPTH];
} di_dev_st;

typedef struct
{
    uint16_t bitmap;
} do_dev_st;

typedef struct
{
    di_dev_st din;
    do_dev_st dout;
} dio_dev_st;

typedef struct
{
    uint16_t pin_id;
    void* pin_base;
} pin_map_st;

// static rt_timer_t pwm_slow_timer;

static uint16_t do_set(int16_t pin_id, BitAction value);
#define Pin_Map_In DI_MAX_CNT
const pin_map_st in_pin_map_inst[Pin_Map_In] =  //数字输入Pin_Map
    {
        {GPIO_Pin_15, GPIOA},  // DI1
        {GPIO_Pin_12, GPIOA},  // DI2
        {GPIO_Pin_11, GPIOA},  // DI3
        {GPIO_Pin_8, GPIOA},   // DI4
        {GPIO_Pin_9, GPIOC},   // DI5
        {GPIO_Pin_8, GPIOC},   // DI6
        {GPIO_Pin_7, GPIOC},   // DI7
        {GPIO_Pin_6, GPIOC},   // DI8
        {GPIO_Pin_15, GPIOB},  // DI9
};
#define Pin_Map_Out 9
const pin_map_st out_pin_map_inst[Pin_Map_Out] =  //数字输出Pin_Map
    {
        {GPIO_Pin_4, GPIOB},   // DO1
        {GPIO_Pin_5, GPIOB},   // DO2
        {GPIO_Pin_6, GPIOB},   // DO3
        {GPIO_Pin_7, GPIOB},   // DO4
        {GPIO_Pin_0, GPIOD},   // DO5
        {GPIO_Pin_1, GPIOD},   // DO6
        {GPIO_Pin_0, GPIOC},   // DO7
        {GPIO_Pin_1, GPIOC},   // DO8
        {GPIO_Pin_13, GPIOC},  // LED
};

// local variable definition
static dio_dev_st dio_dev_inst;

// DI重映射表
const Bit_remap_st DI_remap_table[] = {
    {0, 0},   //高压1
    {1, 1},   //低压1
    {2, 4},   //送风机
    {3, 5},   //气流
    {4, 6},   //自定义
    {5, 10},  //滤网
    {6, 8},   //热开关
    {7, 9},   //远程关机
    {8, 7},   //漏水
};
// DO重映射表
const Bit_remap_st DO_remap_table[] = {
    {0, 0},   //
    {1, 1},   //
    {2, 2},   //
    {3, 3},   //
    {4, 13},  // Heater
    {5, 6},   // Comp
    {6, 12},  // Fan
    {7, 15},  // ALARM
};
// AI重映射表
const Bit_remap_st AI_remap_table[] = {
    {0, 2},   //
    {1, 3},   //
    {2, 5},   //
    {3, 7},   //
    {4, 9},   //
    {5, 10},  //
};
// AO重映射表
const Bit_remap_st AO_remap_table[] = {
    {1, 1},  //
    {2, 4},  //
    {3, 5},  //
};

//重映射端口，兼容M1板
uint16_t Sts_Remap(uint16_t u16IN_Bit, uint8_t Rep_Type, uint8_t Rep_Dir)
{
    uint16_t u16Rep_bit;
    uint8_t i, u8Length;

    u16Rep_bit = 0x00;
    switch (Rep_Type)
    {
        case Rep_DI:
            u8Length = sizeof(DI_remap_table) / 2;
            for (i = 0; i < u8Length; i++)
            {
                if (Rep_Dir)
                {
                    if ((u16IN_Bit >> DI_remap_table[i].u8M1_Bit) & 0x0001)
                    {
                        u16Rep_bit |= (0x0001 << DI_remap_table[i].u8M3_Bit);
                    }
                }
                else
                {
                    if ((u16IN_Bit >> DI_remap_table[i].u8M3_Bit) & 0x0001)
                    {
                        u16Rep_bit |= (0x0001 << DI_remap_table[i].u8M1_Bit);
                    }
                }
            }
            //			rt_kprintf("u16IN_Bit = %X,u8Length = %X,u16Rep_bit = %X\n",u16IN_Bit,u8Length,u16Rep_bit);
            break;
        case Rep_DO:
            u8Length = sizeof(DO_remap_table) / 2;
            for (i = 0; i < u8Length; i++)
            {
                if (Rep_Dir)
                {
                    if ((u16IN_Bit >> DO_remap_table[i].u8M1_Bit) & 0x0001)
                    {
                        u16Rep_bit |= (0x0001 << DO_remap_table[i].u8M3_Bit);
                    }
                }
                else
                {
                    if ((u16IN_Bit >> DO_remap_table[i].u8M3_Bit) & 0x0001)
                    {
                        u16Rep_bit |= (0x0001 << DO_remap_table[i].u8M1_Bit);
                    }
                }
            }
            //			rt_kprintf("u16IN_Bit = %X,u8Length = %X,u16Rep_bit = %X\n",u16IN_Bit,u8Length,u16Rep_bit);
            break;
        case Rep_AI:
            u8Length = sizeof(AI_remap_table) / 2;
            for (i = 0; i < u8Length; i++)
            {
                if (Rep_Dir)
                {
                    if (u16IN_Bit == AI_remap_table[i].u8M1_Bit)
                    {
                        u16Rep_bit = AI_remap_table[i].u8M3_Bit;
                    }
                }
                else
                {
                    if (u16IN_Bit == AI_remap_table[i].u8M3_Bit)
                    {
                        u16Rep_bit = AI_remap_table[i].u8M1_Bit;
                    }
                }
            }
            break;
        case Rep_AO:
            u8Length = sizeof(AO_remap_table) / 2;
            for (i = 0; i < u8Length; i++)
            {
                if (Rep_Dir)
                {
                    if (u16IN_Bit == AO_remap_table[i].u8M1_Bit)
                    {
                        u16Rep_bit = AO_remap_table[i].u8M3_Bit;
                    }
                }
                else
                {
                    if (u16IN_Bit == AO_remap_table[i].u8M3_Bit)
                    {
                        u16Rep_bit = AO_remap_table[i].u8M1_Bit;
                    }
                }
            }
            break;
        default:
            break;
    }
    return u16Rep_bit;
}

// digital input sampling thread
void di_thread_entry(void* parameter)
{
    rt_thread_delay(DI_THREAD_DELAY);
    drv_dio_bsp_init();
    dio_reg_init();
    drv_di_timer_init();
    //    pwm_slow_timer_init();
    rt_thread_delay(500);
    while (1)
    {
        di_reg_update();
        rt_thread_delay(DI_UPDATE_PERIOD);
    }
}

////PWM电加热控制函数
// void slow_pwm_set(uint8_t channel, uint16_t dutycycle)
//{
//    extern sys_reg_st			g_sys;
//    if((channel > 1)|(dutycycle > 100))
//    {
//        return;
//    }
//    else
//    {
//        g_sys.status.pwmout[channel] = dutycycle;
//    }
//}

////PWM电加热定时器中断回调函数,改为1路
// static void pwm_slow_time_cb(void *param)
//{
//		extern sys_reg_st			g_sys;
//    static uint16_t pwm5_cnt_reg = 0;
////    static uint16_t pwm6_cnt_reg = 0;
//
//    if((g_sys.config.dev_mask.pwm_out&(0x0001<<PWM_OUT0))&&(g_sys.status.pwmout[PWM_OUT0] != 0))
//    {
//        if(pwm5_cnt_reg <= g_sys.config.heater.pwm_period1)
//        {
//            if(pwm5_cnt_reg <= g_sys.config.heater.pwm_period1*g_sys.status.pwmout[PWM_OUT0]/100)
//            {
//                GPIO_SetBits(GPIOE,GPIO_Pin_5);
//            }
//            else
//            {
//                GPIO_ResetBits(GPIOE,GPIO_Pin_5);
//            }
//            pwm5_cnt_reg++;
//        }
//        else
//        {
//            pwm5_cnt_reg = 0;
//        }
//    }
//    else
//    {
//        pwm5_cnt_reg = 0;
//        GPIO_ResetBits(GPIOE,GPIO_Pin_5);
//    }

//
////    if((g_sys.config.dev_mask.pwm_out&(0x0001<<PWM_OUT1))&&(g_sys.status.pwmout[PWM_OUT1] != 0))
////    {
////        if(pwm6_cnt_reg <= g_sys.config.heater.pwm_period2)
////        {
////            if(pwm6_cnt_reg <= g_sys.config.heater.pwm_period2*g_sys.status.pwmout[PWM_OUT1]/100)
////            {
////                GPIO_SetBits(GPIOE,GPIO_Pin_6);
////            }
////            else
////            {
////                GPIO_ResetBits(GPIOE,GPIO_Pin_6);
////            }
////            pwm6_cnt_reg++;
////        }
////        else
////        {
////            pwm6_cnt_reg = 0;
////        }
////    }
////    else
////    {
////        pwm6_cnt_reg = 0;
////        GPIO_ResetBits(GPIOE,GPIO_Pin_6);
////    }
//}

////PWM电加热软件定时器
// static void pwm_slow_timer_init(void)
//{
//    pwm_slow_timer = rt_timer_create("timer1",
//                                      pwm_slow_time_cb,
//                                      RT_NULL,
//                                      100,
//                                      RT_TIMER_FLAG_PERIODIC);
//
//    if (pwm_slow_timer != RT_NULL)
//        rt_timer_start(pwm_slow_timer);
//}

/**
 * @brief  digital IOs GPIO initialization
 * @param  none
 * @retval none
 */
//数字输入输出初始化函数
static void drv_dio_bsp_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    uint16_t i;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                               RCC_APB2Periph_GPIOE,
                           ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_PD01, ENABLE);

    //数字输入PIN初始化
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    for (i = 0; i <= Pin_Map_In - 1; i++)
    {
        GPIO_InitStructure.GPIO_Pin = in_pin_map_inst[i].pin_id;
        GPIO_Init(in_pin_map_inst[i].pin_base, &GPIO_InitStructure);
    }

    //数字输出PIN初始化
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    for (i = 0; i <= Pin_Map_Out - 1; i++)
    {
        GPIO_InitStructure.GPIO_Pin = out_pin_map_inst[i].pin_id;
        GPIO_Init(out_pin_map_inst[i].pin_base, &GPIO_InitStructure);
    }

    //				//LED灯带输出PIN初始化
    //		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    //    		WsDat[0] = 0x808080;//显存赋值
    //    GPIO_InitStructure.GPIO_Pin = out_pin_map_inst[i].pin_id;

    //			Led_Gpio_Init();
    //复位
    for (i = 1; i <= Pin_Map_Out - 1; i++)
    {
        do_set(i, Bit_RESET);
    }
}

/**
 * @brief  digital io stucture initialization
 * @param  none
 * @retval none
 */
//数字输入初始化函数�
static void dio_reg_init(void)
{
    uint16_t i;
    //		dio_dev_inst.din.bitmap[0] = 0;
    memset(dio_dev_inst.din.bitmap, 0, sizeof(dio_dev_inst.din.bitmap));
    for (i = 0; i < DI_BUF_DEPTH; i++)
    {
        dio_dev_inst.din.reg_array[0][i] = 0;
        dio_dev_inst.din.reg_array[1][i] = 0;
    }
    dio_dev_inst.dout.bitmap = 0;
}

/**
 * @brief  digital input result caculation
 * @param  none
 * @retval none
 */
//数字输入OOK解调
static void di_reg_update(void)
{
    uint16_t di_data[2], i, j, pusl_mask;
    uint16_t di_reg[DI_MAX_CNT];
    extern sys_reg_st g_sys;

    //		pusl_mask = g_sys.config.dev_mask.din_pusl;
    //重映射
    pusl_mask = Sts_Remap(g_sys.config.dev_mask.din_pusl, Rep_DI, 1);
    //		di_data = 0;
    memset(di_data, 0, sizeof(di_data));

    for (i = 0; i < DI_MAX_CNT; i++)
    {
        di_reg[i] = 0;
    }

    for (i = 0; i < DI_MAX_CNT; i++)  // outer loop caculate each channels di data
    {
        for (j = 0; j < DI_BUF_DEPTH; j++)  // inner loop caculate sum of one channel di data
        {
            if (i < DIN_WORD2)
            {
                di_reg[i] += (dio_dev_inst.din.reg_array[0][j] >> i) & (0x0001);
            }
            else
            {
                di_reg[i] += (dio_dev_inst.din.reg_array[1][j] >> (i - DIN_WORD2)) & (0x0001);
            }
        }
    }
    for (i = 0; i < DI_MAX_CNT; i++)
    {
        if (i < DIN_WORD2)
        {
            if (pusl_mask & (0x01 << i))  //脉冲采集
            {
                //								if(di_reg[i] > (DI_BUF_DEPTH - 15))
                //								{
                //										di_data[0] &= ~(0x0001<<i);
                //								}
                //								else
                //								{
                //										di_data[0] |= (0x0001<<i);
                //								}
                // 20170223,台达风机修改
                if ((di_reg[i] > (DI_BUF_DEPTH - 15)) ||
                    (di_reg[i] < g_sys.config.alarm[ACL_FAN_OVERLOAD2].alarm_param))
                {
                    di_data[0] &= ~(0x0001 << i);
                }
                else
                {
                    di_data[0] |= (0x0001 << i);
                }
            }
            else
            {
                // if((di_reg[i]>(DI_BUF_DEPTH>>2))&&(di_reg[i]<(DI_BUF_DEPTH-(DI_BUF_DEPTH>>2))))//[25%~75%] duty cycle
                // is consider set state, otherwise is considered reset state
                if (di_reg[i] < (DI_BUF_DEPTH -
                                 (DI_BUF_DEPTH >>
                                  2)))  //[0~75%] duty cycle is consider set state, otherwise is considered reset state
                {
                    di_data[0] |= (0x0001 << i);
                }
                else
                {
                    di_data[0] &= ~(0x0001 << i);
                }
            }
        }
        else
        {
            if (di_reg[i] <
                (DI_BUF_DEPTH -
                 (DI_BUF_DEPTH >> 2)))  //[0~75%] duty cycle is consider set state, otherwise is considered reset state
            {
                di_data[1] |= (0x0001 << (i - DIN_WORD2));
            }
            else
            {
                di_data[1] &= ~(0x0001 << (i - DIN_WORD2));
            }
        }
    }
    //				rt_kprintf("di_reg[0] = %d,di_reg[1] = %d,di_reg[2] = %d,di_reg[3] = %d,di_reg[4] =
    //%d\n",di_reg[0],di_reg[1],di_reg[2],di_reg[3],di_reg[4]); 		dio_dev_inst.din.bitmap[0] = di_data[0];
    //		dio_dev_inst.din.bitmap[1] = di_data[1];
    memcpy(dio_dev_inst.din.bitmap, di_data, sizeof(di_data));
    //		rt_kprintf("dio_dev_inst.din.bitmap[0] = %x\n",dio_dev_inst.din.bitmap[0]);
    //		rt_kprintf("dio_dev_inst.din.bitmap[1] = %d\n",dio_dev_inst.din.bitmap[1]);
}

/**
 * @brief  raw digital input data read
 * @param  none
 * @retval 18 channels data, each bit stands for one channel
 */
//数字输入函数，对所有数字输入状态进行更新
// void  di_read(uint16_t pBuff[],uint8_t pNum)
//{
////		uint16_t read_bitmap;
////		uint16_t i;
////		read_bitmap = 0;
////		for(i=0;i<=DI_MAX_CNT-1;i++)
////		{
////				read_bitmap |=
///GPIO_ReadInputDataBit(in_pin_map_inst[DI_MAX_CNT-1-i].pin_base,in_pin_map_inst[DI_MAX_CNT-1-i].pin_id); /
///if(i<DI_MAX_CNT-1) /				{ /						read_bitmap = read_bitmap << 1; /				} /		} /
///return read_bitmap;
//
//		uint16_t read_bitmap[2];
//		uint16_t i;
//
//		memset(read_bitmap, 0, sizeof(read_bitmap));
//		for(i=0;i<=DI_MAX_CNT-1;i++)
//		{
//				read_bitmap[i/16] |=
//GPIO_ReadInputDataBit(in_pin_map_inst[DI_MAX_CNT-1-i].pin_base,in_pin_map_inst[DI_MAX_CNT-1-i].pin_id);
//				read_bitmap[i/16] <<= 1;

//		}
//    for(i=0;i<pNum;i++)
//		{
//       pBuff[i]=read_bitmap[i];
//		}
//}

static uint32_t di_read(void)
{
    uint32_t read_bitmap;
    uint16_t i;
    read_bitmap = 0;
    for (i = 0; i <= DI_MAX_CNT - 1; i++)
    {
        read_bitmap |= GPIO_ReadInputDataBit(in_pin_map_inst[DI_MAX_CNT - 1 - i].pin_base,
                                             in_pin_map_inst[DI_MAX_CNT - 1 - i].pin_id);
        if (i < DI_MAX_CNT - 1)
        {
            read_bitmap = read_bitmap << 1;
        }
    }
    return read_bitmap;
}

//数字输出控制函数；
static uint16_t do_set(int16_t pin_id, BitAction value)
{
    if ((pin_id <= Pin_Map_Out) && (pin_id > 0))
    {
        GPIO_WriteBit(out_pin_map_inst[pin_id - 1].pin_base, out_pin_map_inst[pin_id - 1].pin_id, value);
        return 1;
    }
    else
    {
        return 0;
    }
}

//置位所有数字输出
static void do_set_all(void)
{
    uint16_t i;
    for (i = 1; i <= Pin_Map_Out - 1; i++)
    {
        do_set(i, Bit_SET);
    }
}

//复位所有数字输出
static void do_reset_all(void)
{
    uint16_t i;
    for (i = 1; i <= Pin_Map_Out - 1; i++)
    {
        do_set(i, Bit_RESET);
    }
}

/**
 * @brief  digital input sample interval timeout callback function, calls di_read() each time to update di buffer queue
 * @param  none
 * @retval none
 */
//数字输入定时器回调函数，对数字输入电平进行采样后放入缓冲队列；
static void stimer_di_timeout(void* parameter)
{
    extern sys_reg_st g_sys;
    //			static uint16_t count = 0;
    //		if(count >= DI_BUF_DEPTH)
    //		{
    //				count = count%DI_BUF_DEPTH;
    //		}
    //		dio_dev_inst.din.reg_array[count] = di_read();
    //		count++;

    //		static uint16_t count;
    //		uint16_t pBuffer[2];
    //		uint8_t i;
    //
    //		memset(pBuffer, 0, sizeof(pBuffer));
    //		di_read(pBuffer,2);// 读取DI数据
    //
    //		if(count >= DI_BUF_DEPTH)
    //		{
    //				count = count%DI_BUF_DEPTH;
    //		}
    //		for(i=0;i<2;i++)
    //		{
    //			dio_dev_inst.din.reg_array[i][count] = pBuffer[i];
    //		}
    //		count++;

    static uint16_t count;
    uint32_t pBuf;

    if (count >= DI_BUF_DEPTH)
    {
        count = count % DI_BUF_DEPTH;
    }
    pBuf                                 = di_read();
    g_sys.status.din_bitmap[1]           = di_read();
    dio_dev_inst.din.reg_array[0][count] = pBuf;
    dio_dev_inst.din.reg_array[1][count] = pBuf >> 16;
    count++;
}

/**
 * @brief  digital input sample interval timer initialization, expires in 6 miliseconds pieriod
 * @param  none
 * @retval none
 */
//数字输入定时器，每6ms周期对数字输入进行采样
static uint16_t drv_di_timer_init(void)
{
    rt_timer_t stimer_dio;
    stimer_dio = rt_timer_create("stimer_di", stimer_di_timeout, RT_NULL, SAMPLE_INTERVAL, RT_TIMER_FLAG_PERIODIC);
    rt_timer_start(stimer_dio);
    return 1;
}
/**
 * @brief  digital IO initialization function
 * @param  none
 * @retval none
 */
void drv_dio_init(void)
{
    drv_dio_bsp_init();
    drv_di_timer_init();
}

/**
 * @brief  update global variable g_din_inst and g_ain_inst according to di and ai inputs
 * @param  none
 * @retval none
 **/
void di_sts_update(sys_reg_st* gds_sys_ptr)
{
    //			uint16_t u16DI0;
    //			uint16_t u16DI0_P;

    //
    //		uint16_t din_mask_bitmap[2];
    //		uint16_t din_bitmap_polarity[2];

    //		memcpy(din_mask_bitmap,gds_sys_ptr->config.dev_mask.din,sizeof(din_mask_bitmap));
    //		memcpy(din_bitmap_polarity,gds_sys_ptr->config.dev_mask.din_bitmap_polarity,sizeof(din_bitmap_polarity));
    //	  //mask报警掩码
    //		//重映射
    //		u16DI0_P=din_bitmap_polarity[0];
    //		din_bitmap_polarity[0]=Sts_Remap(u16DI0_P,Rep_DI,1);
    ////		rt_kprintf("dio_dev_inst.din.bitmap[0] = %X\n",dio_dev_inst.din.bitmap[0]);
    //		dio_dev_inst.din.bitmap[0] = (~(dio_dev_inst.din.bitmap[0]^din_bitmap_polarity[0]));
    //		dio_dev_inst.din.bitmap[1] = (~(dio_dev_inst.din.bitmap[1]^din_bitmap_polarity[1]));

    //		// 数字输入掩码
    //		u16DI0=din_mask_bitmap[0];
    //		din_mask_bitmap[0]=Sts_Remap(u16DI0,Rep_DI,1);
    //		gds_sys_ptr->status.din_bitmap[0] = din_mask_bitmap[0] & dio_dev_inst.din.bitmap[0];
    //		gds_sys_ptr->status.din_bitmap[1] &=0xFF;
    //		gds_sys_ptr->status.din_bitmap[1] |= ((din_mask_bitmap[1] & dio_dev_inst.din.bitmap[1])<<8);//放到高16位
    ////		rt_kprintf("gds_sys_ptr->status.din_bitmap[0] = %X,dout_bitmap =
    ///%X\n",gds_sys_ptr->status.din_bitmap[0],gds_sys_ptr->status.dout_bitmap);
    //		u16DI0=gds_sys_ptr->status.din_bitmap[0];
    //		gds_sys_ptr->status.din_bitmap[0]=Sts_Remap(u16DI0,Rep_DI,0);
    ////		rt_kprintf("u16DI0 = %X,gds_sys_ptr->status.din_bitmap[0] = %X,u16DI0_P = %X,,din_bitmap_polarity =
    ///%X\n",u16DI0,gds_sys_ptr->status.din_bitmap[0],u16DI0_P,din_bitmap_polarity[0]);
    uint16_t u16DI0;
    uint16_t u16DI0_P;

    uint16_t din_mask_bitmap     = gds_sys_ptr->config.dev_mask.din[0];
    uint16_t din_bitmap_polarity = gds_sys_ptr->config.dev_mask.din_bitmap_polarity[0];

    // mask报警掩码
    //重映射
    u16DI0_P            = din_bitmap_polarity;
    din_bitmap_polarity = Sts_Remap(u16DI0_P, Rep_DI, 1);
    //		rt_kprintf("dio_dev_inst.din.bitmap[0] = %X\n",dio_dev_inst.din.bitmap[0]);
    dio_dev_inst.din.bitmap[0] = (~(dio_dev_inst.din.bitmap[0] ^ din_bitmap_polarity));

    // 数字输入掩码
    u16DI0                            = din_mask_bitmap;
    din_mask_bitmap                   = Sts_Remap(u16DI0, Rep_DI, 1);
    gds_sys_ptr->status.din_bitmap[0] = din_mask_bitmap & dio_dev_inst.din.bitmap[0];
    u16DI0                            = gds_sys_ptr->status.din_bitmap[0];
    gds_sys_ptr->status.din_bitmap[0] = Sts_Remap(u16DI0, Rep_DI, 0);
}

void dio_set_do(uint16_t channel_id, BitAction data)
{
    do_set(channel_id, data);
}

// LED闪烁
void led_toggle(void)
{
    extern sys_reg_st g_sys;
    static uint8_t led_flag = 0;

    if (led_flag == 0)
    {
        do_set(Pin_Map_Out, Bit_RESET);
        led_flag = 1;
        //				if(g_sys.status.alarm_status_cnt.critical_cnt)
        //				{
        //						LED_Test(RED);
        //				}
        //				else
        //				{
        //						if(g_sys.status.alarm_status_cnt.major_cnt)
        //						{
        //								LED_Test(ORANGE);
        //						}
        //						else
        //						{
        //								LED_Test(BLUE);
        //						}
        //				}
    }
    else
    {
        do_set(Pin_Map_Out, Bit_SET);
        led_flag = 0;
    }
}

// FINSH_FUNCTION_EXPORT(slow_pwm_set, slow pwm set.);
FINSH_FUNCTION_EXPORT(do_set, set data out bit);
FINSH_FUNCTION_EXPORT(do_set_all, set all data bit 1);
FINSH_FUNCTION_EXPORT(do_reset_all, set all data out bit 0);
#endif  //__DIO

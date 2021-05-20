/*
 * FreeModbus Libary: user callback functions and buffer define in slave mode
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: user_mb_app.c,v 1.60 2013/11/23 11:49:05 Armink $
 */
#include "user_mb_app.h"

#include "global_var.h"
#include "event_record.h"
/*------------------------Slave mode use these variables----------------------*/
//Slave mode:HoldingRegister variables



	


USHORT   usSRegHoldStart                              = S_REG_HOLDING_START;
USHORT   usSRegHoldBuf[S_REG_HOLDING_NREGS]           ;
typedef struct 
{
	uint8_t reg_type; //0=config_reg;1 =status_reg;
	uint16_t reg_addr;//地址
	uint8_t reg_w_r; //3 =write&read,2=read_only,3=write_only
}reg_table_st;

typedef struct
{
	uint16_t *reg_ptr;
}mbm_reg_table_st;
static uint16_t mbm_cmd_reg =0;
const mbm_reg_table_st mbm_reg_table[]=
{
	{&mbm_cmd_reg}
};	
//内存到modbus的映射表。
//元素位置对应ModeBus  协议栈中usSRegHoldBuf位置
//元素值对应conf_reg_map_inst，内存数据的位置。

//监控通信寄存器S_REG_HOLDING_NREGS=70
const reg_table_st modebus_slave_reg_table[]={
{0,0},//开关机         0
{0,26},//设定回风温度  1
{0,28},//设定回风湿度  2
{1,61},//回风温度测量值 3
{1,62},//回风湿度测量值 4
{0,25},//设定送风温度 5
{0,27},//设定送风湿度 6
{1,59},//送风温度测量值 7
{1,60},//送风湿度测量值 8
{0,24},//模式控制目标              9
{0,22},//温度控制方式 P or PID     10
{0,23},//湿度控制方式              11
{1,25},//系统状态字    12
{1,32},//告警0         13
{1,33},//告警1         14
{1,34},//告警2         15
{1,35},//告警3         16
{1,36},//告警4         17
{1,37},//告警5         18
{1,54},//告警6,通信告警类  19
//fan运行时间
{1,120}, //            20
{1,121}, //            21
//压缩机1运行时间 //               
{1,122}, //22
{1,123}, //23
//压缩机2运行时间
{1,124}, //24
{1,125}, //25
//RESERVE
{1,124}, //26
{1,125}, //27
//RESERVE
{1,124}, //28
{1,125}, //29

{1,65},//加湿电流 30
{1,64},//电导率 31
{1,117},//干节点输入状态0 32
{1,119},//干结点输出状态 	33
{1,98},//AI0 34
{1,99},//水流量 AI1 35
{1,100},//AI2  36
{1,101},//AI3  37
{1,103},//NTC1  38
{1,104},//NTC2  39
{1,111},//AOUT1 40
{1,112},//AOUT2 41
{1,113},//AOUT3 42
{1,114},//AOUT4 43
{1,115},//AOUT5 44
{0,178},//回风高温告警植 45
{0,181},//回风低温告警植 46
{0,184},//回风高湿告警植 47
{0,187},//回风低湿告警植 48
{0,190},//送风高温告警植 49
{0,193},//送风低温告警植 50

{0,344},//过压告警51
{0,353},//欠压告警52

{0,407},//入水高温告警53
{0,149},//消音54

{1,105},//NTC3  55
{1,106},//NTC4  56
{1,107},//NTC5  57
{1,108},//NTC6  58
{1,68},//A_voltage  59
{1,69},//B_voltage  60
{1,70},//C_voltage  61
{1,71},//power_fre  62
{1,72},//power_status  63
{3,0},//cmd type64
//RESERVE
{1,118},//干节点输入状态1 65
{1,102},//AI4  66
{1,109},//NTC7  67
{1,110},//NTC8  68
/****************************************************************/
{1,46},//Temp4  		69
{1,47},//Humidity4  70
{1,48},//Temp5  		71
{1,49},//Humidity5  72
{1,50},//Temp6  		73
{1,51},//Humidity6  74
{1,52},//Temp7  		75
{1,53},//Humidity7  76
{1,82},//Reserv1  77
{1,83},//Reserv1  78
{0,29},//temp_precision  79

{0,31},//temp_deadband  80
{0,30},//hum_precision  81
{0,32},//hum_deadband   82
{0,91},//fan.startup_delay  83
{0,92},//fan.stop_delay  84
{0,98},//fan.cold_start_delay  85
{0,93},//fan.set_speed   86
{0,94},//fan.min_speed  87
{0,95},//fan.max_speed  88
{0,96},//fan.dehum_ratio  89

{0,89},//fan.mode   90
{0,67},//compressor.startup_delay  91
{0,68},//compressor.stop_delay  92
{0,59},//dehumer.stop_dehum_temp  93
{0,109},//humidifier.hum_en   94

//{0,152},//ext_fan_min_press  95
//{0,153},//ext_fan_max_press  96
//{0,160},//ext_fan1_set_speed  97
//{0,158},//ext_fan_min_speed   98
//{0,159},//ext_fan_max_speed   99
{0,231},//PMINID  95
{0,232},//PMAXID  96
{0,239},//Reserv1  97
{0,237},//MINSPEEDID   98
{0,238},//MAXSPEEDID   99

{0,383},//alarm[ACL_WATER_OVERFLOW].alarm_param  100
{0,263},//alarm[ACL_SHORT_TERM1].enable_mode  	101
{0,271},//alarm[ACL_INV_HIPRESS].alarm_param   102
{0,259},//alarm[ACL_EXTMP1].alarm_param   103
//fan2运行时间
{1,80}, //            104
{1,81}, //            105
//fan3运行时间
{1,83}, //            106
{1,84}, //            107
//fan4运行时间
{1,85}, //            108
{1,86}, //            109
//fan5运行时间
{1,87}, //            110
{1,88}, //            111
};

enum
{
  BAUD_4800 = 0,
	BAUD_9600,
	BAUD_19200,
	BAUD_38400,
	BAUD_57600,
	BAUD_115200,
};


typedef struct
{
		uint16_t baudrate;
		uint16_t com_addr;
}communication_change_st;
static communication_change_st com_change_inst;
static ULONG Get_Baudrate(uint16_t 	baudrate)
{
		ULONG ulBaudRate;	
		switch(com_change_inst.baudrate )
		{
			
			case BAUD_4800:
					ulBaudRate = 4800;
					break;
			case BAUD_9600:
					ulBaudRate = 9600;
					break;
			case BAUD_19200:
					ulBaudRate = 19200;
					break;
			case BAUD_38400:
					ulBaudRate = 38400;
					break;
			case BAUD_57600:
					ulBaudRate = 57600;
					break;
			case BAUD_115200:
					ulBaudRate = 115200;
					break;
			default:
					ulBaudRate = 9600;
				break;
		}	
		return ulBaudRate;
}
static void change_surv_baudrate(void)
{
	extern sys_reg_st					g_sys; 

	if((com_change_inst.baudrate != g_sys.config.general.surv_baudrate)||(g_sys.config.general.surv_addr != com_change_inst.com_addr ))
	{
			com_change_inst.baudrate  =  g_sys.config.general.surv_baudrate;
			com_change_inst.com_addr = g_sys.config.general.surv_addr;
			eMBInit(MB_RTU,(UCHAR)g_sys.config.general.surv_addr, UPORT_MONITOR, Get_Baudrate(com_change_inst.baudrate),  MB_PAR_NONE);
			eMBEnable();
	}
}
void modbus_slave_thread_entry(void* parameter)
{
		extern sys_reg_st		g_sys; 
		eMBErrorCode    eStatus = MB_ENOERR;
	
		rt_thread_delay(MODBUS_SLAVE_THREAD_DELAY);
		com_change_inst.baudrate  =  g_sys.config.general.surv_baudrate;
		com_change_inst.com_addr = g_sys.config.general.surv_addr;
		eStatus = eMBInit(MB_RTU,(UCHAR)g_sys.config.general.surv_addr, UPORT_MONITOR, Get_Baudrate(com_change_inst.baudrate),  MB_PAR_NONE);
		if(eStatus != MB_ENOERR)
		{
				rt_kprintf("MB_SLAVE init failed\n");
		}
		eStatus = eMBEnable();			
		if(eStatus != MB_ENOERR)
		{
				rt_kprintf("MB_SLAVE enable failed\n");	
		}
		while(1)
		{
				eStatus = eMBPoll();
				if(eStatus != MB_ENOERR)
				{
						rt_kprintf("MB_SLAVE enable failed\n");	
				}	
				mbs_sts_update();	
				change_surv_baudrate();
				rt_thread_delay(10);
		}
}

//******************************保持寄存器回调函数**********************************
//函数定义: eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
//描    述：保持寄存器相关的功能（读、连续读、写、连续写）
//入口参数：pucRegBuffer : 如果需要更新用户寄存器数值，这个缓冲区必须指向新的寄存器数值。
//                         如果协议栈想知道当前的数值，回调函数必须将当前值写入这个缓冲区
//			usAddress    : 寄存器的起始地址。
//			usNRegs      : 寄存器数量
//          eMode        : 如果该参数为eMBRegisterMode::MB_REG_WRITE，用户的应用数值将从pucRegBuffer中得到更新。
//                         如果该参数为eMBRegisterMode::MB_REG_READ，用户需要将当前的应用数据存储在pucRegBuffer中
//出口参数：eMBErrorCode : 这个函数将返回的错误码
//备    注：Editor：Armink 2010-10-31    Company: BXXJS
//**********************************************************************************
eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
	extern sys_reg_st  g_sys; 
	eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegHoldingBuf;
    USHORT          REG_HOLDING_START;
    USHORT          REG_HOLDING_NREGS;
    USHORT          usRegHoldStart;

	extern  conf_reg_map_st conf_reg_map_inst[];
	uint16              Writ_Value;
	const reg_table_st *           pt= modebus_slave_reg_table;
  const mbm_reg_table_st*        mem_pt = mbm_reg_table;
	


	pusRegHoldingBuf = usSRegHoldBuf;
	REG_HOLDING_START = S_REG_HOLDING_START;
	REG_HOLDING_NREGS = S_REG_HOLDING_NREGS;
	usRegHoldStart = usSRegHoldStart;
	usAddress--;//FreeModbus功能函数中已经加1，为保证与缓冲区首地址一致，故减1
    if( ( usAddress >= REG_HOLDING_START ) &&
        ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = usAddress - usRegHoldStart;
        switch ( eMode )
        {
            /* Pass current register values to the protocol stack. */
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
								*pucRegBuffer++ = ( unsigned char )( pusRegHoldingBuf[iRegIndex] >> 8 );
							  *pucRegBuffer++ = ( unsigned char )( pusRegHoldingBuf[iRegIndex] & 0xFF );
                iRegIndex++;
                usNRegs--;
            }
            break;

            /* Update current register values with new values from the
             * protocol stack. */
        case MB_REG_WRITE:
					//forbid modbuss option power switch
						if((iRegIndex == 0)&&(g_sys.config.general.power_mode_mb_en ==0))
						{
								eStatus = MB_ENOREG;
								break;//	case MB_REG_WRITE:
						}
            while( usNRegs > 0 )
            {
				
						//超出可写范围报错判断
								if ((pt[iRegIndex].reg_type == 0)||(pt[iRegIndex].reg_type == 3))
								{
									//写入植边界判断
										Writ_Value=0;
										Writ_Value+=(*pucRegBuffer) << 8;
										Writ_Value+=*(pucRegBuffer+1);
										//max min
										
										if((Writ_Value<=conf_reg_map_inst[pt[iRegIndex].reg_addr].max)&&(Writ_Value>=conf_reg_map_inst[pt[iRegIndex].reg_addr].min)&& 
											(pt[iRegIndex].reg_type == 0))
										{
										
												//写入保持寄存器中同时跟新到内存和flash保存
												// 写入寄存器和EEPROM中。
												
												if(reg_map_write(conf_reg_map_inst[pt[iRegIndex].reg_addr].id,&Writ_Value,1,USER_MODEBUS_SLAVE)
													==CPAD_ERR_NOERR)
													{
																pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
																pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
																iRegIndex++;
																usNRegs--;	

													}
													else
													{
														
														eStatus = MB_ENORES;
														break;//	 while( usNRegs > 0 )
													}
											}
											else if((pt[iRegIndex].reg_type == 3)&&(g_sys.config.general.cancel_alarm_mb_en == 1))
											{
														*mem_pt[pt[iRegIndex].reg_addr].reg_ptr = Writ_Value;
														pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
														pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
														iRegIndex++;
														usNRegs--;
												
											}
											else
											{
												
												eStatus = MB_EINVAL;
												break;// while( usNRegs > 0 )
											}
									
									
								}
								else
								{
										
									eStatus = MB_ENOREG;
										break;//  while( usNRegs > 0 )
								}
					
						
						
				}
			break;
        }
    }
    else
    {
    
        eStatus = MB_ENOREG;
    }
    return eStatus;
}



void mbs_sts_update(void)// 更新本地变量到协议栈寄存器中
{
		char i=0;
		const reg_table_st *           pt = modebus_slave_reg_table;
		extern	conf_reg_map_st conf_reg_map_inst[];
	  extern  sts_reg_map_st status_reg_map_inst[];
		extern sys_reg_st  g_sys; 
	
		for(i=0;i<S_REG_HOLDING_NREGS;i++)
		{
				if(pt[i].reg_type == 0)
				{
						usSRegHoldBuf[i]=*(conf_reg_map_inst[pt[i].reg_addr].reg_ptr);
				}
				else
				if(pt[i].reg_type == 3)
				{
						usSRegHoldBuf[i]=mbm_cmd_reg;
				}
				else
				{
						usSRegHoldBuf[i]=*(status_reg_map_inst[pt[i].reg_addr].reg_ptr);
				}
		} 
		
		if(mbm_cmd_reg == 0x01)
		{
        clear_alarm(0xFF);
				mbm_cmd_reg =0;
		}
//	rt_kprintf("mbm_cmd_reg= %d,&mbm_cmd_reg= %d\n",mbm_cmd_reg,&mbm_cmd_reg );		
	
}

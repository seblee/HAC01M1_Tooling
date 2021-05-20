/*
 * FreeModbus Libary: user callback functions and buffer define in master mode
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
 * File: $Id: user_mb_app_m.c,v 1.60 2013/11/23 11:49:05 Armink $
 */
#include "user_mb_app.h"
#include "fifo.h"
#include "global_var.h"
#include "reg_map_check.h"
#include "sys_status.h"
#include "local_status.h"

#define  MBM_FIFO_DEPTH 20
fifo8_cb_td  mbm_data_fifo;

#define MBM_RESPONSE_DELAY	100
#define	MBM_QUEST_DELAY			100
#define	MBM_ERR_NUM			5
extern sys_reg_st					g_sys; 																	//global parameter declairation   

#define MBM_DISCRETE_DEV_CNT 2
/***************************************************EX_FAN**************************************/	
#define EX_FAN_READ_CNT    2
#define EX_FAN_WRITE_CNT   1 
mbm_read_dev_reg EX_FAN_read_reg [EX_FAN_READ_CNT] = {
																							{0x1000,0x0E,&g_sys.status.mbm.EX_FAN[0].Fre,0x00},
																							{0x1037,0x02,&g_sys.status.mbm.EX_FAN[0].Set_Speed_Mode,0x10},
                                              };
                                         
mbm_write_dev_reg EX_FAN_write_reg [EX_FAN_WRITE_CNT] ={ 
	                         {0x1037,414,&g_sys.config.mbm_Conf.EX_FAN[0].Set_Speed_Mode.Data,&g_sys.config.mbm_Conf.EX_FAN[0].Set_Speed_Mode.Flag,&g_sys.status.mbm.EX_FAN[0].Set_Speed_Mode},	
//	                         {0x1038,415,&g_sys.config.mbm_Conf.EX_FAN[0].Set_Comm_Speed.Data,&g_sys.config.mbm_Conf.EX_FAN[0].Set_Comm_Speed.Flag,&g_sys.status.mbm.EX_FAN[0].Set_Comm_Speed},	
                                                 };	
///*****************************************************EX_FAN2************************************/	
//#define EX_FAN2_READ_CNT    2
//#define EX_FAN2_WRITE_CNT   1 
//mbm_read_dev_reg EX_FAN2_read_reg [EX_FAN2_READ_CNT] = {
//																	{0x1000,0x0E,&g_sys.status.mbm.EX_FAN[1].Fre,0x00},
//																	{0x1037,0x02,&g_sys.status.mbm.EX_FAN[1].Set_Speed_Mode,0x10},
//                                              }; 
//mbm_write_dev_reg EX_FAN2_write_reg [EX_FAN2_WRITE_CNT] ={ 
//														{0x1037,416,&g_sys.config.mbm_Conf.EX_FAN[1].Set_Speed_Mode.Data,&g_sys.config.mbm_Conf.EX_FAN[1].Set_Speed_Mode.Flag,&g_sys.status.mbm.EX_FAN[1].Set_Speed_Mode},	
////	                         {0x1038,417,&g_sys.config.mbm_Conf.EX_FAN[1].Set_Comm_Speed.Data,&g_sys.config.mbm_Conf.EX_FAN[1].Set_Comm_Speed.Flag,&g_sys.status.mbm.EX_FAN[1].Set_Comm_Speed},	
//                                                 };	
// /*************************************************INV ************************************/
//#define INV_READ_CNT    2
//#define INV_WRITE_CNT   0 
//mbm_read_dev_reg INV_read_reg [INV_READ_CNT] = {
//															{0x1000,0x0D,&g_sys.status.mbm.INV.Comm,0x00},		
//															{0x8000,0x02,&g_sys.status.mbm.INV.Inv_Err,0x10},	
//                                              };
                                         
//mbm_write_dev_reg INV_write_reg [INV_WRITE_CNT] ={ 
//	                                                 {0x0000,&g_sys.config.mbm_Conf.INV.Set_Frequency,&g_sys.status.mbm.IPM.Addr}	
//                                                 };
/****************************************************EEV*************************************/	
#define EEV_READ_CNT    2
#define EEV_WRITE_CNT   9 
mbm_read_dev_reg EEV_read_reg [EEV_READ_CNT] = {
												{0x78,0x18,&g_sys.status.mbm.EEV.run_mode,0x00},  //offset = 110
												{0x13,0x0B,&g_sys.status.mbm.EEV.Set_Dehumidity_Supperheart,0x18}
                                              };                                 
mbm_write_dev_reg EEV_write_reg [EEV_WRITE_CNT] ={ 
	                         {21,465,&g_sys.config.mbm_Conf.EEV.ntc_cali[0].Data,&g_sys.config.mbm_Conf.EEV.ntc_cali[0].Flag,&g_sys.status.mbm.EEV.Set_ntc_cali[0]},	
													 {22,466,&g_sys.config.mbm_Conf.EEV.ntc_cali[1].Data,&g_sys.config.mbm_Conf.EEV.ntc_cali[1].Flag,&g_sys.status.mbm.EEV.Set_ntc_cali[1]},	
													 {23,467,&g_sys.config.mbm_Conf.EEV.ai_cali[0].Data,&g_sys.config.mbm_Conf.EEV.ai_cali[0].Flag,&g_sys.status.mbm.EEV.Set_ai_cali[0]},	
													 {24,468,&g_sys.config.mbm_Conf.EEV.ai_cali[1].Data,&g_sys.config.mbm_Conf.EEV.ai_cali[1].Flag,&g_sys.status.mbm.EEV.Set_ai_cali[1]},
													 {25,469,&g_sys.config.mbm_Conf.EEV.Set_Superheat[0].Data,&g_sys.config.mbm_Conf.EEV.Set_Superheat[0].Flag,&g_sys.status.mbm.EEV.Set_Superheat[0]},	
													 {26,470,&g_sys.config.mbm_Conf.EEV.Set_Superheat[1].Data,&g_sys.config.mbm_Conf.EEV.Set_Superheat[1].Flag,&g_sys.status.mbm.EEV.Set_Superheat[1]},	
													 {27,471,&g_sys.config.mbm_Conf.EEV.Set_Manual_Mode.Data,&g_sys.config.mbm_Conf.EEV.Set_Manual_Mode.Flag,&g_sys.status.mbm.EEV.Set_Manual_Mode},
													 {28,472,&g_sys.config.mbm_Conf.EEV.Set_Manual_Steps[0].Data,&g_sys.config.mbm_Conf.EEV.Set_Manual_Steps[0].Flag,&g_sys.status.mbm.EEV.Set_Manual_Steps[0]},	
													 {29,473,&g_sys.config.mbm_Conf.EEV.Set_Manual_Steps[1].Data,&g_sys.config.mbm_Conf.EEV.Set_Manual_Steps[1].Flag,&g_sys.status.mbm.EEV.Set_Manual_Steps[1]},	
                                                 };	
													
const mbm_read_st mbm_read_table[MBM_DISCRETE_DEV_CNT]=
{
		// mbmaddr    ,                   read_reg_cnt         read_pt        write_reg_cnt      write_pt
		{  MBM_DEV_EX_FAN_ADDR,          EX_FAN_READ_CNT,     EX_FAN_read_reg,  EX_FAN_WRITE_CNT,     EX_FAN_write_reg },//EX_FAN;
		{  MBM_DEV_EEV_ADDR,             EEV_READ_CNT,        EEV_read_reg,     EEV_WRITE_CNT,     EEV_write_reg },//EEV;
//		{  MBM_DEV_EX_FAN2_ADDR,         EX_FAN2_READ_CNT,    EX_FAN2_read_reg,  EX_FAN2_WRITE_CNT,     EX_FAN2_write_reg },//EX_FAN2;
//		{  MBM_DEV_INV_ADDR,             INV_READ_CNT,        INV_read_reg,    	INV_WRITE_CNT,    NULL },//INV;
};








static uint16_t mbm_dev_poll(uint16_t mb_comp_mask,uint16_t des_bitmap, mbm_dev_st* mbm_dev_inst);
static uint16_t mbm_dev_init(mbm_dev_st* mbm_dev_inst);
static uint16_t mbm_reg_update(mbm_dev_st* mbm_dev_inst);
static void mbm_fsm_init(mbm_dev_st* mbm_dev_inst);
static void mbm_fsm_update(sys_reg_st*	gds_ptr,mbm_dev_st* mbm_dev_inst);


/*-----------------------Master mode use these variables----------------------*/
//Master mode:HoldingRegister variables
static uint16_t   usMRegHoldStart                         = M_REG_HOLDING_START;
static uint16_t   usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS];
static mbm_dev_st mbm_dev_inst;


/**
  * @brief  modbus master poll thread
  * @param  none
  * @retval none
**/

//  mbm_send _data 


static void mbm_fifo_init(void)
{
		uint8_t block_size;
	
		block_size=sizeof(mbm_data_st);
		fifo8_init(&mbm_data_fifo, block_size,MBM_FIFO_DEPTH);		
}

//
//mbm master_send_data
void mbm_write_tset(uint8_t mbm_addr,uint8_t reg_addr,uint8_t reg_value)
{
		extern fifo8_cb_td  mbm_data_fifo;
		mbm_data_st mbm_send_data;
	
	
	
		mbm_send_data.mbm_addr = mbm_addr;
		mbm_send_data.reg_addr =reg_addr;
		mbm_send_data.reg_value = reg_value;
		
	
		
		//压入FIFO;
		if(fifo8_push(&mbm_data_fifo,(uint8_t*)&mbm_send_data) == 0)
		{
				rt_kprintf("mbm_data_fifo  ERRO\n");
		}
	
}
void modbus_master_thread_entry(void* parameter)
{
		extern local_reg_st		l_sys;
		eMBErrorCode    eStatus = MB_ENOERR;
		rt_thread_delay(MODBUS_MASTER_THREAD_DELAY);

		eStatus = eMBMasterInit(MB_RTU,UPORT_MONITOR,9600,MB_PAR_NONE);
		if(eStatus != MB_ENOERR)
		{
				rt_kprintf("MBM init fail\n");
		}
		eStatus = eMBMasterEnable();
		if(eStatus != MB_ENOERR)
		{
				rt_kprintf("MBM enable fail\n");
		}
		//init mbm_fifo
		mbm_fifo_init();
		while(1)
		{ 
				l_sys.u16Uart_Timeout++;
				if(l_sys.u16Uart_Timeout>=30)
				{
						l_sys.u16Uart_Timeout=0;
						eStatus = eMBMasterInit(MB_RTU,UPORT_MONITOR,9600,MB_PAR_NONE);
						if(eStatus != MB_ENOERR)
						{
								rt_kprintf("MBM init fail\n");
						}
						eStatus = eMBMasterEnable();
						if(eStatus != MB_ENOERR)
						{
								rt_kprintf("MBM enable fail\n");
						}
						//init mbm_fifo
						mbm_fifo_init();					
				}				
				eStatus = eMBMasterPoll();	
				if(eStatus != MB_ENOERR)
				{
						rt_kprintf("MBM poll err!\n");
				}
				rt_thread_delay(10);
		}
}

void mbm_fsm_thread_entry(void* parameter)
{	
		extern sys_reg_st	g_sys; 
		rt_thread_delay(MBM_FSM_THREAD_DELAY);
		mbm_fsm_init(&mbm_dev_inst);								//initialize local modbus master register set
//		AC_Conf_Write(0,1);
		while(1)
		{
				mbm_fsm_update(&g_sys,&mbm_dev_inst);		//update modbus slave components into local modbus master register
				rt_thread_delay(1000);
		}
}

//******************************保持寄存器回调函数**********************************
//函数定义: eMBErrorCode eMBMasterRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
//描    述：保持寄存器相关的功能（读、连续读、写、连续写）
//入口参数：pucRegBuffer : 如果需要更新用户寄存器数值，这个缓冲区必须指向新的寄存器数值。
//                         如果协议栈想知道当前的数值，回调函数必须将当前值写入这个缓冲区
//					usAddress    : 寄存器的起始地址。
//					usNRegs      : 寄存器数量
//          eMode        : 如果该参数为eMBRegisterMode::MB_REG_WRITE，用户的应用数值将从pucRegBuffer中得到更新。
//                         如果该参数为eMBRegisterMode::MB_REG_READ，用户需要将当前的应用数据存储在pucRegBuffer中
//出口参数：eMBErrorCode : 这个函数将返回的错误码
//备    注：Editor：Armink 2013-11-25    Company: BXXJS
//**********************************************************************************
eMBErrorCode
eMBMasterRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
		extern sys_reg_st					g_sys;
    eMBErrorCode    eStatus = MB_ENOERR;
    uint16_t          iRegIndex;
    uint16_t *        pusRegHoldingBuf;
    uint16_t          REG_HOLDING_START;
    uint16_t          REG_HOLDING_NREGS;
    uint16_t          usRegHoldStart;
		uint8_t           mbm_dest_addr,index;
		uint8_t           k;
		BOOL           		Find_Flag=FALSE;
		
	
	
		//If mode is read,the master will wirte the received date to bufffer.
		if(eMode == MB_REG_WRITE)
		{
				return(MB_ENOERR);
		}
		eMode = MB_REG_WRITE;		
		mbm_dest_addr = ucMBMasterGetDestAddress() - 1;

		pusRegHoldingBuf = usMRegHoldBuf[mbm_dest_addr];
		REG_HOLDING_START = M_REG_HOLDING_START;
		REG_HOLDING_NREGS = M_REG_HOLDING_NREGS;
		usRegHoldStart = usMRegHoldStart;
		index=0;
		usAddress--;//FreeModbus功能函数中已经加1，为保证与缓冲区首地址一致，故减1
    if( (( usAddress >= REG_HOLDING_START ) &&
        (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS))||
//		(mbm_dest_addr==MBM_DEV_EX_FAN_ADDR)||(mbm_dest_addr==MBM_DEV_EEV_ADDR)||(mbm_dest_addr==MBM_DEV_INV_ADDR))
		(mbm_dest_addr==MBM_DEV_EX_FAN_ADDR)||(mbm_dest_addr==MBM_DEV_EEV_ADDR))
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

//								if((mbm_dest_addr==MBM_DEV_EX_FAN_ADDR) || (mbm_dest_addr==MBM_DEV_EEV_ADDR)||(mbm_dest_addr==MBM_DEV_INV_ADDR))    //地址数据更新
								if((mbm_dest_addr==MBM_DEV_EX_FAN_ADDR) || (mbm_dest_addr==MBM_DEV_EEV_ADDR))    //地址数据更新
								{
										for(k=0;k<mbm_read_table[mbm_dest_addr - MB_MASTER_DISCRETE_OFFSET].reg_rd_cnt;k++)
										{
												if((usAddress==mbm_read_table[mbm_dest_addr - MB_MASTER_DISCRETE_OFFSET].rd_pt[k].dev_reg_addr)
													&&(usNRegs==mbm_read_table[mbm_dest_addr - MB_MASTER_DISCRETE_OFFSET].rd_pt[k].NRegs))	
												{
														index	=k;	
														Find_Flag=TRUE;
												}
										}
//										rt_kprintf("usAddress=%d,usNRegs=%d,index=%d\n",usAddress,usNRegs,index);	
										if((index < mbm_read_table[mbm_dest_addr - MB_MASTER_DISCRETE_OFFSET].reg_rd_cnt)&&(Find_Flag==TRUE))
										{		
												iRegIndex	=mbm_read_table[mbm_dest_addr - MB_MASTER_DISCRETE_OFFSET].rd_pt[index].J_Offset_Addr;								
												while( usNRegs > 0 )
												{
														pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
														pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
														iRegIndex++;
														usNRegs--;
												}														
										}
										else
										{
												rt_kprintf("mbm_read_table[mbm_dest_addr - MB_MASTER_DISCRETE_OFFSET].reg_rd_cn_ERRO\n");
										}								
								}
								/*
								else
								if((mbm_dest_addr==MBM_DEV_EEV_ADDR)||(mbm_dest_addr==MBM_DEV_INV_ADDR))//地址数据更新
								{
										if(index < mbm_read_table[mbm_dest_addr - MB_MASTER_DISCRETE_OFFSET].reg_rd_cnt)
										{		
												iRegIndex	=mbm_read_table[mbm_dest_addr - MB_MASTER_DISCRETE_OFFSET].rd_pt[index].J_Offset_Addr;								
												while( usNRegs > 0 )
												{
														pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
														pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
														iRegIndex++;
														usNRegs--;
												}														
										}
										else
										{
												rt_kprintf("mbm_read_table[mbm_dest_addr - MB_MASTER_DISCRETE_OFFSET].reg_rd_cn_ERRO\n");
										}								
								}
								*/
								else//地址1-8数据更新
								{
										while( usNRegs > 0 )
										{
												pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
												pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
												iRegIndex++;
												usNRegs--;
										}												
								}				
            break;
				default:
						break;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/**
  * @brief  poll modbus master slave device, if exist, set dev_mask bitmap accordingly
  * @param  
			@mb_comp_mask: system modbus slave device bitmap configuration.
			@mbm_dev_inst: modbus master device data struct.
  * @retval 
			@arg 1: all device online
			@arg 0: not all device online
  */
static uint16_t mbm_dev_poll(uint16_t mb_comp_mask,uint16_t des_bitmap, mbm_dev_st* mbm_dev_inst)
{
		eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
		uint16_t 	i;
		uint16_t	dev_poll_bitmap_reg;
		

//		dev_poll_bitmap_reg = mbm_dev_inst->bitmap.poll;
		
		dev_poll_bitmap_reg = 0;

//		for(i=0;i<MB_MASTER_TOTAL_SLAVE_NUM;i++)
		for(i=MB_DEV_TH_ADDR_START;i<MBM_DEV_P_ADDR;i++)
		{
			if((((mb_comp_mask>>i)&0x0001) == 1))
			{
				errorCode = eMBMasterReqReadHoldingRegister((i+1),0,1,MBM_RESPONSE_DELAY);
				if(errorCode == MB_MRE_NO_ERR)
				{
						dev_poll_bitmap_reg |= (0x0001<<i);		//set online flag
				}
				rt_thread_delay(MBM_QUEST_DELAY);
			}	
		}

		mbm_dev_inst->bitmap.poll = dev_poll_bitmap_reg;
		if(dev_poll_bitmap_reg == mb_comp_mask)
		{
				mbm_dev_inst->timeout.poll = 0;
				return 1;
		}
		else
		{
//				mbm_dev_inst->errcnt.poll++;
//				mbm_dev_inst->timeout.poll++;						
				return 0;
		}
}

/**
  * @brief  initialize modbus master slave device registers
  * @param  
			@mbm_dev_inst: modbus master device data struct.
  * @retval 
			@arg 1: all device online
			@arg 0: not all device online
  */
static uint16_t mbm_dev_init(mbm_dev_st* mbm_dev_inst)
{
		eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
		uint16_t dev_poll_bitmap_reg;
		uint16_t dev_init_bitmap_reg;
		uint16_t dev_reg_cnt;
//		uint16_t i,j,des_bitmap;		
		uint16_t i;	
	
		dev_poll_bitmap_reg = mbm_dev_inst->bitmap.poll;
		dev_init_bitmap_reg = mbm_dev_inst->bitmap.init;//mbm_dev_inst->bitmap.discrete
//		for(i=0;i<MB_MASTER_TOTAL_SLAVE_NUM;i++)	
			for(i=MB_DEV_TH_ADDR_START;i<MBM_DEV_P_ADDR;i++)
			{
					if((((dev_init_bitmap_reg>>i)&0x0001) == 1))
					{
							dev_reg_cnt = usMRegHoldBuf[i][MBM_REG_ADDR_CNT_ADDR];
							errorCode = eMBMasterReqReadHoldingRegister((i+1),0,dev_reg_cnt,MBM_RESPONSE_DELAY);
							if(errorCode == MB_MRE_NO_ERR)
							{
									dev_init_bitmap_reg |= (0x0001<<i);
							}
							rt_thread_delay(MBM_QUEST_DELAY);
					}	
					
			}
			mbm_dev_inst->bitmap.init = dev_init_bitmap_reg;
			if(dev_init_bitmap_reg == dev_poll_bitmap_reg)
			{
					mbm_dev_inst->timeout.init = 0;
					return 1;
			}
			else
			{
					mbm_dev_inst->errcnt.init++;
					mbm_dev_inst->timeout.init++;
					if(mbm_dev_inst->timeout.init >= MBM_INIT_TIMEOUT_THRESHOLD)
					{
							mbm_dev_inst->bitmap.poll = dev_init_bitmap_reg;
					}					
					return 0;
			}
}

/**
  * @brief  update local modbus master register map with only variable device reg values
  * @param  
			@mbm_dev_inst: modbus master device data struct.
  * @retval 
			@arg 1: all device online
			@arg 0: not all device online
  */
static uint16_t mbm_reg_update(mbm_dev_st* mbm_dev_inst)
{
			extern sys_reg_st	g_sys; 
		eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
		uint16_t dev_init_bitmap_reg;
		uint16_t dev_update_bitmap_reg;
		uint16_t dev_reg_cnt;
		uint16_t i,j;
		uint16_t u16mb_comp;
		uint16_t u16mb_Start;
		extern local_reg_st l_sys;	
//		dev_init_bitmap_reg = mbm_dev_inst->bitmap.init;
		//区分HAC01S1/HAC01S2
		if(Get_TH()==0)//使用HAC01S1
		{
			u16mb_comp=	g_sys.config.dev_mask.mb_comp&0x7FFF;
		}
		else
		{
			u16mb_comp=	g_sys.config.dev_mask.mb_comp&0x7FFC;			
		}
		dev_init_bitmap_reg = u16mb_comp;
		dev_update_bitmap_reg = 0;		
		
		for(i=MB_DEV_TH_ADDR_START;i<MB_MASTER_TOTAL_SLAVE_NUM;i++)
		{
				if(((dev_init_bitmap_reg>>i)&0x0001) == 1)
				{
						if((i==MBM_DEV_EX_FAN_ADDR)||(i==MBM_DEV_EEV_ADDR))
						{						
							 for(j=0;j<mbm_read_table[i-MB_MASTER_DISCRETE_OFFSET].reg_rd_cnt;j++)
							 {
									//	rt_thread_delay(50);
//											errorCode = eMBMasterReqReadHoldingRegister((i+1),mbm_read_table[i-MB_MASTER_DISCRETE_OFFSET].rd_pt[j].dev_reg_addr,1,MBM_RESPONSE_DELAY);
										errorCode = eMBMasterReqReadHoldingRegister((i+1),mbm_read_table[i-MB_MASTER_DISCRETE_OFFSET].rd_pt[j].dev_reg_addr,
										mbm_read_table[i-MB_MASTER_DISCRETE_OFFSET].rd_pt[j].NRegs,MBM_RESPONSE_DELAY*2);
									if(errorCode == MB_MRE_NO_ERR)
									{
											dev_update_bitmap_reg |= (0x0001<<i);
											g_sys.status.mbm.Err_M.Err_Master0[i]=0;
									}
									else
									{																			
										g_sys.status.mbm.Err_M.Err_Master0[i]++;
									}
										rt_thread_delay(MBM_QUEST_DELAY*2);
							 }						 
						}
						else
						{
//								dev_reg_cnt = usMRegHoldBuf[i][MBM_REG_ADDR_CNT_ADDR] - M_REG_HOLDING_USR_START;
								if(g_sys.status.ICT.u16Status==0)
								{
									return 0;
								}
								if((i==MBM_DEV_P_ADDR)||(i==MBM_DEV_PT_ADDR))
								{
									if(g_sys.config.ac_power_supply.PH_Ver==0)
									{			
										dev_reg_cnt =9;		
										l_sys.u8ICT_Cnt[0]++;										
									}
									else
									{
										dev_reg_cnt =5;													
									}	
									u16mb_Start	=	M_REG_HOLDING_USR_START;							
								}
								else
//								if(i==MBM_DEV_H_ADDR)
//								{
//									dev_reg_cnt =4;									
//								}
								if(i==MB_DEV_AEC01F1_ADDR)
								{
									dev_reg_cnt =9;		
									u16mb_Start	=	10;												
								}
								else
								{
									dev_reg_cnt =3;
									u16mb_Start	=	M_REG_HOLDING_USR_START;			
								}
								errorCode = eMBMasterReqReadHoldingRegister((i+1),u16mb_Start,dev_reg_cnt,MBM_RESPONSE_DELAY);
								if(errorCode == MB_MRE_NO_ERR)
								{
										dev_update_bitmap_reg |= (0x0001<<i);
										g_sys.status.mbm.Err_M.Err_Master0[i]=0;
								}
								else
								{
									g_sys.status.mbm.Err_M.Err_Master0[i]++;
								}	
								rt_thread_delay(MBM_QUEST_DELAY);							
						}

				}				
		}		
		mbm_dev_inst->bitmap.update = dev_update_bitmap_reg;
		if(dev_update_bitmap_reg == mbm_dev_inst->bitmap.init)
		{
				mbm_dev_inst->timeout.update = 0;
				mbm_dev_inst->errcnt.update = 0;
				return 1;
		}
		else
		{
				mbm_dev_inst->timeout.update++;
				mbm_dev_inst->errcnt.update++;
				if(mbm_dev_inst->timeout.update >= MBM_UPDATE_TIMEOUT_THRESHOLD)
				{
						mbm_dev_inst->bitmap.init = dev_update_bitmap_reg;
				}
				return 0;
		}		
}


static void mbm_send_fun(mbm_data_st* send_data)
{
		eMBMasterReqErrCode    errorCode = MB_MRE_NO_ERR;
		//Alair,20161226
		if(send_data->mbm_fun_code==MB_WRITE_MULITE)
		{
			  errorCode = eMBMasterReqWriteMultipleHoldingRegister( send_data->mbm_addr, send_data->reg_addr,send_data->mbm_NRegs,&(send_data->reg_value), MBM_RESPONSE_DELAY );
				if(errorCode != MB_MRE_NO_ERR)
				{
					rt_kprintf("mbm_send_fun erro= %d\n",errorCode);
				}			
		}
		else
		{
			  errorCode = eMBMasterReqWriteHoldingRegister( send_data->mbm_addr, send_data->reg_addr, send_data->reg_value, MBM_RESPONSE_DELAY );
				if(errorCode != MB_MRE_NO_ERR)
				{
					rt_kprintf("mbm_send_fun erro= %d\n",errorCode);
				}
		}
		
		rt_thread_delay(MBM_QUEST_DELAY);
		
}
/**
  * @brief  update local modbus master register map with only variable device reg values(ie. reg addr after 20)
  * @param  mbm_dev_inst: modbus master device data struct.
  * @retval none
  */
static void mbm_fsm_update(sys_reg_st*	gds_ptr,mbm_dev_st* mbm_dev_inst)
{
		uint16_t mbm_fsm_cstate;
		uint8_t i,len;
		mbm_data_st send_data;
		uint16_t u16COM_STS;
		uint16_t u16mb_comp;
	
		//区分HAC01S1/HAC01S2
		if(Get_TH()==0)//使用HAC01S1
		{
			u16mb_comp=	g_sys.config.dev_mask.mb_comp&0x7FFF;
		}
		else
		{
			u16mb_comp=	g_sys.config.dev_mask.mb_comp&0x7FFC;			
		}
	
		mbm_fsm_cstate = mbm_dev_inst->mbm_fsm;
//														rt_kprintf("mbm_fsm_cstate=%d\n",mbm_fsm_cstate);
		switch (mbm_fsm_cstate)
		{
				case (MBM_FSM_IDLE):
				{
						mbm_dev_poll(u16mb_comp,mbm_dev_inst->bitmap.discrete, mbm_dev_inst);
						mbm_dev_init(mbm_dev_inst);					
						mbm_dev_inst->mbm_fsm = MBM_FSM_SYNC;
//						mbm_dev_inst->mbm_fsm = MBM_FSM_UPDATE;
						break;
				}
				case (MBM_FSM_SYNC):
				{
						mbm_dev_poll(u16mb_comp,mbm_dev_inst->bitmap.discrete, mbm_dev_inst);
						mbm_dev_init(mbm_dev_inst);
						mbm_reg_update(mbm_dev_inst);
						mbm_dev_inst->Write_Enable =TRUE;
						if(((mbm_dev_inst->bitmap.update)^(u16mb_comp)) == 0)	//if init succeeded, go into update state, otherwise remain sync state
						{
								mbm_dev_inst->mbm_fsm = MBM_FSM_UPDATE;					
						}
						else if(is_fifo8_empty(&mbm_data_fifo)==0)
						{
								mbm_dev_inst->mbm_fsm = MBM_FSM_SEND;
						}
						else
						{
								mbm_dev_inst->mbm_fsm = MBM_FSM_SYNC;
						}
						break;
				}
				case (MBM_FSM_UPDATE):
				{
						mbm_reg_update(mbm_dev_inst);	
						mbm_dev_inst->Write_Enable =TRUE;					
						if((mbm_dev_inst->timeout.update >= MBM_UPDATE_TIMEOUT_THRESHOLD)||(u16mb_comp != mbm_dev_inst->bitmap.update))	//if update err count timeout, swich to sync state
						{
								mbm_dev_inst->bitmap.poll = mbm_dev_inst->bitmap.update;
								mbm_dev_inst->mbm_fsm = MBM_FSM_SYNC;
						}
						// go to  MBM_FSM_SEND 
						else 
						if(is_fifo8_empty(&mbm_data_fifo)==0)
						{
								mbm_dev_inst->mbm_fsm = MBM_FSM_SEND;
						}
						else
						{
								mbm_dev_inst->mbm_fsm = MBM_FSM_UPDATE;
						}
						break;
				}	
				case MBM_FSM_SEND:
				{
							len = get_fifo8_length(&mbm_data_fifo);
							for(i=0;i<len;i++)
							{
									if(fifo8_pop(&mbm_data_fifo,(uint8_t*)&send_data) == 1)
									{
											mbm_send_fun(&send_data);
									}
							}
							mbm_dev_inst->mbm_fsm = MBM_FSM_UPDATE;
						break;
				}
				default:
				{
						mbm_dev_inst->mbm_fsm = MBM_FSM_IDLE;
						break;
				}						
		}
		
		//通信不畅，不每次更新，偶尔冲突时数据不刷新
		if((mbm_dev_inst->errcnt.update==0)||(mbm_dev_inst->errcnt.update>=5)||(mbm_dev_inst->bitmap.update==u16mb_comp))
		{
//				gds_ptr->status.status_remap[MBM_COM_STS_REG_NO] = mbm_dev_inst->bitmap.update;		
				//更新状态MBM_COM_STS_REG_NO
				u16COM_STS=(gds_ptr->status.status_remap[MBM_COM_STS_REG_NO]&0x03);
				gds_ptr->status.status_remap[MBM_COM_STS_REG_NO] = mbm_dev_inst->bitmap.update;		
				gds_ptr->status.status_remap[MBM_COM_STS_REG_NO] |=u16COM_STS;			
		}
}

/**
  * @brief  modbus local data structure initialization
  * @param  mbm_dev_inst: modbus master device data struct.
  * @retval none
  */
static void mbm_fsm_init(mbm_dev_st* mbm_dev_inst)
{
		mbm_dev_inst->bitmap.poll = 0;
		mbm_dev_inst->bitmap.init = 0;
		mbm_dev_inst->bitmap.update = 0;
		mbm_dev_inst->timeout.poll = 0;
		mbm_dev_inst->timeout.init = 0;
		mbm_dev_inst->timeout.update = 0;
		mbm_dev_inst->errcnt.poll = 0;
		mbm_dev_inst->errcnt.init = 0;
		mbm_dev_inst->errcnt.update = 0;
		mbm_dev_inst->mbm_fsm = MBM_FSM_IDLE;
		mbm_dev_inst->Write_Enable =FALSE;
}

/**
  * @brief  modbus module interface, update global register with designated local modbus register values
  * @param  gds_ptr		global register struct pointer
  * @retval none
**/	
void mbm_sts_update(sys_reg_st* gds_ptr)
{
		extern local_reg_st l_sys;
		uint16_t mbm_dev_update_bitmap=0;
		uint16_t mbm_dev_init_bitmap;
		uint16_t i;//,j,k,h;
        //mbm_data_st mbm_send_data;
		uint16_t u16mb_comp;
		//static uint8_t	u8ComErr=0;
	
		//区分HAC01S1/HAC01S2
		if(Get_TH()==0)//使用HAC01S1
		{
			u16mb_comp=	g_sys.config.dev_mask.mb_comp&0x7FFF;
		}
		else
		{
			u16mb_comp=	g_sys.config.dev_mask.mb_comp&0x7FFC;			
		}
			
//		mbm_dev_init_bitmap = mbm_dev_inst.bitmap.init;
		mbm_dev_update_bitmap = mbm_dev_inst.bitmap.update;	//get modbus update bitmap which could be used to determin which glabal regsiter to update 
//		mbm_dev_init_bitmap = mbm_dev_inst.bitmap.init;	//get modbus init bitmap which could be used to determin which glabal regsiter to update 
//		mbm_dev_init_bitmap = (g_sys.config.dev_mask.mb_comp&0x7FFF);
		mbm_dev_init_bitmap = u16mb_comp;
//		rt_kprintf("mbm_dev_init_bitmap= %x,mbm_dev_update_bitmap= %x\n",mbm_dev_init_bitmap,mbm_dev_update_bitmap);		

//风机板AEC01F1
			if(((mbm_dev_init_bitmap >> MB_DEV_AEC01F1_ADDR)&0x0001) != 0)
			{
					if(((mbm_dev_update_bitmap >> MB_DEV_AEC01F1_ADDR)&0x0001) != 0)	//copy modbus master device register to global register, humidifier daq
					{	
							g_sys.status.ICT.u16Buff[0] = usMRegHoldBuf[MB_DEV_AEC01F1_ADDR][MB_CFG_FANOUT];
							g_sys.status.ICT.u16Buff[1] = usMRegHoldBuf[MB_DEV_AEC01F1_ADDR][MB_AO];
							g_sys.status.ICT.u16Buff[2] = usMRegHoldBuf[MB_DEV_AEC01F1_ADDR][MB_DI_Bitmap];
					}
					else	//if device is not initialized, all date reset to 0
					{
							if(g_sys.status.mbm.Err_M.Err_Master0[i]>5)
							{
								g_sys.status.ICT.u16Buff[0] = 0;
								g_sys.status.ICT.u16Buff[1] = 0;
								g_sys.status.ICT.u16Buff[2] = 0;
							}
					}
			}		
			if(g_sys.status.ICT.u16Status==0)
			{
				return ;
			}
		rt_kprintf("init_bitmap= %x,update_bitmap= %x,u16Buff[0]= %d,u16Buff[1]= %d,u16Buff[2]= %X\n",mbm_dev_init_bitmap,mbm_dev_update_bitmap,g_sys.status.ICT.u16Buff[0],g_sys.status.ICT.u16Buff[1],g_sys.status.ICT.u16Buff[2]);		
//		for(i=MB_DEV_TH_ADDR_START;i<=MB_DEV_TH_ADDR_END;i++)	//copy modbus master device register to global register, temp and hum sensor daq
//		{
//				if(((mbm_dev_init_bitmap >> i)&0x0001) != 0)
//				{
//						if(((mbm_dev_update_bitmap >> i)&0x0001) != 0)
//						{	
//								gds_ptr->status.mbm.tnh[i].dev_sts = usMRegHoldBuf[i][MBM_DEV_H_REG_HT_STATUS_ADDR];
//								gds_ptr->status.mbm.tnh[i].temp = usMRegHoldBuf[i][MBM_DEV_H_REG_TEMP_ADDR]+(int16_t)(gds_ptr->config.general.temp_sensor_cali[i].temp);
//								gds_ptr->status.mbm.tnh[i].hum = usMRegHoldBuf[i][MBM_DEV_H_REG_HUM_ADDR]+(int16_t)(gds_ptr->config.general.temp_sensor_cali[i].hum);
//						}
//						else //if device is not initialized, all date reset to 0
//						{
//								if(g_sys.status.mbm.Err_M.Err_Master0[i]>5)
//								{
//										gds_ptr->status.mbm.tnh[i].dev_sts = 0;
//										gds_ptr->status.mbm.tnh[i].temp = 0;
//										gds_ptr->status.mbm.tnh[i].hum = 0;
//								}
//						}
//					
//				}
//		
//		}
//		//加湿板	
//		if(g_sys.config.ac_power_supply.PH_Ver==0)
//		{
//				//加湿器电流
//				gds_ptr->status.mbm.hum.hum_current = gds_ptr->status.mbm.pwr.p_cur[0];
//				//加湿器水位
//				if(gds_ptr->status.mbm.pwr.p_cur[2]>g_sys.config.ac_power_supply.PH_Vol)
//				{
//					gds_ptr->status.mbm.hum.water_level=1;
//				}
//				else
//				{
//					gds_ptr->status.mbm.hum.water_level=0;					
//				}
//		}
//		else
//		{
//			if(((mbm_dev_init_bitmap >> MBM_DEV_H_ADDR)&0x0001) != 0)
//			{
//					if(((mbm_dev_update_bitmap >> MBM_DEV_H_ADDR)&0x0001) != 0)	//copy modbus master device register to global register, humidifier daq
//					{	
//							//有加湿电流和注水的情况下不更新
//							if((gds_ptr->status.dout_bitmap&(0x01<<DO_FILL_BPOS))&&(gds_ptr->status.dout_bitmap&(0x01<<DO_HUM_BPOS)))
//							{
//									;
//							}
//							else
//							{
//									
//									gds_ptr->status.mbm.hum.conductivity = usMRegHoldBuf[MBM_DEV_H_ADDR][MBM_DEV_H_REG_CONDUCT_ADDR];
//									
//							}
//							gds_ptr->status.mbm.hum.dev_sts = usMRegHoldBuf[MBM_DEV_H_ADDR][MBM_DEV_H_REG_STATUS_ADDR];
//							gds_ptr->status.mbm.hum.hum_current = usMRegHoldBuf[MBM_DEV_H_ADDR][MBM_DEV_H_REG_HUMCUR_ADDR];
//							gds_ptr->status.mbm.hum.water_level = usMRegHoldBuf[MBM_DEV_H_ADDR][MBM_DEV_H_REG_WT_LV_ADDR];
//					}
//					else	//if device is not initialized, all date reset to 0
//					{
//							gds_ptr->status.mbm.hum.dev_sts = 0;
//							gds_ptr->status.mbm.hum.conductivity = 0;
//							gds_ptr->status.mbm.hum.hum_current = 0;
//							gds_ptr->status.mbm.hum.water_level = 0;
//					}
//			}			
//		}
////		//加湿电流0.1A
////		gds_ptr->status.mbm.hum.u16HumCurrent = gds_ptr->status.mbm.hum.hum_current/100;
//		if(l_sys.u8ICT_Start==TRUE)
//		{
//			l_sys.u8ICT_Start=FALSE;
////			memset((uint8_t*)&usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS],0x00,sizeof(usMRegHoldBuf[MB_MASTER_TOTAL_SLAVE_NUM][M_REG_HOLDING_NREGS]));
//			memset((uint8_t*)&usMRegHoldBuf[MBM_DEV_P_ADDR][0],0x00,M_REG_HOLDING_NREGS*2);
//			g_sys.status.mbm.pwr[0].dev_sts = 0;
//			g_sys.status.mbm.pwr[0].pa_volt = 0;
//			g_sys.status.mbm.pwr[0].pb_volt =	0;
//			g_sys.status.mbm.pwr[0].pc_volt =	0;
//			g_sys.status.mbm.pwr[0].freq = 0;
//			g_sys.status.mbm.pwr[0].pe_bitmap =	0;
//			g_sys.status.mbm.pwr[0].p_cur[0] =	0;	
//			g_sys.status.mbm.pwr[0].p_cur[1] =	0;
//			g_sys.status.mbm.pwr[0].p_cur[2] =	0;	
//			
//			g_sys.status.mbm.pwr[1].dev_sts = 0;
//			g_sys.status.mbm.pwr[1].pa_volt = 0;
//			g_sys.status.mbm.pwr[1].pb_volt =	0;
//			g_sys.status.mbm.pwr[1].pc_volt =	0;
//			g_sys.status.mbm.pwr[1].freq = 0;
//			g_sys.status.mbm.pwr[1].pe_bitmap =	0;
//			g_sys.status.mbm.pwr[1].p_cur[0] =	0;	
//			g_sys.status.mbm.pwr[1].p_cur[1] =	0;
//			g_sys.status.mbm.pwr[1].p_cur[2] =	0;		
//		}
//		if(g_sys.status.ICT.u16Status==0)
//		{
//			return ;
//		}
////	电源板
//		if(((mbm_dev_init_bitmap >> MBM_DEV_PT_ADDR)&0x0001) != 0)
//		{		
//				if(((mbm_dev_update_bitmap >> MBM_DEV_PT_ADDR)&0x0001) != 0)	//copy modbus master device register to global register, power daq
//				{		
//						u8ComErr=0;
//						gds_ptr->status.mbm.pwr[0].dev_sts = usMRegHoldBuf[MBM_DEV_PT_ADDR][MBM_DEV_H_REG_P0WER_STATUS_ADDR];
//						gds_ptr->status.mbm.pwr[0].pa_volt = usMRegHoldBuf[MBM_DEV_PT_ADDR][MBM_DEV_H_REG_PA_VOLT_ADDR];
//						gds_ptr->status.mbm.pwr[0].pb_volt =	usMRegHoldBuf[MBM_DEV_PT_ADDR][MBM_DEV_H_REG_PB_VOLT_ADDR];
//						gds_ptr->status.mbm.pwr[0].pc_volt =	usMRegHoldBuf[MBM_DEV_PT_ADDR][MBM_DEV_H_REG_PC_VOLT_ADDR];
//						gds_ptr->status.mbm.pwr[0].freq = usMRegHoldBuf[MBM_DEV_PT_ADDR][MBM_DEV_H_REG_FREQ_ADDR];
//						gds_ptr->status.mbm.pwr[0].pe_bitmap =	usMRegHoldBuf[MBM_DEV_PT_ADDR][MBM_DEV_H_REG_PE_ADDR];	
//						gds_ptr->status.mbm.pwr[0].p_cur[0] =	usMRegHoldBuf[MBM_DEV_PT_ADDR][MBM_DEV_H_REG_PA_CUR_ADDR];	
//						gds_ptr->status.mbm.pwr[0].p_cur[1] =	usMRegHoldBuf[MBM_DEV_PT_ADDR][MBM_DEV_H_REG_PB_CUR_ADDR];	
//						gds_ptr->status.mbm.pwr[0].p_cur[2] =	usMRegHoldBuf[MBM_DEV_PT_ADDR][MBM_DEV_H_REG_PC_CUR_ADDR];	
//					//TEST
////						gds_ptr->status.mbm.pwr.dev_sts=0;
////						gds_ptr->status.mbm.pwr.pa_volt = 2300;
////						gds_ptr->status.mbm.pwr.pb_volt =	2300;
////						gds_ptr->status.mbm.pwr.pc_volt =	2300;
////						gds_ptr->status.mbm.pwr.freq = 499;
////						gds_ptr->status.mbm.pwr.pe_bitmap =	1;	
////						gds_ptr->status.mbm.pwr.p_cur[0] =	50;	
////						gds_ptr->status.mbm.pwr.p_cur[1] =	50;	
////						gds_ptr->status.mbm.pwr.p_cur[2] =	2300;						
//				}
//				else	//if device is not initialized, all date reset to 0
//				{
//						u8ComErr++;
//						if(u8ComErr>=10)
//						{
//							u8ComErr=10;
//							gds_ptr->status.mbm.pwr[0].dev_sts = 0;
//							gds_ptr->status.mbm.pwr[0].pa_volt = 0;
//							gds_ptr->status.mbm.pwr[0].pb_volt =	0;
//							gds_ptr->status.mbm.pwr[0].pc_volt =	0;
//							gds_ptr->status.mbm.pwr[0].freq = 0;
//							gds_ptr->status.mbm.pwr[0].pe_bitmap =	0;
//							gds_ptr->status.mbm.pwr[0].p_cur[0] =	0;	
//							gds_ptr->status.mbm.pwr[0].p_cur[1] =	0;
//							gds_ptr->status.mbm.pwr[0].p_cur[2] =	0;
//						}
//				}
//		}
//		rt_kprintf("dev_sts= %x,pa_volt= %d,p_cur[0]= %d,p_cur[1]= %d,p_cur[2]= %d,pe_bitmap= %d\n",gds_ptr->status.mbm.pwr[0].dev_sts,gds_ptr->status.mbm.pwr[0].pa_volt,
//		gds_ptr->status.mbm.pwr[0].p_cur[0],gds_ptr->status.mbm.pwr[0].p_cur[1],gds_ptr->status.mbm.pwr[0].p_cur[2],gds_ptr->status.mbm.pwr[0].pe_bitmap);		
////	电源板
//		if(((mbm_dev_init_bitmap >> MBM_DEV_P_ADDR)&0x0001) != 0)
//		{		
//				if(((mbm_dev_update_bitmap >> MBM_DEV_P_ADDR)&0x0001) != 0)	//copy modbus master device register to global register, power daq
//				{		
//						l_sys.u8ICT_Cnt[1]++;		
//						u8ComErr=0;
//						gds_ptr->status.mbm.pwr[1].dev_sts = usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_P0WER_STATUS_ADDR];
//						gds_ptr->status.mbm.pwr[1].pa_volt = usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_PA_VOLT_ADDR];
//						gds_ptr->status.mbm.pwr[1].pb_volt =	usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_PB_VOLT_ADDR];
//						gds_ptr->status.mbm.pwr[1].pc_volt =	usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_PC_VOLT_ADDR];
//						gds_ptr->status.mbm.pwr[1].freq = usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_FREQ_ADDR];
//						gds_ptr->status.mbm.pwr[1].pe_bitmap =	usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_PE_ADDR];	
//						gds_ptr->status.mbm.pwr[1].p_cur[0] =	usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_PA_CUR_ADDR];	
//						gds_ptr->status.mbm.pwr[1].p_cur[1] =	usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_PB_CUR_ADDR];	
//						gds_ptr->status.mbm.pwr[1].p_cur[2] =	usMRegHoldBuf[MBM_DEV_P_ADDR][MBM_DEV_H_REG_PC_CUR_ADDR];					
//				}
//				else	//if device is not initialized, all date reset to 0
//				{
//						u8ComErr++;
//						if(u8ComErr>=10)
//						{
//							u8ComErr=10;
//							gds_ptr->status.mbm.pwr[1].dev_sts = 0;
//							gds_ptr->status.mbm.pwr[1].pa_volt = 0;
//							gds_ptr->status.mbm.pwr[1].pb_volt =	0;
//							gds_ptr->status.mbm.pwr[1].pc_volt =	0;
//							gds_ptr->status.mbm.pwr[1].freq = 0;
//							gds_ptr->status.mbm.pwr[1].pe_bitmap =	0;
//							gds_ptr->status.mbm.pwr[1].p_cur[0] =	0;	
//							gds_ptr->status.mbm.pwr[1].p_cur[1] =	0;
//							gds_ptr->status.mbm.pwr[1].p_cur[2] =	0;
//						}
//				}
//		}
//		rt_kprintf("dev_sts1= %x,pa_volt1= %d,p_cur[0]1= %d,p_cur[1]1= %d,p_cur[2]1= %d,pe_bitmap= %d,u8ICT_Cnt[0]= %d,u8ICT_Cnt[1]= %d\n",gds_ptr->status.mbm.pwr[1].dev_sts,gds_ptr->status.mbm.pwr[1].pa_volt,
//		gds_ptr->status.mbm.pwr[1].p_cur[0],gds_ptr->status.mbm.pwr[1].p_cur[1],gds_ptr->status.mbm.pwr[1].p_cur[2],gds_ptr->status.mbm.pwr[1].pe_bitmap,l_sys.u8ICT_Cnt[0],l_sys.u8ICT_Cnt[1]);	
		
//					//  EX_FAN,EEV,INV　information update
//			for(j=0;j<MBM_DISCRETE_DEV_CNT; j++)//EX_FAN,EEV,INV特殊读取
//			{
//					if(((mbm_dev_init_bitmap >> mbm_read_table[j].mbm_addr)&0x0001) != 0)
//					{						
//							if(((mbm_dev_update_bitmap >> mbm_read_table[j].mbm_addr)&0x0001) != 0)	//copy modbus master device register to global register, power daq
//							{			
//									for(k=0;k < mbm_read_table[j].reg_rd_cnt;k++)
//									{									
//											for(h=0;h<mbm_read_table[j].rd_pt[k].NRegs;h++)
//											{
//													*(mbm_read_table[j].rd_pt[k].data_ptr+h) = usMRegHoldBuf[ mbm_read_table[j].mbm_addr][mbm_read_table[j].rd_pt[k].J_Offset_Addr+h];															
//											}												
//									}												
////									//test	
////												rt_kprintf("Fre=%d,M_Voltage=%d,Buf_Voltage=%d\n",g_sys.status.mbm.EX_FAN.Fre,g_sys.status.mbm.EX_FAN.M_Voltage,usMRegHoldBuf[ MBM_DEV_EX_FAN_ADDR][1]);																   					

//									if(mbm_read_table[j].reg_w_cnt != 0)
//									{
//										if(mbm_dev_inst.Write_Enable!=TRUE)
//										{
//												continue;
//										}
//										for(k = 0;k < mbm_read_table[j].reg_w_cnt;k++)
//										{
//												if(*(mbm_read_table[j].w_pt[k].conf_Flag) != FALSE)
//												{
////																							rt_kprintf("j=%d,reg_w_cnt=%d\n",j,mbm_read_table[j].reg_w_cnt);	
////													rt_kprintf("conf_ptr=%d,status_ptr=%d,dev_reg_addr=%d\n",*(mbm_read_table[j].w_pt[k].conf_ptr),*(mbm_read_table[j].w_pt[k].status_ptr),mbm_read_table[j].w_pt[k].dev_reg_addr);	
//														if(*(mbm_read_table[j].w_pt[k].conf_ptr) != *(mbm_read_table[j].w_pt[k].status_ptr))					
//														{
//															mbm_send_data.mbm_addr = mbm_read_table[j].mbm_addr + 1;
//															mbm_send_data.reg_addr = mbm_read_table[j].w_pt[k].dev_reg_addr;
//															mbm_send_data.reg_value= *(mbm_read_table[j].w_pt[k].conf_ptr);
//															mbm_send_data.mbm_NRegs = 1;
//															mbm_send_data.mbm_fun_code = MB_WRITE_SINGLE;
//															if(fifo8_push(&mbm_data_fifo,(uint8_t*)&mbm_send_data) == 0)
//															{
//																	rt_kprintf("\n mbm_read_table_01 ERRO\n");
//															}												
//														}
//														else
//														{
//															*(mbm_read_table[j].w_pt[k].conf_Flag) = 0;
//														}
//												}
//											
//										}
//									}
//							}
//						 else	//if device is not initialized, all date reset to 0
//						 {
//								for(k=0;k < mbm_read_table[j].reg_rd_cnt;k++)
//								{
////													*(mbm_read_table[j].rd_pt[k].data_ptr) = 0;	
//											for(h=0;h<mbm_read_table[j].rd_pt[k].NRegs;h++)
//											{
//													*(mbm_read_table[j].rd_pt[k].data_ptr+h) = 0;
//											}														
//								}
//						 }	
//				 }      
//		 }				
} 

 
 
   
         



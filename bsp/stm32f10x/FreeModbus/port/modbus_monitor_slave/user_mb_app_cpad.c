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
#include "mb_event_cpad.h"


#include "mbport_cpad.h"
#include "global_var.h"
#include "event_record.h"
/*------------------------Slave mode use these variables----------------------*/
//Slave mode:HoldingRegister variables
extern cpad_slave_st  cpad_slave_inst;
static uint16_t mbs_read_reg(uint16_t read_addr);

USHORT   cpad_usSRegHoldBuf[CMD_REG_SIZE]   ;
USHORT   cpad_usSRegHoldStart = CPAD_S_REG_HOLDING_START;


typedef struct 
{
    uint8_t reg_type ;
    //0=config_reg;1 =status_reg;
    uint16_t reg_addr ;
    uint8_t reg_w_r ;
    //3 =write&read,2=read_only,3=write_only
}
reg_table_st ;

//�ڴ浽modbus��ӳ���
//Ԫ��λ�ö�ӦModeBus  Э��ջ��usSRegHoldBufλ��
//Ԫ��ֵ��Ӧconf_reg_map_inst���ڴ����ݵ�λ�á�

void cpad_modbus_slave_thread_entry(void* parameter)
{
		extern sys_reg_st		g_sys; 
	
		rt_thread_delay(MODBUS_SLAVE_THREAD_DELAY);

	  cpad_MBRTUInit(2 , UPORT_CPAD, 19200,  MB_PAR_NONE);
		
		rt_kprintf("cpad_modbus_slave_thread_entry\n");
		while(1)
		{
				cpad_MBPoll();
				rt_thread_delay(10);
		}
}

uint8_t COM_SINGLE_eMBRegHoldingCB(uint16_t usAddress, uint16_t usValue)
{
//    extern local_reg_st l_sys;
    extern sys_reg_st g_sys;
    extern conf_reg_map_st conf_reg_map_inst[];
    eMBErrorCode eStatus = MB_ENOERR;
    uint16_t temp = 0;
    uint16_t u16RegAddr = usAddress;

    switch (u16RegAddr)
    {
    case FACTORY_RESET: //��������
    {
        temp = usValue;
        if (temp == 0x3C) //�ָ�ԭʼ����
        {
            reset_runtime(0xFF); //������������ʱ��
            set_load_flag(0x02);
            rt_thread_delay(1000);
            NVIC_SystemReset();
            return MB_ENOERR;
        }
        else if (temp == 0x5A) //�ָ���������
        {
            set_load_flag(0x01);
            rt_thread_delay(1000);
            NVIC_SystemReset();
            return MB_ENOERR;
        }
        else if (temp == 0x69) //�����������
        {
            save_conf_reg(0x01);
            rt_thread_delay(1000);
            NVIC_SystemReset();
            return MB_ENOERR;
        }
        else if (temp == 0x2D) //����
        {
//            Close_DIS_PWR(1);
            rt_thread_delay(1000);
            NVIC_SystemReset();
            return MB_ENOERR;
        }
        else if (temp == 0x1E) //������
        {
//            Close_DIS_PWR(1);
            rt_thread_delay(1000);
//            Close_DIS_PWR(0);
            return MB_ENOERR;
        }
				else
        if (temp == 0x15) //��������ʱ��
        {
            reset_runtime(0xFF); //������������ʱ��
            rt_thread_delay(100);
            return MB_ENOERR;
        }
        else
        {
            eStatus = MB_ENORES;
        }
    }
    break;
    case MANUAL_TSET: //����ģʽ
    {
        temp = usValue;
        if (temp == MANUAL_TEST_UNABLE)
        {
            rt_thread_delay(500);
            NVIC_SystemReset();
            return MB_ENOERR;
        }
        else
        {
            if (reg_map_write(conf_reg_map_inst[u16RegAddr].id, &usValue, 1, USER_CPAD) == CPAD_ERR_NOERR)
            {
                //										iRegIndex++;
                //										usNRegs--;
                eStatus = MB_ENOERR;
            }
            else
            {

                eStatus = MB_ENORES;
                //	 while( usNRegs > 0 )
            }
        }
    }
    break;
    case CLEAR_RT: //���㲿������ʱ��
    {
        temp = usValue;
        if (temp) //���㲿������ʱ��
        {
            reset_runtime(temp);
            return MB_ENOERR;
        }
        else
        {
            eStatus = MB_ENORES;
        }
    }
    break;
    case CLEAR_ALARM: //����澯
    {
        temp = usValue;
        if (temp == 0x5A) //���㲿������ʱ��
        {
            clear_alarm(0xFF);
            return MB_ENOERR;
        }
        else
        {
            eStatus = MB_ENORES;
        }
    }
    break;
//    case SET_TL: //ϵͳʱ���λ
//    {
//        temp = u16Value;
//        if (temp != NULL) //ϵͳʱ���λ
//        {
//            l_sys.Set_Systime_Delay = SETTIME_DELAY;
//            l_sys.Set_Systime_Flag |= 0x01;
//            g_sys.config.ComPara.u16Set_Time[0] = temp;
//            return MB_ENOERR;
//        }
//        else
//        {
//            eStatus = MB_ENORES;
//        }
//    }
//    break;
//    case SET_TH: //ϵͳʱ���λ
//    {
//        temp = u16Value;
//        if (temp != NULL) //ϵͳʱ���λ
//        {
//            l_sys.Set_Systime_Delay = SETTIME_DELAY;
//            l_sys.Set_Systime_Flag |= 0x02;
//            g_sys.config.ComPara.u16Set_Time[1] = temp;
//            return MB_ENOERR;
//        }
//        else
//        {
//            eStatus = MB_ENORES;
//        }
//    }
//    break;
    default:
    {
        if (reg_map_write(conf_reg_map_inst[u16RegAddr].id, &usValue, 1, USER_CPAD) == CPAD_ERR_NOERR)
        {
            //										iRegIndex++;
            //										usNRegs--;
            eStatus = MB_ENOERR;
        }
        else
        {

            eStatus = MB_ENORES;
        }
    }
    break;
    }
    return eStatus;
}

//******************************���ּĴ����ص�����**********************************
//��������: eMBErrorCode eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
//��    �������ּĴ�����صĹ��ܣ�������������д������д��
//��ڲ�����pucRegBuffer : �����Ҫ�����û��Ĵ�����ֵ���������������ָ���µļĴ�����ֵ��
//                         ���Э��ջ��֪����ǰ����ֵ���ص��������뽫��ǰֵд�����������
//			usAddress    : �Ĵ�������ʼ��ַ��
//			usNRegs      : �Ĵ�������
//          eMode        : ����ò���ΪeMBRegisterMode::MB_REG_WRITE���û���Ӧ����ֵ����pucRegBuffer�еõ����¡�
//                         ����ò���ΪeMBRegisterMode::MB_REG_READ���û���Ҫ����ǰ��Ӧ�����ݴ洢��pucRegBuffer��
//���ڲ�����eMBErrorCode : ������������صĴ�����
//��    ע��Editor��Armink 2010-10-31    Company: BXXJS
//**********************************************************************************

eMBErrorCode
cpad_eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, uint8_t eMode )
{
    extern sys_reg_st g_sys ;
    eMBErrorCode eStatus=MB_ENOERR ;
    USHORT iRegIndex ;
    USHORT*pusRegHoldingBuf ;
    USHORT REG_HOLDING_START ;
    USHORT REG_HOLDING_NREGS ;
    USHORT usRegHoldStart ;
    USHORT i ;
    extern conf_reg_map_st conf_reg_map_inst[];
    uint16 cmd_value ;
    uint16_t u16RegAddr = 0;	
	
    pusRegHoldingBuf=cpad_usSRegHoldBuf ;
    REG_HOLDING_START=CPAD_S_REG_HOLDING_START ;
    REG_HOLDING_NREGS=CPAD_S_REG_HOLDING_NREGS ;
    usRegHoldStart=cpad_usSRegHoldStart ;
    //usAddress--;//FreeModbus���ܺ������Ѿ���1��Ϊ��֤�뻺�����׵�ַһ�£��ʼ�1
    if((usAddress>=REG_HOLDING_START)&&
    (usAddress+usNRegs<=REG_HOLDING_START+REG_HOLDING_NREGS))
    {
        iRegIndex=usAddress-usRegHoldStart ;
        switch(eMode)
        {
            /* Pass current register values to the protocol stack. */
            case CPAD_MB_REG_READ :
            while(usNRegs>0)
            {
                cmd_value=mbs_read_reg(iRegIndex);
                *pucRegBuffer++=(unsigned char)(cmd_value>>8);
                *pucRegBuffer++=(unsigned char)(cmd_value&0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break ;
            
            /* Update current register values with new values from the
                         * protocol stack. */
            case CPAD_MB_REG_SINGLE_WRITE :
            //						//forbid modbuss option power switch
            //						if((iRegIndex == 0)&&(g_sys.config.general.power_mode_mb_en ==0))
            //						{
            //								eStatus = MB_ENOREG;
            //								break;//	case MB_REG_WRITE:
            //						}
            while(usNRegs>0)
            {
                
                //������д��Χ�����ж�
                if((usAddress+usNRegs)<=(REG_HOLDING_START+CPAD_REG_HOLDING_WRITE_NREGS))
                {
                    
                    if((usAddress+usNRegs)>=(REG_HOLDING_START+CONFIG_REG_MAP_OFFSET+1))
                    {
                        cmd_value=(*pucRegBuffer)<<8 ;
                        cmd_value+=*(pucRegBuffer+1);
                        //д�뱣�ּĴ�����ͬʱ���µ��ڴ��flash����
                        // д��Ĵ�����EEPROM�С�
                        
//                        if(reg_map_write(conf_reg_map_inst[iRegIndex-CONFIG_REG_MAP_OFFSET].id,&cmd_value,1,USER_CPAD)
//                        ==CPAD_ERR_NOERR)
//                        {
//                            iRegIndex++;
//                            usNRegs--;
//                        }
                        u16RegAddr = iRegIndex - CONFIG_REG_MAP_OFFSET;
                        if (COM_SINGLE_eMBRegHoldingCB(u16RegAddr, cmd_value) == MB_ENOERR)
                        {
                            usNRegs--;
                        }
                        else 
                        {
                            
                            eStatus=MB_ENORES ;
                            break ;
                            //	 while( usNRegs > 0 )
                        }
                    }
                    else 
                    {
                        pusRegHoldingBuf[iRegIndex]=*pucRegBuffer++<<8 ;
                        pusRegHoldingBuf[iRegIndex]|=*pucRegBuffer++;
                        iRegIndex++;
                        usNRegs--;
                    }
                }
                else 
                {
                    
                    eStatus=MB_ENOREG ;
                    break ;
                    //  while( usNRegs > 0 )
                }
                
            }
            break ;
            
            case CPAD_MB_REG_MULTIPLE_WRITE :
            //�ֲ�����֡���䣬һ֡�����16���Ĵ���
            if((usNRegs>0)&&(usNRegs<=16))
            {
                //������д��Χ�����ж�
                if((usAddress+usNRegs)<=(REG_HOLDING_START+CPAD_REG_HOLDING_WRITE_NREGS))
                {
                    if((usAddress+usNRegs)>=(REG_HOLDING_START+CONFIG_REG_MAP_OFFSET+1))
                    {
                        for(i=0;i<usNRegs;i++)
                        {
                            cmd_value=(*pucRegBuffer)<<8 ;
                            cmd_value+=*(pucRegBuffer+1);                           
                            *(conf_reg_map_inst[usAddress-CONFIG_REG_MAP_OFFSET+i].reg_ptr)=cmd_value ;						
                            pucRegBuffer+=2 ;
                        }
                        if(CONF_REG_MAP_NUM==(usAddress-CONFIG_REG_MAP_OFFSET+i))
                        {
                            rt_kprintf("modbus multiple write complete.\n");                            
                            save_conf_reg(0);           //д�뱣�ּĴ�����ͬʱ���µ��ڴ��flash����  // д��Ĵ�����EEPROM�С�
												}
                    }
                }
                else 
                {
                    
                    eStatus=MB_ENOREG ;
                    break ;
                }
                
            }
            break ;
        }
    }
    else 
    {
        eStatus=MB_ENOREG ;
    }
    return eStatus ;
}




static uint16_t mbs_read_reg(uint16_t read_addr)
{
    extern conf_reg_map_st conf_reg_map_inst[];
    extern sts_reg_map_st status_reg_map_inst[];
    if(read_addr<CMD_REG_SIZE)
    {
        return(cpad_usSRegHoldBuf[read_addr]);
    }
    else if((CMD_REG_SIZE<=read_addr)&&(read_addr<(CONF_REG_MAP_NUM+CMD_REG_SIZE)))
    {
        return(*(conf_reg_map_inst[read_addr-CMD_REG_SIZE].reg_ptr));
    }
    else if((STATUS_REG_MAP_OFFSET<=read_addr)&&(read_addr<(STATUS_REG_MAP_OFFSET+STATUS_REG_MAP_NUM)))
    {
        return(*(status_reg_map_inst[read_addr-STATUS_REG_MAP_OFFSET].reg_ptr));
    }
    else 
    {
        return(ABNORMAL_VALUE);
    }
}





























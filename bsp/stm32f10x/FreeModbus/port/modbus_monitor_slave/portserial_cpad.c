/*
 * FreeModbus Libary: STM32 Port
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
 * File: $Id: portserial.c,v 1.60 2013/08/13 15:07:05 Armink $
 */

#include "port.h"
#include <rtthread.h>
/* ----------------------- Modbus includes ----------------------------------*/
#include "fifo.h"
#include "mbport_cpad.h"

/* ----------------------- Start implementation -----------------------------*/
static fifo8_cb_td mnt_tx_fifo;

uint8_t Monitor_isr_flag;
extern cpad_slave_st  cpad_slave_inst;

void cpad_MBPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
		eMBParity eParity)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	USART_DeInit(USART2);
	//======================ʱ�ӳ�ʼ��=======================================
//	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD ,	ENABLE);
//	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_UART5,ENABLE);
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	//======================IO��ʼ��=========================================	
	//UART2_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//UART2_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//����485���ͺͽ���ģʽ
	//TODO   ��ʱ��дA1 ��֮����������ʱ���޸�
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//======================���ڳ�ʼ��=======================================
	USART_InitStructure.USART_BaudRate = ulBaudRate;
	//����У��ģʽ
	switch (eParity)
	{
	case MB_PAR_NONE: //��У��
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		break;
	case MB_PAR_ODD: //��У��
		USART_InitStructure.USART_Parity = USART_Parity_Odd;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		break;
	case MB_PAR_EVEN: //żУ��
		USART_InitStructure.USART_Parity = USART_Parity_Even;
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		break;
	default:
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		break;
	}

	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_HardwareFlowControl =	USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);
	
	


	

	//=====================�жϳ�ʼ��======================================
	//����NVIC���ȼ�����ΪGroup2��0-3��ռʽ���ȼ���0-3����Ӧʽ���ȼ�
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	CPAD_SLAVE_RS485_RECEIVE_MODE;
	fifo8_init(&mnt_tx_fifo,1,MNT_RX_LEN);

}




void cpad_xMBPortSerialPutByte(uint8_t *ucbyte, uint16_t len)
{
		uint8_t tx_data;
		uint16_t i;
	
		for(i=0;i<len;i++)
		{
			fifo8_push(&mnt_tx_fifo,ucbyte);
			ucbyte++;
		}
	//	rt_kprintf("MonitorxMBPortSerialPutByte\n");

		USART_ClearFlag(USART2, USART_FLAG_TC);
		CPAD_SLAVE_RS485_SEND_MODE;
		USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
		fifo8_pop(&mnt_tx_fifo,&tx_data);
		USART_SendData(USART2, tx_data);
		USART_ITConfig(USART2, USART_IT_TC, ENABLE);
		
}
/***************************************************************************
 * Function Name  : UART5_IRQHandler
 * Description    : This function handles UART5 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/


void USART2_IRQHandler(void)
{
	uint8_t rec_data,tx_data;
	rt_interrupt_enter();
	//�������
	if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) == SET)
	{
		rec_data = USART_ReceiveData(USART2);
		//USART_ClearFlag(UART5 USART_FLAG_ORE);
	}
	//�����ж�
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
			USART_ClearITPendingBit( USART2, USART_IT_RXNE);
			rec_data = USART_ReceiveData(USART2);
			cpad_slave_inst.rx_flag =1;
			if(cpad_slave_inst.rx_ok == 0)
			 {
					switch(cpad_slave_inst.rec_state)
					{
						
						case REC_ADDR_STATE:
						{
							
								if(rec_data == cpad_slave_inst.addr)
								{
									cpad_slave_inst.rec_cnt = 0;
									cpad_slave_inst.rxbuf[cpad_slave_inst.rec_cnt++] = rec_data;	
									cpad_slave_inst.rec_state = REC_DATA_STATE;
								}
								break;
						}
						case REC_DATA_STATE:
						{	
								if(cpad_slave_inst.rec_cnt < MNT_RX_LEN)
								{	
									cpad_slave_inst.rxbuf[cpad_slave_inst.rec_cnt++] = rec_data;
                                    if(cpad_slave_inst.rxbuf[1] != 0x10)            //��������д
									{
										if(cpad_slave_inst.rec_cnt >= MNT_CMD_LEN) 
										{
												cpad_slave_inst.rx_ok = 1;
												cpad_slave_inst.rec_state = REC_ADDR_STATE;
										}									
									}
									else
									{
										if(cpad_slave_inst.rec_cnt == cpad_slave_inst.rxbuf[6] + 9)  //�ֽ����Ƿ�һ��
										{
											cpad_slave_inst.rx_ok = 1;
											//temp = (((uint16_t)cpad_slave_inst.rxbuf[2] << 8) + cpad_slave_inst.rxbuf[3] - 0x0100) / 16;
					                        //rt_kprintf("��%d֡��ȷ\n",temp);
											cpad_slave_inst.rec_state = REC_ADDR_STATE;
										}									
									}									
								}
								break;
						}
						default:
						{
								cpad_slave_inst.rec_state = REC_ADDR_STATE;
								break;
						}
				}
//			}
		}
	}
	//�����ж�
	if (USART_GetITStatus(USART2, USART_IT_TC) == SET)
	{
			USART_ClearFlag(USART2, USART_FLAG_TC);
			if(is_fifo8_empty(&mnt_tx_fifo) == 0)
			{
					fifo8_pop(&mnt_tx_fifo,&tx_data);
					USART_SendData(USART2, tx_data);
			}				
			else
			{
					USART_ITConfig(USART2, USART_IT_TC, DISABLE);
					USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
					CPAD_SLAVE_RS485_RECEIVE_MODE;
			}		
			
	}
	
	rt_interrupt_leave();
}

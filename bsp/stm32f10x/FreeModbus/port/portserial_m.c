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
 * File: $Id: portserial_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master Functions $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "local_status.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED > 0
/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);
/* ----------------------- Start implementation -----------------------------*/

void vMBMasterPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
//	if (xRxEnable)
//	{
//		/* 485ͨ��ʱ���ȴ�������λ�Ĵ����е����ݷ�����ɺ���ȥʹ��485�Ľ��ա�ʧ��485�ķ���*/
//		while (!USART_GetFlagStatus(USART1,USART_FLAG_TC));
//		MASTER_RS485_RECEIVE_MODE;
//		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//	}
//	else
//	{
//		MASTER_RS485_SEND_MODE;
//		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
//	}
//	if (xTxEnable)
//	{
//		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
//	}
//	else
//	{
//		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
//	}
	if (xRxEnable)
	{
		/* 485ͨ��ʱ���ȴ�������λ�Ĵ����е����ݷ�����ɺ���ȥʹ��485�Ľ��ա�ʧ��485�ķ���*/
		while (!USART_GetFlagStatus(UART4,USART_FLAG_TC));
		SLAVE_RS485_RECEIVE_MODE;
		USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	}
	else
	{
		SLAVE_RS485_SEND_MODE;
		USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
	}
	if (xTxEnable)
	{
		USART_ITConfig(UART4, USART_IT_TXE, ENABLE);
	}
	else
	{
		USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
	}
}

void vMBMasterPortClose(void)
{
//	USART_ITConfig(USART1, USART_IT_TXE | USART_IT_RXNE, DISABLE);
//	USART_Cmd(USART1, DISABLE);
	USART_ITConfig(UART4, USART_IT_TXE | USART_IT_RXNE, DISABLE);
	USART_Cmd(UART4, DISABLE);
}
//Ĭ��һ������ ����1 �����ʿ�����  ��ż���������
BOOL xMBMasterPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits,
		eMBParity eParity)
{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	USART_InitTypeDef USART_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	//======================ʱ�ӳ�ʼ��=======================================
////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB , ENABLE);
////	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB , ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
//	//======================IO��ʼ��=======================================	
//	//USART1_TX
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	//USART1_RX
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
//	GPIO_Init(GPIOA, &GPIO_InitStructure);
//	//����485���ͺͽ���ģʽ
////    TODO   ��ʱ��дA0 ��֮����������ʱ���޸�
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//	//======================���ڳ�ʼ��=======================================
//	USART_InitStructure.USART_BaudRate = ulBaudRate;
//	//����У��ģʽ
//	switch (eParity)
//	{
//		case MB_PAR_NONE: //��У��
//			USART_InitStructure.USART_Parity = USART_Parity_No;
//			USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//			break;
//		case MB_PAR_ODD: //��У��
//			USART_InitStructure.USART_Parity = USART_Parity_Odd;
//			USART_InitStructure.USART_WordLength = USART_WordLength_9b;
//			break;
//		case MB_PAR_EVEN: //żУ��
//			USART_InitStructure.USART_Parity = USART_Parity_Even;
//			USART_InitStructure.USART_WordLength = USART_WordLength_9b;
//			break;
//		default:
//			return FALSE;
//	}

//	USART_InitStructure.USART_StopBits = USART_StopBits_1;
//	USART_InitStructure.USART_HardwareFlowControl =	USART_HardwareFlowControl_None;
//	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//	if (ucPORT != UPORT_MBMASTER)
//		return FALSE;

//	ENTER_CRITICAL_SECTION(); //��ȫ���ж�

//	USART_Init(USART1, &USART_InitStructure);
//	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
//	USART_Cmd(USART1, ENABLE);

//	//=====================�жϳ�ʼ��======================================
//	//����NVIC���ȼ�����ΪGroup2��0-3��ռʽ���ȼ���0-3����Ӧʽ���ȼ�
////	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

//	EXIT_CRITICAL_SECTION(); //��ȫ���ж�

//	return TRUE;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//======================ʱ�ӳ�ʼ��=======================================
//	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA ,	ENABLE);
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOC ,	ENABLE);
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_UART4,ENABLE);
	
	//======================IO��ʼ��=========================================	
	//UART4_TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//UART4_RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//����485���ͺͽ���ģʽ
	//TODO   ��ʱ��дC5 ��֮����������ʱ���޸�
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
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
		return FALSE;
	}

	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_HardwareFlowControl =	USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//	if (ucPORT != UPORT_MONITOR)
//		return FALSE;

	ENTER_CRITICAL_SECTION(); //��ȫ���ж�

	USART_Init(UART4, &USART_InitStructure);
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	USART_Cmd(UART4, ENABLE);

	//=====================�жϳ�ʼ��======================================
	//����NVIC���ȼ�����ΪGroup2��0-3��ռʽ���ȼ���0-3����Ӧʽ���ȼ�
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	EXIT_CRITICAL_SECTION(); //��ȫ���ж�

	return TRUE;
}

BOOL xMBMasterPortSerialPutByte(CHAR ucByte)
{
//	USART_SendData(USART1, ucByte);
	USART_SendData(UART4, ucByte);
	return TRUE;
}

BOOL xMBMasterPortSerialGetByte(CHAR * pucByte)
{
//	*pucByte = USART_ReceiveData(USART1);
	*pucByte = USART_ReceiveData(UART4);
	return TRUE;
}

/* 
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static void prvvUARTTxReadyISR(void)
{
	pxMBMasterFrameCBTransmitterEmpty();
}

/* 
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void prvvUARTRxISR(void)
{
	pxMBMasterFrameCBByteReceived();
}
///*******************************************************************************
// * Function Name  : USART1_IRQHandler
// * Description    : This function handles USART1 global interrupt request.
// * Input          : None
// * Output         : None
// * Return         : None
// *******************************************************************************/
//void USART1_IRQHandler(void)
//{
//	extern local_reg_st		l_sys;
//	rt_interrupt_enter();
//	//�������
//	if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) == SET)
//	{
//		prvvUARTRxISR();
//	}
//	//�����ж�
//	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
//	{
//		l_sys.u16Uart_Timeout=0;
//		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//		prvvUARTRxISR();
//	}
//	//�����ж�
//	if (USART_GetITStatus(USART1, USART_IT_TXE) == SET)
//	{
//		prvvUARTTxReadyISR();
//	}
//	rt_interrupt_leave();
//}
/*******************************************************************************
 * Function Name  : UART4_IRQHandler
 * Description    : This function handles UART4 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void UART4_IRQHandler(void)
{
	extern volatile uint8_t rx1_cnt;
	rt_interrupt_enter();
	//�������
	if (USART_GetFlagStatus(UART4, USART_FLAG_ORE) == SET)
	{
		prvvUARTRxISR();
	}
	//�����ж�
	if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
	{
		USART_ClearITPendingBit( UART4, USART_IT_RXNE);
		prvvUARTRxISR();
	}
	//�����ж�
	if (USART_GetITStatus(UART4, USART_IT_TXE) == SET)
	{
		prvvUARTTxReadyISR();
	}
	rt_interrupt_leave();
}
#endif

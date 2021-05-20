/*
 * File      : led.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

#ifndef __LED_H__
#define __LED_H__
#include "stdint.h"
#include <stm32f10x.h>

void drv_led_init(void);
void led_on(uint32_t led);
void led_off(uint32_t led);


#define nWs 64                //WS2811��������
enum
{
	GREEN=0,
	BLUE,
	YELLOW,	
	ORANGE,	
	RED,	
};

//#define   A_SET_DATA     GPIOB->ODR |= 0x8000;    //PB15= 1
//#define   A_CLR_DATA     GPIOB->ODR &= 0x7FFF;    //PB15= 0
//��������
#define   A_SET_DATA     GPIOB->ODR &= 0x7FFF;    //PB15= 0
#define   A_CLR_DATA     GPIOB->ODR |= 0x8000;    //PB15= 1
extern void Led_Gpio_Init(void);
extern void LED_Test(uint8_t Color);

struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

void LED_SPI_LowLevel_Init(void);
void LED_SPI_WriteByte(uint16_t Data);
void LED_SPI_SendBits(uint8_t bits);
void LED_SPI_SendPixel(struct Pixel pixel);
ErrorStatus LED_SPI_Update(struct Pixel buffer[], uint32_t length);
void PixelRound(uint8_t r,uint8_t g,uint8_t b,uint16_t pluse);

#endif

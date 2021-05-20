#include <stm32f10x.h>
#include "Delay.h"

/**************************
*内部延时
***************************/

/*---------------------------------
延时us
-----------------------------------*/
void Delay_ms(unsigned long u32us)
{
		unsigned long i;
		
		while(u32us--)
		{
				for(i=0;i<5000;i++);
		}	
}

void Delay_us(unsigned long u32us)
{
		unsigned char i;
		
		while(u32us--)
		{
				for(i=0;i<9;i++);
		}	
}

void Delay05us(void)
{
		unsigned char i;
		for(i=0; i<1; i++);
}



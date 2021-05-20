#define _IIC_H
#include "stm32f10x.h"
#include <rtthread.h>
#include "i2c_bsp.h"
#include "Delay.h"

/**
  * @brief  Initializes peripherals used by the I2C EEPROM driver.
  * @param  None
  * @retval : None
  */
#define   USE_MUTEX 1 //ʹ��
#define   I2CSPEED 2    //I2Cģ����ʱ
#define   I2CPAGEWriteDelay  5
static void init_IIC_Mutex(void);

static void I2C_Config(void)
{
		GPIO_InitTypeDef  GPIO_InitStructure; 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);		
	
		//Configure I2C1 pins: SCL 
		GPIO_InitStructure.GPIO_Pin =  II_SCL_Pin;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(II_SCL_GPIO, &GPIO_InitStructure);
		
		GPIO_SetBits(II_SCL_GPIO,II_SCL_Pin);
		
	 //Configure I2C1 pins: SDA
	 
		GPIO_InitStructure.GPIO_Pin =  II_SDA_Pin;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(II_SDA_GPIO, &GPIO_InitStructure);
		
		GPIO_SetBits(II_SDA_GPIO,II_SDA_Pin);

		GPIO_InitStructure.GPIO_Pin =  II_WP_Pin;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(II_WP_GPIO, &GPIO_InitStructure);
		//writ_disable
		GPIO_SetBits(II_SDA_GPIO,II_SDA_Pin);
  
 
}

static void IIC_SCL( uint8_t n)  
{  
		if(n == 1)
		{
				 GPIO_SetBits(II_SCL_GPIO,II_SCL_Pin);
		}
		else
		{
				 GPIO_ResetBits(II_SCL_GPIO,II_SCL_Pin);
		}
}

static void IIC_SDA( uint8_t n)   
{	
		if(n == 1)
		{
				 GPIO_SetBits(II_SDA_GPIO,II_SDA_Pin);
		}
		else
		{
				GPIO_ResetBits(II_SDA_GPIO,II_SDA_Pin);
		} 
}



void drv_i2c_init(void)
{
		I2C_Config();
#if USE_MUTEX
		init_IIC_Mutex(); 
#endif
}



static rt_mutex_t iic_mutex;
//���?��?��3a��?
#if USE_MUTEX
static void init_IIC_Mutex(void)
{
		rt_err_t erro;	
		
		iic_mutex= rt_mutex_create ("IIC_Mutex", RT_IPC_FLAG_PRIO);
		erro= rt_mutex_init(iic_mutex, "IIC_Mutex", RT_IPC_FLAG_PRIO);
		if(erro != RT_EOK)
		{
				rt_kprintf("IICBSP: rt_mutex_init erro\n");	
		}
	
}
#endif



static void SDA_OUT(void)
{
	  GPIO_InitTypeDef  GPIO_InitStructure; 
	  GPIO_InitStructure.GPIO_Pin =  II_SDA_Pin;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_Init(II_SDA_GPIO, &GPIO_InitStructure);
}


static void SDA_IN(void)
{
	  GPIO_InitTypeDef  GPIO_InitStructure; 
	  GPIO_InitStructure.GPIO_Pin =  II_SDA_Pin;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  	GPIO_Init(II_SDA_GPIO, &GPIO_InitStructure);
}

static void delay_us(u32 nus)
{
		u8 i;
		u32 temp;
		for(i = 0;i <4;i++)
		{
				for(temp = nus; temp != 0; temp--)
				{
						;
				}
		}
}


//2������IIC?e��?D?o?
static void IIC_Start(void)
{
		SDA_OUT();     
		IIC_SDA(1);	  	  
		IIC_SCL(1);
		delay_us(I2CSPEED);
		IIC_SDA(0);//START:when CLK is high,DATA change form high to low 
		delay_us(I2CSPEED);
		IIC_SCL(0);//?����?I2C����??��?��?��?����?��?��?����?��y?Y 
}	  
//2������IIC����?1D?o?
static void IIC_Stop(void)
{
		SDA_OUT();//sda??��?3?
		IIC_SCL(0);
		IIC_SDA(0);//STOP:when CLK is high DATA change form low to high
		delay_us(I2CSPEED);
		IIC_SCL(1); 
		IIC_SDA(1);//����?��I2C����???����?D?o?
		delay_us(I2CSPEED);							   	
}
//�̨���y��|��eD?o?��?����
//����???�̡�o1��??����?��|��e����㨹
//        0��??����?��|��e3��1|
static uint8_t IIC_Wait_Ack(void)
{
		uint8_t ucerrtime=0;
		SDA_IN();  //SDA����???a��?��?  
		IIC_SDA(1);
		delay_us(I2CSPEED);   
		IIC_SCL(1);
		delay_us(I2CSPEED);	 
		while(READ_SDA())
		{
				ucerrtime++;
				if(ucerrtime>250)
				{
						IIC_Stop();
						return 1;
				}
		}
		IIC_SCL(0);//����?����?3?0 	   
		return 0;  
} 
//2������ACK��|��e
static void IIC_Ack(void)
{
		IIC_SCL(0);
		SDA_OUT();
		IIC_SDA(0);
		delay_us(I2CSPEED);
		IIC_SCL(1);
		delay_us(I2CSPEED);
		IIC_SCL(0);
}
//2?2������ACK��|��e		    
static void IIC_NAck(void)
{
		IIC_SCL(0);
		SDA_OUT();
		IIC_SDA(1);
		delay_us(I2CSPEED);
		IIC_SCL(1);
		delay_us(I2CSPEED);
		IIC_SCL(0);
}					 				     
//IIC����?����???��??��
//����??�䨮?����D?T��|��e
//1��?��D��|��e
//0��??T��|��e			  
static void IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t; 
  
	  SDA_OUT(); 	    
    IIC_SCL(0);//��-�̨�����?��?a��?��y?Y��?��?
    for(t=0;t<8;t++)
    {              
        IIC_SDA((txd&0x80)>>7);
        txd<<=1; 	  
				delay_us(I2CSPEED);   //??TEA5767?a��y???������??��?��?D?��?
				IIC_SCL(1);
				delay_us(I2CSPEED);
				IIC_SCL(0);	
				delay_us(I2CSPEED);
    }	 
} 	    
//?��1??��??����?ack=1������?����?��ACK��?ack=0��?����?��nACK   
static uint8_t IIC_Read_Byte(uint8_t ack)
{
		uint8_t i,receive=0;
		SDA_IN();//SDA����???a��?��?
    for(i=0;i<8;i++ )
		{
        IIC_SCL(0); 
        delay_us(I2CSPEED);
				IIC_SCL(1);
        receive<<=1;
        if(READ_SDA())receive++;   
				delay_us(I2CSPEED); 
    }					 
    if (!ack)
        IIC_NAck();//����?��nACK
    else
        IIC_Ack(); //����?��ACK   
    return receive;
}


static int8_t  I2c_write_byte(uint8_t data)
{
	IIC_Send_Byte(data);
	if(IIC_Wait_Ack()==0)
	{
		return(1);
	}
	else
	{
		return(0);
	}
	
}
static int8_t WriteEEROMPage(uint8_t* write_buffer, uint16_t write_addr, uint8_t num_byte_write)
{
		u16 len;
		WP_Enable();
		IIC_Start();
		//I2C Slave addr
		if(I2c_write_byte(SLAVE_ADDR&0xFE)==0)
		{
				IIC_Stop();
				WP_Diable();
				//rt_kprintf("\n IICBSP: I2C_EE_BufWrite  ERR \n");	
				return(EEPROM_BUSSERRO);	
		}
		// Data high 8 addr
		if(I2c_write_byte(write_addr>>8)==0)
		{
				IIC_Stop();
				WP_Diable();
				rt_kprintf("\n IICBSP: WriteEEROMPage high 8\n");	
				return(EEPROM_BUSSERRO);	
		}
		// data low 8 addr
		if(I2c_write_byte(write_addr&0x00ff)==0)
		{
				IIC_Stop();
				WP_Diable();
				rt_kprintf("\n IICBSP: WriteEEROMPage low 8 \n");	
				return(EEPROM_BUSSERRO);	
		}

		for(len=0;len<num_byte_write;len++)
		{
				if(I2c_write_byte(*(write_buffer+len))==0)
				{
						IIC_Stop();
						WP_Diable();
						rt_kprintf("\n IICBSP: WriteEEROMPage len=%d \n",len);	
						return(EEPROM_BUSSERRO);
				}
		}
		IIC_Stop();
		WP_Diable();
		return(EEPROM_NOERRO);

}







int8_t I2C_EE_BufWrite_bsp(uint8_t* write_buffer, uint16_t write_addr, uint16_t num_byte_write)
{
	u8 byte_count;
	u16 page_number;
	u8 cnt;
	

	// �����һ����Ҫд��ĳ���
	byte_count =  I2C2_EE_PageSize - (write_addr % I2C2_EE_PageSize);
	
	if(num_byte_write <= byte_count)
	{
		byte_count = num_byte_write;
	}
	// ��д���һҳ������
	if(WriteEEROMPage(write_buffer,write_addr,byte_count) != EEPROM_NOERRO )
	{
		// д�����,���ش���
		return EEPROM_BUSSERRO;
	}
	
	// ���¼��㻹��Ҫд����ֽ�
	//ҳ��ʱ

	num_byte_write -= byte_count;
	if(!num_byte_write)
	{
		// �����Ѿ�д�����
		rt_thread_delay(I2CPAGEWriteDelay);
		return EEPROM_NOERRO;
	}
	write_addr += byte_count;
	write_buffer += byte_count;
	// ������Ҫ���е�ҳд����
	page_number = num_byte_write / I2C2_EE_PageSize;
	// ѭ��д������



while(page_number--)
	{
	cnt=10;
	while(cnt)
		{
			//���д���ɹ�
			if((WriteEEROMPage(write_buffer,write_addr,I2C2_EE_PageSize)) != EEPROM_NOERRO)
			{
				// ��ʱ
				cnt--;
				rt_thread_delay(1);

			}
			else//д�ɹ���
			{
				break;
				
			}
		
	 }
		//���дʧ��
		if(cnt==0)
		{
			rt_kprintf("\n IICBSP: I2C_EE_BufWrite  ERR \n");	
		}
		
		num_byte_write -= I2C2_EE_PageSize;
		write_addr += I2C2_EE_PageSize;
		write_buffer += I2C2_EE_PageSize;
	}	
	rt_thread_delay(I2CPAGEWriteDelay);
	if(!num_byte_write)
	{

		return EEPROM_NOERRO;
	}
	// д�����һҳ������
	if((WriteEEROMPage(write_buffer,write_addr,num_byte_write)) != EEPROM_NOERRO)
	{
		// д�����,���ش���

		return EEPROM_BUSSERRO;
	}	
	 rt_thread_delay(I2CPAGEWriteDelay);	
	return EEPROM_NOERRO;
}


int8_t I2C_EE_BufWrite(uint8_t* write_buffer, uint16_t write_addr, uint16_t num_byte_write)
{
		int8_t erro;
		rt_err_t mutex_erro;
		
		erro = EEPROM_NOERRO;
	
#if USE_MUTEX
		mutex_erro = rt_mutex_take(iic_mutex, 50);
		if(mutex_erro != RT_EOK)
		{
				erro = EEPROM_MUXERRO; 
				return(erro);
		}
#endif
	
		erro = I2C_EE_BufWrite_bsp(write_buffer,write_addr,num_byte_write);
#if USE_MUTEX
		rt_mutex_release(iic_mutex);			
#endif
	
	 return(erro);
}



int8_t I2C_EE_BufRead_bsp(uint8_t* read_buffer, uint16_t read_addr, uint16_t num_byte_Read)
{
	
	uint16_t len;
	
	//��ȡ������
	WP_Enable();
	IIC_Start();
	//I2C Slave addr
	if(I2c_write_byte(SLAVE_ADDR&0xFE)==0)
	{
		WP_Diable();
		IIC_Stop();
	
		rt_kprintf("\n IICBSP: I2C_EE_BufRead SLAVE_ADDR\n");	
		return(EEPROM_BUSSERRO);	
	}
	// Data high 8 addr
	if(I2c_write_byte(read_addr>>8)==0)
	{
		WP_Diable();
		IIC_Stop();
		rt_kprintf("\n IICBSP: I2C_EE_BufRead ADDR high 8\n");
		return(EEPROM_BUSSERRO);	
	}
	// data low 8 addr
	if(I2c_write_byte(read_addr&0x00ff)==0)
	{
		WP_Diable();
		IIC_Stop();
		rt_kprintf("\n IICBSP: I2C_EE_BufRead ADDR LOW  8\n");
		return(EEPROM_BUSSERRO);	
	}
	

	
	IIC_Start();

	if(I2c_write_byte(SLAVE_ADDR|0x01)==0)
	{
		WP_Diable();
		IIC_Stop();

		rt_kprintf("\n IICBSP: I2C_EE_BufRead SLAVE_ADDR  123\n");	
		return(EEPROM_BUSSERRO);	
	}
	
	WP_Diable();//write protect
	
	for(len=0;len<num_byte_Read-1;len++)
	{

		*(read_buffer++)=IIC_Read_Byte(1);
	}
	*(read_buffer)=IIC_Read_Byte(0);
	IIC_Stop();


	return(EEPROM_NOERRO);
	
	
}



int8_t I2C_EE_BufRead(uint8_t* read_buffer, uint16_t read_addr, uint16_t num_byte_read)
{
		int8_t erro;
		rt_err_t mutex_erro;
		
		erro = EEPROM_NOERRO;
	
#if USE_MUTEX
		mutex_erro = rt_mutex_take(iic_mutex, 50);
		if(mutex_erro != RT_EOK)
		{
				erro = EEPROM_MUXERRO; 
				return(erro);
		}
#endif
	
		erro = I2C_EE_BufRead_bsp(read_buffer,read_addr,num_byte_read);
#if USE_MUTEX
		rt_mutex_release(iic_mutex);			
#endif
	  return(erro);
}



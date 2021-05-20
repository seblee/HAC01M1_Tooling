#include <rtthread.h>
#include "Delay.h"
#include "TH_SENSOR_BSP.h"
#include "string.h"
#include "user_mb_app.h"
#include "sys_status.h"
#include "calc.h"


void AM_BUS_Config(void)
{
		GPIO_InitTypeDef  GPIO_InitStructure; 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
	 //Configure BUS pins: SDA_00 
		GPIO_InitStructure.GPIO_Pin =  II_AM_SDA_00_Pin;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(II_AM_SDA_00_GPIO, &GPIO_InitStructure);
		
		GPIO_SetBits(II_AM_SDA_00_GPIO,II_AM_SDA_00_Pin); 

		GPIO_InitStructure.GPIO_Pin =  II_AM_SDA_01_Pin;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(II_AM_SDA_01_GPIO, &GPIO_InitStructure);
		
		GPIO_SetBits(II_AM_SDA_01_GPIO,II_AM_SDA_01_Pin); 
}

void AM_Init(void)
{
	AM_BUS_Config();
}

static void AM_SDA_H(uint8_t u8SN)   
{	
	switch(u8SN)
	{
		case 0x00:
			{
					GPIO_SetBits(II_AM_SDA_00_GPIO,II_AM_SDA_00_Pin);
			}
			break;
		case 0x01:
			{
					GPIO_SetBits(II_AM_SDA_01_GPIO,II_AM_SDA_01_Pin);
			}
			break;
		default:
			break;			
	}	
}

static void AM_SDA_L(uint8_t u8SN)   
{	
	switch(u8SN)
	{
		case 0x00:
			{
					GPIO_ResetBits(II_AM_SDA_00_GPIO,II_AM_SDA_00_Pin);
			}
			break;
		case 0x01:
			{
					GPIO_ResetBits(II_AM_SDA_01_GPIO,II_AM_SDA_01_Pin);
			}
			break;
		default:
			break;			
	} 
}

static uint8_t AM_SDA_READ(uint8_t u8SN)
{
	uint8_t u8Read_SDA=0;
	switch(u8SN)
	{
		case 0x00:
			{
					u8Read_SDA=GPIO_ReadInputDataBit(II_AM_SDA_00_GPIO,II_AM_SDA_00_Pin);
			}
			break;
		case 0x01:
			{
					u8Read_SDA=GPIO_ReadInputDataBit(II_AM_SDA_01_GPIO,II_AM_SDA_01_Pin);
			}
			break;
		default:
			break;			
	}	
	return u8Read_SDA;
}
static void AM_SDA_OUT(uint8_t u8SN)
{
	GPIO_InitTypeDef  GPIO_InitStructure; 
	switch(u8SN)
	{
		case 0x00:
			{
				GPIO_InitStructure.GPIO_Pin =  II_AM_SDA_00_Pin;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
				GPIO_Init(II_AM_SDA_00_GPIO, &GPIO_InitStructure);		
			}
			break;
		case 0x01:
			{
				GPIO_InitStructure.GPIO_Pin =  II_AM_SDA_01_Pin;
				GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
				GPIO_Init(II_AM_SDA_01_GPIO, &GPIO_InitStructure);		
			}
			break;
		default:
			break;			
	}
}


static void AM_SDA_IN(uint8_t u8SN)
{
	GPIO_InitTypeDef  GPIO_InitStructure; 

	switch(u8SN)
	{
		case 0x00:
			{
				GPIO_InitStructure.GPIO_Pin =  II_AM_SDA_00_Pin;
//				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ;
				GPIO_Init(II_AM_SDA_00_GPIO, &GPIO_InitStructure);	
			}
			break;
		case 0x01:
			{
				GPIO_InitStructure.GPIO_Pin =  II_AM_SDA_01_Pin;
//				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ;
				GPIO_Init(II_AM_SDA_01_GPIO, &GPIO_InitStructure);	
			}
			break;
		default:
			break;			
	}
}

/********************************************\
|* ���ܣ� ��ʼ�ź�       *|
\********************************************/
void AM23XX_start(uint8_t u8SN)
{
	//����������ʼ�ź�
	AM_SDA_OUT(u8SN);//��Ϊ���ģʽ
  AM_SDA_L(u8SN);	//�������������ߣ�SDA������
	Delay_us(1500);  //��ʱ1.5Ms//����һ��ʱ�䣨����800us����֪ͨ������׼������
  AM_SDA_H(u8SN);	//�ͷ�����
  AM_SDA_IN(u8SN); ;	//��Ϊ����ģʽ���жϴ�������Ӧ�ź�
	Delay_us(40);//��ʱ30us
}

/********************************************\
|* ���ܣ� �����������͵ĵ����ֽ�	        *|
\********************************************/
unsigned char Read_SensorData(uint8_t u8SN)
{
	uint8_t i;
	uint16_t j;
	uint8_t data=0,bit=0;
	
	for(i=0;i<8;i++)
	{
		while(!AM_SDA_READ(u8SN))	//����ϴε͵�ƽ�Ƿ����
		{
			if(++j>=5000) //��ֹ������ѭ��
			{
				break;
			}
		}
		//��ʱMin=26us Max70us ��������"0" �ĸߵ�ƽ		 
		Delay_us(30);//��ʱ30us

		//�жϴ�������������λ
		bit=0;
		if(AM_SDA_READ(u8SN))
		{
			bit=1;
		}
		j=0;
		while(AM_SDA_READ(u8SN))		//�ȴ��ߵ�ƽ ����
		{
			if(++j>=5000) //��ֹ������ѭ��
			{
				break;
			}		
		}
		data<<=1;
		data|=bit;
	}
	return data;
}



/********************************************\
|* ���ܣ�AM2320��ȡ��ʪ�Ⱥ���       *|
\********************************************/
//������Humi_H��ʪ�ȸ�λ��Humi_L��ʪ�ȵ�λ��Temp_H���¶ȸ�λ��Temp_L���¶ȵ�λ��Temp_CAL��У��λ
//���ݸ�ʽΪ��ʪ�ȸ�λ��ʪ��������+ʪ�ȵ�λ��ʪ��С����+�¶ȸ�λ���¶�������+�¶ȵ�λ���¶�С����+ У��λ
//У�飺У��λ=ʪ�ȸ�λ+ʪ�ȵ�λ+�¶ȸ�λ+�¶ȵ�λ
uint8_t Read_Sensor(uint16_t *u16TH_Buff,uint8_t u8SN)
{
	uint16_t j;
	uint8_t Humi_H,Humi_L,Temp_H,Temp_L,Temp_CAL,temp;
//	float Temprature,Humi;//������ʪ�ȱ��� ���˱���Ϊȫ�ֱ���
	uint8_t Sensor_AnswerFlag;  //�յ���ʼ��־λ
	uint8_t Sensor_ErrorFlag;   //��ȡ�����������־
	int16_t i16Temprature;//������ʪ�ȱ���
	uint16_t u16Humi;//������ʪ�ȱ��� 
    Sensor_ErrorFlag=Sensor_ErrorFlag;
	ENTER_CRITICAL_SECTION(); //��ȫ���ж�		
	AM23XX_start(u8SN);//�ӻ�������ʼ�ź�

	Sensor_AnswerFlag=0;	//��������Ӧ��־
	//�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������	  
	if(AM_SDA_READ(u8SN)==0)
	{
		Sensor_AnswerFlag=1;	//�յ���ʼ�ź�
		j=0;
		while((!AM_SDA_READ(u8SN))) //�жϴӻ����� 80us �ĵ͵�ƽ��Ӧ�ź��Ƿ����	
		{
			if(++j>=500) //��ֹ������ѭ��
			{
				Sensor_ErrorFlag=1;
				break;
			}
		}
		Sensor_AnswerFlag|=0x02;
		j=0;
		while(AM_SDA_READ(u8SN))//�жϴӻ��Ƿ񷢳� 80us �ĸߵ�ƽ���緢����������ݽ���״̬
		{
			if(++j>=800) //��ֹ������ѭ��
			{
				Sensor_ErrorFlag=1;
				break;
			}		
		}
		Sensor_AnswerFlag|=0x04;
		//��������
		Humi_H=Read_SensorData(u8SN);
		Humi_L=Read_SensorData(u8SN);
		Temp_H=Read_SensorData(u8SN);	
		Temp_L=Read_SensorData(u8SN);
		Temp_CAL=Read_SensorData(u8SN);

		temp=(uint8_t)(Humi_H+Humi_L+Temp_H+Temp_L);//ֻȡ��8λ
//			rt_kprintf("Humi_H=%d,Humi_L=%d,Temp_H=%d,Temp_L=%d,Temp_CAL=%d,temp=%d,\n",Humi_H,Humi_L,Temp_H,Temp_L,Temp_CAL,temp);
		if(Temp_CAL==temp)//���У��ɹ�����������
		{
					Sensor_AnswerFlag|=0x08;

			u16Humi=Humi_L|((uint16_t)Humi_H<<8);//ʪ��
			
			if(Temp_H&0X80)	//Ϊ���¶�
			{
				i16Temprature =0-((Temp_L&0x7F)|((uint16_t)Temp_H<<8));
			}
			else   //Ϊ���¶�
			{
				i16Temprature=Temp_L|((uint16_t)Temp_H<<8);//�¶�
			}
			//�ж������Ƿ񳬹����̣��¶ȣ�-40��~80�棬ʪ��0��RH~99��RH��
			if(u16Humi>999) 
			{
			  u16Humi=999;
			}

			if(i16Temprature>800)
			{
			  i16Temprature=800;
			}
			if(i16Temprature<-400)
			{
				i16Temprature = -400;
			}			
			u16TH_Buff[0]=(uint16_t)i16Temprature;//�¶�
			u16TH_Buff[1]=(uint16_t)u16Humi;//ʪ��			
//			rt_kprintf("\r\nTemprature:  %.1f  ��\r\n",i16Temprature); //��ʾ�¶�
//			rt_kprintf("Humi:  %.1f  %%RH\r\n",u16Humi);//��ʾʪ��
		}	
		else
		{
			Sensor_AnswerFlag|=0x10;	
//			rt_kprintf("Humi_H=%d,Humi_L=%d,Temp_H=%d,Temp_L=%d,Temp_CAL=%d,temp=%d,\n",Humi_H,Humi_L,Temp_H,Temp_L,Temp_CAL,temp);
		}
	}
	else
	{
		Sensor_ErrorFlag=0;  //δ�յ���������Ӧ
		rt_kprintf("Sensor Error!!\r\n");
	}
	EXIT_CRITICAL_SECTION(); //��ȫ���ж�	
	return Sensor_AnswerFlag;
}  

#define NUM_1 8       //�˲�����
#define T_MAX 2       //�˲�����
unsigned short AVGfilter_1(int8_t i8Type,int16_t i16Value)
{
		static int8_t i8Num[T_MAX]={0};
		static int16_t i16Value_buf[T_MAX][NUM_1];

		int16_t i16CvtValue;		
		if(i8Num[i8Type]<NUM_1)
		{
			i8Num[i8Type]++;
		}
		else
		{
			i8Num[i8Type]=0;		
		}		
		i16Value_buf[i8Type][i8Num[i8Type]] = i16Value;	
		i16CvtValue=MedianFilter((uint16_t *)i16Value_buf[i8Type],NUM_1);	
		
    return i16CvtValue;
}

#define AM_SENSOR_NUM  2
uint16_t u16TH_Sensor[2]={0};
#define TH_AVE_NUM  5
/********************************************\
|* ���ܣ� ��ʪ�ȸ���             	        *|
\********************************************/
uint8_t AM_Sensor_update(sys_reg_st* gds_ptr)
{
		static 	uint8_t  u8CNT=0;
		static 	uint8_t  u8Err_CNT[AM_SENSOR_NUM]={0};
		
		uint8_t i=0,j=0;  //�յ���ʼ��־λ
		uint8_t u8SenFlag[AM_SENSOR_NUM]={0};  //�յ���ʼ��־λ
		Com_tnh_st u16TH_Buff={0};

		if(Get_TH()==0)//ʹ��HAC01S1
		{
				return 0;
		}		
		u8CNT++;
		if(u8CNT>=0xFF)
		{
			u8CNT=0x00;	
		}
		i=u8CNT%(AM_SENSOR_NUM+1);
		memset(&u16TH_Sensor[0],0x00,4);
//		//���ζ�ȡ�������2S
//		Delay_ms(500);//��ʱ500ms
		if((i!=MBM_DEV_A1_ADDR)&&(i!=MBM_DEV_A2_ADDR))
		{
				return 0;			
		}
		//ӳ��ط���ʪ��
		if((i==MBM_DEV_A1_ADDR)&&((gds_ptr->config.dev_mask.mb_comp&(0X01<<MBM_DEV_A1_ADDR))==0))
		{
				return u8SenFlag[i];			
		}
		else if((i==MBM_DEV_A2_ADDR)&&((gds_ptr->config.dev_mask.mb_comp&(0X01<<MBM_DEV_A2_ADDR))==0))
		{
				return u8SenFlag[i];				
		}

		u8SenFlag[i]=Read_Sensor(&u16TH_Sensor[0],i);
		if(u8SenFlag[i])
		{
				if((u16TH_Sensor[0]==0)&&(u16TH_Sensor[0]==0))
				{
					u8Err_CNT[i]++;						
				}
				else
				{
					u8Err_CNT[i]=0;
					u16TH_Buff.Temp = u16TH_Sensor[0]+(int16_t)(gds_ptr->config.general.temp_sensor_cali[i].temp);
					u16TH_Buff.Hum = u16TH_Sensor[1]+(int16_t)(gds_ptr->config.general.temp_sensor_cali[i].hum);	
					//����״̬MBM_COM_STS_REG_NO
					gds_ptr->status.status_remap[MBM_COM_STS_REG_NO] |= (0x0001<<i);	
				}					
		}
		else
		{
				u8Err_CNT[i]++;			
		}
		
		if(u8Err_CNT[i]>ERROR_CNT_MAX)
		{
//			u8Err_CNT[i]=0;
			gds_ptr->status.mbm.tnh[i].temp = 0;
			gds_ptr->status.mbm.tnh[i].hum = 0;		
			//����״̬MBM_COM_STS_REG_NO
			gds_ptr->status.status_remap[MBM_COM_STS_REG_NO] &= ~(0x0001<<i);			
				AM_Init();				//AM Sensor init	
				i=0;			
		}
		else if(u8Err_CNT[i]==0)
		{		
			gds_ptr->status.mbm.tnh[i].temp=u16TH_Buff.Temp;
			gds_ptr->status.mbm.tnh[i].hum=u16TH_Buff.Hum;		
			//ȡƽ��ֵ
			for(j=0;j<T_MAX;j++)
			{
					gds_ptr->status.mbm.tnh[j].temp=AVGfilter_1(j,gds_ptr->status.mbm.tnh[j].temp);
			}
		}	
//		g_sys.status.ComSta.u16TH[0].Temp=285;
//		g_sys.status.ComSta.u16TH[0].Temp=567;
//		rt_kprintf("u8CNT=%x,i=%x,u8SenFlag[0]= %x,u16TH_Sensor[0]= %x,[1] = %x,u8Err_CNT[0]=%x,Temp=%d,Hum=%d\n",u8CNT,i,u8SenFlag[0],u16TH_Sensor[0],u16TH_Sensor[1],u8Err_CNT[0],gds_ptr->status.mbm.tnh[0].temp,gds_ptr->status.mbm.tnh[0].hum);			
	
		return u8SenFlag[i];
}






#include "includes.h"
#include "helper.h"

#define I2C1_RCC               RCC_APB1Periph_I2C1

#define I2C1_GPIO_RCC    	  RCC_APB2Periph_GPIOB
#define I2C1_GPIO       	   GPIOB
#define I2C1_SCL_Pin           GPIO_Pin_8

#define I2C1_SDA_GPIO_RCC      I2C1_GPIO_RCC
#define I2C1_SDA_Pin           GPIO_Pin_9


#define I2C2_RCC               RCC_APB1Periph_I2C2

#define I2C2_GPIO_RCC      		RCC_APB2Periph_GPIOB
#define I2C2_GPIO        	  GPIOB
#define I2C2_SCL_Pin           GPIO_Pin_10

#define I2C2_SDA_GPIO_RCC      I2C2_GPIO_RCC
#define I2C2_SDA_Pin           GPIO_Pin_11


#define udelay(x)	{int ii = 72 * x; do {} while (ii--);}


void i2c_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(I2C1_GPIO_RCC, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	

	/*********************************I2C1**************************************/
	GPIO_InitStructure.GPIO_Pin    = I2C1_SCL_Pin | I2C1_SDA_Pin;
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_AF_OD;           

	GPIO_Init(I2C1_GPIO, &GPIO_InitStructure);
	
	I2C_DeInit(I2C1);
	
	RCC_APB1PeriphClockCmd(I2C1_RCC, ENABLE);

	I2C_InitStructure.I2C_ClockSpeed          = 100000;    //100KHz I2C
	I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;   
	I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2; 
	I2C_InitStructure.I2C_OwnAddress1         = 0x30;     
	I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;  
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

	I2C_Init(I2C1, &I2C_InitStructure);
	
	I2C_Cmd(I2C1, ENABLE);
	
	I2C_AcknowledgeConfig(I2C1, ENABLE);
#if 0	
	//≈–∂œ «∑Ò «
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) {
		
		GPIO_InitStructure.GPIO_Pin    = I2C1_SCL_Pin ;
		GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_Out_OD;
		
		GPIO_Init(I2C1_GPIO, &GPIO_InitStructure);
		
		while (GPIO_ReadInputDataBit(I2C1_GPIO, I2C1_SDA_Pin) != Bit_SET) {
			GPIO_WriteBit(I2C1_GPIO, I2C1_SCL_Pin, Bit_RESET);
			udelay(10);
			GPIO_WriteBit(I2C1_GPIO, I2C1_SCL_Pin, Bit_SET);
			udelay(10);
		}
			
		GPIO_WriteBit(I2C1_GPIO, I2C1_SCL_Pin, Bit_RESET);
		udelay(10);
		GPIO_WriteBit(I2C1_GPIO, I2C1_SCL_Pin, Bit_SET);
		
		GPIO_InitStructure.GPIO_Pin    = I2C1_SCL_Pin | I2C1_SDA_Pin;
		GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_AF_OD;           

		GPIO_Init(I2C1_GPIO, &GPIO_InitStructure);
		
		I2C_SoftwareResetCmd(I2C1, ENABLE);
		msleep(1);
		
		I2C_SoftwareResetCmd(I2C1, DISABLE);
		
		I2C_Init(I2C1, &I2C_InitStructure);
	
		I2C_Cmd(I2C1, ENABLE);

		I2C_AcknowledgeConfig(I2C1, ENABLE);	
	}
#endif
#if 0	
	/*********************************I2C2**************************************/

	
	GPIO_InitStructure.GPIO_Pin    = I2C2_SCL_Pin | I2C2_SDA_Pin;
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_AF_OD;           

	GPIO_Init(I2C2_GPIO, &GPIO_InitStructure);
	
	I2C_DeInit(I2C2);
	
	RCC_APB1PeriphClockCmd(I2C2_RCC, ENABLE);
	
	I2C_InitStructure.I2C_ClockSpeed          = 100000;    //100KHz I2C
	I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;   
	I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2; 
	I2C_InitStructure.I2C_OwnAddress1         = 0x30;     
	I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;  
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

	I2C_Init(I2C2, &I2C_InitStructure);
	
	I2C_Cmd(I2C2, ENABLE);

	I2C_AcknowledgeConfig(I2C2, ENABLE);
		
	if (I2C_GetFlagStatus(I2C2, I2C_FLAG_BUSY)) {
		
		GPIO_InitStructure.GPIO_Pin    = I2C2_SCL_Pin ;
		GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_Out_OD;
		
		GPIO_Init(I2C2_GPIO, &GPIO_InitStructure);
		
		while (GPIO_ReadInputDataBit(I2C2_GPIO, I2C2_SDA_Pin) != Bit_SET) {
			GPIO_WriteBit(I2C2_GPIO, I2C2_SCL_Pin, Bit_RESET);
			udelay(10);
			GPIO_WriteBit(I2C2_GPIO, I2C2_SCL_Pin, Bit_SET);
			udelay(10);
		}
			
		GPIO_WriteBit(I2C2_GPIO, I2C2_SCL_Pin, Bit_RESET);
		udelay(10);
		GPIO_WriteBit(I2C2_GPIO, I2C2_SCL_Pin, Bit_SET);
		
		GPIO_InitStructure.GPIO_Pin    = I2C2_SCL_Pin | I2C2_SDA_Pin;
		GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_AF_OD;           

		GPIO_Init(I2C2_GPIO, &GPIO_InitStructure);
		
		I2C_SoftwareResetCmd(I2C2, ENABLE);
		
		msleep(1);
		
		I2C_SoftwareResetCmd(I2C2, DISABLE);
		
		I2C_Init(I2C2, &I2C_InitStructure);
	
		I2C_Cmd(I2C2, ENABLE);

		I2C_AcknowledgeConfig(I2C2, ENABLE);	
	}
#endif
}

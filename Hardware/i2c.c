#include "includes.h"

#define I2C                   I2C2
#define I2C_RCC               RCC_APB1Periph_I2C2

#define I2C_SCL_GPIO_RCC      RCC_APB2Periph_GPIOB
#define I2C_SCL_GPIO          GPIOB
#define I2C_SCL_Pin           GPIO_Pin_10

#define I2C_SDA_GPIO_RCC      RCC_APB2Periph_GPIOB
#define I2C_SDA_GPIO          GPIOB
#define I2C_SDA_Pin           GPIO_Pin_11

void i2c_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	RCC_APB2PeriphClockCmd(I2C_SCL_GPIO_RCC, ENABLE);          

	GPIO_InitStructure.GPIO_Pin    = I2C_SCL_Pin|I2C_SDA_Pin;
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_AF_OD;           

	GPIO_Init(I2C_SCL_GPIO, &GPIO_InitStructure);

	RCC_APB1PeriphClockCmd(I2C_RCC, ENABLE);

	I2C_DeInit(I2C);

	I2C_InitStructure.I2C_ClockSpeed          = 100000;    //100KHz I2C
	I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;   
	I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2; 
	I2C_InitStructure.I2C_OwnAddress1         = 0x30;     
	I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;  
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

	I2C_Init(I2C, &I2C_InitStructure);

	I2C_Cmd(I2C, ENABLE);

	I2C_AcknowledgeConfig(I2C, ENABLE);

}
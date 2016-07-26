#include "includes.h"

void irda_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);          

	GPIO_InitStructure.GPIO_Pin    = GPIO_Pin_9 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_AF_OD;           

	GPIO_Init(GPIOB, &GPIO_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	I2C_DeInit(I2C1);

	I2C_InitStructure.I2C_ClockSpeed          = 100000;    //100KHz I2C
	I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;   
	I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2; 
	I2C_InitStructure.I2C_OwnAddress1         = 0x30;     
	I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;  
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

	I2C_Init(I2C1, &I2C_InitStructure);
	
	I2C_ITConfig(I2C1, I2C_IT_BUF, ENABLE);

	I2C_Cmd(I2C1, ENABLE);

	I2C_AcknowledgeConfig(I2C1, ENABLE);
}

void I2C1_EV_IRQHandler(void)
{
	if (I2C_GetITStatus(I2C1, I2C_IT_ADDR) == SET)
		I2C_ClearITPendingBit(I2C1, I2C_IT_ADDR);
	else if (I2C_GetITStatus(I2C1, I2C_IT_RXNE) == SET) {
		I2C_ClearITPendingBit(I2C1, I2C_IT_RXNE);
		//reduce_blod((s8)I2C_ReceiveData(I2C1));
	} else if (I2C_GetITStatus(I2C1,I2C_IT_STOPF) == SET) {
		I2C_ClearITPendingBit(I2C1, I2C_IT_STOPF);
		
	}
}
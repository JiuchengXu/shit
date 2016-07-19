#ifndef _I2C_AT24C02_H
#define _I2C_AT24C02_H

#include"stm32f10x.h"


#define I2C                   I2C1
#define I2C_RCC               RCC_APB1Periph_I2C2

#define I2C_SCL_GPIO_RCC      RCC_APB2Periph_GPIOB
#define I2C_SCL_GPIO          GPIOB
#define I2C_SCL_Pin           GPIO_Pin_6

#define I2C_SDA_GPIO_RCC      RCC_APB2Periph_GPIOB
#define I2C_SDA_GPIO          GPIOB
#define I2C_SDA_Pin           GPIO_Pin_7

void I2C_AT24Cx_Init(void); 

void I2C_AT24Cx_Reads(u8 Address,u8 *ReadBuffer,u16 ReadNumber); 

void I2C_AT24Cx_WriteByte(u8 Address,u8 WriteData); 

void I2C_AT24Cx_WritePage(u8 Address,u8 *WriteData,u16 WriteNumber); 

void I2C_AT24Cx_Writes(u8 Address,u8 *WriteData,u16 WriteNumber); 

#define AT24Cx_Address           0xa0 
#define AT24Cx_PageSize          8  

#endif
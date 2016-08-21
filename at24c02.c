#include "includes.h"

#define I2C                   I2C2

#define AT24Cx_Address           0xa0 
#define AT24Cx_PageSize          8  

#define EEPROM1_WP				GPIOC
#define EEPROM1_WP_Pin			GPIO_Pin_4

void at24c02_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); 
	
	GPIO_InitStructure.GPIO_Pin    = EEPROM1_WP_Pin;
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_Out_PP;
	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_ResetBits(EEPROM1_WP, EEPROM1_WP_Pin);	
}

void I2C_AT24Cx_Reads(u8 Address,u8 *ReadBuffer,u16 ReadNumber)
{
	if(ReadNumber==0)  
		return;


	while(I2C_GetFlagStatus(I2C,I2C_FLAG_BUSY));


	I2C_AcknowledgeConfig(I2C, ENABLE);


	I2C_GenerateSTART(I2C,ENABLE);
	while(!I2C_CheckEvent(I2C,I2C_EVENT_MASTER_MODE_SELECT));   

	I2C_Send7bitAddress(I2C,AT24Cx_Address, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));


	I2C_SendData(I2C, Address);
	while (!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 

	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT));


	I2C_Send7bitAddress(I2C,AT24Cx_Address, I2C_Direction_Receiver);  
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));


	while(ReadNumber)
	{

		if(ReadNumber==1)
		{
			I2C_AcknowledgeConfig(I2C, DISABLE);  
			I2C_GenerateSTOP(I2C,ENABLE); 
		}

		while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_RECEIVED)); 
		*ReadBuffer=I2C_ReceiveData(I2C);
		ReadBuffer++;
		ReadNumber--;
	}
	I2C_AcknowledgeConfig(I2C, ENABLE);
}



static void I2C_AT24Cx_WaitForComplete(void)
{
	vu16 SR1_Tmp;
	do
	{

		I2C_GenerateSTART(I2C, ENABLE); 

		SR1_Tmp = I2C_ReadRegister(I2C, I2C_Register_SR1); 

		I2C_Send7bitAddress(I2C,AT24Cx_Address, I2C_Direction_Transmitter);
	}while(!(I2C_ReadRegister(I2C, I2C_Register_SR1) & 0x0002)); 


	I2C_ClearFlag(I2C, I2C_FLAG_AF);  

	I2C_GenerateSTOP(I2C, ENABLE);
}



void I2C_AT24Cx_WriteByte(u8 Address,u8 WriteData)
{

	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT)); 


	I2C_Send7bitAddress(I2C,AT24Cx_Address, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 


	I2C_SendData(I2C, Address);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));


	I2C_SendData(I2C, WriteData);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));


	I2C_GenerateSTOP(I2C, ENABLE);  

	I2C_AT24Cx_WaitForComplete();
}



void I2C_AT24Cx_WritePage(u8 Address,u8 *WriteData,u16 WriteNumber)
{
	while(I2C_GetFlagStatus(I2C, I2C_FLAG_BUSY));


	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT));


	I2C_Send7bitAddress(I2C,AT24Cx_Address, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));


	I2C_SendData(I2C,Address);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	while(WriteNumber--) 
	{
		I2C_SendData(I2C,*WriteData);
		WriteData++;
		while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}


	I2C_GenerateSTOP(I2C, ENABLE);
}



void I2C_AT24Cx_Writes(u8 Address,u8 *WriteData,u16 WriteNumber)
{
	u8 Temp;


	Temp=Address%AT24Cx_PageSize; 
	if(Temp)       
	{
		Temp=AT24Cx_PageSize-Temp;  
		I2C_AT24Cx_WritePage(Address,WriteData,Temp);
		Address+=Temp;
		WriteData+=Temp;
		WriteNumber-=Temp;
		I2C_AT24Cx_WaitForComplete(); 
	}

	while(WriteNumber)
	{

		if(WriteNumber>=AT24Cx_PageSize)
		{
			I2C_AT24Cx_WritePage(Address,WriteData,AT24Cx_PageSize);
			Address+=AT24Cx_PageSize;
			WriteData+=AT24Cx_PageSize;
			WriteNumber-=AT24Cx_PageSize;
			I2C_AT24Cx_WaitForComplete();
		}

		else
		{
			I2C_AT24Cx_WritePage(Address,WriteData,WriteNumber);
			WriteNumber=0;
			I2C_AT24Cx_WaitForComplete();
		}
	}
}

void AT24Cxx_test(void)
{
	char a[]="abcdefghijklmnopqstuvwxyz0123456789";
	
	I2C_AT24Cx_Writes(0, (u8 *)a, sizeof(a));
	memset(a, 0, sizeof(a));
	I2C_AT24Cx_Reads(0, (u8 *)a, sizeof(a));
}

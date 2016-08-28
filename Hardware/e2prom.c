#include "includes.h"

#define AT24Cx_PageSize          8

void e2prom_Reads(I2C_TypeDef *I2C, u8 slave_addr, u8 Address,u8 *ReadBuffer,u16 ReadNumber)
{
	if(ReadNumber == 0)  
		return;

	while(I2C_GetFlagStatus(I2C, I2C_FLAG_BUSY));

	I2C_AcknowledgeConfig(I2C, ENABLE);


	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT));   

	I2C_Send7bitAddress(I2C, slave_addr, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));


	I2C_SendData(I2C, Address);
	while (!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 

	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT));


	I2C_Send7bitAddress(I2C, slave_addr, I2C_Direction_Receiver);  
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));


	while (ReadNumber) {

		if (ReadNumber==1) {
			I2C_AcknowledgeConfig(I2C, DISABLE);  
			I2C_GenerateSTOP(I2C, ENABLE); 
		}

		while (!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_RECEIVED)); 
		*ReadBuffer=I2C_ReceiveData(I2C);
		ReadBuffer++;
		ReadNumber--;
	}
	I2C_AcknowledgeConfig(I2C, ENABLE);
}

static void e2prom_WaitForComplete(I2C_TypeDef *I2C, u8 slave_addr)
{
	vu16 SR1_Tmp;
	do {
		I2C_GenerateSTART(I2C, ENABLE); 

		SR1_Tmp = I2C_ReadRegister(I2C, I2C_Register_SR1); 

		I2C_Send7bitAddress(I2C, slave_addr, I2C_Direction_Transmitter);
	} while (!(I2C_ReadRegister(I2C, I2C_Register_SR1) & 0x0002)); 


	I2C_ClearFlag(I2C, I2C_FLAG_AF);  

	I2C_GenerateSTOP(I2C, ENABLE);
}

void e2prom_WriteByte(I2C_TypeDef *I2C, u8 slave_addr, u8 Address,u8 WriteData)
{

	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT)); 


	I2C_Send7bitAddress(I2C, slave_addr, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 

	I2C_SendData(I2C, Address);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));


	I2C_SendData(I2C, WriteData);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));


	I2C_GenerateSTOP(I2C, ENABLE);  

	e2prom_WaitForComplete(I2C, slave_addr);
}

void e2prom_WritePage(I2C_TypeDef *I2C, u8 slave_addr, u8 Address, u8 *WriteData, u16 WriteNumber)
{
	while(I2C_GetFlagStatus(I2C, I2C_FLAG_BUSY));

	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT));

	I2C_Send7bitAddress(I2C, slave_addr, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(I2C, Address);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	while (WriteNumber--)  {
		I2C_SendData(I2C, *WriteData);
		WriteData++;
		while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}

	I2C_GenerateSTOP(I2C, ENABLE);
}

void e2prom_Writes(I2C_TypeDef *I2C, u8 slave_addr, u8 Address, u8 *WriteData,u16 WriteNumber)
{
	u8 Temp;

	Temp = Address % AT24Cx_PageSize; 
	if (Temp) {
		Temp = AT24Cx_PageSize - Temp;  
		e2prom_WritePage(I2C, slave_addr, Address, WriteData,Temp);
		Address += Temp;
		WriteData += Temp;
		WriteNumber -= Temp;
		e2prom_WaitForComplete(I2C, slave_addr); 
	}

	while (WriteNumber) {
		if (WriteNumber >= AT24Cx_PageSize) {
			e2prom_WritePage(I2C, slave_addr, Address, WriteData, AT24Cx_PageSize);
			Address += AT24Cx_PageSize;
			WriteData += AT24Cx_PageSize;
			WriteNumber -= AT24Cx_PageSize;
			e2prom_WaitForComplete(I2C, slave_addr);
		}

		else {
			e2prom_WritePage(I2C, slave_addr, Address, WriteData, WriteNumber);
			WriteNumber = 0;
			e2prom_WaitForComplete(I2C, slave_addr);
		}
	}
}

void e2prom_test(I2C_TypeDef *I2C, u8 slave_addr)
{
	char a[]="abcdefghijklmnopqstuvwxyz0123456789";
	
	e2prom_Writes(I2C, slave_addr, 0, (u8 *)a, sizeof(a));
	memset(a, 0, sizeof(a));
	e2prom_Reads(I2C, slave_addr, 0, (u8 *)a, sizeof(a));
}

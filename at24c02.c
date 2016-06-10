#include "at24c02.h"

void at24c02_init(void)
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
   
    I2C_GenerateSTART(I2C1, ENABLE); 
   
    SR1_Tmp = I2C_ReadRegister(I2C1, I2C_Register_SR1); 
   
 I2C_Send7bitAddress(I2C1,AT24Cx_Address, I2C_Direction_Transmitter);
  }while(!(I2C_ReadRegister(I2C1, I2C_Register_SR1) & 0x0002)); 
 
 
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

#include "includes.h"
#include "delay.h"
#include "libe2prom.h"


#define I2C                   I2C2

#define AT24Cx_Address           0xa6 
#define AT24Cx_PageSize          8  

struct eeprom_key_info {
	char sn[16];
	char user_id[16];
	char ip_suffix[3];
	char blod_def[3];
	char menoy[3];
};

enum {
	KEY_UNINSERT = 0,
	KEY_INSERTING,
	KEY_INSERTED,
	KEY_UNUSED,
};

static struct eeprom_key_info key;

char eeprom[] = "SN145784541458720000018092719086250100100";

void key_Reads(u8 Address, u8 *ReadBuffer, u16 ReadNumber)
{
	e2prom_Reads(I2C, AT24Cx_Address, Address, ReadBuffer, ReadNumber);
}

void key_Writes(u8 Address, u8 *WriteData, u16 WriteNumber)
{
	e2prom_Writes(I2C, AT24Cx_Address, Address, WriteData, WriteNumber);
}

#if 0

void key_Reads(u8 Address,u8 *ReadBuffer,u16 ReadNumber)
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

void key_WriteByte(u8 Address,u8 WriteData)
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

void key_WritePage(u8 Address,u8 *WriteData,u16 WriteNumber)
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

void key_Writes(u8 Address,u8 *WriteData,u16 WriteNumber)
{
	u8 Temp;


	Temp=Address%AT24Cx_PageSize; 
	if(Temp)       
	{
		Temp=AT24Cx_PageSize-Temp;  
		key_WritePage(Address,WriteData,Temp);
		Address+=Temp;
		WriteData+=Temp;
		WriteNumber-=Temp;
		I2C_AT24Cx_WaitForComplete(); 
	}

	while(WriteNumber)
	{

		if(WriteNumber>=AT24Cx_PageSize)
		{
			key_WritePage(Address,WriteData,AT24Cx_PageSize);
			Address+=AT24Cx_PageSize;
			WriteData+=AT24Cx_PageSize;
			WriteNumber-=AT24Cx_PageSize;
			I2C_AT24Cx_WaitForComplete();
		}

		else
		{
			key_WritePage(Address,WriteData,WriteNumber);
			WriteNumber=0;
			I2C_AT24Cx_WaitForComplete();
		}
	}
}

void key_test(void)
{
	char a[]="abcdefghijklmnopqstuvwxyz0123456789";
	
	key_Writes(0, (u8 *)a, sizeof(a));
	memset(a, 0, sizeof(a));
	key_Reads(0, (u8 *)a, sizeof(a));
}

#endif

void read_key_from_eeprom(void)
{
	struct eeprom_key_info tmp_key;
	
	memcpy(&key, eeprom, strlen(eeprom));
	return;
	
	key_Reads(0, (u8 *)&key, sizeof(tmp_key));
	tmp_key = key;
	
	int2chars(tmp_key.blod_def, 0, sizeof(tmp_key.blod_def));
	
	key_Writes(0, (u8 *)&tmp_key, sizeof(tmp_key));
}

void get_key_sn(char *s)
{
	memcpy(s, key.sn, 16);
}

void get_ip_suffix(char *s)
{
	memcpy(s, key.ip_suffix, 3);
}

s8 get_key_blod(void)
{
	return char2u32(key.blod_def, sizeof(key.blod_def));	
}

s8 get_key_gpio(void)
{	
	//return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET;
	return 1;
}

static s8 status = KEY_UNINSERT;

s8 key_state_machine(void)
{
	s8 ret = 0;
	
	switch (status) {
		case KEY_UNINSERT:
			if (get_key_gpio())
				status = KEY_INSERTING;
			break;
			
		case KEY_INSERTING:
			if (get_key_gpio())
				status = KEY_INSERTED;
			break;
			
		case KEY_INSERTED:
			read_key_from_eeprom();
			status = KEY_UNUSED;
			ret = 1;
			// fall through
		case KEY_UNUSED:
			if (get_key_gpio() == 0)
				status = KEY_UNINSERT;
			break;
	}
	
	return ret;
}

void key_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
 	GPIO_Init(GPIOA, &GPIO_InitStructure);	
}

void key_test(void)
{
	char a[]="SN145784541458720000018092719086250100100";
	
	key_Writes(0, (u8 *)a, sizeof(a));
	memset(a, 0, sizeof(a));
	key_Reads(0, (u8 *)a, sizeof(a));
}

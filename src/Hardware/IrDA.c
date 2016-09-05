#include "includes.h"
#include "helper.h"
#include "time.h"
#include "blod.h"

#if 0

#define I2C_OR_OP		1
#define I2C_OW_OP		2
#define I2C_RW_OP		3
#define I2C_WW_OP		4

#define I2C_OP_STEP_1	1
#define I2C_OP_STEP_2	2

struct i2c_buf {
	volatile u8 op;
	volatile u8 step;
	volatile u8 reg_addr;
	volatile u8 slave_addr;
	u8 *buf;
	volatile u8 idx;
	u16 len;
	OS_SEM sem;
};

static struct i2c_buf i2c_buf;

static volatile s8 i2c_status;

#if 1
void e2prom_Reads(I2C_TypeDef *I2C, u8 slave_addr, u8 Address, u8 *ReadBuffer,u16 ReadNumber)
{	
	OS_ERR err;

	if (ReadNumber <= 0 || ReadNumber > 256)
		return;
	
	i2c_buf.op = I2C_RW_OP;
	i2c_buf.step = I2C_OP_STEP_1;
	i2c_buf.slave_addr = slave_addr;
	i2c_buf.reg_addr = Address;
	i2c_buf.idx = 0;
	i2c_buf.buf = ReadBuffer;
	i2c_buf.len = ReadNumber;
	
	I2C_ITConfig(I2C2, I2C_IT_EVT, ENABLE);
	I2C_ITConfig(I2C2, I2C_IT_BUF, ENABLE);
	
	while(I2C_GetFlagStatus(I2C, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART(I2C, ENABLE);
				
	OSSemPend(&i2c_buf.sem, NULL, OS_OPT_PEND_BLOCKING, NULL, &err);
}
#endif

void e2prom_WriteByte(I2C_TypeDef *I2C, u8 slave_addr, u8 Address, u8 WriteData)
{
	OS_ERR err;
	
	i2c_buf.op = I2C_WW_OP;
	i2c_buf.step = I2C_OP_STEP_1;
	i2c_buf.slave_addr = slave_addr;
	i2c_buf.reg_addr = Address;
	i2c_buf.idx = 0;
	i2c_buf.buf = &WriteData;
	i2c_buf.len = 1;
	
	I2C_ITConfig(I2C2, I2C_IT_EVT, ENABLE);
	I2C_ITConfig(I2C2, I2C_IT_BUF, ENABLE);
	
	while(I2C_GetFlagStatus(I2C, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART(I2C, ENABLE);
	
	OSSemPend(&i2c_buf.sem, NULL, OS_OPT_PEND_BLOCKING, NULL, &err);
	
	msleep(5);
}

void e2prom_WritePage(I2C_TypeDef *I2C, u8 slave_addr, u8 Address, u8 *WriteData, u16 WriteNumber)
{
	OS_ERR err;

	if (WriteNumber <= 0 || WriteNumber > AT24Cx_PageSize) //page size is 8 bytes
		return;
	
	i2c_buf.op = I2C_WW_OP;
	i2c_buf.step = I2C_OP_STEP_1;
	i2c_buf.slave_addr = slave_addr;
	i2c_buf.reg_addr = Address;
	i2c_buf.idx = 0;
	i2c_buf.buf = WriteData;
	i2c_buf.len = WriteNumber;
	
	I2C_ITConfig(I2C2, I2C_IT_EVT, ENABLE);
				I2C_ITConfig(I2C2, I2C_IT_BUF, ENABLE);
	while(I2C_GetFlagStatus(I2C, I2C_FLAG_BUSY));
	
	I2C_GenerateSTART(I2C, ENABLE);
	
	OSSemPend(&i2c_buf.sem, NULL, OS_OPT_PEND_BLOCKING, NULL, &err);
	
	msleep(5);
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
		//e2prom_WaitForComplete(I2C, slave_addr); 
	}

	while (WriteNumber) {
		if (WriteNumber >= AT24Cx_PageSize) {
			e2prom_WritePage(I2C, slave_addr, Address, WriteData, AT24Cx_PageSize);
			Address += AT24Cx_PageSize;
			WriteData += AT24Cx_PageSize;
			WriteNumber -= AT24Cx_PageSize;
			//e2prom_WaitForComplete(I2C, slave_addr);
		}

		else {
			e2prom_WritePage(I2C, slave_addr, Address, WriteData, WriteNumber);
			WriteNumber = 0;
			//e2prom_WaitForComplete(I2C, slave_addr);
		}
	}
}

void I2C2_EV_IRQHandler(void)
{
	CPU_SR_ALLOC();
	
	CPU_INT_DIS(); 
	switch (I2C_GetLastEvent(I2C2)) {
		case I2C_EVENT_MASTER_MODE_SELECT:
			I2C_AcknowledgeConfig(I2C2, ENABLE);
		
			if (i2c_buf.op == I2C_RW_OP) {
				if (i2c_buf.step == I2C_OP_STEP_1)
					I2C_Send7bitAddress(I2C2, i2c_buf.slave_addr, I2C_Direction_Transmitter);
				else
					I2C_Send7bitAddress(I2C2, i2c_buf.slave_addr, I2C_Direction_Receiver);
				
			} else if (i2c_buf.op == I2C_WW_OP)
				I2C_Send7bitAddress(I2C2, i2c_buf.slave_addr, I2C_Direction_Transmitter);
			
			break;
		case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:
			break;
		
		case I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED:
			if (i2c_buf.len == 1) {
				I2C_AcknowledgeConfig(I2C2, DISABLE);
				I2C_GenerateSTOP(I2C2, ENABLE);
				break;
			}
			
			break;
		
		case I2C_EVENT_MASTER_BYTE_TRANSMITTING:
			if (i2c_buf.op == I2C_WW_OP && i2c_buf.step == I2C_OP_STEP_1) {
				I2C_SendData(I2C2, i2c_buf.reg_addr);
				i2c_buf.step = I2C_OP_STEP_2;
				
				break;
			}
			
			if (i2c_buf.op == I2C_WW_OP && i2c_buf.step == I2C_OP_STEP_2) {
				if (i2c_buf.idx < i2c_buf.len)
					I2C_SendData(I2C2, i2c_buf.buf[i2c_buf.idx++]);
				
				break;
			}
			
			if (i2c_buf.op == I2C_RW_OP && i2c_buf.step == I2C_OP_STEP_1) {
				I2C_SendData(I2C2, i2c_buf.reg_addr);
				i2c_buf.step = I2C_OP_STEP_2;
				
				break;
			}
			
			break;
		case I2C_EVENT_MASTER_BYTE_TRANSMITTED:
			if (i2c_buf.op == I2C_RW_OP)
				I2C_GenerateSTART(I2C2, ENABLE);
			else if (i2c_buf.op == I2C_WW_OP) {
				OS_ERR err;
				
				if (i2c_buf.idx == i2c_buf.len) {
					I2C_GenerateSTOP(I2C2, ENABLE);
					I2C_ITConfig(I2C2, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
					OSSemPost(&i2c_buf.sem, OS_OPT_POST_ALL, &err);
				}
			}
			
			break;
		case I2C_EVENT_MASTER_BYTE_RECEIVED:
			i2c_buf.buf[i2c_buf.idx++] = I2C_ReceiveData(I2C2);
		
			if ((u16)i2c_buf.idx == (i2c_buf.len - 1)) {
				I2C_AcknowledgeConfig(I2C2, DISABLE);
				I2C_GenerateSTOP(I2C2, ENABLE);
				break;
			}
			
			if ((u16)i2c_buf.idx == i2c_buf.len) {
				OS_ERR err;
				
				I2C_ITConfig(I2C2, I2C_IT_BUF | I2C_IT_EVT, DISABLE);
				OSSemPost(&i2c_buf.sem, OS_OPT_POST_ALL, &err);	
				
				break;
			}
		break;
	}
	
	CPU_INT_EN();
}

void e2prom_init(void)
{
	OS_ERR err;
	
	OSSemCreate(&i2c_buf.sem, "blod Sem", 0, &err);
}
#endif

#define I2C_OP_CODE_GET_INFO	0
#define I2C_OP_CODE_LED_COLOR	1
#define I2C_OP_CODE_SMOKING		2

void irda_init(void)
{

}

#if 1
void IrDA_Reads(I2C_TypeDef *I2C, u8 slave_addr, u8 op, u8 *ReadBuffer, u16 ReadNumber)
{
	if(ReadNumber == 0)  
		return;

	while(I2C_GetFlagStatus(I2C, I2C_FLAG_BUSY));

	I2C_AcknowledgeConfig(I2C, ENABLE);

	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT));   

	I2C_Send7bitAddress(I2C, slave_addr, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 

	I2C_SendData(I2C, op);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	
	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT));

	I2C_Send7bitAddress(I2C, slave_addr, I2C_Direction_Receiver);  
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

	while (ReadNumber) {
		int timeout = 7200;
		if (ReadNumber == 1) {
			I2C_AcknowledgeConfig(I2C, DISABLE);  
			I2C_GenerateSTOP(I2C, ENABLE); 
		}

		while (!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_RECEIVED) && timeout--); 
		*ReadBuffer = I2C_ReceiveData(I2C);
		ReadBuffer++;
		ReadNumber--;
	}
	
	I2C_AcknowledgeConfig(I2C, ENABLE);
}
#endif
#if 1

void IrDA_WriteBytes(I2C_TypeDef *I2C, u8 slave_addr, u8 op, u8 *WriteData, u16 WriteNumber)
{
	while(I2C_GetFlagStatus(I2C, I2C_FLAG_BUSY));

	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT));

	I2C_Send7bitAddress(I2C, slave_addr, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(I2C, op);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

	while (WriteNumber--)  {
		I2C_SendData(I2C, *WriteData);
		WriteData++;
		while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}

	I2C_GenerateSTOP(I2C, ENABLE);
}
#endif

void IrDA_test(void)
{
	u8 buf[2];
	u16 charcode;

	retry:
	msleep(20);
	IrDA_Reads(I2C1, 0x30, I2C_OP_CODE_GET_INFO, buf, 2);
	
	charcode = buf[1] << 8 | buf[0];
	
	if (charcode == 0x55aa)
		return;
	
	else
		goto retry;
}

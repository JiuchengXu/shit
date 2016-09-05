#include "includes.h"
#include "helper.h"

#define I2C                   I2C2

/* ADX345��I2C��ַ */
#define ADX345_ADDR        0xA6   //ADX345��I2C��ַ

/* ADXL345��ָ��� */
#define ADX_DEVID          0x00   //����ID
#define ADX_DATA_FORMAT    0x31   //���ݸ�ʽ����
#define ADX_BW_RATE        0x2C   //�������ʼ�����ģʽ����
#define ADX_POWER_CTL      0x2D   //ʡ�����Կ���
#define ADX_INT_ENABLE     0x2E   //�ж�ʹ�ܿ���
#define ADX_OFSX           0x1E   //�û���ֵ
#define ADX_OFSY           0x1F   //X��ƫ��
#define ADX_OFSZ           0x20   //Y��ƫ��


#define ADX345_DelayMs(x)     {msleep(x);}  //��ʱ����

/****************************************************************************
* Function Name  : ADX345_WriteReg
* Description    : ����ADX345�Ĵ���
* Input          : addr���Ĵ�����ַ
*                * dat����������
* Output         : None
* Return         : None
****************************************************************************/

static int8_t ADX345_WriteReg(uint8_t addr, uint8_t dat)
{
	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT)); 


	I2C_Send7bitAddress(I2C, ADX345_ADDR, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); 


	I2C_SendData(I2C, addr);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));


	I2C_SendData(I2C, dat);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED));


	I2C_GenerateSTOP(I2C, ENABLE);  
    return 0;   
}

/****************************************************************************
* Function Name  : ADX345_ReadReg
* Description    : ��ȡADX345�Ĵ���
* Input          : addr����ȡ��ַ
* Output         : None
* Return         : ��ȡ����8λ����
****************************************************************************/

static uint8_t ADX345_ReadReg(uint8_t addr)
{
	uint8_t ret;
	
	while(I2C_GetFlagStatus(I2C,I2C_FLAG_BUSY));


	I2C_AcknowledgeConfig(I2C, ENABLE);


	I2C_GenerateSTART(I2C,ENABLE);
	while(!I2C_CheckEvent(I2C,I2C_EVENT_MASTER_MODE_SELECT));   

	I2C_Send7bitAddress(I2C, ADX345_ADDR, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));


	I2C_SendData(I2C, addr);
	while (!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 

	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT));


	I2C_Send7bitAddress(I2C, ADX345_ADDR, I2C_Direction_Receiver);  
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));


	I2C_AcknowledgeConfig(I2C, DISABLE);  
	I2C_GenerateSTOP(I2C,ENABLE); 


	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_RECEIVED)); 
	
	ret = I2C_ReceiveData(I2C);
	
	I2C_AcknowledgeConfig(I2C, ENABLE);

    return ret;
}

static uint8_t ADX345_ReadReg_M(uint8_t addr, uint8_t *buf, uint8_t len)
{
	uint8_t i;
	
	if (len <= 0)
		return 0;
	
	while(I2C_GetFlagStatus(I2C,I2C_FLAG_BUSY));


	I2C_AcknowledgeConfig(I2C, ENABLE);


	I2C_GenerateSTART(I2C,ENABLE);
	while(!I2C_CheckEvent(I2C,I2C_EVENT_MASTER_MODE_SELECT));   

	I2C_Send7bitAddress(I2C, ADX345_ADDR, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	I2C_SendData(I2C, addr);
	while (!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); 

	I2C_GenerateSTART(I2C, ENABLE);
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT));

	I2C_Send7bitAddress(I2C, ADX345_ADDR, I2C_Direction_Receiver);  
	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

	for (i = 0; i < len; i++) {
		while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_RECEIVED)); 
		
		buf[i] = I2C_ReceiveData(I2C);
	}

	I2C_AcknowledgeConfig(I2C, DISABLE);  
	I2C_GenerateSTOP(I2C,ENABLE); 

	I2C_AcknowledgeConfig(I2C, ENABLE);

	return len;
}

/****************************************************************************
* Function Name  : ADX345_ReadXYZ
* Description    : ��ȡX,Y,Z��ADֵ
* Input          : xValue��X��ı����ַ
*                * yValue��Y��ı����ַ
*                * zValue��Z��ı����ַ
* Output         : None
* Return         : 0����ȡ�ɹ���0xFF����ȡʧ��
****************************************************************************/

static int8_t ADX345_ReadXYZ(int16_t *xValue, int16_t *yValue, int16_t *zValue)
{
	uint8_t readValue[6];
	
	ADX345_ReadReg_M(0x32, readValue, sizeof(readValue));
	
    /* �����ȡ�������� */
	*xValue = (uint16_t)(readValue[1] << 8) + readValue[0]; 	    
	*yValue = (uint16_t)(readValue[3] << 8) + readValue[2]; 	    
	*zValue = (uint16_t)(readValue[5] << 8) + readValue[4]; 	   

   return 0;
}

/****************************************************************************
* Function Name  : ADX345_Init
* Description    : ��ʼ��ADX345�����˶�ADX��ID
* Input          : None
* Output         : None
* Return         : 0���ɹ���0xFF��ʧ��
****************************************************************************/

int ADX345_Init(void)
{
    if(ADX345_ReadReg(ADX_DEVID) == 0xE5)
    {
        ADX345_WriteReg(ADX_DATA_FORMAT, 0x2B);//13λȫ�ֱ���,��������Ҷ���,16g���� 
		ADX345_WriteReg(ADX_BW_RATE, 0x0A);	  //��������ٶ�Ϊ100Hz
		ADX345_WriteReg(ADX_POWER_CTL, 0x28);  //����ʹ��,����ģʽ
		ADX345_WriteReg(ADX_INT_ENABLE, 0x00); //��ʹ���ж�		 
	 	ADX345_WriteReg(ADX_OFSX, 0x00);       //�û���ֵ
		ADX345_WriteReg(ADX_OFSY, 0x00);       //X��ƫ��
		ADX345_WriteReg(ADX_OFSZ, 0x00);       //Y��ƫ��
        return 0;
    }
   
    return 0xFF; //���س�ʼ��ʧ��
}

/****************************************************************************
* Function Name  : ADX345_Adjust
* Description    : ADX345����У����
* Input          : None
* Output         : None
* Return         : None
****************************************************************************/

void ADX345_Adjust(void)
{
    int32_t offx = 0, offy = 0, offz = 0;
    int16_t xValue, yValue, zValue;
    uint8_t i;

	ADX345_WriteReg(ADX_POWER_CTL, 0x00);	 //�Ƚ�������ģʽ.
	ADX345_DelayMs(100);
	ADX345_Init(); 
    ADX345_DelayMs(20);
    
    /* ��ȡʮ����ֵ */
    for(i=0; i<10; i++)
    {
        ADX345_ReadXYZ(&xValue, &yValue, &zValue);
        offx += xValue;
        offy += yValue;
        offz += zValue;
        ADX345_DelayMs(10);   //����Ƶ����100HZ��10ms����һ�����         
    }
    
    /* ���ƽ��ֵ */
    offx /= 10;
    offy /= 10;
    offz /= 10;
    
    /* ȫ�ֱ����£�ÿ�����LSBΪ3.9 mg��ƫ�ƼĴ���LSB���ķ�֮һ�����Գ���4 */
    xValue = -(offx / 4);
	yValue = -(offy / 4);
	zValue = -((offz - 256) / 4);
    
    /* ����ƫ���� */
	ADX345_WriteReg(ADX_OFSX, xValue);
	ADX345_WriteReg(ADX_OFSY, yValue);
	ADX345_WriteReg(ADX_OFSZ, zValue); 
            
}

/****************************************************************************
* Function Name  : ADX_GetXYZData
* Description    : ��ȡADX��XYZ���ֵ�����й����ݴ���
* Input          : xValue��X��ı����ַ
*                * yValue��Y��ı����ַ
*                * zValue��Z��ı����ַ
* Output         : None
* Return         : None
****************************************************************************/

void ADX_GetXYZData(int16_t *xValue, int16_t *yValue, int16_t *zValue)
{
    int32_t xTotal = 0, yTotal = 0, zTotal = 0;
    uint8_t i;
    
    *xValue = 0;
    *yValue = 0;
    *zValue = 0;

    /* ��ȡʮ�β���ֵ */
    for(i=0; i<10; i++)
    {
        ADX345_ReadXYZ(xValue, yValue, zValue);
        xTotal += *xValue;
        yTotal += *yValue;
        zTotal += *zValue;
        ADX345_DelayMs(10);  //����Ƶ����100HZ��10ms����һ����� 
    }
    
    /* ���ƽ��ֵ */
    *xValue = xTotal / 10;
    *yValue = yTotal / 10;
    *zValue = zTotal / 10;       
}

/****************************************************************************
* Function Name  : ADX_GetAngle
* Description    : ��ADֵת���ɽǶ�ֵ
* Input          : xValue��x��ֵ
*                * yValue��y��ֵ
*                * zValue��z��ֵ
*                * dir��0����Z��ĽǶ�;1����X��ĽǶ�;2����Y��ĽǶ�.
* Output         : None
* Return         : None
****************************************************************************/

int16_t ADX_GetAngle(float xValue, float yValue, float zValue, uint8_t dir)
{
	float temp;
 	float res = 0;

	switch(dir)
	{   
        /* ����ȻZ��ĽǶ� */
		case 0:
 			temp = sqrt((xValue * xValue + yValue * yValue)) / zValue;
 			res = atan(temp);
 			break;
        
        /* ����ȻX��ĽǶ� */
		case 1:
 			temp = xValue / sqrt((yValue * yValue + zValue * zValue));
 			res = atan(temp);
 			break;

        /* ����ȻY��ĽǶ� */
 		case 2:
 			temp = yValue / sqrt((xValue * xValue + zValue * zValue));
 			res = atan(temp);
 			break;

        default:
            break;
 	}

    res = res * 1800 / 3.14; 

	return res;
}

#define ADX345_CS		GPIOC
#define ADX345_CS_Pin	GPIO_Pin_11

void adx345_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); 
	
	GPIO_InitStructure.GPIO_Pin    = ADX345_CS_Pin;
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode   = GPIO_Mode_Out_PP;
	
	GPIO_Init(ADX345_CS, &GPIO_InitStructure);
	
	GPIO_SetBits(ADX345_CS, GPIO_Pin_11);		
}

void adx345_init(void)
{
	int16_t Xval, Yval, Zval, Xang, Yang, Zang;
	int32_t i;
	
	adx345_gpio_init();
	
	while (ADX345_Init()) {
		msleep(100);
	}
	
	ADX345_Adjust();
	
	while (1) {
		i++;
        if(i > 0x02FFFF)
        {
            /* ��ȡX,Y,Z�ļ��ٶ�ֵ */
            ADX_GetXYZData(&Xval, &Yval, &Zval);

            /* ����ȡ���ļ��ٶ�ֵת��Ϊ�Ƕ�ֵ */
            Xang=ADX_GetAngle(Xval, Yval, Zval,1);    
			Yang=ADX_GetAngle(Xval, Yval, Zval,2);   
			Zang=ADX_GetAngle(Xval, Yval, Zval,0);
			
			Xang = Xang;
			Yang = Yang;
			Zang = Zang;
			
            i = 0; 
        }
	}
}

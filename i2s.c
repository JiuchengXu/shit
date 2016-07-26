#include "stm32f10x.h"
#include "i2s.h"

#if 0
static void I2S_GPIO_Config(void);
static void I2S_Mode_Config(void);

static uint8_t  buffer[1024*4]; 		// 文件缓冲区
static short   outBuf[2][2500];		    // PCM流缓冲，使用两个缓冲区 
static int bufflag = 0;
#if 0
/**
  * @brief  I2S_Bus_Init 初始化iis总线
  * @param  none
  * @retval none
  */
void I2S_Bus_Init(void)
{
	/* 配置I2S GPIO用到的引脚 */
	I2S_GPIO_Config(); 

	/* 配置I2S工作模式 */
	I2S_Mode_Config();

	I2S_Cmd(SPI2, ENABLE);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
}


/**
  * @brief  I2S_Stop 停止iis总线
  * @param  none
  * @retval none
  */
void I2S_Stop(void)
{		
	/* 禁能 SPI2/I2S2 外设 */
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, DISABLE);
	I2S_Cmd(SPI2, DISABLE);	

	DMA_Cmd(DMA1_Channel5, DISABLE);
}


/**
  * @brief  I2S_Freq_Config 根据采样频率配置iis总线，在播放音频文件时可从文件中解码获取
  * @param  SampleFreq 采样频率
  * @retval none
  */
void I2S_Freq_Config(uint16_t SampleFreq)
{
	I2S_InitTypeDef I2S_InitStructure;

	I2S_Stop();
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;					// 配置I2S工作模式
	I2S_InitStructure.I2S_Standard = I2S_Standard_LSB;				// 接口标准 
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;			// 数据格式，16bit 
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;		// 主时钟模式 
	I2S_InitStructure.I2S_AudioFreq = SampleFreq;					// 音频采样频率 
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;  			
	I2S_Init(SPI2, &I2S_InitStructure);
	
	I2S_Cmd(SPI2, ENABLE);	//使能iis总线
}


/**
  * @brief  DMA_I2S_Configuration 配置DMA总线
  * @param  addr:数据源地址
	*	@param	size:要传输的数据大小
  * @retval none
  */
void DMA_I2S_Configuration(uint32_t addr, uint32_t size)
{
    DMA_InitTypeDef DMA_InitStructure;				  

    /* DMA 通道配置*/
    DMA_Cmd(DMA1_Channel5, DISABLE);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&(SPI2->DR));		//目的地址，iis的数据寄存器
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) addr;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = size;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);	//使能DMA请求

    DMA_Cmd(DMA1_Channel5, ENABLE);
}

/**
  * @brief  I2S_GPIO_Config 配置IIS总线用到的GPIO引脚
  * @param  none
  * @retval none
  */
static void I2S_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable GPIOB, GPIOC */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB , ENABLE);
	
	/* 打开 I2S2 APB1 时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	
	/* I2S2 SD, CK  WS 配置 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* I2S2 MCK  */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

/**
  * @brief  I2S_Mode_Config 配置IIS总线的工作模式(默认采样频率)
  * @param  none
  * @retval none
  */
static void I2S_Mode_Config(void)
{
	I2S_InitTypeDef I2S_InitStructure; 
			
	/* I2S2 外设配置 */
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;					// 配置I2S工作模式 
	I2S_InitStructure.I2S_Standard = I2S_Standard_LSB;				// 接口标准 
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;			// 数据格式，16bit
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;		// 主时钟模式 
	I2S_InitStructure.I2S_AudioFreq = I2S_AudioFreq_44k;			// 音频采样频率
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;  					// 默认为低	
	I2S_Init(SPI2, &I2S_InitStructure);

}

static void mp3_player(const char *filename)
{
	int err, i, outputSamps, current_sample_rate = 0;	
	int flag = 1;	//读完一个文件跳出大循环的标志
	int count = 0;//循环次数，设置音量时需要
	int				read_offset = 0;				/* 读偏移指针*/
	int				bytes_left = 0;					/* 剩余字节数*/	
	unsigned long	Frames = 0;						/* mP3帧计数*/
	unsigned char	*read_ptr = buffer;				/* 缓冲区指针*/
	HMP3Decoder		Mp3Decoder;						/* mp3解码器指针*/
	
	while (1)
	{		
		//读取mp3文件
		fres = f_read(&file, buffer, sizeof(buffer), &rw_num);
		if(fres != FR_OK)
		{
			printf("读取%s失败！ %d\r\n",filename,fres);
			return;
		}
		
		read_ptr = buffer;									//指向mp3输入流
		bytes_left = rw_num;								//实际读到的输入流大小大小

		//按帧处理	
		while(1)	//循环2
		{							
				if (outputSamps > 0)
				{
					if (Mp3FrameInfo.nChans == 1)	//单声道
					{
						//单声道数据需要复制一份到另一个声道
						for (i = outputSamps - 1; i >= 0; i--)
						{
							outBuf[bufflag][i * 2] = outBuf[bufflag][i];
							outBuf[bufflag][i * 2 + 1] = outBuf[bufflag][i];
						}
						outputSamps *= 2;
					}
				
					//非单声道数据可直接由DMA传输到IIS交给DAC
					/* 等待DMA播放完，这段时间我们可以干其他的事，扫描事件进行处理 */
					while((DMA1_Channel5->CCR&DMA_CCR1_EN) && !(DMA1->ISR&DMA1_IT_TC5));
						
					/*DMA传输完毕*/
					DMA_ClearFlag(DMA1_FLAG_TC5 | DMA1_FLAG_TE5);
					DMA_I2S_Configuration((uint32_t)outBuf[bufflag], outputSamps);
					bufflag = 1 - bufflag;	//切换buffer
					
					//加上以下这两行代码就可以初始化播放音量,神奇之处!!
					//意思是要执行两次上面的代码才能使设置音量有效，估计是DMA传输要执行两次，一次不行~~~根本原因不详~~
					if (count++ == 1)
						PCM1770_VolumeSet(30);
				}
			}
		}
	}

	I2S_Stop();
}

#endif
#endif
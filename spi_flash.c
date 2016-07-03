#include "stm32f10x.h"

#define FLASH_SECTOR_SIZE	512
#define FLASH_SECTOR_COUNT	4096


/* Select SPI FLASH: ChipSelect pin low  */
#define Select_Flash()     GPIO_ResetBits(GPIOA, GPIO_Pin_2)
/* Deselect SPI FLASH: ChipSelect pin high */
#define NotSelect_Flash()    GPIO_SetBits(GPIOA, GPIO_Pin_2)

#define FLASH_CS_0()     GPIO_ResetBits(GPIOA,GPIO_Pin_2)
#define FLASH_CS_1()     GPIO_SetBits(GPIOA,GPIO_Pin_2)

#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F

#define Dummy_Byte								0xA5

void spi1_init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable SPI1 GPIOA and GPIOB clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure SPI1 pins: NSS, SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PB.02 as Output push-pull, used as Flash Chip select */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Deselect the FLASH: Chip Select high */
	NotSelect_Flash();

	/* SPI1 configuration */ 
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

	//SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx,ENABLE);

	/* Enable SPI2 Rx buffer DMA transfer request */
	//  SPI_DMACmd(SPI1, SPI_DMAReq_Rx | SPI_DMAReq_Tx, ENABLE);

	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);  
}

int32_t SPIFLASH_disk_initialize(void)
{
	spi1_init();
	return 0;

}

uint8_t spi_write_byte(uint8_t data)
{
	/*u32 Count = 0;*/
  /* Loop while DR register in not emplty */
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  /* Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(SPI1, data);

  /* Wait to receive a byte */
  while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
 

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
}


uint8_t spi_read_byte(void)
{
  return (spi_write_byte(Dummy_Byte));
}

void FlashWaitBusy(void)
{
	NotSelect_Flash();
	Select_Flash();
	  
	spi_write_byte(W25X_ReadStatusReg);    //???????????     
	
	while ((spi_read_byte() & 0x01) == 0x01);
	
	NotSelect_Flash();	
}

void flash_page_read_si(uint32_t sector,uint8_t *data)
{
	uint32_t i;

	u32 ReadAddr = sector * FLASH_SECTOR_SIZE;
	
	FlashWaitBusy();
	Select_Flash();

    spi_write_byte(W25X_ReadData);         //??????   
    spi_write_byte((u8)((ReadAddr)>>16));  //??24bit??    
    spi_write_byte((u8)((ReadAddr)>>8));   
    spi_write_byte((u8)ReadAddr);   
	
    for(i = 0; i < FLASH_SECTOR_SIZE; i++)
        data[i] = spi_read_byte();   //????  
	
	NotSelect_Flash();
}

void flash_page_read_mu(uint32_t sector,uint8_t count,uint8_t *data)
{
	uint32_t i;
	
	for (i = 0; i < count; i++)
	{
		flash_page_read_si(sector+i,((uint8_t *)data + FLASH_SECTOR_SIZE * i));
	}	
	
}
void flash_page_write_si(uint32_t sector,uint8_t *data)
{
	uint16_t i;	
	u32 WriteAddr = sector * FLASH_SECTOR_SIZE;

	FLASH_CS_0();

    spi_write_byte(W25X_PageProgram);      //??????   
    spi_write_byte((u8)((WriteAddr)>>16)); //??24bit??    
    spi_write_byte((u8)((WriteAddr)>>8));   
    spi_write_byte((u8)WriteAddr);   
	
    for(i = 0; i < FLASH_SECTOR_SIZE; i++)
		spi_write_byte(data[i]);

	FLASH_CS_1();

	FlashWaitBusy();	

}
void flash_page_write_mu(uint32_t sector,uint8_t count,uint8_t *data)
{
	uint32_t index;

	for (index = 0; index < count; index++)
	{
		/* only supply single block write: block size 512Byte */
		flash_page_read_si((sector + index), ((uint8_t *) data + index * FLASH_SECTOR_SIZE));
	}
}

u32 SPI_FLASH_ReadDeviceID(void)
{
  u32 Temp = 0;

  /* Select the FLASH: Chip Select low */
  Select_Flash();   //????,?????

  /* Send "RDID " instruction */
  spi_write_byte(0x90);//??????ID,???????????ID?(??????) 0XAB
  spi_write_byte(Dummy_Byte);     //??3???????,25X16????24??,??????????,?????????DEVICEID!
  spi_write_byte(Dummy_Byte);
  spi_write_byte(0);
  
  /* Read a byte from the FLASH */
  Temp = spi_read_byte();
  Temp <<= 8;
  Temp |= spi_read_byte(); 

  /* Deselect the FLASH: Chip Select high */
  NotSelect_Flash();

  return Temp;
}

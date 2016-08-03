#ifndef __WAVE_PLAYER_H
#define __WAVE_PLAYER_H

//#include "includes.h"

typedef enum
{
	LittleEndian,
	BigEndian
} Endianness;

typedef struct
{
	u32  RIFFchunksize;
	u16  FormatTag;
	u16  NumChannels;
	u32  SampleRate;
	u32  ByteRate;
	u16  BlockAlign;
	u16  BitsPerSample;
	u32  DataSize;
} WAVE_TypeDef;

/* Exported constants --------------------------------------------------------*/
#define  CHUNK_ID                            0x52494646  /* correspond to the letters 'RIFF' */
#define  FILE_FORMAT                         0x57415645  /* correspond to the letters 'WAVE' */
#define  FORMAT_ID                           0x666D7420  /* correspond to the letters 'fmt ' */
#define  DATA_ID                             0x64617461  /* correspond to the letters 'data' */
#define  FACT_ID                             0x66616374  /* correspond to the letters 'fact' */
#define  WAVE_FORMAT_PCM                     0x01
#define  FORMAT_CHNUK_SIZE                   0x10
#define  CHANNEL_MONO                        0x01
#define  CHANNEL_STEREO                      0x02
#define  SAMPLE_RATE_8000                    8000
#define  SAMPLE_RATE_11025                   11025
#define  SAMPLE_RATE_22050                   22050
#define  SAMPLE_RATE_44100                   44100
#define  BITS_PER_SAMPLE_8                   8
#define  BITS_PER_SAMPLE_16                  16
#define DMA_MAX_SZE          0xFFFF
#define I2S_ENABLE_MASK      0x0400
#define DMA_MAX(x)           (((x) <= DMA_MAX_SZE)? (x):DMA_MAX_SZE)

void AUDIO_Init(u32 AudioFreq);
void I2S_GPIO_Init(void);
void Audio_I2S_Init(u32 AudioFreq);
void Audio_DMA_Init(void);
uint32_t AUDIO_Play(u16* pBuffer, u32 Size);
void Audio_MAL_Play(u32 Addr, u32 Size);

void Wavplay(void);
u8 WaveParsing(void);
u32 ReadUnit(u8 *buffer, u8 idx, u8 NbrOfBytes, Endianness BytesFormat);

#endif 

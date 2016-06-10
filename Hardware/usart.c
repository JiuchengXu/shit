#include "includes.h"
#include "usart.h"
#include "lib_str.h"
#include "bus.h"

static OS_SEM  UartSem;

#define BUF_LENGTH	2048
static char RxBuf[BUF_LENGTH];
static int w_index, r_start;

//重定义fputc函数 

int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}

void USART1_IRQHandler(void) 
{ 
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		USART_ClearFlag(USART1,USART_FLAG_ORE);  //读 SR
		RxBuf[w_index++] = USART_ReceiveData(USART1);
			
		if (w_index == BUF_LENGTH)
			w_index = 0;
	}
}

void uart_putc(char c)
{
    USART_SendData(USART1, c);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void uart_puts(char *str)
{
    while(*str)
    {
       USART_SendData(USART1, *str++);
       while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    }
}

static char uart_recieve(void)
{
	char c =  RxBuf[r_start++];
	
	RxBuf[r_start - 1] = 0;
	
	if (r_start == BUF_LENGTH)
		r_start = 0;
	
	if (r_start == w_index)
		return '\0';
	
	return c;
}

static void uart_send(char *buf, int len)
{
	int i;
	
	for (i = 0; i < len; i++) {
		uart_putc(buf[i]);
	}	
}

void uart_inint(void)
{
	OS_ERR err;
	u32 bound = 115200;
	
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode =USART_Mode_Rx|USART_Mode_Tx;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
	USART_Init(USART1, &USART_InitStructure);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); 

	OSSemCreate(&UartSem, "Uart Sem", 0, &err);
	
	register_bus(uart_send, uart_recieve);
}

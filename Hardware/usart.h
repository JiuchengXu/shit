#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"
#include <stdio.h>

void uart_inint(void);//���ڳ�ʼ��
void USART1_IRQHandler(void);//�жϽ���

void uart_putc(char c);//����һ���ַ�
void uart_puts(char *str);//�����ַ���


#endif


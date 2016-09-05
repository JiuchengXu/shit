#include "includes.h"
#include "priority.h"
#include "helper.h"
#include "net.h"
#include "bulet.h"
#include "wav.h"

#ifdef  GUN

#define OS_TRIGGER_TASK_STACK_SIZE   	 64
static CPU_STK  TriggerTaskStk[OS_TRIGGER_TASK_STACK_SIZE];
static OS_TCB TriggerTaskStkTCB;

void trigger_task(void)
{
	while (1) {
		if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2) == Bit_RESET) {
			msleep(20);
			if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2) == Bit_RESET) {
				if (get_actived_state() && get_buletLeft() > 0) {
					while (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_2) == Bit_RESET) {
						wav_play_up();
						reduce_bulet();
					}
						wav_play_down();
				}
			}
		}
		
		msleep(50);
	}
}

void trigger_init(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
	OS_ERR err;
	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
 	GPIO_Init(GPIOD, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
 	GPIO_Init(GPIOC, &GPIO_InitStructure);
		
    OSTaskCreate((OS_TCB *)&TriggerTaskStkTCB, 
            (CPU_CHAR *)"Gun trigger task", 
            (OS_TASK_PTR)trigger_task, 
            (void * )0, 
            (OS_PRIO)OS_TASK_TRIGGER_PRIO, 
            (CPU_STK *)&TriggerTaskStk[0], 
            (CPU_STK_SIZE)OS_TRIGGER_TASK_STACK_SIZE/10, 
            (CPU_STK_SIZE)OS_TRIGGER_TASK_STACK_SIZE, 
            (OS_MSG_QTY) 0, 
            (OS_TICK) 0, 
            (void *)0,
            (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
            (OS_ERR*)&err);	
}

#endif

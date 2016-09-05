#include "includes.h"
#include "priority.h"
#include "helper.h"

#define OS_SOFTRTC_TASK_STACK_SIZE   	 64
static CPU_STK  SoftRTCTaskStk[OS_SOFTRTC_TASK_STACK_SIZE];
static OS_TCB SoftRTCTaskStkTCB;

static volatile u32 time;

void set_time(char *s, s8 len)
{
	s8 i;
	
	for (i = 0; i < len; i++) {
		time *= 10;
		time += s[i] - '0';
	}
}

u32 get_time(void)
{
	return time;
}

void softRTC_task(void *data)
{
	while (1) {
		time++;
		sleep(1);
	}
}

void timer_init(void)
{
	OS_ERR err;
	
    OSTaskCreate((OS_TCB *)&SoftRTCTaskStkTCB, 
        (CPU_CHAR *)"software RTC task", 
        (OS_TASK_PTR)softRTC_task, 
        (void * )0, 
        (OS_PRIO)OS_TASK_SOFTRTC_PRIO, 
        (CPU_STK *)&SoftRTCTaskStk[0], 
        (CPU_STK_SIZE)OS_SOFTRTC_TASK_STACK_SIZE/10, 
        (CPU_STK_SIZE)OS_SOFTRTC_TASK_STACK_SIZE, 
        (OS_MSG_QTY) 0, 
        (OS_TICK) 0, 
        (void *)0,
        (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
        (OS_ERR*)&err);	
}

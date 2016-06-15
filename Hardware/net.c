#include "includes.h"

#define HOST_IP		((192 << 24) | (168 << 16) | (1 << 8) | 1)
#define LOCAL_IP	((192 << 24) | (168 << 16) | (1 << 8) | 1)

#define GUN_IP		((192 << 24) | (168 << 16) | (4 << 8) | 2)
#define LCD_IP		((192 << 24) | (168 << 16) | (4 << 8) | 3)
#define LOCAL_PORT_BASE		8888

#define HOST_PORT			LOCAL_PORT_BASE
#define GUN_PORT			LOCAL_PORT_BASE
#define LCD_PORT			LOCAL_PORT_BASE

#define OS_TASK_RECV_PRIO           9
#define OS_RECV_TASK_STACK_SIZE     64

CPU_STK  RecvTaskStk[OS_RECV_TASK_STACK_SIZE];
OS_TCB RecvTaskStkTCB;

extern s8 udp_setup(u32 ip, u16 src_port, u16 dst_port);
extern s8 send_data(u32 ip, u16 src_port, u16 dst_port, char *data, u16 len);
extern s8 set_ip(u32 ip);
extern void recv_data(u32 *ip, u16 *port, char *buf, u16 *buf_len);

static char recv_buf[1024];

s8 sendto_host(char *buf, u16 len)
{
	return send_data(HOST_IP, LOCAL_PORT_BASE, HOST_PORT, buf, len);
}

s8 sendto_gun(char *buf, u16 len)
{
	return send_data(GUN_IP, LOCAL_PORT_BASE, GUN_PORT, buf, len);
}

s8 sendto_lcd(char *buf, u16 len)
{
	return send_data(LCD_IP, LOCAL_PORT_BASE, LCD_PORT, buf, len);
}

static void recv_gun_handler(char *buf, u16 len)
{
	
}

static void recv_host_handler(char *buf, u16 len)
{
	
}

static void recv_lcd_handler(char *buf, u16 len)
{
	
}

void recv_task(void)
{
	u32 ip;
	u16 remote_port;
	u16 len;
	
	while (1) {
		recv_data(&ip, &remote_port, recv_buf, &len);
		switch (ip) {
			case HOST_IP:
				recv_host_handler(recv_buf, len);
				break;
			case LCD_IP:
				recv_lcd_handler(recv_buf, len);
				break;
			case GUN_IP:
				recv_gun_handler(recv_buf, len);
				break;
			default:
				break;			
		}
	}
}

#define OS_TASK_RECV_PRIO           9
#define OS_RECV_TASK_STACK_SIZE     64

CPU_STK  RecvTaskStk[OS_RECV_TASK_STACK_SIZE];
OS_TCB RecvTaskStkTCB;

void net_init(void)
{
	OS_ERR err;
	
	udp_setup(HOST_IP, HOST_PORT, LOCAL_PORT_BASE);
	udp_setup(GUN_IP, GUN_PORT, LOCAL_PORT_BASE);
	udp_setup(LCD_IP, LCD_PORT, LOCAL_PORT_BASE);
	
    OSTaskCreate((OS_TCB *)&RecvTaskStkTCB, 
                (CPU_CHAR *)"net reciv task", 
                (OS_TASK_PTR)recv_task, 
                (void * )0, 
                (OS_PRIO)OS_TASK_RECV_PRIO, 
                (CPU_STK *)&RecvTaskStk[0], 
                (CPU_STK_SIZE)OS_RECV_TASK_STACK_SIZE/10, 
                (CPU_STK_SIZE)OS_RECV_TASK_STACK_SIZE, 
                (OS_MSG_QTY) 0, 
                (OS_TICK) 0, 
                (void *)0,
                (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                (OS_ERR*)&err);	
}


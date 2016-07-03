#include "includes.h"

#if 0
#define HOST_IP		((192 << 24) | (168 << 16) | (4 << 8) | 1)
#define GUN_IP		((192 << 24) | (168 << 16) | (4 << 8) | 2)
#define LCD_IP		((192 << 24) | (168 << 16) | (4 << 8) | 3)

#define GUN_PORT			8889
#define LCD_PORT			8890

#define OS_TASK_RECV_PRIO           9
#define OS_RECV_TASK_STACK_SIZE     64

CPU_STK  RecvTaskStk[OS_RECV_TASK_STACK_SIZE];
OS_TCB RecvTaskStkTCB;
static char recv_buf[1024];

extern s8 key_get_sid(char *sid);
extern s8 key_get_passwd(char *passwd);
extern s8 key_get_host_sid(char *sid);
extern s8 key_get_host_passwd(char *passwd);
extern s8 key_get_server_ip(char *ip);
extern s8 key_get_clothe_ip(char *ip);

extern s8 set_mode(u8 mode);
extern s8 set_show_ip(int mode);
extern s8 connect_ap(char *id, char *passwd, s8 channel);
extern s8 set_auto_conn(u8 i);
extern s8 set_ap(char *sid, char *passwd);
extern s8 set_echo(s8 on);
extern s8 set_mux(s8 mode);
extern s8 udp_setup(u32 ip, u16 remote_port, u16 local_port);
extern s8 send_data(u32 ip, u16 src_port, u16 dst_port, char *data, u16 len);
extern s8 set_ip(char *ip);
extern void recv_data(u32 *ip, u16 *port, char *buf, u16 *buf_len);
extern s8 udp_close(u8 id);


s8 sendto_host(char *buf, u16 len)
{
	return send_data(HOST_IP, GUN_PORT, GUN_PORT, buf, len);
}


s8 sendto_lcd(char *buf, u16 len)
{
	return send_data(LCD_IP, LCD_PORT, LCD_PORT, buf, len);
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
			default:
				break;			
		}
	}
}

void net_init(void)
{
	OS_ERR err;
	s8 ret;
	u8 i;
	
	char sid[20], passwd[20], host[20], host_passwd[20], ip[16];
	
	key_get_sid(sid);
	key_get_passwd(passwd);
	key_get_host_sid(host);
	key_get_host_passwd(host_passwd);
	
	if (set_echo(0) < 0)
		err_log("set_echo");
	
	if (set_mode(1) < 0)
		err_log("set_mode");
	
	if (connect_ap(sid, passwd, 3) < 0)
		err_log("connect_ap");
	
	sprintf(ip, "%d.%d.%d.%d",  (GUN_IP >> 24) & 0xff,
								(GUN_IP >> 16) & 0xff,
								(GUN_IP >> 8) & 0xff,
								(GUN_IP >> 0) & 0xff);
	
	if (set_ip(ip) < 0)
		err_log("set_ip");
	
	if (set_mux(1) < 0)
		err_log("set_mux");
	
	for (i = 0; i < 10; i++)
		udp_close(i);
	
	if (udp_setup(HOST_IP, GUN_PORT, GUN_PORT) < 0)
		err_log("");

	if (udp_setup(LCD_IP, LCD_PORT, LCD_PORT) < 0)
		err_log("");
	
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
#endif
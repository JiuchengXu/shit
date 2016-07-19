#include "includes.h"
#include "net.h"

#define HOST_IP		((192 << 24) | (168 << 16) | (1 << 8) | 1)

#define GUN_IP		((192 << 24) | (168 << 16) | (4 << 8) | 2)
#define LCD_IP		((192 << 24) | (168 << 16) | (4 << 8) | 3)

#define HOST_PORT			8888
#define GUN_PORT			8889
#define LCD_PORT			8890

#define OS_RECV_TASK_STACK_SIZE     128
#define OS_HB_TASK_STACK_SIZE     16

CPU_STK  RecvTaskStk[OS_RECV_TASK_STACK_SIZE];
OS_TCB RecvTaskStkTCB;

CPU_STK  HBTaskStk[OS_HB_TASK_STACK_SIZE];
OS_TCB HBTaskStkTCB;

static char recv_buf[1024];                                                    
static char characCode[16];
static s8 tasks_working;
static s8 blod;
static s8 actived;

struct sub_device_info {
	char type[10];	//最多支持10个子设备
	char numb;
};

static struct sub_device_info sub_device;

#define INT2CHAR(x, i)	int2chars(x, i, sizeof(x))

s8 sendto_host(char *buf, u16 len)
{
	return send_data(HOST_IP, HOST_PORT, HOST_PORT, buf, len);
}

s8 sendto_gun(char *buf, u16 len)
{
	return send_data(GUN_IP, GUN_PORT, GUN_PORT, buf, len);
}

s8 sendto_lcd(char *buf, u16 len)
{
	return send_data(LCD_IP, LCD_PORT, LCD_PORT, buf, len);
}

static u16 packageID = 0;

static int active_request(void)
{
	struct ActiveRequestData data;
	
	INT2CHAR(data.transMod, 0);
	INT2CHAR(data.packTye, ACTIVE_REQUEST_TYPE);
	INT2CHAR(data.packageID, packageID++);
	
	return sendto_host((char *)&data, sizeof(data));
}

int get_deviceSubType(void)
{
	return 0;
}

s8 get_lifeLeft(void)
{
	return blod;
}

u8 get_power(void)
{
	return 100;
}

static int upload_status_data(void)
{
	struct ClothesStatusData data;
	
	INT2CHAR(data.transMod, 0);
	INT2CHAR(data.packTye, CLOTHES_STATUS_TYPE);
	INT2CHAR(data.packageID, packageID++);
	INT2CHAR(data.deviceType, 0);
	INT2CHAR(data.deviceSubType, get_deviceSubType());
	INT2CHAR(data.lifeLeft, (int)get_lifeLeft());
	get_key_sn(data.keySN);
	INT2CHAR(data.transMod, get_power());
	
	return sendto_host((char *)&data, sizeof(data));
}

static int upload_heartbeat(void)
{
	struct HeartBeat data;
	
	INT2CHAR(data.transMod, 0);
	INT2CHAR(data.packTye, HEART_BEAT_TYPE);
	INT2CHAR(data.packageID, packageID++);
	INT2CHAR(data.deviceType, 0);
	INT2CHAR(data.deviceSubType, get_deviceSubType());
	get_key_sn(data.deviceSN);
	
	return sendto_host((char *)&data, sizeof(data));
}

static void recv_gun_handler(char *buf, u16 len)
{
	sendto_host(buf, len);
}

void reduce_blod(s8 i)
{
	blod -= i;
	
	if (blod < 0)
		blod = 0;
}

static void recv_host_handler(char *buf, u16 len)
{
	struct ActiveAskData *data = (void *)buf;
	u32 packTye;
	
	packTye = char2u32(data->packTye, sizeof(data->packTye));
	if (packTye == ACTIVE_RESPONSE_TYPE) {
		actived = 1;
		
	}
}

static void recv_lcd_handler(char *buf, u16 len)
{
	sendto_host(buf, len);
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
				sendto_host(recv_buf, len);
				break;			
		}
	}
}
 
void hb_task(void)
{
	u16 i = 0;
	s8 blod_bak = blod;
	
	while (1) {
		if (actived) {
			if (i > 400)	// 20s
				upload_heartbeat();

			if (blod_bak != blod)
				upload_status_data();
		} else {
			blod_bak = blod;
			i = 0;
		}
			
		msleep(50);		
		i++;
		
		if (i > 400)
			i = 0;
	}
}
	
void net_init(void)
{
	s8 ret;
	u8 i;
	char sid[20] = "CSsub", passwd[20] = "12345678", host[20] = "CS001", host_passwd[20] = "12345678", ip[16];
	
	// ip最后一位作为一个标识
	get_ip_suffix(&sid[5]);
	
	if (set_echo(0) < 0)
		err_log("set_echo");
	
	if (set_mode(3) < 0)
		err_log("set_mode");
	
	if (connect_ap(host, host_passwd, 3) < 0)
		err_log("connect_ap");
	
	if (set_ip(ip) < 0)
		err_log("set_ip");
	
	if (set_ap(sid, passwd) < 0)
		err_log("set_ap");
	
	if (set_mux(1) < 0)
		err_log("set_mux");
	
	for (i = 0; i < 10; i++)
		udp_close(i);
	
	if (udp_setup(HOST_IP, HOST_PORT, HOST_PORT) < 0)
		err_log("");
	
	if (udp_setup(GUN_IP, GUN_PORT, GUN_PORT) < 0)
		err_log("");
	
	if (udp_setup(LCD_IP, LCD_PORT, LCD_PORT) < 0)
		err_log("");
}

void start_clothe_tasks(void)
{
	OS_ERR err;
	
	tasks_working = 1;
	
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
				
    OSTaskCreate((OS_TCB *)&HBTaskStkTCB, 
            (CPU_CHAR *)"net reciv task", 
            (OS_TASK_PTR)hb_task, 
            (void * )0, 
            (OS_PRIO)OS_TASK_HB_PRIO, 
            (CPU_STK *)&HBTaskStk[0], 
            (CPU_STK_SIZE)OS_HB_TASK_STACK_SIZE/10, 
            (CPU_STK_SIZE)OS_HB_TASK_STACK_SIZE, 
            (OS_MSG_QTY) 0, 
            (OS_TICK) 0, 
            (void *)0,
            (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
            (OS_ERR*)&err);
}

void delete_clothe_tasks(void)
{
	OS_ERR err;
	
	tasks_working = 0;
	
	OSTaskDel((OS_TCB *)&RecvTaskStkTCB, &err);
	OSTaskDel((OS_TCB *)&HBTaskStkTCB, &err);	
}

void main_loop(void)
{
	while (1) {
		if (key_state_machine()) {
			
			actived = 0;
			
			blod += get_key_blod();
			
			if (blod > 100)
				blod = 100;
			
			net_init();
			
			if (tasks_working == 0)
				start_clothe_tasks();
					
			while (actived == 0) {
				active_request();
				sleep(1);
			}
			
			red_led_on();
		}
		
		sleep(1);
	}
}

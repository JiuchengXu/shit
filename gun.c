#include "includes.h"
#include "net.h"

#ifdef GUN
#define HOST_IP		(((u32)192 << 24) | ((u32)168 << 16) | ((u32)4 << 8) | 1)

#define GUN_IP		(((u32)192 << 24) | ((u32)168 << 16) | ((u32)4 << 8) | 2)
#define RIFLE_IP	(((u32)192 << 24) | ((u32)168 << 16) | ((u32)4 << 8) | 3)
#define LCD_IP		(((u32)192 << 24) | ((u32)168 << 16) | ((u32)4 << 8) | 5)

#define HOST_PORT			(u16)8888
#define GUN_PORT			(u16)8889
#define RIFLE_PORT			(u16)8890
#define LCD_PORT			(u16)8891

#define OS_RECV_TASK_STACK_SIZE     128
#define OS_HB_TASK_STACK_SIZE   	 64
#define BLOD_MAX					100

enum {
	INIT,
	ACTIVE,
	LIVE,
	DEAD,
	SUPPLY,
};

static CPU_STK  RecvTaskStk[OS_RECV_TASK_STACK_SIZE];
static OS_TCB RecvTaskStkTCB;

static CPU_STK  HBTaskStk[OS_HB_TASK_STACK_SIZE];
static OS_TCB HBTaskStkTCB;

static char recv_buf[1024];                                                    
static u16 characCode;
static s8 bulet;
static s8 actived;

#define INT2CHAR(x, i)	int2chars(x, i, sizeof(x))

static s8 sendto_host(char *buf, u16 len)
{
	return send_data((u32)HOST_IP, (u16)GUN_PORT, (u16)GUN_PORT, buf, len);
}

static s8 sendto_rifle(char *buf, u16 len)
{
	return send_data(RIFLE_IP, RIFLE_PORT, RIFLE_PORT, buf, len);
}

static s8 sendto_lcd(char *buf, u16 len)
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

static int get_deviceSubType(void)
{
	return 0;
}

static s8 get_buletLeft(void)
{
	return bulet;
}

static void set_buletLeft(s8 v)
{
	bulet = v;
	if (bulet > 100)
		bulet = 100;
}

static void add_buletLeft(s8 v)
{
	v += get_buletLeft();
	set_buletLeft(v);
}

void reduce_bulet(void)
{
	if (bulet > 0)
		bulet--;
}

static u8 get_power(void)
{
	return 100;
}

static int upload_status_data(void)
{
	struct GunStatusData data;
	
	memset(&data, '0', sizeof(data));
	
	INT2CHAR(data.transMod, 0);
	INT2CHAR(data.packTye, GUN_STATUS_TYPE);
	INT2CHAR(data.packageID, packageID++);
	INT2CHAR(data.deviceType, 1);
	INT2CHAR(data.deviceSubType, get_deviceSubType());
	
	memcpy(data.deviceSN, "0987654321abcdef", sizeof(data.deviceSN));
	INT2CHAR(data.bulletLeft, get_buletLeft());
	get_key_sn(data.keySN);
	INT2CHAR(data.characCode, characCode);

	INT2CHAR(data.PowerLeft, get_power());
	
	return sendto_host((char *)&data, sizeof(data));
}

static int upload_heartbeat(void)
{
	struct HeartBeat data;
	
	INT2CHAR(data.transMod, 0);
	INT2CHAR(data.packTye, HEART_BEAT_TYPE);
	INT2CHAR(data.packageID, packageID++);
	INT2CHAR(data.deviceType, 1);
	INT2CHAR(data.deviceSubType, get_deviceSubType());
	get_key_sn(data.deviceSN);
	
	return sendto_host((char *)&data, sizeof(data));
}

static void recv_rifle_handler(char *buf, u16 len)
{
	sendto_host(buf, len);
}

static void recv_host_handler(char *buf, u16 len)
{
	struct ActiveAskData *data = (void *)buf;
	u32 packTye;
	
	packTye = char2u32(data->packTye, sizeof(data->packTye));
	if (packTye == ACTIVE_RESPONSE_TYPE) {
		actived = 1;
		characCode = (u16)char2u32(data->characCode, sizeof(data->characCode));
	}
}

static void recv_lcd_handler(char *buf, u16 len)
{
	sendto_host(buf, len);
}

static void dead(void)
{
	//release_smoke();
	//disable_guns();
}

static void recv_task(void)
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
			case RIFLE_IP:
				recv_rifle_handler(recv_buf, len);
				break;
			default:
				sendto_host(recv_buf, len);
				break;			
		}
	}
}
 
static void hb_task(void)
{
	u16 i = 0;
	s8 bulet_bak = get_buletLeft();
	
	while (1) {
		if (actived) {
			if (i == 400)	// 20s
				upload_heartbeat();

			if (bulet_bak != get_buletLeft()) {
				upload_status_data();
				bulet_bak = get_buletLeft();
			}
		} else {
			bulet_bak = get_buletLeft();
			i = 0;
		}
			
		msleep(50);		
		i++;
		
		if (i > 400)
			i = 0;
	}
}
	
static void net_init(void)
{
	u8 i;
	char host[20] = "CSsub", host_passwd[20] = "12345678", \
	    ip[16];
	
	// ip最后一位作为一个标识
	get_ip_suffix(&host[5]);
	
	sprintf(ip, "192.168.4.%d", GUN_IP & 0xff);
	
	if (set_auto_conn(0) < 0)
		err_log("set_echo");
	
	if  (close_conn() < 0)
		err_log("set_echo");

	if (set_echo(1) < 0)
		err_log("set_echo");
	
	if (set_mode(1) < 0)
		err_log("set_mode");
	
	if (connect_ap(host, host_passwd, 3) < 0)
		err_log("connect_ap");
	
	if (set_ip(ip) < 0)
		err_log("set_ip");
		
	if (set_mux(1) < 0)
		err_log("set_mux");
	
	for (i = 0; i < 4; i++)
		udp_close(i);
	
	if (udp_setup(HOST_IP, GUN_PORT, GUN_PORT) < 0)
		err_log("");
	
	if (udp_setup(RIFLE_IP, RIFLE_PORT, RIFLE_PORT) < 0)
		err_log("");
	
	if (udp_setup(LCD_IP, LCD_PORT, LCD_PORT) < 0)
		err_log("");
}

static void start_gun_tasks(void)
{
	OS_ERR err;
	
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
            (CPU_CHAR *)"heart beat task", 
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

static s8 bulet_state_machine(void)
{
	if (get_buletLeft() <= 0)
		return 1;
	
	return 0;
}
void main_loop(void)
{
	s8 status = INIT;
	
	while (1) {
		switch (status) {
			case INIT:
				if (key_state_machine()) {
					actived = 0;
	
					net_init();
					
					start_gun_tasks();
					
					status = ACTIVE;
				} 
				
				break;
			case ACTIVE:
				if (actived == 0) {
					active_request();
					sleep(1);
				} else {
					red_led_on();					
					status = LIVE;					
				}
					
				break;
			case LIVE:
				if (get_buletLeft() == 0)
					status = DEAD;
				else
					status = SUPPLY;
				
				break;
			case DEAD:
				dead();
				status = SUPPLY;
			case SUPPLY:
				if (bulet_state_machine())					
					add_buletLeft(100);				

				if (get_buletLeft() > 0)
					status = LIVE;					
		}
		
		msleep(200);
	}
}
#endif

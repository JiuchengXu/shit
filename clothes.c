#include "includes.h"
#include "net.h"

#ifdef CLOTHE

#define HOST_IP		(((u32)192 << 24) | ((u32)168 << 16) | ((u32)0 << 8) | 114)

#define GUN_IP		(((u32)192 << 24) | ((u32)168 << 16) | ((u32)4 << 8) | 2)
#define RIFLE_IP	(((u32)192 << 24) | ((u32)168 << 16) | ((u32)4 << 8) | 3)
#define LCD_IP		(((u32)192 << 24) | ((u32)168 << 16) | ((u32)4 << 8) | 5)

#define HOST_PORT			(u16)5335
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
static s8 blod;
static s8 actived;

#define INT2CHAR(x, i)	int2chars(x, i, sizeof(x))

static s8 sendto_host(char *buf, u16 len)
{
	return send_data((u32)HOST_IP, (u16)HOST_PORT, (u16)HOST_PORT, buf, len);
}

static s8 sendto_gun(char *buf, u16 len)
{
	return send_data(GUN_IP, GUN_PORT, GUN_PORT, buf, len);
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
	get_key_sn(data.keySN);
	INT2CHAR(data.packageID, packageID++);
	
	return sendto_host((char *)&data, sizeof(data));
}

static int get_deviceSubType(void)
{
	return 0;
}

static s8 get_lifeLeft(void)
{
	return blod;
}

static void set_lifeLeft(s8 v)
{
	blod = v;
	if (blod > 100)
		blod = 100;
}

static void add_lifeLeft(s8 v)
{
	v += get_lifeLeft();
	set_lifeLeft(v);
}

void reduce_blod(s8 i)
{
	blod -= i;
	
	if (blod < 0)
		blod = 0;
}

static char test_string[] = "03011100028SN1457845414587200010002000300040005000600070008000900101234567012345671123456721234567312345674123456751234567612345677123456781234567967";
static int upload_status_data(void)
{
	struct ClothesStatusData data;
	
	memset(&data, '0', sizeof(data));
	
	INT2CHAR(data.transMod, 0);
	INT2CHAR(data.packTye, CLOTHES_STATUS_TYPE);
	INT2CHAR(data.packageID, packageID++);
	INT2CHAR(data.deviceType, 0);
	INT2CHAR(data.deviceSubType, get_deviceSubType());	
	memcpy(&data, test_string, sizeof(data));
	INT2CHAR(data.lifeLeft, (int)get_lifeLeft());
	get_key_sn(data.keySN);

	INT2CHAR(data.PowerLeft, get_power());
	
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
	struct ActiveRequestData *data = (void *)buf;
	u32 packTye;
	
	packTye = char2u32(data->packTye, sizeof(data->packTye));
	
	if (packTye == ACTIVE_REQUEST_TYPE) {
		struct GunActiveAskData ask_data;
		u16 tmp; 
		
		INT2CHAR(ask_data.transMod, 0);
		INT2CHAR(ask_data.packTye, ACTIVE_RESPONSE_TYPE);
		INT2CHAR(ask_data.packageID, packageID++);
		tmp = characCode + 0x100;
		INT2CHAR(ask_data.characCode, tmp);
		sendto_gun((void *)&ask_data, sizeof(ask_data));
	} else
		sendto_host(buf, len);
}

static void recv_rifle_handler(char *buf, u16 len)
{
	struct ActiveRequestData *data = (void *)buf;
	u32 packTye;
	
	packTye = char2u32(data->packTye, sizeof(data->packTye));
	
	if (packTye == ACTIVE_REQUEST_TYPE) {
		struct GunActiveAskData ask_data;
		u16 tmp;
		
		INT2CHAR(ask_data.transMod, 0);
		INT2CHAR(ask_data.packTye, ACTIVE_RESPONSE_TYPE);
		INT2CHAR(ask_data.packageID, packageID++);
		tmp = characCode + 0x200;
		INT2CHAR(ask_data.characCode, tmp);
		sendto_rifle((void *)&ask_data, sizeof(ask_data));
	} else
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
		set_time(data->curTime, sizeof(data->curTime));
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
			case GUN_IP:
				recv_gun_handler(recv_buf, len);
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
	s8 blod_bak = blod;
	
	while (1) {
		if (actived) {
			if (i == 400)	// 20s
				upload_heartbeat();

			if (blod_bak != blod) {
				upload_status_data();
				blod_bak = blod;
			}
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
	
static void net_init(void)
{
	u8 i;
	char sid[20] = "CSsub", passwd[20] = "12345678", \
		host[20] = "shunwei888", host_passwd[20] = "swhotels88198618", \
	    ip[16] = "192.168.0.";
	
	// ip最后一位作为一个标识
	get_ip_suffix(&sid[5]);
	
	get_ip_suffix(&ip[10]);
	
	if (set_bound() < 0)
		err_log("set_bound");
	
	if (set_auto_conn(0) < 0)
		err_log("set_echo");
	
	if  (close_conn() < 0)
		err_log("set_echo");

	if (set_echo(1) < 0)
		err_log("set_echo");
	
	if (set_mode(3) < 0)
		err_log("set_mode");
	
	if (set_mac_addr() < 0)
		err_log("set_mac_addr");
	
	if (connect_ap(host, host_passwd, 3) < 0)
		err_log("connect_ap");
	
//	if (set_ip(ip) < 0)
//		err_log("set_ip");
	
	if (set_ap(sid, passwd) < 0)
		err_log("set_ap");
	
	if (set_mux(1) < 0)
		err_log("set_mux");
	
	for (i = 0; i < 4; i++)
		udp_close(i);
	
	if (udp_setup(HOST_IP, HOST_PORT, HOST_PORT) < 0)
		err_log("");
	
	if (udp_setup(GUN_IP, GUN_PORT, GUN_PORT) < 0)
		err_log("");
	
	if (udp_setup(RIFLE_IP, RIFLE_PORT, RIFLE_PORT) < 0)
		err_log("");
	
	if (udp_setup(LCD_IP, LCD_PORT, LCD_PORT) < 0)
		err_log("");
}

static void start_clothe_tasks(void)
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

void main_loop(void)
{
	s8 status = INIT;

	while (1) {
		switch (status) {
			case INIT:
				if (key_state_machine()) {
					actived = 0;
			
					add_lifeLeft(get_key_blod());
					
					net_init();
					
					start_clothe_tasks();
					
					status = ACTIVE;
				} 
				
				break;
			case ACTIVE:
				if (actived == 0) {
					active_request();
					//char s[]="abcde";
					//sendto_gun(s, sizeof(s)-1);
					sleep(1);
				} else {
					red_led_on();					
					status = LIVE;					
				}
					
				break;
			case LIVE:
				if (get_lifeLeft() == 0)
					status = DEAD;
				else
					status = SUPPLY;
				
				break;
			case DEAD:
				dead();
				status = SUPPLY;
			case SUPPLY:
				if (key_state_machine())					
					add_lifeLeft(get_key_blod());				

				if (get_lifeLeft() > 0)
					status = LIVE;					
		}
		
		msleep(200);
	}
}
#endif

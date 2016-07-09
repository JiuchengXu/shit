#include "includes.h"

#define HOST_IP		((192 << 24) | (168 << 16) | (1 << 8) | 1)

#define GUN_IP		((192 << 24) | (168 << 16) | (4 << 8) | 2)
#define LCD_IP		((192 << 24) | (168 << 16) | (4 << 8) | 3)

#define HOST_PORT			8888
#define GUN_PORT			8889
#define LCD_PORT			8890

#define OS_TASK_RECV_PRIO           9
#define OS_RECV_TASK_STACK_SIZE     128

#define OS_TASK_HB_PRIO           8
#define OS_HB_TASK_STACK_SIZE     128

CPU_STK  RecvTaskStk[OS_RECV_TASK_STACK_SIZE];
OS_TCB RecvTaskStkTCB;

CPU_STK  HBTaskStk[OS_HB_TASK_STACK_SIZE];
OS_TCB HBTaskStkTCB;

static char recv_buf[1024];                                                    
static char characCode[8];

extern void get_key_sn(char *sn);
extern void get_ip_suffix(char *s);

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

struct ActiveRequestData {
	char transMod [1];
	char packageID [4];
	char activeRequest [10];	
};

struct ActiveAskData  {
	char transMod [1];
	char packageID [8];
	char characCode [8];
	char curTime [8];
};

struct ClothesStatusData  {
	char transMod [1];
	char packageID [4];
	char deviceType [1];
	char deviceSubType [1];
	char lifeLeft [3];
	char keySN [16];
	char characCode [10][4];
	char attachTime [10][8];
	char PowerLeft [2];
};

struct StatusRespData {
	char transMod [1];
	char packageID [4];
	char errorNum [2];	
};

struct HeartBeat {
	char transMod [1];
	char packageID [4];
	char deviceType [1];
	char deviceSubType [1];
	char deviceSN [16];
	char heartBeatID [10];
};

struct sub_device_info {
	char type[10];	//最多支持10个子设备
	char numb;
};

static struct sub_device_info sub_device;

static void int2chars(char *str, int v, int len)
{
	int i;
	
	for (i = 0; i < len; i++) {
		char tmp = (char)(v >> (len - i - 1)) & 0xf;
		if (tmp >= 0 && tmp < 10)
			str[i] = tmp + '0';
		else if (tmp >= 0xa && tmp <=0xf)
			str[i] = 'a' + (tmp - 0xa);
	}
}

#define INT2CHAR(x, i)	int2chars(x, i, sizeof(x))

static void str2chars(char *dst, char *str)
{
	int len = strlen(str);
	memcpy(dst, str, len);
}


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
	INT2CHAR(data.packageID, packageID++);
	str2chars(data.activeRequest, "active--me");
	
	return sendto_host((char *)&data, sizeof(data));
}

int get_deviceSubType(void)
{
	return 0;
}

int get_lifeLeft(void)
{
	return 100;
}

u8 get_power(void)
{
	return 100;
}

static int upload_status_data(void)
{
	struct ClothesStatusData data;
	
	INT2CHAR(data.transMod, 0); 
	INT2CHAR(data.packageID, packageID++);
	INT2CHAR(data.deviceType, 0);
	INT2CHAR(data.deviceSubType, get_deviceSubType());
	INT2CHAR(data.lifeLeft, get_lifeLeft());
	get_key_sn(data.keySN);
	INT2CHAR(data.transMod, get_power());
	
	return sendto_host((char *)&data, sizeof(data));
}

static int upload_heartbeat(void)
{
	struct HeartBeat data;
	
	INT2CHAR(data.transMod, 0);
	INT2CHAR(data.packageID, packageID++);
	INT2CHAR(data.deviceType, 0);
	INT2CHAR(data.deviceSubType, get_deviceSubType());
	get_key_sn(data.deviceSN);
	str2chars(data.heartBeatID, "heart-beat");
	
	return sendto_host((char *)&data, sizeof(data));
}

static void recv_gun_handler(char *buf, u16 len)
{
	sendto_host(buf, len);
}

static void recv_host_handler(char *buf, u16 len)
{

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
	while (1) {
		upload_heartbeat();
		sleep(20);
	}
}
	
void net_init(void)
{
	OS_ERR err;
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

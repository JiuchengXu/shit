#ifndef __NET_H__
#define __NET_H__

#include "includes.h"

void get_key_sn(char *sn);
void get_ip_suffix(char *s);

s8 set_mode(u8 mode);
s8 set_show_ip(int mode);
s8 connect_ap(char *id, char *passwd, s8 channel);
s8 set_auto_conn(u8 i);
s8 set_ap(char *sid, char *passwd);
s8 set_echo(s8 on);
s8 set_mux(s8 mode);
s8 close_conn(void);
s8 udp_setup(u32 ip, u16 remote_port, u16 local_port);
s8 send_data(u32 ip, u16 src_port, u16 dst_port, char *data, u16 len);
s8 set_ip(char *ip);
void recv_data(u32 *ip, u16 *port, char *buf, u16 *buf_len);
s8 udp_close(u8 id); 
s8 key_state_machine(void);
s8 get_key_blod(void);
s8 get_power(void);

#define ACTIVE_REQUEST_TYPE		0
#define ACTIVE_RESPONSE_TYPE	1
#define GUN_STATUS_TYPE			2
#define CLOTHES_STATUS_TYPE		3
#define STATUS_RESPONSE_TYPE	4
#define HEART_BEAT_TYPE			5

struct ActiveRequestData {
	char transMod [1];
	char packTye[1];
	char keySN[16];
	char packageID [4];
};

struct ActiveAskData  {
	char transMod [1];
	char packTye[1];
	char packageID [4];
	char characCode [4];
	char curTime [8];
};

struct GunActiveAskData  {
	char transMod [1];
	char packTye[1];
	char packageID [4];
	char characCode [4];
};

struct ClothesStatusData  {
	char transMod [1];
	char packTye[1];
	char packageID [4];
	char deviceType [1];
	char deviceSubType [1];
	char lifeLeft [3];
	char keySN [16];
	char characCode [10][4];
	char attachTime [10][8];
	char PowerLeft [2];
};

struct GunStatusData  {
	char transMod [1];
	char packTye[1];
	char packageID [4];
	char deviceType [1];
	char deviceSubType [1];
	char deviceSN [16];
	char bulletLeft [3];
	char keySN [16];
	char characCode [4];
	char PowerLeft [2];
};

struct StatusRespData {
	char transMod [1];
	char packTye[1];
	char packageID [4];
	char errorNum [2];	
};

struct HeartBeat {
	char transMod [1];
	char packTye[1];
	char packageID [4];
	char deviceType [1];
	char deviceSubType [1];
	char deviceSN [16];
};

void red_led_on(void);
void green_led_on(void);
void yellow_led_on(void);

void int2chars(char *str, int v, int len);
void str2chars(char *dst, char *str);
u32 char2u32(char *s, s8 bit_len);

void reduce_blod(s8 i);

void esp8266_gpio_init(void);

void update_esp8266(void);
void work_esp8266(void);

void err_log(char *log);

void flash_bytes_read(u32 addr, u8 *buf, u16 len);
void flash_page_write(uint32_t page, uint8_t *data);

void set_time(char *s, s8 len);
u32 get_time(void);

#endif

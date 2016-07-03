#include "includes.h"
#include "delay.h"

#define SID_LEN			20
#define PASSWD_LEN		20

struct eeprom_key_info {
	char sn[16];
	char user_id[16];
	char ip_suffix[3];
	char blob_def[3];
	char menoy[3];
};

struct key {
	char server_ip[SID_LEN];
	char clothe_ip[16];
	char host_sid[SID_LEN];
	char host_passwd[SID_LEN];
	char sid[SID_LEN];
	char passwd[PASSWD_LEN];
	
	u16 blod;
};

static struct key key;

s8 key_get_sid(char *sid)
{
	strcpy(sid, key.sid);
	return 0;
}

s8 key_get_passwd(char *passwd)
{
	strcpy(passwd, key.passwd);
	return 0;
}

s8 key_get_host_sid(char *sid)
{
	strcpy(sid, key.host_sid);
	return 0;
}

s8 key_get_host_passwd(char *passwd)
{
	strcpy(passwd, key.host_passwd);
	return 0;
}

s8 key_get_server_ip(char *ip)
{
	strcpy(ip, key.server_ip);
	return 0;
}

s8 key_get_clothe_ip(char *ip)
{
	strcpy(ip, key.clothe_ip);
	return 0;	
}

void key_read_wifi_info(void)
{
	strcpy(key.server_ip, "192.168.1.20");
	strcpy(key.clothe_ip, "192.168.1.240");
	strcpy(key.host_sid, "1103");
	strcpy(key.host_passwd, "Q!W@E#r4");
	strcpy(key.sid, "ESP8266");
	strcpy(key.passwd, "1234567890");
}

void key_read_blod(void)
{
	
}

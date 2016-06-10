#include "includes.h"
#include "delay.h"

#define SID_LEN			20
#define PASSWD_LEN		20

struct key {
	char sid[SID_LEN];
	char passwd[PASSWD_LEN];
	
	u16 blod;
};

static struct key key;

int key_get_sid(char *sid)
{
	strcpy(sid, key.sid);
	return 0;
}

int key_get_passwd(char *passwd)
{
	strcpy(passwd, key.passwd);
	return 0;
}

void key_read_wifi_info(void)
{
	
}

void key_read_blod(void)
{
	
}

void EXTI0_IRQHandler(void)
{
	key_read_wifi_info();
	
	key_read_blod();
}
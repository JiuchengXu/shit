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

static struct eeprom_key_info key;

char eeprom[] = "1234567890abcdefasdfghjklqwertyu001100100";

void read_key_from_eeprom(void)
{
	memcpy(&key, eeprom, strlen(eeprom));
	
}

void get_key_sn(char *s)
{
	memcpy(s, key.sn, 16);
}

void get_ip_suffix(char *s)
{
	memcpy(s, key.ip_suffix, 3);
}

void key_read_wifi_info(void)
{

}

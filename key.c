#include "includes.h"
#include "delay.h"

struct eeprom_key_info {
	char sn[16];
	char user_id[16];
	char ip_suffix[3];
	char blod_def[3];
	char menoy[3];
};

enum {
	KEY_UNINSERT = 0,
	KEY_INSERTING,
	KEY_INSERTED,
	KEY_UNUSED,
};

static struct eeprom_key_info key;

char eeprom[] = "SN145784541458720000018092719086250100100";

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

s8 get_key_blod(void)
{
	return char2u32(key.blod_def, sizeof(key.blod_def));	
}

s8 get_key_gpio(void)
{
	return 1;
}

static s8 status = KEY_UNINSERT;

s8 key_state_machine(void)
{
	s8 ret = 0;
	
	switch (status) {
		case KEY_UNINSERT:
			if (get_key_gpio())
				status = KEY_INSERTING;
			break;
			
		case KEY_INSERTING:
			if (get_key_gpio())
				status = KEY_INSERTED;
			break;
			
		case KEY_INSERTED:
			read_key_from_eeprom();
			status = KEY_UNUSED;
			ret = 1;
			// fall through
		case KEY_UNUSED:
			if (get_key_gpio() == 0)
				status = KEY_UNINSERT;
			break;
	}
	
	return ret;
}

void key_init(void)
{	

}

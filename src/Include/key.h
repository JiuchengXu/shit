#ifndef __KEY__
#define __KEY__
void get_key_sn(char *sn);
void get_ip_suffix(char *s);

void key_read_wifi_info(void);
s8 key_state_machine(void);

s8 get_key_blod(void);
void key_init(void *cb);

#endif /* __KEY__ */

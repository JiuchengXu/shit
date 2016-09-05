#ifndef __DELAY_H
#define __DELAY_H 

#include "includes.h"

#define sleep(s) \
do { \
	OS_ERR err; \
	OSTimeDlyHMSM(0, 0, s, 0, OS_OPT_TIME_HMSM_STRICT, &err); \
} while (0) \

#define msleep(ms) \
do { \
	OS_ERR err;\
	OSTimeDlyHMSM(0, 0, 0, ms, OS_OPT_TIME_HMSM_STRICT, &err); \
} while (0) \

#endif


static void int2chars(char *str, int v, int len)
{
	int i;
	
	for (i = 0; i < len; i++) {
		char tmp = (char)(v >> (len - i - 1) * 4) & 0xf;
		
		if ((int)tmp >= 0 && (int)tmp < 10)
			str[i] = tmp + '0';
		else if (tmp >= 0xa && tmp <=0xf)
			str[i] = 'a' + (tmp - 0xa);
	}
}

static void str2chars(char *dst, char *str)
{
	int len = strlen(str);
	memcpy(dst, str, len);
}


static u32 char2u32(char *s, s8 bit_len)
{
	s8 i;
	u32 ret = 0;
	
	for (i = 0; i < bit_len; i++) {
		ret *= 10;
		ret += s[i] - '0';
	}
	
	return ret;
}

static void err_log(char *log)
{
	
}



























#include "includes.h"

static u64 time;

void set_time(char *s, s8 len)
{
	s8 i;
	
	for (i = 0; i < len; i++) {
		time *= 10;
		time += s[i] - '0';
	}
}

u64 get_time(void)
{
	return time;
}

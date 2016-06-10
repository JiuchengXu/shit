#ifndef __DELAY_H
#define __DELAY_H 

#include "os.h"

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






























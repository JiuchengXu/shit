#include "includes.h"
#include "libe2prom.h"
#include "helper.h"
#include "priority.h"

#define I2C                   I2C1

#define AT24Cx_Address           0xa6 
#define AT24Cx_PageSize          8  

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

#define OS_TASK_STACK_SIZE   	 64

static CPU_STK  TaskStk[OS_TASK_STACK_SIZE];
static OS_TCB TaskStkTCB;

static struct eeprom_key_info key;

char eeprom[] = "SN145784541458720000018092719086250100100";

void key_Reads(u8 Address, u8 *ReadBuffer, u16 ReadNumber)
{
	e2prom_Reads(I2C, AT24Cx_Address, Address, ReadBuffer, ReadNumber);
}

void key_Writes(u8 Address, u8 *WriteData, u16 WriteNumber)
{
	e2prom_Writes(I2C, AT24Cx_Address, Address, WriteData, WriteNumber);
}

void read_key_from_eeprom(void)
{
#if 1	
	memcpy(&key, eeprom, strlen(eeprom));
	return;
#else
	struct eeprom_key_info tmp_key;
	
	key_Reads(0, (u8 *)&key, sizeof(tmp_key));
	
#ifdef CLOTHE
	tmp_key = key;
	
	int2chars(tmp_key.blod_def, 0, sizeof(tmp_key.blod_def));
	
	key_Writes(0, (u8 *)&tmp_key, sizeof(tmp_key));
#endif
	
#endif
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
	s8 ret = (s8)char2u32(key.blod_def, sizeof(key.blod_def));
	int2chars(key.blod_def, 0, sizeof(key.blod_def));
	
	return ret;
}

s8 get_key_gpio(void)
{	
	//return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == Bit_RESET;
	return 1;
}

void (*call_back)(void);

void key_state_machine_task(void)
{
	s8 status = KEY_UNUSED;
	
	while (1) {
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
			
				if (call_back)
					call_back();
				
				status = KEY_UNUSED;
				// fall through
			case KEY_UNUSED:
				if (get_key_gpio() == 0)
					status = KEY_UNINSERT;
				break;
		}
		msleep(250);
	}
}

void key_init(void *cb)
{
	OS_ERR err;
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	while (1) {
		if (get_key_gpio()) {
			msleep(200);
			if (!get_key_gpio())
				continue;
			
			read_key_from_eeprom();
			break;
		}
		
		msleep(200);
	}
	
	call_back = (void (*)(void))cb;
		
    OSTaskCreate((OS_TCB *)&TaskStkTCB, 
            (CPU_CHAR *)"key_state_machine_task", 
            (OS_TASK_PTR)key_state_machine_task, 
            (void * )0, 
            (OS_PRIO)OS_TASK_KEY_PRIO, 
            (CPU_STK *)&TaskStk[0], 
            (CPU_STK_SIZE)OS_TASK_STACK_SIZE/10, 
            (CPU_STK_SIZE)OS_TASK_STACK_SIZE, 
            (OS_MSG_QTY) 0, 
            (OS_TICK) 0, 
            (void *)0,
            (OS_OPT)(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
            (OS_ERR*)&err);
}

void key_test(void)
{
	char a[]="SN145784541458720000018092719086250100100";
	
	key_Writes(0, (u8 *)a, sizeof(a));
	memset(a, 0, sizeof(a));
	key_Reads(0, (u8 *)a, sizeof(a));
}

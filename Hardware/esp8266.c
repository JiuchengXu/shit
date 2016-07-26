#include <os.h>
#include "includes.h"
#include "delay.h"
#include "net.h"
#include "bus.h"


struct ip_port_map {
	u32 ip;
	u16 remote_port;
	u16 local_port;
};

struct ip_port_map ip_map[20];

static char temp[100];
static char output[2048];
static u8 gID;

void err_log(char *s)
{
	
}

s8 close_udp(u8 id);

void bus_send_string(char *buf)
{
	u16 i;
	CPU_SR_ALLOC();
	
	OS_CRITICAL_ENTER_CPU_CRITICAL_EXIT();
	for (i = 0; buf[i] != '\0'; i++)
		bus_send(&buf[i], 1);
	OS_CRITICAL_EXIT();
}

void bus_recieve_string(char *buf)
{
	u16 i = 0;
	
	memset(output, 0, sizeof(output));
	
	while ((buf[i++] = bus_recieve()) != '\0');
}

s8 str_include(char *buf, char *str)
{	
	u16 i = 0, j = 0, k;

	for (i = 0; buf[i] != '\0'; i++) {
		for (j = 0, k = i; str[j] != '\0' && buf[k] != '\0'; j++, k++) {
			if (buf[k] == str[j])
				continue;
			else
				break;
		}
		
		if (str[j] == '\0')
			return 0;
		
		if (buf[k] == '\0')
			return -1;
	}
	
	return -1;
}

s8 set_mode(u8 mode)
{
	sprintf(temp, "AT+CWMODE=%d\r\n", mode);
	bus_send_string(temp);
	
	msleep(200);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

s8 set_show_ip(int mode)
{
	sprintf(temp, "AT+CIPDINFO=%d\r\n", mode);
	bus_send_string(temp);
		
	msleep(200);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

s8 connect_ap(char *id, char *passwd, s8 channel)
{	
	sprintf(temp, "AT+CWJAP=\"%s\",\"%s\"\r\n", id, passwd);
	bus_send_string(temp);	

	sleep(30);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

s8 set_auto_conn(u8 i)
{
	sprintf(temp, "AT+CWAUTOCONN=%d\r\n", i);
	bus_send_string(temp);
	
	msleep(200);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");	
}
s8 close_conn(void)
{
	sprintf(temp, "AT+CWQAP\r\n");
	bus_send_string(temp);
	
	msleep(200);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

s8 set_ap(char *sid, char *passwd)
{
	sprintf(temp, "AT+CWSAP=\"%s\",\"%s\",5,3,4\r\n", sid, passwd);
	bus_send_string(temp);
	
	msleep(200);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");	
}

extern void reset_buffer(void);

s8 set_echo(s8 on)
{
	reset_buffer();
	
	sprintf(temp, "ATE%d\r\n", on);
	bus_send_string(temp);
	
	msleep(200);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

s8 set_mux(s8 mode)
{
	sprintf(temp, "AT+CIPMUX=%d\r\n", mode);
	bus_send_string(temp);
	
	msleep(200);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");	
}

s8 udp_setup(u32 ip, u16 remote_port, u16 local_port)
{
	char ip_str[16];
	
	sprintf(ip_str, "%d.%d.%d.%d", ip >> 24, (ip >> 16) & 0xff, (ip >> 8) & 0xff, ip &0xff);
	
	if (gID >= sizeof(ip_map)/sizeof(ip_map[0]))
		return -1;
	
	ip_map[gID].ip = ip;
	ip_map[gID].remote_port = remote_port;
	ip_map[gID].local_port = local_port;
	
	//sprintf(temp, "AT+CIPSTART=%d,\"UDP\",\"%s\",%d,%d,2\r\n", gID++, ip_str, remote_port, local_port);
	sprintf(temp, "AT+CIPSTART=%d,\"UDP\",\"%s\",%d,%d\r\n", gID++, ip_str, remote_port, local_port);
	
	bus_send_string(temp);
	
	msleep(200);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

static s8 get_id(u32 ip, u16 local_port, u16 remote_port)
{
	u8 i;
	
	for (i = 0; i < sizeof(ip_map) / sizeof(ip_map[0]); i++)
		if (ip_map[i].ip == ip &&
			local_port == ip_map[i].local_port &&
			remote_port == ip_map[i].remote_port)
			return i;
		
	return -1;
}

static struct ip_port_map *get_ip_port(u8 id)
{
	return &ip_map[id];
}

s8 send_data(u32 ip, u16 src_port, u16 dst_port, char *data, u16 len)
{

	u8 id = get_id(ip, src_port, dst_port);
	
	sprintf(temp, "AT+CIPSEND=%d,%d\r\n", id, len);
	
	bus_send_string(temp);
	
	msleep(20);
	
	bus_recieve_string(output);
	
	if (str_include(output, ">") == 0) {
		bus_send(data, len);
		return 0;
	} else
		return -1;
}

static u16 get_len(char *buf)
{
	u8 i;
	u16 tmp = 0;
		
	for (i = 0; (buf[i] = bus_recieve()) != ':'; i++) {
		tmp *= 10;
		tmp += buf[i] - '0';
	}
	
	return tmp;
}

void recv_data(u32 *ip, u16 *port, char *buf, u16 *buf_len)
{
	char tmp[10], c;
	u16 len;
	u8 fd;
	u16 i;
	struct ip_port_map *map;

	while (1) {
		c = bus_recieve();
		
		if (c == '+') {
			msleep(20);		
		} else if (c == '\0') {
			msleep(20);
			continue;
		} else {
			msleep(1);
			continue;
		}
		
		/* read IPD */
		for (i = 0; i < 4; i++)
			tmp[i] = bus_recieve();
		
		if (strncmp(tmp, "IPD,", 4) != 0)
			continue;
		
		/* read fd */
		for (i = 0; i < 2; i++)
			tmp[i] = bus_recieve();
		
		//fd is from '0' to '9'
		fd  = tmp[0] - '0';
		
		/* get length */
		len = get_len(tmp);
		
		for (i = 0; i < len; i++)
			buf[i] = bus_recieve();
		
		break;
	}
	
	map = get_ip_port(fd);
	*ip = map->ip;
	*port = map->remote_port;
	*buf_len = len;
}

s8 udp_close(u8 id)
{
	sprintf(temp, "AT+CIPCLOSE=%d\r\n", id);
	bus_send_string(temp);
	
	msleep(20);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

s8 set_ip(char *ip)
{
	sprintf(temp, "AT+CIPSTA=\"%s\"\r\n", ip);
								
	bus_send_string(temp);
	
	msleep(20);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");	
}

s8 set_bound(void)
{
	sprintf(temp, "AT+UART=115200,8,1,0,0\r\n");
								
	bus_send_string(temp);
	
	msleep(20);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");	

}

s8 esp_reset(void)
{
	sprintf(temp, "AT+RST\r\n");
								
	bus_send_string(temp);
	
	msleep(200);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");	
}

char data_i[100];

void send_test(void)
{
	u32 ip = (192 << 24) | (168 << 16) | (1 << 8) | 20;
	u16 src_port;
	u16 port = 8888;
	char data[] = "hello world!";
	u16 len;

	udp_close(0);
	
	if (udp_setup(ip, port, port) < 0)
		err_log("udp_setup");
	
	send_data(ip, port, port, data, strlen(data));
	
	recv_data(&ip, &src_port, data_i, &len);
}


void update_esp8266(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_0, 0);
}

void work_esp8266(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_0, 1);
}

void enbale_esp8266(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_8, 1);
	//GPIO_WriteBit(GPIOB, GPIO_Pin_8, 0);
}

void disable_esp8266(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_8, 0);
}

void reset_esp8266(void)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_12, 1);
	msleep(100);
	GPIO_WriteBit(GPIOA, GPIO_Pin_12, 0);
	msleep(100);
	GPIO_WriteBit(GPIOA, GPIO_Pin_12, 1);
}

void esp8266_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB |RCC_APB2Periph_GPIOA, ENABLE);
	
	// download
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	// enable, rst
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_12 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//update_esp8266();
	work_esp8266();
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, 0);
	enbale_esp8266();
	msleep(100);	
	reset_esp8266();
	
	set_bound();
	esp_reset();

	//while (1)
	//	sleep(1);
}

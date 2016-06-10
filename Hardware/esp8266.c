#include "includes.h"
#include "delay.h"
#include "net.h"
#include "bus.h"

char temp[100];
char output[100];
static int gID = 0;
static int local_port_based=8888;

void err_log(char *s)
{
	
}

int close_udp(int id);

void bus_send_string(char *buf)
{
	int i;
	
	for (i = 0; buf[i] != '\0'; i++)
		bus_send(&buf[i], 1);
}

void bus_recieve_string(char *buf)
{
	int i = 0;
	
	while ((buf[i++] = bus_recieve()) != '\0');
}

int str_include(char *buf, char *str)
{	
	int i = 0, j = 0, k;

	for (i = 0; buf[i] != '\0'; i++) {
		for (j = 0, k = i; str[j] != '\0' && buf[k] != '\0'; j++, k++) {
			if (buf[k] == str[j])
				continue;
			else
				break;
		}
		
		if (str[j] == '\0')
			return 1;
		
		if (buf[k] == '\0')
			return 0;
	}
	
	return 0;
}

int set_mode(int mode)
{
	sprintf(temp, "AT+CWMODE=%d\r\n", mode);
	bus_send_string(temp);
	
	msleep(20);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

int set_show_ip(int mode)
{
	sprintf(temp, "AT+CIPDINFO=%d\r\n", mode);
	bus_send_string(temp);
		
	msleep(20);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

int connect_ap(char *id, char *passwd, int channel)
{	
	sprintf(temp, "AT+CWJAP=\"%s\",\"%s\"\r\n", id, passwd);
	bus_send_string(temp);	

	sleep(7);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

int set_auto_conn(int i)
{
	sprintf(temp, "AT+CWAUTOCONN=%d\r\n", i);
	bus_send_string(temp);
	
	msleep(20);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");	
}

int set_echo(int on)
{
	sprintf(temp, "ATE%d\r\n", on);
	bus_send_string(temp);
	
	msleep(20);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

int set_mux(int mode)
{
	sprintf(temp, "AT+CIPMUX=%d\r\n", mode);
	bus_send_string(temp);
	
	msleep(20);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");	
}

int udp_setup(char *ip, int dst_port)
{
	sprintf(temp, "AT+CIPSTART=%d,\"UDP\",\"%s\",%d,%d,2\r\n", gID++, ip, local_port_based++, dst_port);
	bus_send_string(temp);
	
	msleep(20);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}

static int get_id(char *ip, int port)
{
	return 0;
}

int send_data(char *ip, int port, char *data, int len)
{

	int id = get_id(ip, port);
	
	sprintf(temp, "AT+CIPSEND=%d,%d\r\n", id, len);
	bus_send_string(temp);
	
	msleep(20);
	
	bus_recieve_string(output);
	
	if (str_include(output, ">")) {
		bus_send(data, len);
		return 0;
	} else
		return -1;
}

static int get_len(char *buf, int *len)
{
	int i;
	int tmp = 0;
	
	for (i = 0; buf[i] != ':'; i++) {
		tmp *= 10;
		tmp += buf[i] - '0';
	}
	
	*len = tmp;
	
	return i;
}

static void get_data(char *src, char *dst, int len)	
{
	int i;
	
	for (i = 0; i < len; i++)
		dst[i] = src[i];
}

int recieve_data(char *ip, int port, char *data)
{
	char *tmp = data;
	int len;
	char port_str[10];
	int ip_len = strlen(ip);
	int port_len;
	int id = get_id(ip, port);
	char c;

	while (1) {
		c = bus_recieve();
		
		if (c == '+') {
			msleep(20);
			bus_recieve_string(data);			
		} else if (c == '\0') {
			msleep(20);
			continue;
		} else {
			msleep(1);
			continue;
		}
		
		if (strncmp(tmp, "IPD,", 3) != 0) {
			tmp++;
			continue;
		}
		
		tmp += 5;
		
		//id is from '0' to '9'
		if ((char)((char)id + '0') == *tmp)
			tmp += 2;
		else
			continue;
		
		tmp += get_len(tmp, &len) + 1;
		
		get_data(tmp, data, len);
		
		break;
	}
	
	return len;
}

int close_udp(int id)
{
	sprintf(temp, "AT+CIPCLOSE=%d\r\n", id);
	bus_send_string(temp);
	
	msleep(20);
	
	bus_recieve_string(output);
	
	return str_include(output, "OK");
}


char data_i[100];

void send_test(void)
{
	char ip[] = "192.168.1.20";
	int port = 8888;
	char data[] = "hello world!";

	close_udp(0);
	
	if (udp_setup(ip, port) < 0)
		err_log("udp_setup");
	
	send_data(ip, port, data, strlen(data));
	
	recieve_data(ip, port, data_i);
}

void wifi_init(void)
{
	if (set_echo(0) < 0)
		err_log("set_echo");
	
	if (set_mode(3) < 0)
		err_log("set_mode");
	
	if (connect_ap("1103", "Q!W@E#r4", 3) < 0)
		err_log("connect_ap");
	
	if (set_mux(1) < 0)
		err_log("set_mux");
	
	send_test();
	
	register_net_ops(send_data, recieve_data);
}

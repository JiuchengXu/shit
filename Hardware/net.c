#include "includes.h"

struct net_device {
	int (*send)(char *ip, int port, char *buf, int len);
	int (*recv)(char *ip, int port, char *buf);	
};

static struct net_device net_dev;
static int init_flag = 0;

int net_send(char *ip, int port, char *buf, int len)
{
	if (init_flag == 1)
		return net_dev.send(ip, port, buf, len);
	else
		return -1;
}

int net_recv(char *ip, int port, char *buf)
{
	if (init_flag == 1)
		return net_dev.recv(ip, port, buf);
	else
		return -1;
}

void register_net_ops(void *send, void *recv)
{
	if (init_flag == 0) {
		net_dev.recv = recv;
		net_dev.send = send;
		init_flag = 1;
	}
}

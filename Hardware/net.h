#ifndef __NET_H__
#define __NET_H__

int net_send(char *ip, int port, char *buf, int len);
int net_recv(char *ip, int port, char *buf);
void register_net_ops(void *send, void *recv);

#endif

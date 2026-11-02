//
// Created by ktypc on 2026/11/2.
//

#ifndef HTTP_CLIENT_NET_H
#define HTTP_CLIENT_NET_H

#include <winsock2.h>

int net_init(WSADATA *pwsa);
void net_cleanup(void);
SOCKET net_connect(const char *host, const char *port, char *peer, size_t cap);
void net_close(SOCKET s);
const char *net_last_error(void);
int net_last_error_code(void);
int net_send_all(SOCKET s, const const char *data, int len);
int net_recv_some(SOCKET s, char *buf, int cap);
#endif //HTTP_CLIENT_NET_H

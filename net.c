//
// Created by ktypc on 2026/11/2.
//

#include "net.h"

#include <string.h>
#include <ws2tcpip.h>

static int g_last_err;


static void set_error(int code)
{
    g_last_err = code;
}

int net_init(WSADATA *pwsa)
{
    WSADATA wsa;
    int rc;
    rc = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (rc != 0)
    {
        set_error(rc);
        return rc;
    }
    if (wsa.wVersion != MAKEWORD(2, 2))
    {
        set_error(10092);
        //判断Winsock版本
        WSACleanup();
        return 10092; // WSA VER NOT SUPPORTED
    }
    memcpy(pwsa, &wsa, sizeof(wsa));
    return 0;
}

void net_cleanup(void)
{
    WSACleanup();
}

SOCKET net_connect(const char *host, const char *port, char *peer, size_t cap)
{
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    SOCKET sock = INVALID_SOCKET;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       //IPV4
    hints.ai_socktype = SOCK_STREAM; //要能跑 TCP 的地址
    hints.ai_protocol = IPPROTO_TCP; //TCP协议
    int rc = getaddrinfo(host, port, &hints, &result);
    if (rc != 0)
    {
        set_error(rc);
        return INVALID_SOCKET;
    }
    struct addrinfo *p;
    for (p = result; p != NULL; p = p->ai_next)
    {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == INVALID_SOCKET)
        {
            set_error(WSAGetLastError());
            continue;
        }
        if (connect(sock, p->ai_addr, (int) p->ai_addrlen) == 0)
        {
            break;
        }
        set_error(WSAGetLastError());
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    freeaddrinfo(result);
    if (sock == INVALID_SOCKET)
    {
        return INVALID_SOCKET;
    }
    if (peer != NULL && cap >= INET_ADDRSTRLEN)
    {
        struct sockaddr_in *sin = (struct sockaddr_in *) p->ai_addr;
        inet_ntop(AF_INET, &sin->sin_addr, peer, (size_t) cap);
    }
    return sock;
}

void net_close(SOCKET s)
{
    if (s != INVALID_SOCKET)
    {
        closesocket(s);
    }
}

const char *net_last_error(void)
{
    switch (g_last_err)
    {
        case 0:
            return "无错误";
        case 10051:
            return "网络不可达";
        case 10054:
            return "连接被对端重置";
        case 10060:
            return "连接超时";
        case 10061:
            return "连接被拒绝";
        case 10093:
            return "Winsock未初始化";
        case 11001:
            return "主机未找到";
        case 11002:
            return "DNS临时失败";
        default:
            return "其他Winsock错误";
    }
}

int net_last_error_code(void)
{
    return g_last_err; //当中文描述看不出来错误时，返回对应错误码
}

int net_send_all(SOCKET s, const char *data, int len)
{
    int sent = 0;
    while (sent < len)
    {
        int n = send(s, data + sent, len - sent, 0);
        if (n == SOCKET_ERROR)
        {
            set_error(WSAGetLastError());
            return -1;
        }
        if (n == 0)
        {
            set_error(10054);//0不代表发送成功，而是连接不可用
            return -1;
        }
        sent += n;
    }
    return sent;
}

int net_recv_some(SOCKET s, char *buf, int cap)
{
    if (cap <= 0)
    {
        return -1;
    }
    int n = recv(s, buf, cap, 0);

    if (n == SOCKET_ERROR)
    {
        set_error(WSAGetLastError());
        return -1;
    }

    return n;
}

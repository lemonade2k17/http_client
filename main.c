#include <winsock2.h>


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ws2tcpip.h>

#include "net.h"
char g_err_describe[256];
const char host_name[] = "example.com";
const char host_port[] = "80";
static char response[65536];

int main(int argc, char* argv[])
{
  WSADATA wsa_data;
  char peer[INET_ADDRSTRLEN];
  SOCKET sock;
  int rc;
  rc = net_init(&wsa_data);
  if (rc != 0)
  {
    printf("初始化失败，%s(%d)", net_last_error(), net_last_error_code());
    return 1;
  }
  else
  {
    printf("初始化成功，当前协商winsock2版本：0x%x，当前支持winsock2最高版本：0x%x.\r\n", wsa_data.wVersion, wsa_data.wHighVersion);
  }
  sock = net_connect(host_name, host_port, peer, sizeof(peer));
  if (sock == INVALID_SOCKET)
  {
    printf("连接失败, %s(%d).\r\n", net_last_error(), net_last_error_code());
    net_cleanup();
    return 1;
  }
  printf("conntected %s:%s\r\n", peer, host_port);
  /**拼报文**/
  char request[1024];
  int len;
  int i;

  len = snprintf(request, sizeof(request),
                 "GET / HTTP/1.1\r\n"
                 "Host: example.com\r\n"
                 "User-Agent: handcoded-http/1.0\r\n"
                 "Accept: */*\r\n"
                 "Connection: close\r\n"
                 "\r\n");
  if (len < 0 || (size_t)len >= sizeof(request))
  {
    printf("请求被截断，缓冲区不够");
    net_close(sock);
    net_cleanup();
    return 1;
  }
  for (i = 0; i < len; i++)
  {
    if (request[i] == '\r') printf("<CR>");
    else if (request[i] == '\n') printf("<LF>\n");
    else putchar(request[i]);
  }
  /*********/

  /**发报文**/
  int sent = net_send_all(sock, request, len);
  printf("--- 发送 %d / %d 字节 ---\n", sent, len);
  if (sent != len)
  {
    printf("发送失败：%s (%d)\n", net_last_error(), net_last_error_code());
    net_close(sock);
    net_cleanup();
    return 1;
  }
  /*********/

  /**收响应**/
  int total = 0;
  int truncated = 0;
  for (;;)
  {
    int n;
    if (total >= (int)sizeof(response) - 1)
    {
      printf("  缓冲区已满，停止接收\n");
      truncated = 1; //超出缓冲区大小
      break;
    }
    n = net_recv_some(sock, response + total, (int)sizeof(response) - 1 - total);
    if (n > 0)
    {
      total += n;
      printf("  本轮收到 %d 字节，累计 %d\n", n, total);
      continue;
    }
    if (n == 0)
    {
      printf("  对端关闭连接，接收结束\n");
      break;
    }
    printf("  接收出错：%s (%d)\n", net_last_error(), net_last_error_code());
    break;
  }
  /*********/
  printf("\n--- 响应正文（%d 字节%s）---\n",
         total, (truncated != 0) ? "，已截断" : "");
  if (total > 0)
  {
    fwrite(response, 1, (size_t)total, stdout); /* 按字节原样输出 */
    fflush(stdout);                             /* 确保立刻显示 */
  }
  printf("\n--- 响应结束 ---\n");
  net_close(sock);
  net_cleanup();
  return 0;
}

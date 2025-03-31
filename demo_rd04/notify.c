#include "notify.h"
#include <FreeRTOS.h>
#include <blog.h>
#include <cli.h>
#include <http_client.h>
#include <lwip/err.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <lwip/tcp.h>
#include <stdio.h>
#include <task.h>

#define BOUNDARY "---"
static char REQUEST[512];

typedef struct {
  const char *title;
  const char *message;
  const char *priority;
} Message;

void notify_task(void *pvParameters) {

  Message *params = (Message *)pvParameters; // 接收参数
  printf("title=%s \r\nmessage=%s\r\npriority=%s\r\n", params->title,
         params->message, params->priority);
  //
  const struct addrinfo hints = {
      .ai_family = AF_INET,
      .ai_socktype = SOCK_STREAM,
  };
  struct addrinfo *res;
  struct in_addr *addr;
  int s, r;
  char recv_buf[4096];

  while (1) {
    int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

    if (err != 0 || res == NULL) {
      blog_error("DNS lookup failed err=%d res=%p", err, res);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }

    /* Code to print the resolved IP.
       Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code
     */
    addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
    blog_info("DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

    s = socket(res->ai_family, res->ai_socktype, 0);
    if (s < 0) {
      blog_error("... Failed to allocate socket.");
      freeaddrinfo(res);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }
    blog_info("... allocated socket");

    if (connect(s, res->ai_addr, res->ai_addrlen) != 0) {
      blog_error("... socket connect failed errno=%d", errno);
      close(s);
      freeaddrinfo(res);
      vTaskDelay(4000 / portTICK_PERIOD_MS);
      continue;
    }

    blog_info("... connected");
    freeaddrinfo(res);

    // 构造请求体
    char body[1024];
    snprintf(body, sizeof(body),
             "--%s\r\n"
             "Content-Disposition: form-data; name=\"title\"\r\n\r\n"
             "%s\r\n"
             "--%s\r\n"
             "Content-Disposition: form-data; name=\"message\"\r\n\r\n"
             "%s\r\n"
             "--%s\r\n"
             "Content-Disposition: form-data; name=\"priority\"\r\n\r\n"
             "%s\r\n"
             "--%s--\r\n",
             BOUNDARY, params->title, BOUNDARY, params->message, BOUNDARY,
             params->priority, BOUNDARY);

    snprintf(REQUEST, sizeof(REQUEST),
             "POST %s%s HTTP/1.0\r\n"
             "Host: %s:%s\r\n"
             "User-Agent: aithinker wb2\r\n"
             "Content-Type: multipart/form-data; boundary=%s\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             WEB_PATH_BASE, QUERY_STRING, WEB_SERVER, WEB_PORT, BOUNDARY,
             strlen(body), body);
    printf(REQUEST);
    if (write(s, REQUEST, strlen(REQUEST)) < 0) {
      blog_error("... socket send failed");
      close(s);
      vTaskDelay(4000 / portTICK_PERIOD_MS);
      continue;
    }
    blog_info("... socket send success");

    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                   sizeof(receiving_timeout)) < 0) {
      blog_error("... failed to set socket receiving timeout");
      close(s);
      vTaskDelay(4000 / portTICK_PERIOD_MS);
      continue;
    }
    blog_info("... set socket receiving timeout success");

    // FIXME fix putchar
    extern int bl_putchar(int c);

    /* Read HTTP response */
    do {
      bzero(recv_buf, sizeof(recv_buf));
      r = read(s, recv_buf, sizeof(recv_buf) - 1);
      for (int i = 0; i < r; i++) {
        bl_putchar(recv_buf[i]);
      }
    } while (r > 0);

    blog_info("... done reading from socket. Last read return=%d "
              "errno=%d\r\n",
              r, errno);
    close(s);
    for (int countdown = 10; countdown >= 0; countdown--) {
      blog_info("%d... ", countdown);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    blog_info("Starting again!");
    vTaskDelete(NULL);
  }
}

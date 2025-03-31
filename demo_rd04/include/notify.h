#ifndef __NOTIFY_H__
#define __NOTIFY_H__

#define WEB_SERVER "gotify的服务器地址"
#define WEB_PORT "端口"

#define WEB_PATH_BASE "/message"
#define QUERY_STRING "?token=(你的token)"

void notify_task(void *pvParameters);

#endif

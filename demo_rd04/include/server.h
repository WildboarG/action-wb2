#ifndef _SERVER_H_
#define _SERVER_H_

#include "FreeRTOS.h"
#include "event_groups.h"
#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sys.h"
#include "portmacro.h"
#include "string.h"
#include "web_server.h"

extern int switch_state;

void server_task(void *pvParameters);

#endif

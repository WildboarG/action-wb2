#include "hosal_i2c.h"
#include "include/rd04.h"
#include "include/server.h"
#include "notify.h"
#include "server.h"
#include <FreeRTOS.h>
#include <aos/kernel.h>
#include <aos/yloop.h>
#include <blog.h>
#include <cli.h>
#include <hal_wifi.h>
#include <hosal_gpio.h>
#include <lwip/init.h>
#include <lwip/sockets.h>
#include <lwip/tcpip.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <task.h>
#include <wifi_mgmr_ext.h>

#define ROUTER_SSID "WIFI_NAME"
#define ROUTER_PWD "PASSWORD"

typedef struct {
  const char *title;
  const char *message;
  const char *priority;
} Message;

static wifi_conf_t conf = {
    .country_code = "CN",
};

hosal_i2c_dev_t i2c0;
hosal_gpio_dev_t iicen;
hosal_gpio_dev_t show;

// uint8_t iic_slave;
uint8_t status[1] = {0};

// enable rd04
void init_gpio(void) {
  iicen.port = 17;
  iicen.config = OUTPUT_OPEN_DRAIN_NO_PULL;
  hosal_gpio_init(&iicen);
  hosal_gpio_output_set(&iicen, 1);
  show.port = 14;
  show.config = INPUT_HIGH_IMPEDANCE;
  hosal_gpio_init(&show);
  printf("iic enable");
}

// init iic
void i2c_master_init(void) {
  int ret = -1;
  int i = 0;
  int count = 0;
  i2c0.port = 0;
  i2c0.config.freq = 100000; /* only support 305Hz~100000Hz */
  i2c0.config.address_width =
      HOSAL_I2C_ADDRESS_WIDTH_7BIT;         /* only support 7bit */
  i2c0.config.mode = HOSAL_I2C_MODE_MASTER; /* only support master */
  i2c0.config.scl = 12;
  i2c0.config.sda = 11;

  /* init i2c with the given settings */
  ret = hosal_i2c_init(&i2c0);
  if (ret != 0) {
    hosal_i2c_finalize(&i2c0);
    blog_error("hosal i2c init failed!\r\n");
    return;
  }
}
void rd_task(void *pvParameters) {
  // 构建消息内容结构体
  static const char title[] = "RD04";
  static const char message[] = "Trigger the radar notification";
  static const char priority[] = "0";
  static Message msg = {title, message, priority};
  printf("title=%s \r\nmessage=%s\r\npriority=%s\r\n", msg.title, msg.message,
         msg.priority);

  while (1) {
    // 循环检测rd输出
    hosal_gpio_input_get(&show, status);
    printf("status = %d\r\n", status[0]);
    vTaskDelay(3000);
    // 触发通知
    if (status[0] && switch_state) {
      //
      printf("发起通知\r\n");
      xTaskCreate(&notify_task, "notify_task", 1024 * 10, &msg, 5, NULL);
    }
  }
}

/**
 * @brief wifi_sta_connect
 *        wifi station mode connect start
 * @param ssid
 * @param password
 */
static void wifi_sta_connect(char *ssid, char *password) {
  wifi_interface_t wifi_interface;
  wifi_interface = wifi_mgmr_sta_enable();
  wifi_mgmr_sta_connect(wifi_interface, ssid, password, NULL, NULL, 0, 0);
}

/**
 * @brief event_cb_wifi_event
 *      wifi connet ap event Callback function
 * @param event
 * @param private_data
 */
static void event_cb_wifi_event(input_event_t *event, void *private_data) {
  static char *ssid;
  static char *password;

  switch (event->code) {
  case CODE_WIFI_ON_INIT_DONE: {
    printf("[APP] [EVT] INIT DONE %lld\r\n", aos_now_ms());
    wifi_mgmr_start_background(&conf);
  } break;
  case CODE_WIFI_ON_MGMR_DONE: {
    printf("[APP] [EVT] MGMR DONE %lld\r\n", aos_now_ms());
    //_connect_wifi();

    wifi_sta_connect(ROUTER_SSID, ROUTER_PWD);
  } break;
  case CODE_WIFI_ON_SCAN_DONE: {
    printf("[APP] [EVT] SCAN Done %lld\r\n", aos_now_ms());
    // wifi_mgmr_cli_scanlist();
  } break;
  case CODE_WIFI_ON_DISCONNECT: {
    printf("[APP] [EVT] disconnect %lld\r\n", aos_now_ms());
  } break;
  case CODE_WIFI_ON_CONNECTING: {
    printf("[APP] [EVT] Connecting %lld\r\n", aos_now_ms());
  } break;
  case CODE_WIFI_CMD_RECONNECT: {
    printf("[APP] [EVT] Reconnect %lld\r\n", aos_now_ms());
  } break;
  case CODE_WIFI_ON_CONNECTED: {
    printf("[APP] [EVT] connected %lld\r\n", aos_now_ms());
  } break;
  case CODE_WIFI_ON_PRE_GOT_IP: {
    printf("[APP] [EVT] connected %lld\r\n", aos_now_ms());
  } break;
  case CODE_WIFI_ON_GOT_IP: {
    printf("[APP] [EVT] GOT IP %lld\r\n", aos_now_ms());
    printf("[SYS] Memory left is %d Bytes\r\n", xPortGetFreeHeapSize());
    // wifi connection succeeded, create tcp server task
    xTaskCreate(&rd_task, "RD_task", 1024, NULL, 5, NULL);
  } break;
  case CODE_WIFI_ON_PROV_SSID: {
    printf("[APP] [EVT] [PROV] [SSID] %lld: %s\r\n", aos_now_ms(),
           event->value ? (const char *)event->value : "UNKNOWN");
    if (ssid) {
      vPortFree(ssid);
      ssid = NULL;
    }
    ssid = (char *)event->value;
  } break;
  case CODE_WIFI_ON_PROV_BSSID: {
    printf("[APP] [EVT] [PROV] [BSSID] %lld: %s\r\n", aos_now_ms(),
           event->value ? (const char *)event->value : "UNKNOWN");
    if (event->value) {
      vPortFree((void *)event->value);
    }
  } break;
  case CODE_WIFI_ON_PROV_PASSWD: {
    printf("[APP] [EVT] [PROV] [PASSWD] %lld: %s\r\n", aos_now_ms(),
           event->value ? (const char *)event->value : "UNKNOWN");
    if (password) {
      vPortFree(password);
      password = NULL;
    }
    password = (char *)event->value;
  } break;
  case CODE_WIFI_ON_PROV_CONNECT: {
    printf("[APP] [EVT] [PROV] [CONNECT] %lld\r\n", aos_now_ms());
    printf("connecting to %s:%s...\r\n", ssid, password);
    wifi_sta_connect(ssid, password);
  } break;
  case CODE_WIFI_ON_PROV_DISCONNECT: {
    printf("[APP] [EVT] [PROV] [DISCONNECT] %lld\r\n", aos_now_ms());
  } break;
  default: {
    printf("[APP] [EVT] Unknown code %u, %lld\r\n", event->code, aos_now_ms());
    /*nothing*/
  }
  }
}

static void proc_main_entry(void *pvParameters) {

  aos_register_event_filter(EV_WIFI, event_cb_wifi_event, NULL);
  hal_wifi_start_firmware_task();
  aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0);
  vTaskDelete(NULL);
}

void main(void) {
  init_gpio();
  puts("[USER] init i2c master\r\n");
  i2c_master_init();
  puts("[USER] INIT RD04\r\n");
  vTaskDelay(1000);
  RD_04_Init();
  puts("[OS] Starting TCP/IP Stack...\r\n");
  tcpip_init(NULL, NULL);
  puts("[OS] proc_main_entry task...\r\n");
  xTaskCreate(proc_main_entry, (char *)"main_entry", 1024, NULL, 15, NULL);
  xTaskCreate(server_task, NULL, 1024 * 10, NULL, 15, NULL);
}

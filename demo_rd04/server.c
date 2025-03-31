#include <server.h>

#define SERVER_PORT 80

int switch_state = 0; // 0: 关, 1: 开

const static char http_html_hdr[] =
    "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";

const static char http_index_html[] =
    "<!DOCTYPE html>"
    "<html lang=\"zh\">"
    "<head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
    "<title>哨兵模式</title>"
    "<link rel=\"stylesheet\" "
    "href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/"
    "bootstrap.min.css\">"
    "<script "
    "src=\"https://cdn.jsdelivr.net/npm/jquery@3.6.0/dist/jquery.min.js\"></"
    "script>"
    "<style>"
    "body {"
    "    background: linear-gradient(135deg, #e3f2fd, #fce4ec);"
    "    font-family: Arial, sans-serif;"
    "    height: 100vh;"
    "    display: flex;"
    "    justify-content: center;"
    "    align-items: center;"
    "    text-align: center;"
    "}"
    ".card {"
    "    background: rgba(255, 255, 255, 0.9);"
    "    padding: 30px;"
    "    border-radius: 12px;"
    "    box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);"
    "    width: 300px;"
    "}"
    ".switch-container {"
    "    margin-top: 20px;"
    "}"
    ".form-switch .form-check-input {"
    "    width: 3rem;"
    "    height: 1.5rem;"
    "}"
    ".status-text {"
    "    font-size: 1.2rem;"
    "    font-weight: bold;"
    "    margin-top: 10px;"
    "    color: #555;"
    "}"
    "</style>"
    "</head>"
    "<body>"
    "<div class=\"card\">"
    "  <h2>哨兵模式</h2>"
    "  <div class=\"switch-container\">"
    "    <div class=\"form-check form-switch\">"
    "      <input class=\"form-check-input\" type=\"checkbox\" "
    "id=\"toggleSwitch\">"
    "      <label class=\"form-check-label\" for=\"toggleSwitch\">"
    "        <span id=\"statusText\">加载中...</span>"
    "      </label>"
    "    </div>"
    "  </div>"
    "</div>"
    "<script>"
    "function updateStatus(state) {"
    "  $('#toggleSwitch').prop('checked', state == 1);"
    "  $('#statusText').text(state == 1 ? '开' : '关');"
    "}"
    "$(document).ready(function() {"
    "  $.get('/get_state', function(response) { "
    "updateStatus(parseInt(response)); });"
    "  $('#toggleSwitch').change(function() {"
    "    let newState = $(this).is(':checked') ? 1 : 0;"
    "    $.get('/set_state?value=' + newState, function() { "
    "updateStatus(newState); });"
    "  });"
    "});"
    "</script>"
    "</body>"
    "</html>";

static void web_httpserver(struct netconn *conn) {
  struct netbuf *inputbuf;
  char *buf;
  uint16_t buflen;
  err_t err;

  err = netconn_recv(conn, &inputbuf);
  if (err != ERR_OK) {
    return;
  }
  netbuf_data(inputbuf, (void **)&buf, &buflen);
  // printf("Received HTTP request:%s\r\n", buf);
  //
  //  logic func
  if (strstr(buf, "GET /get_state")) {
    char response[20];
    sprintf(response, "HTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\n%d",
            switch_state);
    netconn_write(conn, response, strlen(response), NETCONN_NOCOPY);
  } else if (strstr(buf, "GET /set_state?value=1")) {
    switch_state = 1;
    printf("[哨兵1号]: 开启警戒\r\n");
    netconn_write(conn, "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK", 37,
                  NETCONN_NOCOPY);
  } else if (strstr(buf, "GET /set_state?value=0")) {
    switch_state = 0;
    printf("[哨兵1号]: 警戒解除\r\n");
    netconn_write(conn, "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK", 37,
                  NETCONN_NOCOPY);
  } else {
    netconn_write(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n",
                  44, NETCONN_NOCOPY);
    netconn_write(conn, http_index_html, sizeof(http_index_html) - 1,
                  NETCONN_NOCOPY);
  }

  netconn_close(conn);
  netbuf_delete(inputbuf);
}

void server_task(void *pvParameters) {
  struct netconn *conn, *newconn;
  err_t err;
  conn = netconn_new(NETCONN_TCP);

  netconn_bind(conn, NULL, SERVER_PORT);
  netconn_listen(conn);

  while (1) {
    err = netconn_accept(conn, &newconn);
    if (err != ERR_OK) {
      netconn_close(conn);
      netconn_delete(conn);
      break;
    }

    web_httpserver(newconn);
    netconn_delete(newconn);
  }
}

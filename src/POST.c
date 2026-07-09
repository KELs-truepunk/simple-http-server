#include "POST.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>

// Превращаем строку в понятное для switch число
post_route_t get_post_route(const char* path) {
    if (strcmp(path, "/api/echo") == 0) {
        return ROUTE_TEST_ECHO;
    }
    return ROUTE_NOT_FOUND;
}

// Тестовый обработчик: просто печатает тело в консоль и отвечает клиенту
void handle_test_echo(int newsockfd, const char* body) {
    printf("[POST /api/echo] Пришли данные: %s\n", body);

    // Формируем простой текст ответа
    char json_resp[] = "{\"status\":\"ok\",\"msg\":\"Server received your POST!\"}";
    char http_resp[256];

    snprintf(http_resp, sizeof(http_resp),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             strlen(json_resp), json_resp);

    send(newsockfd, http_resp, strlen(http_resp), 0);
}

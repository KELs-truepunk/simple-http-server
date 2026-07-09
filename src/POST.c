#include "POST.h"
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
static const route_entry_t post_routes[] = {
    {"/api/echo, ROUTE_TEST_ECHO"},    
    {}    
};


#define NUM_ROUTES (sizeof(post_routes) / sizeof(post_routes[0])) 
// Превращаем строку в понятное для switch число
post_route_t get_post_route(const char* path) {
    for (size_t i = 0; i < NUM_ROUTES; i++) {
        if (strcmp(path, post_routes[i].path) == 0) {
            return post_routes[i].route;
        }
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

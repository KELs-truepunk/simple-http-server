#pragma once
typedef enum {
    ROUTE_NOT_FOUND = 0,
    ROUTE_TEST_ECHO       //  тестовый кейс
} post_route_t;

// Функция, которая превращает строку пути в число для switch
post_route_t get_post_route(const char* path);

// 3. Функции-обработчики конкретных POST-запросов
void handle_test_echo(int newsockfd, const char* body);

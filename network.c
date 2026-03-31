//
// Created by Admin on 31.03.2026.
//

#include "network.h"


void print_peer_info(int client_fd) {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    // Получаем данные об удаленном узле
    if (getpeername(client_fd, (struct sockaddr *)&addr, &addr_len) == 0) {
        char ip_str[INET_ADDRSTRLEN];
        // Преобразуем IP в читаемый вид и достаем порт
        inet_ntop(AF_INET, &addr.sin_addr, ip_str, sizeof(ip_str));
        int port = ntohs(addr.sin_port);

        printf("Подключен клиент: %s:%d\n\n", ip_str, port);
    } else {
        perror("Ошибка getpeername\n");
    }
}
char* get_new_request(int sockfd) {
    char* request = (char*)calloc(4096, sizeof(char));
    if (request == NULL) {
        perror("calloc");
        free(request);
        return NULL;
    }
    int bytes = (int)recv(sockfd, request, 4096 - 1, 0);
    if (bytes <= 0) {
        perror("recv");
        free(request);
        return NULL;
    }
    request[bytes] = '\0';
    return request;
}


// Очистка зомби-процессов
void sigchld_handler(int s) {
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void server_hints(addrinfo* hints) {
    memset(hints, 0, sizeof(addrinfo));//хыхыхы нолики
    hints->ai_flags = AI_PASSIVE;//дайте ка мне любой айпи на любом интерфейсе, и пачку сухариков
    hints->ai_family = AF_UNSPEC;//ну ipv6 круто
    hints->ai_socktype = SOCK_STREAM;//TCP тк UDP просто не уместно
}
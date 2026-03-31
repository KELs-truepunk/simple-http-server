

#include <signal.h>
#include "http.h"
#include "file.h"
#include "network.h"

int err = 0;

int main(void){
        addrinfo hints;
        addrinfo *res;
        server_hints(&hints);

        int status = getaddrinfo(NULL, "8080", &hints, &res); //res - основа
        if (status != 0) {
                freeaddrinfo(res);
                return status;
        }

        int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        int yes = 1;
        //след строка говорит то, что после завершения порт сразу освободится, хз так в доках написано
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (sockfd == -1) {
                perror("socket");
                return -1;
        }
        //выпрашиваем у linux-а 8080-ой порт
        if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
                perror("bind"); // Если порт 8080 занят, тут будет ошибка
                return -1;
        }
        //начинаем слушать наш порт
        if (listen(sockfd, 5) == -1) {
                perror("listen");
                return -1;
        }
        freeaddrinfo(res); //освобождаем память res, тк она уже не нужна потому

        //Day-Z или PvZ :-)
        struct sigaction sa;
        sa.sa_handler = sigchld_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGCHLD, &sa, NULL);

        puts("Server is running on port 8080...");

        while (1) {

                int newsockfd = accept(sockfd, NULL, NULL);//новое соединение
                if (newsockfd == -1) {
                        perror("accept"); //если не получилось выходим
                        err = 1;
                        continue;
                }

                pid_t pid = fork();
                if (pid == -1) {
                        perror("fork"); //если не получился новый процесс
                        err = 1;
                }
                if (pid == 0) {
                        err = 0;
                        print_peer_info(newsockfd);
                        //дочерний процесс обработки подключений
                        close(sockfd);
                        char* request = get_new_request(newsockfd);
                        if (request == NULL) {
                                goto clean;
                        }
                        char* first_line = strtok(request, "\r\n");
                        if (!first_line) {
                                err = 1;
                                goto clean;
                        }
                        char method[16], path[256];
                        if (sscanf(first_line, "%15s %255s", method, path) != 2) {
                                goto clean;
                        }
                        //если пришел GET запрос, то возращаем страницу
                        if (strcmp("GET", method) == 0) {
                                NEW_FILE file;

                                file.filepath = path;
                                if (file.filepath[0] == '/') {
                                        file.filepath++;
                                }
                                if (strlen(file.filepath) == 0 || file.filepath[0] == '\0') {
                                        strcat(file.filepath, "index.html");
                                }
                                char* ext = get_extension(path);
                                void* file_status = file_open(&file);
                                char* status_line = HTTP.success.ok;

                                if (file_status == NULL) {
                                        status_line = HTTP.client_error.not_found;
                                        file.filepath = "404.html"; // Пытаемся открыть страницу ошибки 404
                                        file_status = file_open(&file);
                                        if (file_status == NULL) {
                                                // Если даже 404.html нет, шлем пустой ответ или текст
                                                send(newsockfd, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n", 44, 0);
                                                err = 2;
                                                file_close(&file);
                                                goto clean; // Переход к очистке ресурсов
                                        }
                                }

                                file_size(&file);
                                char* mime = get_mime_type(ext);

                                const int head_status = send_header(newsockfd, file.fsize, mime, status_line);
                                if (head_status >= 0) {
                                        printf("%s\n", status_line);
                                }else {
                                        printf("%s\n", status_line);
                                }

                                if (send_file(newsockfd, &file) == 0) {
                                        printf("page ""%s"" successfully sent (%zu bytes)\n", file.filepath, (file.fsize * sizeof(size_t)));//отправили что просили
                                }else {
                                        perror("send_file");
                                        file_close(&file);
                                        goto clean;
                                }
                                file_close(&file); //закрываем файл
                        }else if (strcmp("HEAD", method) == 0) {
                                NEW_FILE file;

                                file.filepath = path;
                                if (file.filepath[0] == '/') {
                                        file.filepath++;
                                }
                                if (strlen(file.filepath) == 0 || file.filepath[0] == '\0') {
                                        strcat(file.filepath, "index.html");
                                }
                                char* ext = get_extension(path);
                                void* file_status = file_open(&file);
                                char* status_line = HTTP.success.ok;

                                if (file_status == NULL) {
                                        // Если даже 404.html нет, шлем пустой ответ или текст
                                        send(newsockfd, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n", 44, 0);
                                        file_close(&file);
                                        err = 2;
                                        goto clean; // Переход к очистке ресурсов
                                        }

                                file_size(&file);
                                char* mime = get_mime_type(ext);

                                const int head_status = send_header(newsockfd, file.fsize, mime, status_line);
                                if (head_status >= 0) {
                                        printf("%s\n", status_line);
                                }else {
                                        printf("%s\n", status_line);
                                }
                                file_close(&file);
                        }
                        clean:
                        free(request);
                        shutdown(newsockfd, 2); //закрываем и запрещаем нам стучаться(2)
                        close(newsockfd);//закрываем новое подключение
                        exit(err);
                }else {
                        //родительский процесс только закрывает новые подключения
                        close(newsockfd);
                }
        }
        close(sockfd); //закрываем всё
        return 0;
}

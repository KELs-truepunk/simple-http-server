
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>

typedef struct addrinfo addrinfo;
const char* get_mime_type(const char* ext) {
        if (strcmp(ext, ".html") == 0) return "text/html";
        if (strcmp(ext, ".js") == 0)   return "application/javascript";
        if (strcmp(ext, ".css") == 0)  return "text/css";
        if (strcmp(ext, ".jpg") == 0)  return "image/jpeg";
        if (strcmp(ext, ".png") == 0)  return "image/png";
        return "application/octet-stream"; // Тип по умолчанию для бинарных файлов
}

FILE* page_open(const char* file_path) {
        return fopen(file_path, "rb");
}
char* get_extension(const char* file_path) {
        char* extension = strrchr(file_path, '.');
        if (extension == NULL) {
                extension = strrchr(file_path, '\0');
        }
        return extension;
}
int send_file(int socketfd, FILE* file) {
        char* buffer = malloc(4096 * sizeof(char));
        if (buffer == NULL) {
                perror("malloc");
                return -1;
        }
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, 4096, file)) > 0) {
                send(socketfd, buffer, bytes_read, 0);
        }
        free(buffer);
        return 0;
}
// Очистка зомби-процессов
void sigchld_handler(int s) {
        while(waitpid(-1, NULL, WNOHANG) > 0);
}
//вычисление размера файла
size_t file_size(FILE* f) {
        fseek(f, 0, SEEK_END); //перемещаемся в конец файла
        size_t sz = ftell(f); // понимаем в какой жопе оказались относительно одного char :-)
        //переход обратно, тк работаем с указателем (где взял, туда и положил)
        if (fseek(f, 0, SEEK_SET) != 0) {
                perror("fseek");//ну всякое бывает
                return -1;
        }
        //здесь раньше было это, но кто-то открыл документацию glib-а - rewind(f)
        return sz;//возращаем размер файла
}
void server_hints(addrinfo* hints) {
        memset(hints, 0, sizeof(addrinfo));//хыхыхы нолики
        hints->ai_flags = AI_PASSIVE;//дайте ка мне любой айпи на любом интерфейсе, и пачку сухариков
        hints->ai_family = AF_UNSPEC;//ну ipv6 круто
        hints->ai_socktype = SOCK_STREAM;//TCP тк UDP просто не уместно
}

int main(void){
        addrinfo hints;
        addrinfo *res, *p;
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
                        return -1;
                }

                pid_t pid = fork();
                if (pid == -1) {
                        perror("fork"); //если не получился новый процесс
                        return -1;
                }
                if (pid == 0) {
                        //дочерний процесс обработки подключений
                        close(sockfd);
                        char* request = (char*)malloc(4096 * sizeof(char));
                        if (request == NULL) {
                                perror("malloc");
                                free(request);
                                return -1;
                        }
                        int bytes = (int)recv(newsockfd, request, 4096 - 1, 0);
                        if (bytes <= 0) {
                                perror("recv");
                                free(request);
                                close(newsockfd);
                                return -1;
                        }
                        request[bytes] = '\0';
                        char* first_line = strtok(request, "\r\n");
                        if (!first_line) {
                                free(request);
                                close(newsockfd);
                                return -1;
                        }
                        char method[16], path[256];
                        if (sscanf(first_line, "%s %s", method, path) != 2) {
                                free(request);
                                close(newsockfd);
                                return -1;
                        }
                        //если пришео GET запрос то возращаем страницу
                        if (strcmp("GET", method) == 0) {
                                const char* file_path = path;
                                if (file_path[0] == '/') {
                                        file_path++;
                                }
                                if (strlen(file_path) == 0 || file_path[0] == '\0') {
                                        strcat(file_path, "index.html");
                                }
                                char* ext = get_extension(path);
                                FILE* file = page_open(file_path);
                                const char* status_line = "HTTP/1.1 200 OK";

                                if (file == NULL) {
                                        status_line = "HTTP/1.1 404 Not Found";
                                        file = fopen("404.html", "rb"); // Пытаемся открыть страницу ошибки
                                        if (file == NULL) {
                                                // Если даже 404.html нет, шлем пустой ответ или текст
                                                send(newsockfd, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n", 44, 0);
                                                goto clean; // Переход к очистке ресурсов
                                        }
                                }

                                const size_t fsize = file_size(file);
                                const char* mime = get_mime_type(ext);

                                char header[512] = {0};
                                memset(header, 0, sizeof(header));

                                sprintf(header,
                                        "%s\r\n"
                                        "Content-Type: %s; charset=utf-8\r\n"
                                        "Content-Length: %ld\r\n"
                                        "Connection: close\r\n"
                                        "\r\n",status_line, mime, fsize);
                                send(newsockfd, header,  strlen(header), 0);//отвечаем

                                if (send_file(newsockfd, file) == 0) {
                                        printf("page successfully sent\n");//отправили что просили
                                }else {
                                        perror("send_file");
                                        close(newsockfd);
                                        free(request);
                                        fclose(file);
                                        return -1;
                                }
                                fclose(file); //закрываем файл
                        }
                        clean:
                        free(request);
                        shutdown(newsockfd, 2); //закрываем и запрещаем нам стучаться(2)
                        close(newsockfd);//закрываем новое подключение
                        exit(0);
                }else {
                        //родительский процесс только закрывает новые подключения
                        close(newsockfd);
                }
        }
        close(sockfd); //закрываем всё
        return 0;

}

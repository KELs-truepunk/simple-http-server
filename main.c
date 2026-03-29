
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

                        FILE* index = fopen("index.html", "rb"); //открываем файл с html
                        if (index == NULL) {
                                perror("fopen"); //если не получилось открыть выходим
                                return -1;
                        }
                        const size_t fsize = file_size(index);

                        char header[256] = {0};
                        memset(header, 0, sizeof(header));
                        sprintf(header,
                                         "HTTP/1.1 200 OK\r\n"
                                        "Content-Type: text/html; charset=utf-8\r\n"
                                        "Content-Length: %ld\r\n"
                                        "Connection: close\r\n"
                                        "\r\n", fsize);
                        send(newsockfd, header,  strlen(header), 0);

                        char buffer[1024];
                        size_t bytes_read;
                        while ((bytes_read = fread(buffer, 1, strlen(buffer), index)) > 0) {
                                send(newsockfd, buffer, bytes_read, 0);
                        }

                        fclose(index); //закрываем файл
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

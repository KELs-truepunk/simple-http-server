//
// Created by Admin on 01.04.2026.
//
#include "methods.h"
#include "file.h"
#include "http.h"
#include "POST.h"

int GET( int newsockfd, char* path) {
    NEW_FILE file;

    file.filepath = path;
    if (file.filepath[0] == '/') {
        file.filepath++;
    }
    if (strlen(file.filepath) == 0 || file.filepath[0] == '\0') {
        strcat(file.filepath, "public/index.html");
    }
    const char* ext = get_extension(path);
    void* file_status = file_open(&file);
    char* status_line = HTTP.success.ok;

    if (file_status == NULL) {
        status_line = HTTP.client_error.not_found;
        file.filepath = "public/err_pages/404.html"; // Пытаемся открыть страницу ошибки 404
        file_status = file_open(&file);
        if (file_status == NULL) {
            // Если даже 404.html нет, шлем пустой ответ или текст
            send(newsockfd, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n", 44, 0);
            file_close(&file);
            return -1;
        }
    }

    file_size(&file);
    char* mime = get_mime_type(ext);

    const int head_status = send_header(newsockfd, file.fsize, mime, status_line);
    if (head_status >= 0) {
        printf("%s\n", status_line);
    }else {
        printf("%s\n", status_line);
        return head_status;
    }

    if (send_file(newsockfd, &file) == 0) {
        printf("page ""%s"" successfully sent (%zu bytes)\n", file.filepath, (file.fsize * sizeof(size_t)));//отправили что просили
    }else {
        perror("send_file");
        file_close(&file);
        return -1;
    }
    file_close(&file); //закрываем файл
    return 0;
}

int HEAD(int newsockfd, char* path) {
    NEW_FILE file;

    file.filepath = path;
    if (file.filepath[0] == '/') {
        file.filepath++;
    }
    if (strlen(file.filepath) == 0 || file.filepath[0] == '\0') {
        strcat(file.filepath, "public/index.html");
    }
    const char* ext = get_extension(path);
    void* file_status = file_open(&file);
    char* status_line = HTTP.success.ok;

    if (file_status == NULL) {
        status_line = HTTP.client_error.not_found;
        file.filepath = "public/err_pages/404.html"; // Пытаемся открыть страницу ошибки 404
        file_status = file_open(&file);
        if (file_status == NULL) {
            // Если даже 404.html нет, шлем пустой ответ или текст
            send(newsockfd, "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n", 44, 0);
            file_close(&file);
            return -1;
        }
    }

    file_size(&file);
    char* mime = get_mime_type(ext);

    const int head_status = send_header(newsockfd, file.fsize, mime, status_line);
    if (head_status >= 0) {
        printf("%s\n", status_line);
    }else {
        printf("%s\n", status_line);
        return head_status;
    }
    file_close(&file);
}
int POST(int newsockfd, char* request, char* path) {
    // 1. Ищем, где в буфере get_new_request заканчиваются заголовки
    char *body_start = strstr(request, "\r\n\r\n");
    if (!body_start) {
        send(newsockfd, "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n", 44, 0);
        return -1;
    }
    body_start += 4; // Сдвигаем указатель на начало тела (пропускаем \r\n\r\n)

    // 2. Вытаскиваем размер тела из заголовка Content-Length
    int content_length = 0;
    char *content_length_ptr = strstr(request, "Content-Length:");
    if (content_length_ptr) {
        content_length = atoi(content_length_ptr + 15); // "Content-Length:" — это 15 символов
    }

    // Если клиент прислал POST без тела
    if (content_length <= 0) {
        char *empty_resp = "HTTP/1.1 200 OK\r\nContent-Length: 15\r\n\r\n{\"body\":\"empty\"}";
        send(newsockfd, empty_resp, strlen(empty_resp), 0);
        return 0;
    }

    // 3. Выделяем память под ПОЛНОЕ тело POST-запроса (+1 для '\0')
    char *full_body = (char*)malloc(content_length + 1);
    if (full_body == NULL) {
        perror("malloc full_body");
        return -1;
    }

    // Считаем, сколько байт тела мы УЖЕ прочитали за один вызов recv в get_new_request
    int bytes_already_read = strlen(request) - (body_start - request);
    if (bytes_already_read > content_length) {
        bytes_already_read = content_length;
    }

    // Копируем то, что уже успело долететь
    memcpy(full_body, body_start, bytes_already_read);
    int total_bytes_read = bytes_already_read;

    // 4. Дочитываем хвост из сокета, если тело зашло не полностью (привет, Beej's Guide!)
    while (total_bytes_read < content_length) {
        int n = recv(newsockfd, full_body + total_bytes_read, content_length - total_bytes_read, 0);
        if (n < 0) {
            perror("recv error in POST loop");
            free(full_body);
            return -1;
        }
        if (n == 0) {
            break; // Клиент неожиданно закрыл соединение
        }
        total_bytes_read += n;
    }
    full_body[total_bytes_read] = '\0'; // Закрываем Си-строку нулем для безопасности

    post_route_t route = get_post_route(path);

    switch (route) {
        case ROUTE_TEST_ECHO:
            handle_test_echo(newsockfd, full_body);
            break;

        case ROUTE_NOT_FOUND:
        default: {
            // Если путь неизвестен — шлем честный 404
            char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
            send(newsockfd, not_found, strlen(not_found), 0);
            break;
        }
    }

    // Освобождаем выделенную память, чтобы не было утечек в процессе fork()
    free(full_body);
    return 0;
}


//
// Created by Admin on 01.04.2026.
//
#include "methods.h"
#include "file.h"
#include "http.h"

int GET( int newsockfd, char* path) {
    NEW_FILE file;

    file.filepath = path;
    if (file.filepath[0] == '/') {
        file.filepath++;
    }
    if (strlen(file.filepath) == 0 || file.filepath[0] == '\0') {
        strcat(file.filepath, "index.html");
    }
    const char* ext = get_extension(path);
    void* file_status = file_open(&file);
    char* status_line = HTTP.success.ok;

    if (file_status == NULL) {
        status_line = HTTP.client_error.not_found;
        file.filepath = "err_pages/404.html"; // Пытаемся открыть страницу ошибки 404
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
        strcat(file.filepath, "index.html");
    }
    const char* ext = get_extension(path);
    void* file_status = file_open(&file);
    char* status_line = HTTP.success.ok;

    if (file_status == NULL) {
        status_line = HTTP.client_error.not_found;
        file.filepath = "err_pages/404.html"; // Пытаемся открыть страницу ошибки 404
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

int POST(int newsockfd, char* request) {
    char *body = strstr(request, "\r\n\r\n");
    if (body) {
        body += 4; // Пропускаем \r\n\r\n
        printf("new POST: %s\n", body);
    }

    // Обязательно отвечаем клиенту, иначе он будет ждать вечно
    char *resp = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
    send(newsockfd, resp, strlen(resp), 0);

    return 0;
}

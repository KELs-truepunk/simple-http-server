#include "http.h"

char* new_header(char* mime, char* status_line, size_t fsize) {
    char* header = (char*)calloc(4096, sizeof(char));
    if (header == NULL) {
        perror("calloc");
        return NULL;
    }
    sprintf(header,
            "%s\r\n"
            "Content-Type: %s; charset=utf-8\r\n"
            "Content-Length: %ld\r\n"
            "Connection: close\r\n"
            "\r\n",status_line, mime, fsize);

    return header;
}
int send_header(int newsockfd, size_t fsize, char* mime, char* status_line) {
    char* header = new_header(mime, status_line, fsize);
    if (header == NULL) {
        perror("header error");
        return -1;
    }else {
        const int send_status = (int)send(newsockfd, header,  strlen(header), 0);//отвечаем
        free(header);
        return send_status;
    }
}
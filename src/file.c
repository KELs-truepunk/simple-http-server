#include "file.h"

char* get_extension(const char* file_path) {
    char* extension = strrchr(file_path, '.');
    if (extension == NULL) {
        extension = strrchr(file_path, '\0');
    }
    return extension;
}

char* get_mime_type(const char* ext) {
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".js") == 0)   return "application/javascript";
    if (strcmp(ext, ".css") == 0)  return "text/css";
    if (strcmp(ext, ".jpg") == 0)  return "image/jpeg";
    if (strcmp(ext, ".png") == 0)  return "image/png";
    return "application/octet-stream"; // Тип по умолчанию для бинарных файлов
}

NEW_FILE* file_open(NEW_FILE* file) {
    file->file = fopen(file->filepath, "rb");
    if (file->file == NULL) {
        perror("fopen");
        return NULL;
    }
    return file;
}
//вычисление размера файла
size_t file_size(NEW_FILE* file) {
    fseek(file->file, 0, SEEK_END); //перемещаемся в конец файла
    size_t sz = ftell(file->file); // понимаем в какой жопе оказались относительно одного char :-)
    //переход обратно, тк работаем с указателем (где взял, туда и положил)
    if (fseek(file->file, 0, SEEK_SET) != 0) {
        perror("fseek");//ну всякое бывает
        return -1;
    }
    //здесь раньше было это, но кто-то открыл документацию glib-а - rewind(f)
    file->fsize = sz;
    return sz;//возращаем размер файла
}

int send_file(int socketfd, NEW_FILE* file) {
    char* buffer = calloc(4096, sizeof(char));
    if (buffer == NULL) {
        perror("calloc");
        return -1;
    }
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, 4096, file->file)) > 0) {
        int send_status = (int)send(socketfd, buffer, bytes_read, 0);
        if (send_status >= 0) continue;
        else {
            free(buffer);
            return -1;
        }
    }
    free(buffer);
    return 0;
}

int file_close(NEW_FILE* file) {
    int close_status = fclose(file->file);
    file->filepath = NULL;
    file->fsize = 0;
    file->ext = NULL;
    return close_status;
}
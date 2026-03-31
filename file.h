//
// Created by Admin on 31.03.2026.
//
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
typedef struct  {
    FILE* file;
    size_t fsize;
    char* filepath;
    char* ext;
}NEW_FILE;

char* get_extension(const char* file_path);

char* get_mime_type(const char* ext);
//открываем файл с компа по пути
NEW_FILE* file_open(NEW_FILE* file);
//вычисление размера файла
size_t file_size(NEW_FILE* file);

//отправка файла по сокету
int send_file(int socketfd, NEW_FILE* file);

int file_close(NEW_FILE* file);
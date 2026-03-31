//
// Created by Admin on 31.03.2026.
//
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>

char* get_extension(const char* file_path);

char* get_mime_type(const char* ext);
//открываем файл с компа по пути
FILE* file_open(const char* file_path);
//вычисление размера файла
size_t file_size(FILE* f);

//отправка файла по сокету
int send_file(int socketfd, FILE* file);

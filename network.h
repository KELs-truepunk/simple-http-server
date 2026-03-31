//
// Created by Admin on 31.03.2026.
//
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#pragma once
typedef struct addrinfo addrinfo;

void print_peer_info(int client_fd);
char* get_new_request(int sockfd);
void sigchld_handler(int s);
void server_hints(addrinfo* hints);
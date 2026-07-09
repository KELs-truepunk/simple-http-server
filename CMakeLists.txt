cmake_minimum_required(VERSION 4.2)
project(server C)

set(CMAKE_C_STANDARD 11)

add_executable(server 
main.c 
file.c file.h
http.c http.h  
network.c network.h 
methods.c methods.h
POST.c POST.h
)

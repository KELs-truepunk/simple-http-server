cmake_minimum_required(VERSION 4.2)
project(server C)

set(CMAKE_C_STANDARD 11)

add_executable(server 
src/main.c 
src/file.c   
src/http.c     
src/network.c 
src/methods.c
src/POST.c   
)

target_include_directories(server PRIVATE include)
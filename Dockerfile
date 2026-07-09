# === Этап 1: Сборка проекта на Alpine ===
FROM alpine:3.20 AS builder
RUN apk add --no-cache build-base cmake gcc
WORKDIR /app
COPY CMakeLists.txt .
COPY include/ ./include/
COPY src/ ./src/
COPY public/ ./public/
RUN mkdir build && cd build && cmake .. && make

# === Этап 2: Финальный ультра-минимальный образ ===
FROM alpine:3.20
WORKDIR /server

# 1. Бинарник по-прежнему забираем из компилятора, на диске его нет
COPY --from=builder /app/build/server .

# 2. А папку public копируем НАПРЯМУЮ с твоего компьютера
COPY public/ ./public/

EXPOSE 8080
CMD ["./server"]

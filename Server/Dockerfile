FROM gcc:latest AS builder

WORKDIR /app

COPY main.cpp Server.cpp Server.h ./

RUN g++ -std=c++11 -o server main.cpp Server.cpp -pthread

FROM archlinux:latest

WORKDIR /app/server

COPY --from=builder /app/server /app/server

CMD ["./server"]


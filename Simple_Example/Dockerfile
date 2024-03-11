FROM gcc:latest AS builder

WORKDIR /app

COPY main.cpp Client/Client.cpp Client/Client.h ./

RUN g++ -std=c++11 -o demo main.cpp Client.cpp -pthread

FROM archlinux:latest

WORKDIR /app/demo

COPY --from=builder /app/demo /app/demo

CMD ["./demo"]

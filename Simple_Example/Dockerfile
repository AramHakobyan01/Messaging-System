FROM gcc:latest AS builder

RUN apt-get update && apt-get install -y cmake

WORKDIR /app

COPY . .

RUN mkdir -p build
WORKDIR /app/build

# Configure and build the project using CMake
RUN cmake ..
RUN make

FROM archlinux:latest

WORKDIR /app/Simple_Example

COPY --from=builder /app/build/Simple_Example /app/Simple_Example
COPY Client/libclient.so /app/Simple_Example
ENV LD_LIBRARY_PATH=/app/Simple_Example

CMD ["./Simple_Example"]

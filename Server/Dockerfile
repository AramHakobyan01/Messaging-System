FROM gcc:latest AS builder

# Install CMake
RUN apt-get update && apt-get install -y cmake

WORKDIR /app

# Copy all project files to the container
COPY . .

# Create a build directory and change to it
RUN mkdir -p build
WORKDIR /app/build

# Configure and build the project using CMake
RUN cmake ..
RUN make

# Use a minimal base image for the runtime environment
FROM archlinux:latest

# Set the working directory
WORKDIR /app/server

# Copy the built application from the builder stage
COPY --from=builder /app/build/server /app/server

# Set the command to run the server
CMD ["./server"]

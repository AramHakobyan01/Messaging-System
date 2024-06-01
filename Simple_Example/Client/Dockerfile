FROM gcc:latest AS builder

WORKDIR /app

# Copy all project files to the container
COPY . .

# Create a build directory and change to it
RUN mkdir -p build
WORKDIR /app/build

# Use a minimal base image for the runtime environment
FROM archlinux:latest

# Set the working directory
WORKDIR /app/Client

# Copy the built application from the builder stage
COPY --from=builder /app/build/Client /app/Client

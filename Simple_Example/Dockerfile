# Stage 1: Build
FROM gcc:latest AS builder

# Install dependencies
RUN apt-get update && apt-get install -y cmake

# Create and set working directory
WORKDIR /app

# Copy all project files to the container
COPY . .

# Create a build directory
RUN mkdir -p build
WORKDIR /app/build

# Configure and build the project using CMake
RUN cmake ..
RUN make

# Stage 2: Run
FROM archlinux:latest

# Install any runtime dependencies (e.g., libstdc++)
RUN pacman -Sy --noconfirm libstdc++6

# Set the working directory
WORKDIR /app/Simple_Example

# Copy the built application from the builder stage
COPY --from=builder /app/build/Simple_Example /app/Simple_Example

# Set the command to run the server
CMD ["./Simple_Example"]

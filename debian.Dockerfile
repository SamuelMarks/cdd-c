FROM debian:bullseye-slim AS builder
RUN apt-get update && apt-get install -y build-essential cmake pkg-config curl jq git libssl-dev zlib1g-dev

WORKDIR /app
COPY . .

# Build
RUN mkdir -p build_cmake && cd build_cmake && cmake .. && cmake --build . --config Release

# Stage 2
FROM debian:bullseye-slim
WORKDIR /app
COPY --from=builder /app/build_cmake/bin/cdd-c /usr/local/bin/cdd-c

ENTRYPOINT ["cdd-c"]
CMD ["serve_json_rpc", "--port", "8082", "--listen", "0.0.0.0"]

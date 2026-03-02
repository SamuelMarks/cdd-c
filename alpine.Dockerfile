FROM alpine:latest AS builder
RUN apk update && apk add --no-cache build-base cmake pkgconf curl jq git openssl-dev zlib-dev

WORKDIR /app
COPY . .

# Build
RUN mkdir -p build_cmake && cd build_cmake && cmake .. && cmake --build . --config Release

# Stage 2
FROM alpine:latest
RUN apk update && apk add --no-cache libgcc libstdc++
WORKDIR /app
COPY --from=builder /app/build_cmake/bin/cdd-c /usr/local/bin/cdd-c

ENTRYPOINT ["cdd-c"]
CMD ["serve_json_rpc", "--port", "8082", "--listen", "0.0.0.0"]

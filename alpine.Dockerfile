FROM alpine:latest
RUN apk update && apk add --no-cache build-base cmake pkgconf curl jq git openssl-dev zlib-dev libgcc libstdc++

WORKDIR /app
COPY . .

RUN mkdir -p build_cmake && cd build_cmake && cmake .. && cmake --build . --config Release

CMD ["cdd-c", "serve_json_rpc", "--port", "8082", "--listen", "0.0.0.0"]

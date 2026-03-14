FROM alpine:latest
RUN apk update && apk add --no-cache build-base cmake pkgconf curl jq git openssl-dev zlib-dev libgcc libstdc++ dos2unix zip unzip tar linux-headers perl

WORKDIR /app
COPY . .

RUN mkdir -p build_cmake && cd build_cmake && cmake .. && cmake --build . --config Release

ENTRYPOINT ["/bin/sh", "-c", "dos2unix /workspace_src/.run_alpine_tests.sh 2>/dev/null; exec \"$@\"", "--"]
CMD ["cdd-c", "serve_json_rpc", "--port", "8082", "--listen", "0.0.0.0"]

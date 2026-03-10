FROM debian:bullseye-slim
RUN apt-get update && apt-get install -y build-essential cmake pkg-config curl jq git libssl-dev zlib1g-dev

WORKDIR /app
COPY . .

RUN mkdir -p build_cmake && cd build_cmake && cmake .. && cmake --build . --config Release

CMD ["cdd-c", "serve_json_rpc", "--port", "8082", "--listen", "0.0.0.0"]

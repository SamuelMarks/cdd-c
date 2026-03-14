FROM debian:bullseye-slim
RUN apt-get update && apt-get install -y build-essential cmake pkg-config curl jq git libssl-dev zlib1g-dev dos2unix zip unzip tar linux-libc-dev perl

WORKDIR /app
COPY . .

RUN mkdir -p build_cmake && cd build_cmake && cmake .. && cmake --build . --config Release

ENTRYPOINT ["/bin/bash", "-c", "dos2unix /workspace_src/.run_ubuntu_tests.sh 2>/dev/null; exec \"$@\"", "--"]
CMD ["cdd-c", "serve_json_rpc", "--port", "8082", "--listen", "0.0.0.0"]

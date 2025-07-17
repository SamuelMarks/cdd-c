FROM debian

RUN apt-get update -qq && \
    apt-get install -qy cmake gcc git make pkg-config libc-dev && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt

COPY . /c_cdd

WORKDIR /c_cdd

RUN cmake -DCMAKE_BUILD_TYPE='Debug' \
          -DBUILD_TESTING='1' \
          -DC_CDD_BUILD_TESTING='1' \
          -S . -B 'build' && \
    cmake --build 'build'

CMD cd build && ctest .

FROM alpine

RUN apk add --no-cache gcc cmake git make musl-dev linux-headers

COPY . /c_cdd

WORKDIR /c_cdd

RUN cmake -DCMAKE_BUILD_TYPE='Debug' \
          -DCMAKE_PREFIX_PATH='cmake/modules' \
          -DBUILD_TESTING=1 \
          -DC_CDD_BUILD_TESTING=1 \
          -S . -B build && \
    cmake --build build

CMD cd build && ctest .

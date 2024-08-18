ARG RE2_TAG=2024-07-02
ARG ABSEIL_TAG=20240722.0

# start with the emscripten builder image
FROM emscripten/emsdk:3.1.64

# get the build tools
RUN apt-get update && \
    apt-get install -y git cmake build-essential

# build abseil
ARG ABSEIL_TAG
RUN git clone --branch ${ABSEIL_TAG} \
    --depth 1 https://github.com/abseil/abseil-cpp.git /abs

WORKDIR /abs

RUN mkdir build && \
    cd build && \
    emcmake cmake .. \
    -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_FLAGS="-DSTANDALONE_WASM=1 -fno-rtti -fno-exceptions -flto \
    -DABSL_MIN_LOG_LEVEL=99 -DABSL_MAX_VLOG_VERBOSITY=-99"


RUN cd build && \
    emmake make -j$(nproc) && \
    emmake make install

# build re2
ARG RE2_TAG
RUN git clone --branch ${RE2_TAG} \
    --depth 1 https://github.com/google/re2.git /re2

WORKDIR /re2

RUN mkdir -p build && \
    cd build && \
    emcmake cmake .. \
    -DRE2_BUILD_TESTING=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_CXX_FLAGS="-DSTANDALONE_WASM=1 -fno-rtti -fno-exceptions -flto \
    -DABSL_MIN_LOG_LEVEL=99 -DABSL_MAX_VLOG_VERBOSITY=-99"

RUN cd build && \
    emmake make -j$(nproc)

# build Match and MatchExtract
COPY main.cpp /main.cpp
COPY mem.cpp /mem.cpp
COPY abs.cpp /abs.cpp

# Match
RUN emcc -DCOMPILE_MATCH -o /match.js \
    /main.cpp /mem.cpp /abs.cpp \
    -O3 -s STANDALONE_WASM -s MALLOC=emmalloc -s EVAL_CTORS=2 \
    -fno-rtti -fno-exceptions -flto --no-entry \
    -s EXPORTED_FUNCTIONS='[ \
    "_canonical_abi_realloc", \
    "_canonical_abi_free", \
    "_Match"]' \
    -I/re2 \
    /re2/build/libre2.a \
    /abs/build/absl/strings/*.a \
    /abs/build/absl/hash/*.a \
    /abs/build/absl/container/*.a \
    /abs/build/absl/base/*.a

# MatchExtract
RUN emcc -DCOMPILE_MATCH_EXTRACT -o /match_extract.js \
    /main.cpp /mem.cpp /abs.cpp \
    -O3 -s STANDALONE_WASM -s MALLOC=emmalloc -s EVAL_CTORS=2 \
    -fno-rtti -fno-exceptions -flto --no-entry \
    -s EXPORTED_FUNCTIONS='[ \
    "_canonical_abi_realloc", \
    "_canonical_abi_free", \
    "_MatchExtract"]' \
    -I/re2 \
    /re2/build/libre2.a \
    /abs/build/absl/strings/*.a \
    /abs/build/absl/hash/*.a \
    /abs/build/absl/container/*.a \
    /abs/build/absl/base/*.a

# extract imports/exports for debugging
RUN apt-get install -y wabt
RUN wasm2wat /match.wasm | \
    grep -E "(import|export)" \
    > /match.txt
RUN wasm2wat /match_extract.wasm | \
    grep -E "(import|export)" \
    > /match_extract.txt

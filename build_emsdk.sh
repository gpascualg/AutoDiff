#!/bin/sh

mkdir -p build_emsdk
cd build_emsdk
cmake   -DCMAKE_TOOLCHAIN_FILE=$EMSCRIPTEN/cmake/Modules/Platform/Emscripten.cmake \
        -DCMAKE_BUILD_TYPE=Release \
	-G "Unix Makefiles" \
	-DWITH_BLAS_SUPPORT=OFF \
	-DBUILD_EMSCRIPTEN=ON \
	..
make

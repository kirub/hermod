pushd ..\libs\googletest
cmake -Wno-deprecated -B build -DBUILD_SHARED_LIBS=On -DCMAKE_WARN_DEPRECATED=OFF && cmake --build build
popd
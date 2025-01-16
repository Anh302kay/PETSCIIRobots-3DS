git clone -b SDL2 https://github.com/libsdl-org/SDL.git
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/3DS.cmake" -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build
cd ..
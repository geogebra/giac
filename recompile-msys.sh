mkdir -p "$1"
cd "$1" || exit 2
CXX=clang++ CC=clang cmake -G "MinGW Makefiles" ..
mingw32-make clean
mingw32-make || exit 1
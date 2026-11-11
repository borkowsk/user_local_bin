# (Umieść powyższy CMakeLists.txt w korzeniu)
set -e
echo MKDIR build/
mkdir -p build
cd build
echo CMAKE
cmake ..
echo MAKE
make

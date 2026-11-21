#!/bin/bash
set -e

echo "Tworzę katalog build/..."
mkdir -p build
cd build

echo "Konfiguruję projekt (CMake)..."
cmake ..

echo "Kompiluję (make)..."
make

exit 0

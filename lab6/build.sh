#!/bin/bash
set -e
cd "$(dirname "$0")"
cmake -B build -S .
cmake --build build
echo "Build complete. Binary: build/lab6"

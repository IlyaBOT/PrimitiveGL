#!/bin/sh
set -e

mkdir -p Build

g++ -O2 main.cpp -o Build/PrimitiveGL \
    -framework OpenGL \
    -framework GLUT

echo "Built: Build/PrimitiveGL"

# PrimitiveGL

A lightweight OpenGL 3D engine for legacy and low-end hardware, focused on simplicity, portability and performance.

## Current target

- Mac OS X 10.6 Snow Leopard
- Intel x86 / x86_64
- Legacy OpenGL fixed-function pipeline
- System OpenGL and GLUT frameworks
- GCC / Xcode toolchain
- No third-party dependencies for the basic renderer

## Build

```sh
chmod +x build.sh
./build.sh
```

The binary is written to:

```text
Build/PrimitiveGL
```

Run it with:

```sh
./Build/PrimitiveGL
```

The initial demo opens a 640x480 window with a gray gradient background and a white 3D cube.

## License

PrimitiveGL is released under the MIT License.

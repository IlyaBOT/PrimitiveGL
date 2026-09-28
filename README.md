# PrimitiveGL

A lightweight OpenGL 3D engine for legacy and low-end hardware, focused on simplicity, portability and performance.

## Current target

- Mac OS X 10.6 Snow Leopard
- Intel x86 / x86_64
- Legacy OpenGL fixed-function pipeline
- System OpenGL, GLUT and ApplicationServices frameworks
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

## Controls

- `W` - move forward
- `S` - move backward
- `A` - move left
- `D` - move right
- `Q` - turn left
- `R` - turn right
- `Space` - move up
- `Shift` - move down

The current demo opens a 640x480 window with a gray gradient background and an RGB color cube.

## License

PrimitiveGL is released under the MIT License.

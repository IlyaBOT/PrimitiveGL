# PrimitiveGL

A lightweight OpenGL 3D engine for legacy and low-end hardware, focused on simplicity, portability and performance.

## Current target

- Mac OS X 10.6 Snow Leopard
- Intel x86 / x86_64
- Legacy OpenGL fixed-function pipeline
- System OpenGL, GLUT, ApplicationServices and IOKit frameworks
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

Enable the debug overlay with:

```sh
./Build/PrimitiveGL --debug
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

## Debug information

The window title always shows the current FPS.

With `--debug`, PrimitiveGL also displays:

- engine version and compile date
- FPS
- detected OpenGL version
- active OpenGL renderer / GPU
- GPU utilization when exposed by the driver
- driver-reported VRAM usage and renderer VRAM capacity when available
- scene object and rendered object counters
- texture count and texture memory counter
- viewport size and OpenGL vendor

GPU utilization and VRAM statistics are read from the system driver. Some older or unsupported drivers may not expose these counters; PrimitiveGL shows `N/A` instead of inventing a value.

The current demo opens a 640x480 window with a gray gradient background and an RGB color cube.

## License

PrimitiveGL is released under the MIT License.

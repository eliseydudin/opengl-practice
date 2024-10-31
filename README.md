# OpenGL Practice
When I started learning OpenGL I found a surprising lack of modern examples. This repository contains random stuff I did in it.

# Dependencies
The following packages are required:
```
SDL2
assimp
OpenGL (which is probably preinstalled)
meson
```
You also need the stb image header, which you can find [here](https://github.com/nothings/stb). Put the `stb_image.h` file in the root of the project and rename it to `stbi.h`.

# Compiling
To have multiple examples this project uses `meson_options.txt`, find the example you need and change the option's value to `true`.
Then just run `meson setup builddir` and `meson compile -C builddir`.
After that you can run the executable like this: `./builddir/example/example`.

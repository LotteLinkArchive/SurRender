#!/bin/bash
CC=clang meson bin --reconfigure
cd bin
meson configure -Dc_args="-Ofast -march=native -mtune=native -g"
cd ..
meson compile -C bin

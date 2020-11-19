#!/bin/bash
if [ ! -d "./bin" ]; then
	CC=gcc meson bin
fi

cd bin
meson configure -Dc_args="-Ofast -march=native -mtune=native -g"
cd ..

meson compile -C bin

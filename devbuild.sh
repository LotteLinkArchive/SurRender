#!/bin/bash
if [ ! -d "./bin" ]; then
	CC=gcc meson bin
	cd bin
	meson configure -Dc_args="-Ofast -march=native -mtune=native -g"
	cd ..
fi
meson compile -C bin

#!/bin/sh
libtoolize
autoreconf --force --install -I config -I m4

AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libsurrender.la
libsurrender_la_SOURCES = src/2dscene.c src/canvas.c src/image.c src/selections.c src/shapes.c src/textrender.c
pkginclude_HEADERS = src/2dscene.h src/canvas.h src/image.h src/selections.h src/shapes.h src/textrender.h src/colours.h src/glbl.h src/surrender.h src/srtex.h

bin_PROGRAMS = surdemo img2srtex
img2srtex_SOURCES = img2srtex.c
surdemo_SOURCES = demo.c
surdemo_LDADD = libsurrender.la

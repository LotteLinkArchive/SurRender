FLAGS := -std=gnu17 -Wall -Wextra -Wl,-rpath='$$ORIGIN',-rpath='$$ORIGIN/radix' -lm -Wl,--allow-multiple-definition
CFLAGS := ${FLAGS} -fPIC -Ofast -g -march=core2 -mtune=generic
LDFLAGS := ${CFLAGS} -shared

SRC = ${wildcard src/*.c}
OBJ = ${SRC:.c=.o}

all: libs/radix/libradix.a libsurrender.so  demo/a.out

update:
	git submodule update --init --recursive

libs/radix/libradix.a: update
	${MAKE} libradix.a -C libs/radix

libs/radix/libradix.so: update
	${MAKE} libradix.so -C libs/radix

libsurrender.so: ${OBJ}
	${CC} -o $@ $^ ${LDFLAGS}

libsurrender.a: ${OBJ}
	ar rcs $@ $^

demo/a.out: libsurrender.a libs/radix/libradix.a
	${MAKE} -C demo

clean:
	rm -f libsurrender.* ${OBJ}
	${MAKE} clean -C libs/radix
	${MAKE} clean -C demo

.PHONY: update all clean

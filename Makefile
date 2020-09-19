FLAGS := -std=gnu17 -Wall -Wextra -Wl,-rpath='$$ORIGIN' -lm -Wl,--allow-multiple-definition
CFLAGS := ${FLAGS} -fPIC -Ofast -g -march=core2 -mtune=generic
LDFLAGS := ${CFLAGS} -shared

SRC = ${wildcard src/*.c}
OBJ = ${SRC:.c=.o}

all: libsurrender.so demo/a.out

libsurrender.so: ${OBJ}
	${CC} -o $@ $^ ${LDFLAGS}

libsurrender.a: ${OBJ}
	ar rcs $@ $^

demo/a.out: libsurrender.a
	${MAKE} -C demo

clean:
	rm -f libsurrender.* ${OBJ}
	${MAKE} clean -C demo

.PHONY: all clean

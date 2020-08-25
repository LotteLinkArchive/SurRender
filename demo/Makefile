FLAGS := -std=gnu17 -pedantic -Wall -Wextra -Wl,-rpath='$$ORIGIN',-rpath='$$ORIGIN/radix' -lm -Wl,--allow-multiple-definition
CFLAGS := ${FLAGS} -fPIC -Ofast -g -march=core2 -mtune=generic
LDFLAGS := ${CFLAGS} -shared

-include flags.mk

SRC = ${wildcard src/*.c}
OBJ = ${SRC:.c=.o}

all: flags.mk clean a.out

a.out: main.c ../libsurrender.a ../libs/radix/libradix.a
	${CC} $^ -lSDL2 ${CFLAGS} ${DFLAGS}

flags.mk:
	cp flags.def.mk $@

clean:
	rm -f a.out ${OBJ}

.PHONY: all clean

CC ?= gcc
LD ?= gcc
AR ?= ar

CFLAGS ?= -Os -Werror -Wall -Wextra -std=c11
LDFLAGS ?= -L. -lbtree

CFLAGS += -Iinclude

SRC := src/btree.c
OBJ := $(SRC:%.c=%.o)

all: lib example
lib: libbtree.a
example: test.exe

libbtree.a: $(OBJ)
	$(AR) rcs $@ $^

test.exe: lib examples/test.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.a
	rm -f *.exe
	rm -f src/*.o
	rm -f examples/*.o

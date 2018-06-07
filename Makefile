
CC := gcc
CFLAGS := -O3

LIBS := common.o add.o

bench: bench.o $(LIBS)

test: test.o $(LIBS)

clean:
	rm -f *.o
.PHONY: clean

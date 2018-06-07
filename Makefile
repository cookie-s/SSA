
CC := gcc
CFLAGS := -O3 -lprofiler

LIBS := common.o add.o

bench: bench.o $(LIBS)

test: test.o $(LIBS)

clean:
	rm -f *.o
.PHONY: clean

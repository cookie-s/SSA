CC := gcc
CFLAGS := -O3 -lprofiler -Wall

LIBS := common.o add.o io.o

bench: bench.o $(LIBS)

test: test.o $(LIBS)

i: i.o $(LIBS)

clean:
	rm -f *.o
.PHONY: clean

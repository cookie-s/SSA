CC := gcc
CFLAGS := -O3 -Wall

LIBS := common.o arith.o mult.o fft.o io.o

bench: $(LIBS)

test: $(LIBS)

i: $(LIBS)

clean:
	rm -f *.o
.PHONY: clean

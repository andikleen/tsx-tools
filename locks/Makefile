CFLAGS := -g -O2 -I ../include

all: libtsxlocks.a

libtsxlocks.a: spin-rtm.o spin-hle.o
	ar q libtsxlocks.a $^

clean:
	rm -f libtsxlocks.a spin-rtm.o spin-hle.o

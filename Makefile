CFLAGS := -I include
CLEAN := ignore-xend.so assert-test assert-test.o has-tsx

all: ignore-xend.so assert-test has-tsx tsx-assert.o 

ignore-xend.so: ignore-xend.c
	gcc -fPIC -shared -g -o ignore-xend.so ignore-xend.c

assert-test: assert-test.o tsx-assert.o
	gcc -o assert-test assert-test.o tsx-assert.o

clean:
	rm -f ${CLEAN}


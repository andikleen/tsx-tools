CLEAN := ignore-xend.so
# do not remove -g here
CFLAGS := -g -I ../include

all: ignore-xend.so assert-test

ignore-xend.so: ignore-xend.c
	gcc -fPIC -shared -g -o ignore-xend.so ignore-xend.c

assert-test: assert-test.patched.o txn-assert.o a.o
	gcc -pthread -o assert-test assert-test.patched.o txn-assert.o a.o

a.c assert-test.patched.o: assert-test.o assign-assert.py
	cp assert-test.o assert-test.patched.o
	./assign-assert.py assert-test.patched.o > a.c || rm assert-test.patched.o

clean:
	rm -f ${CLEAN}


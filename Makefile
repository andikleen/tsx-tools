CLEAN := ignore-xend.so

all: ignore-xend.so

ignore-xend.so: ignore-xend.c
	gcc -fPIC -shared -g -o ignore-xend.so ignore-xend.c

clean:
	rm -f ${CLEAN}


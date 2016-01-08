CFLAGS = -g -Wall -fPIC
LDFLAGS = -lpthread -lelf

# enable for test
# CFLAGS += -DTESTMAIN=1

all: gtune.so

gtune.so: gtune.o sym.o
	gcc -shared -fPIC -o gtune.so $^ ${CFLAGS} ${LDFLAGS}

test: gtune.o sym.o
	$(CC) -o test gtune.o sym.o $(LDFLAGS)

clean:
	rm -f tmaps test gtune.o sym.o gtune.so gtune

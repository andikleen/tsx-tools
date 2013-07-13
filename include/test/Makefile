ALL := hle hle2 rtm-test rtm-goto-test rtm-test rtm-patched
CLEAN := ${ALL}

CFLAGS := -Wall -g -I .. -O2

all: ${ALL}

hle2: hle2.c ../hle-ms.h

hle: hle.c ../hle-emulation.h

rtm-test: ../rtm.h

rtm-goto-test: ../rtm-goto.h

rtm-patched: ../rtm-patched.h 
	${CC} -o rtm-patched -DHDR=\"rtm-patched.h\" ${CFLAGS} rtm-test.c

clean:
	rm -f ${CLEAN}

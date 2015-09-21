ALL := hle hle2 rtm-test rtm-goto-test rtm-test rtm-patched
CLEAN := ${ALL}

CFLAGS := -Wall -g -I .. -O2

all: ${ALL}

hle2.c: ../hle-ms.h

hle.c: ../hle-emulation.h

rtm-test.c: ../rtm.h

rtm-goto-test.c: ../rtm-goto.h

rtm-patched: ../rtm-patched.h  rtm-test.c
	${CC} -o rtm-patched -DHDR=\"rtm-patched.h\" ${CFLAGS} rtm-test.c

clean:
	rm -f ${CLEAN}

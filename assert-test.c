#include "tsx-assert.h"
#include "rtm.h"

int foo;

int main(void)
{
	unsigned status;
	if ((status = _xbegin()) == _XBEGIN_STARTED) { 
		tsx_assert(foo);
	}
	return 0;
}

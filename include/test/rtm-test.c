#ifndef HDR 
#define HDR "rtm.h"
#endif
#include HDR
#include <stdio.h>

/* Requires TSX support in the CPU */

int main(void)
{
	int status;
	if ((status = _xbegin()) == _XBEGIN_STARTED) {
		if (_xtest())
			_xabort(1);
		_xend();
	} else
		printf("aborted %x, %d", status, _XABORT_CODE(status));
	return 0;
}

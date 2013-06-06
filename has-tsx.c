/* Determine whether the current system has HLE or RTM */
#include <stdio.h>

#include "tsx-cpuid.h"

int main(void)
{
	printf("RTM: %s\n", cpu_has_rtm() ? "Yes" : "No");
	printf("HLE: %s\n", cpu_has_hle() ? "Yes" : "No");
	return 0;
}

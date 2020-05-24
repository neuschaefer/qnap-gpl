#include <stdio.h>

int main ()
{
	int result = 0;
	result =  system ("./gengetstatus -r -d /dev/ttyS0 PhxTec-A1000");
	printf ("result = %d\n", result >>= 8);
}

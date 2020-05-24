#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gen_common.h"

int str_to_hex (char * str)
{
	int result = 0, len = 0, i = 0;
	
	len = strlen (str);

	if (len == 0)
		return -1;

	while (i < len)
	{
		if (str[i] == 10)
			str[i] = '\0';
		else if ((str[i] < 48 || str[i] > 57) && 
		    (str[i] < 65 || str[i] > 70) &&
		    (str[i] < 97 || str[i] > 102))
			return -1;

		i++;
	}

	sscanf (str, "%x", &result);

	return result;
}

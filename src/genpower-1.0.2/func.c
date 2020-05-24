#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gen_common.h"

int str_to_hex (char * str)
{
	int i = 0, j = 0;
	int len = strlen(str);
	char ch;
	int result = 0, num = 0, mut = 0, r = 0;

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

	i = len;

	while (i--)
	{
		num = 0;
		if ((str[i] >= 'A' && str[i] <= 'F') ||
		    (str[i] >= 'a' && str[i] <= 'f'))
		{
			switch (str[i])
			{
			case 'A':
			case 'a':
				num  = 10;
				break;
			case 'B':
			case 'b':
				num  = 11;
				break;
			case 'C':
			case 'c':
				num  = 12;
				break;
			case 'D':
			case 'd':
				num  = 13;
				break;
			case 'E':
			case 'e':
				num  = 14;
				break;
			case 'F':
			case 'f':
				num  = 15;
				break;
			}
		}
		else
		{
			ch = str[i];
			num = atoi ((char *)&ch);
		}

		mut = len - i - 1;
		if (mut == 0)
			mut = 1;
		else if (mut == 1)
			mut = 16;
		else {
			r = 16;
			for (j=1; j<mut; j++)
				r *= 16;
			mut = r;
		}

		result += num * mut;
	}
	return result;
}

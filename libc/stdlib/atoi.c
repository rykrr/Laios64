#include <types.h>

imax atoi(const char *nstr) {
	imax value = 0;
	i8 sign = 1;

	if (*nstr && *nstr == '-') {
		sign = -1;
		nstr++;
	}

	while (*nstr && *nstr == '0')
		nstr++;

	while (*nstr && '0' <= *nstr && *nstr <= '9') {
		value *= 10;
		value += *nstr - '0';
		nstr++;
	}

	return sign * value;
}

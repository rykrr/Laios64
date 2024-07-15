#include <stdio.h>
#include <types.h>

int atoi(const char *nstr) {
	i64 value = 0;
	i64 sign = 1;

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





#include "boot.h"

int strcmp(const char *str1, const char *str2)
{
	const unsigned char *s1 = (const unsigned char *)str1;
	const unsigned char *s2 = (const unsigned char *)str2;
	int delta = 0;

	while (*s1 || *s2) {
		delta = *s2 - *s1;
		if (delta)
			return delta;
		s1++;
		s2++;
	}
	return 0;
}

size_t strnlen(const char *s, size_t maxlen)
{
	const char *es = s;
	while (*es && maxlen) {
		es++;
		maxlen--;
	}

	return (es - s);
}

unsigned int atou(const char *s)
{
	unsigned int i = 0;
	while (isdigit(*s))
		i = i * 10 + (*s++ - '0');
	return i;
}

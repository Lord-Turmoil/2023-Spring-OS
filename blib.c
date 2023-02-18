#include <blib.h>

size_t strlen(const char* s)
{
	// panic("please implement");
	size_t len = 0;
	for (const char* p = s; *p; p++)
		len++;
	return len;
}

char* strcpy(char* dst, const char* src)
{
	// panic("please implement");
	char* ret = dst;

	while (*src)
		*(dst++) = *(src++);
	*dst = '\0';

	return ret;
}

char* strncpy(char* dst, const char* src, size_t n)
{
	char* res = dst;
	while (*src && n--)
	{
		*dst++ = *src++;
	}
	*dst = '\0';
	return res;
}

char* strcat(char* dst, const char* src)
{
	// panic("please implement");
	char* ret = dst;

	while (*dst)
		dst++;
	while (*src)
		*(dst++) = *(src++);
	*dst = '\0';

	return ret;
}

int strcmp(const char* s1, const char* s2)
{
	// panic("please implement");
	while (*s1 || *s2)
	{
		if (*s1 != *s2)
			return *s1 - *s2;
		++s1;
		++s2;
	}

	return 0;
}

int strncmp(const char* s1, const char* s2, size_t n)
{
	while (n--)
	{
		if (*s1 != *s2)
		{
			return *s1 - *s2;
		}
		if (*s1 == 0)
		{
			break;
		}
		s1++;
		s2++;
	}
	return 0;
}

void* memset(void* s, int c, size_t n)
{
	// panic("please implement");
	unsigned char* mem = (unsigned char*)s;
	for (size_t i = 0; i < n; ++i)
		*(mem++) = c;

	return s;
}

void* memcpy(void* out, const void* in, size_t n)
{
	char* csrc = (char*)in;
	char* cdest = (char*)out;
	for (int i = 0; i < n; i++)
	{
		cdest[i] = csrc[i];
	}
	return out;
}

int memcmp(const void* s1, const void* s2, size_t n)
{
	// panic("please implement");
	const unsigned char* m1 = (const unsigned char*)s1;
	const unsigned char* m2 = (const unsigned char*)s2;
	for (size_t i = 0; i < n; ++i)
	{
		if (*m1 != *m2)
			return *m1 - *m2;
		++m1;
		++m2;
	}

	return 0;
}

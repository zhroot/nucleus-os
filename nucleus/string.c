#include <datatypes.h>
#include <string.h>

// v0.1: ???
// v0.2: Doug Gale
//	- Fixed return value of some string functions to behave like Standard C
//	- Added strstr

size_t strlen( const char* s )
{
    dword len = 0;
    while ( *s++ != 0 )
		++len;
    return len;
}

int strcmp( const char* s1, const char* s2 )
{
    while ( 1 ) {
		int cmp = *s1 - *s2;
		if ( cmp || *s1 == 0 || *s2 == 0 )
			return cmp;
		++s1;
		++s2;
    }
}

char *strcpy(char *dest, const char *src)
{
    while(*src)
		*dest++ = *src++;            
    *dest = 0;

    return dest;
} 

char *strncpy(char *dst, const char *src, size_t size)
{
    while(size && *src) size--, *dst++ = *src++;            
    if(size)
		*dst = 0;

    return dst;
}

int toupper(int c)
{
	return (c >= 'a' && c <= 'z') ? c - 32 : c;
}

int tolower(int c)
{
	return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

char *strupr(char *s)
{
	char *orig_s = s ;
	char c ;
   
	if (s)
	{
		while (*s)
		{
			c = *s ;
			*s++ = toupper(c);
		}
    }
	return orig_s ;
}

char *strlwr(char *s)
{
	char *orig_s = s ;
	char c ;
   
	if (s)
	{
		while (*s)
		{
			c = *s ;
			*s++ = tolower(c);
		}
    }
	return orig_s ;
}

char *strstr(const char *string, const char *word)
{
	const char *strptr, *wordptr;
	unsigned i;

	if (!word[0] || !string[0])
		return 0;

	wordptr = word;

	// Brute force look for bytes that match first byte of word
	for (strptr = string; *strptr; strptr++) {
		if (*strptr == word[0]) {
			wordptr = word;
			i = 0;
			while (strptr[++i] && 
					*++wordptr &&
					strptr[i] == *wordptr);

			// Did I reach end of match?
			if (*wordptr == 0) {
				// Success
				return (char*)strptr;
			}
		}
	}

	return 0;
}

int is_digit(int chr)
{
	return (chr >= '0') && (chr <= '9');
}

long atol(const char *str)
{
	long n = 0;
	while (is_digit(*str))
	{
		n = (n * 10) + *str - '0';
		str++;
	}
	return n;
} 


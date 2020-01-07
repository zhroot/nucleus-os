#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen( const char* s );
int strcmp( const char* s1, const char* s2 );
char* strcpy( char* dest, const char* src );
char *strncpy(char *dst, const char *src, size_t size);
char *strupr(char *s);
char *strlwr(char *s);
char *strstr(const char *string, const char *word);

int toupper(int c);
int tolower(int c);

int is_digit(int chr);
long atol(const char *str);

#endif // STRING_H

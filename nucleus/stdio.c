#include <stdio.h>
#include <datatypes.h>
#include <stdarg.h>
#include <string.h>
#include <nucleus.h>
#include <support.h>
#include <video/graphic.h>
#include <drivers/input.h>

void prints ( char* s )
{
	int i1 ;
	i1 = 0 ;
	while ( s[i1] != 0 )
	{
		putch(0, 0, s[i1] ) ;
		i1 += 1 ;
	}
}

#define S_SIZE  160

void newline(void)
{
	prints("\n\r");
}

long _formatout(char *outptr, char *fmt, char output, va_list argptr)
{
	char numstk[33], *ptr, justify, zero, minus, chr[2], us = 0, chr2[2];
	unsigned long width, value, i, total, counter;
	int x;
	
	chr[1] = '\0';	x = 1;
	chr2[1] = '\0';	total = 0;
	while ((chr[0] = *fmt++) != 0)
	{
		if (chr[0] == '%')
		{				/* format code */
			chr[0] = *fmt++;
			ptr = &numstk[32];
			*ptr = justify = minus = 0;
			width = value = i = 0;
			zero = ' ';
			if (chr[0] == '-')
			{			/* left justify */
				--justify;
				chr[0] = *fmt++;
			}
			if (chr[0] == '0')	/* leading zeros */
				zero = '0';
			while (chr[0] >= '0' && chr[0] <= '9')
			{			/* field width specifier */
				width = (width * 10) + (chr[0] - '0');
				chr[0] = *fmt++;
			}
			/* first switch allows for ld Ld ld ud Ud etc... */
			switch(chr[0])
			{
			case 'U' :                                      /* unsigned number */
			case 'u' :
				i = 10;
				us = 1;
					chr[0] = *fmt++;
				break;
			case 'l' :                                      /* Long (it is anyway) */
			case 'L' :
				chr[0] = *fmt++;
				break;
			default:                                        /* all others */
				us = 0;                                 /* not unsigned */
				break;
			}
			
			switch(chr[0])
			{
			case 'd' :                                      /* decimal number */
				value = va_arg(argptr, int);	/* get parameter value */
				i = 10;
					if (!us)
						if(value & 0x8000000)
				{
					value = -value;
					++minus;
				}
				break;
				case 'X' :                                      /* hexidecimal number */
				case 'x' :                                      /* hexidecimal number */
				value = va_arg(argptr, long);
				i = 16;
				break;
			case 'o' :                                      /* octal number */
				value = va_arg(argptr, int);
				i = 8;
				break;
			case 'c' :                                      /* character data */
				value = (char)va_arg(argptr, int);	/* get parameter value */
				*--ptr = value;
				break;
			case 's':
				ptr = va_arg(argptr, char *);
				break;
			}
			
			if (i)	/* for all numbers, generate the ASCII string */
			{
				do {
					if((chr[0] = (value % i) + '0') > '9')
						chr[0] += 7;
					*--ptr = chr[0]; }
				while(value /= i);
			}
			/* output sign if any */
			
			if(minus)
			{
				chr2[0]='-';
				*outptr++ = '-';
				if (output)
					prints(chr2);
				++total;
				if(width)
					--width;
			}
			
			/* pad with 'zero' value if right justify enabled  */
			
			if(width && !justify)
			{
				chr2[0]=zero;
				for(i = strlen(ptr); i < width; ++i)
				{
					if (output)
						prints(chr2);
					*outptr++ = zero;
				}
				++total;
			}
			
			/* move in data */
			
			i = 0;
			value = width - 1;
			
			while((*ptr) && (i <= value))
			{
				chr2[0]=*ptr;
				if (chr2[0] == '\n')
				{ 
					newline();
				}
				else 
				if(chr2[0] == '\t')
				{
					chr2[0]=' ';
					if (output)
						for(counter = 1;counter<=5;counter++)
							prints(chr2);
				}
				else
					if (output)
						prints(chr2);
				*outptr++ = *ptr++;
				++total;
				++i; 
			}
			
			/* pad with 'zero' value if left justify enabled */
			
			if(width && justify)
			{
				while(i < width)
				{
					chr2[0]=zero;
					if (output)
						prints(chr2);
					*outptr++ = zero;
					++total;
					++i;
				}
			}
		}
		else
		if (chr[0] == '\n') 
		{
			if (output)
				newline();
		}       
		else
		if (chr[0] == '\t')
		{
			chr[0]=' ';
			if (output)
				for(counter = 1;counter<=5;counter++)
					prints(chr);
		}
		else
		{
			/* not format char, just move into string  */
			*outptr++ = chr[0];
			if (output)
				prints(chr);
			++total;
		}
	}
	
	*outptr = 0;
	return total;
}

/************************************
	Formatted print to stdout
*************************************/
int printf(char *fmt, ...)
{
	va_list ap;
	long total;
	char buffer[S_SIZE];

	va_start(ap, fmt);              /* set up ap pointer */
	
	total = _formatout(buffer, fmt, 1, ap);
	
	if (total >= S_SIZE)
		panic();	// I just overwrote memory, die here, where it happened
	va_end(ap);
	
	return total;
}

int dprintf(char *fmt, ...)
{
#ifdef DEBUG
	va_list ap;
	long total;
	char buffer[S_SIZE];

	if (nucleuskl_debug == 0) return 0;

	va_start(ap, fmt);              /* set up ap pointer */
	
	total = _formatout(buffer, fmt, 1, ap);
    if (total >= S_SIZE)          
          panic();    // I just overwrote memory, die here, where it happened
	
	va_end(ap);
	return total;
#else
	return 0;
#endif
}

int sprintf(char* str, const char *fmt, ...)
{
	va_list ap;
	long total;
	char buffer[S_SIZE];

	va_start(ap, fmt);              /* set up ap pointer */
	
	total = _formatout(buffer, (char *)fmt, 0, ap);
	strcpy(str, buffer);
	
	va_end(ap);
	return total;
}

int scanf(const char *format, ...)
{
	va_list ap;
	char *result, input[120];
	int i = 0, j = 0;
	
	va_start(ap, format);
	
	while(!(format[i] == '\0'))
	{
		switch(format[i++])
		{
		case '%':
			switch(format[i++])
			{
			case 's':
				while(j < 120)
				{
					input[j++] = input_getch();
					if(input[j - 1] == 0x08)
					{
						if(j == 1)
						{
							j = 0;
						}
						else
						{
							j -= 2;
							gotoxy(wherex()-1, wherey());
							printf(" ");
							gotoxy(wherex()-1, wherey());
						}
					}
					else if(input[j - 1] == 0x0D)
					{
						input[--j] = '\0';
						break;
					}
					else
						printf("%c", input[j - 1]);
				}
				j = 0;
				result = va_arg(ap, char *);
				
				strcpy(result, input);
				break;
				
			default:
				break;
			}
			break;
			
		default:
			putch(0, 0, format[i] - 1);
		}
	}
	
	va_end(ap);
	return 1;
}

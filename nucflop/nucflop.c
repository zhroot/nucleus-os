#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "fat.h"

char cmd[128] = "";
char args[10][50];

char dir[100] = "";
dword dirptr = 0;


int formatted = 0;

unsigned char *image = NULL;
dword imgsize = 0;
dword kernelsize = 0;

int analyze(void)
{
	int i, j = 0, k = 0, ii, count = 0;

	for(i = 0; i < 100; i++)
	{
		if(cmd[i] == ' ' || cmd[i] == '\0')
		{
			for(ii = j; ii < i; ii++)
			{
				args[k][ii - j] = cmd[ii];
			}
			args[k][i - j] = '\0';
			k++;
			j = i + 1;
			if(i != 0)count++;
		}
		if(cmd[i] == '\0')break;
	}

	return count;
}

int create_image(void)
{
	if(image)free(image);

	image = (char*)malloc(1474560);
	
	memset(image, 0, 1474560);
	formatted = 0;
	imgsize = 1474560;

	return 0;
}

int load_binary(dword offset, char *filename, dword size)
{
	FILE *bin = NULL;

	bin = fopen(filename, "rb");

	if(!bin) return 1;

	if(size)fread(image + offset, size, 1, bin);
	
	else
	{
		while(!feof(bin))
		{
			image[offset++] = fgetc(bin);
		}
	}

	fclose(bin);

	return 0;
}

int save_binary(dword offset, char *filename, dword size)
{
	FILE *bin = NULL;

	bin = fopen(filename, "wb");

	if(!bin) return 1;

	if(size)fwrite(image + offset, size, 1, bin);
	
	fclose(bin);

	return 0;
}

int load_image(char *filename)
{
	FILE *img = NULL;
	dword size = 0;

	img = fopen(filename, "rb");

	if(!img) return 1;

	fseek(img, 0, SEEK_END);
	size = ftell(img);
	fseek(img, 0, SEEK_SET);

	if(!size)
	{
		fclose(img);
		return 2;
	}

	if(image)free(image);

	image = (byte *)malloc(size);
	imgsize = size;

	fread(image, size, 1, img);

	fclose(img);

	return 0;
}

int save_image(char *filename)
{
	FILE *img = NULL;

	// Calculate image size and store it in 

	if(!image) return 2;

	img = fopen(filename, "wb");

	if(!img) return 1;

	fwrite(image, imgsize, 1, img);

	fclose(img);

	return 0;
}

int load_file(char *filename)
{
	int size, i;
	FILE *file = NULL;
	char filename11[11] = "           ";

	if(!image) return 0;

	file = fopen(filename, "rb");

	if(!file) return 0;

	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);

	for(i = 0; i < 11; i++)
	{
		if(filename[i] == '\0') break;
		else if(filename[i] == '.') filename11[i] = ' ';
		else filename11[i] = filename[i];
	}

	write_file(file, size, filename11);

	fclose(file);

	return size;
}

int prompt(void)
{
	unsigned long int count, i, j, k;
	char *stopstring;

	printf("\n(nucflop) ");
	gets(cmd);
	
	count = analyze();

	strcpy(args[0], strlwr(args[0]));

	if(!strcmp(args[0], "exit")) return 1;

	else if(!strcmp(args[0], "create"))
	{
		if(count < 2)
		{
			printf("creating 1440 KB floppy image...");
			if(!create_image()) printf("[OK]\n");
			else printf("[ERROR]\n");
		}
		else
		{
			printf("too much args\n");
		}
	}

	else if(!strcmp(args[0], "load"))
	{
		if(count < 2)
		{
			printf("not enough parameters\n");
		}
		else
		{
			load_image(args[1]);
		}
	}

	else if(!image)
	{
		printf("you have to create or load an image first\n");
		return 0;
	}

	else
	{
		if(!strcmp(args[0], "format"))
		{
			if(count == 1)format_disk(0, "NO NAME    ", 0);
			else format_disk(0, args[1], 0);
			printf("disk has been formatted\n");
			formatted = 1;
		}
	
		else if(!strcmp(args[0], "disp"))
		{
			if(count == 1)
			{
				i = 0;
				j = 24;
			}
			else if(count == 2)
			{
				i = strtol(args[1], &stopstring, 16);
				j = 24;
			}
			else
			{
				i = strtol(args[1], &stopstring, 16);
				j = strtol(args[2], &stopstring, 0);
			}

			for(; j > 0; j--)
			{
				printf("%2X ", image[i++]);
			}
			printf("\n");
		}
	
		else if(!strcmp(args[0], "dispbpb"))
		{
			disp_bs12(0);
		}

		else if(!strcmp(args[0], "dispde"))
		{
			if(count < 2)
			{
				printf("not enough parameters\n");
			}
			
			else if(count == 2)
			{
				i = strtol(args[1], &stopstring, 16);
				disp_dirent(read_bs12(0), i, 0);
			}
			
			else
			{
				i = strtol(args[1], &stopstring, 16);
				j = strtol(args[2], &stopstring, 16);
				disp_dirent(read_bs12(0), i, j);
			}
		}

		else if(!strcmp(args[0], "set"))
		{
			if(count < 3)
			{
				printf("not enough parameters\n");
			}
			else if(count == 3)
			{
				i = strtoul(args[1], &stopstring, 16);
				j = strtoul(args[2], &stopstring, 16);
				image[i] = (unsigned char) (j & 0xFF);
				if((unsigned long)j > 0xFF)
				{
					printf("second parameter is too large, using lower 8 bits only\n");
				}
			}
			else
			{
				printf("too many parameters\n");
			}
		}

		else if(!strcmp(args[0], "loadbin"))
		{
			if(count < 2)
			{
				printf("not enough parameters\n");
			}
			else
			{
				if(count == 2)
				{
					load_binary(0, args[1], 0);
				}
				else if(count == 3)
				{
					load_binary(strtoul(args[2], &stopstring, 16), args[1], 0);
				}
				else
				{
					load_binary(strtoul(args[2], &stopstring, 16), args[1], strtoul(args[3], &stopstring, 0));
				}
			}
		}

		else if(!strcmp(args[0], "savebin"))
		{
			if(count < 2)
			{
				printf("not enough parameters\n");
			}
			else
			{
				if(count == 2)
				{
					save_binary(0, args[1], 0);
				}
				else if(count == 3)
				{
					save_binary(strtoul(args[2], &stopstring, 16), args[1], 0);
				}
				else
				{
					save_binary(strtoul(args[2], &stopstring, 16), args[1], strtoul(args[3], &stopstring, 0));
				}
			}
		}

		else if(!strcmp(args[0], "save"))
		{
			if(count < 2)
			{
				printf("not enough parameters\n");
			}
			else
			{
				save_image(args[1]);
			}
		}

		else if(!strcmp(args[0], "dir"))
		{
			if(count == 1)
			{
				if(dir[0] == '/') show_dir(dir + 1, &dirptr);
				else show_dir(dir, &dirptr);
			}
			else if(count == 2)
			{
				if(args[1][0] == '/')
				{
					k = 0;
					show_dir(args[1] + 1, &k); // don't include trailing /
				}
				else
				{
					sprintf(args[2], "%s/%s", dir, args[1]);
					k = 0;
					if(args[2][0] == '/') show_dir(args[2] + 1, &k);
					else show_dir(args[2], &k);
				}
			}
			else
			{
				i = strtol(args[1], &stopstring, 16);
				j = strtol(args[2], &stopstring, 16);
				dirptr = i * 512 + j;
				show_dir(args[2], &dirptr);
			}
		}
		else if(!strcmp(args[0], "cd"))
		{
			if(count == 1)
			{
				printf("/%s (0x%08lX)", dir, dirptr);
			}
			else
			{
				if(args[1][0] == '/')
				{
					if(check_dir(args[1] + 1, &dirptr))
					{
						printf("directory doesn't exist!\n");
					}
					else strcpy(dir, args[1] + 1);
				}
				else
				{
					if(dir[0] == '\0')
					{
						if(check_dir(args[1], &dirptr))
						{
							printf("directory doesn't exist!\n");
						}
						else strcpy(dir, args[1]);
					}
					else
					{
						sprintf(args[0], "%s/%s", dir, args[1]);
						
						if(check_dir(args[0], &dirptr))
						{
							printf("directory doesn't exist!\n");
						}
						else strcpy(dir, args[0]);
					}
				}
			}
		}


		else printf("command parsing error\n");
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	printf("Nucleus FAT12 Disk Tool\n");
	
	if(argc == 1) while(!prompt());

	else if(!strcmp(argv[1] , "--nucinst"))
	{
		create_image();
		format_disk(0, "NUCLEUSBOOT", 0);
		load_binary(0, "boot_fd.bin", 512);
		load_file("nucload.sys");
		load_file("nucleus.bin");

		// Patch kernel size into nucload.sys
		if (kernelsize & 0x03ff) {
			printf("WARNING: kernel not even multiple of 1KB!\n");
		}

		save_image("nucleus.img");
	}
	
	if (image)
		free(image);

	return 0;
}

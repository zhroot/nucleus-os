#include <stdio.h>
#include <string.h>
#include <nucleus.h>
#include <support.h>
#include <multi.h>
#include <drivers/fs/fat.h>
#include <drivers/mem/mem.h>
#include <video/graphic.h>

char cmd[100];
char drive = 'C';
char path[100] = "C:";
char args[10][20];
char version[] =
	"CDSL v0.1-0.3 JensMoppel\n\r"\
	"CDSL v0.04 TDS\n\r";

int _echo = 1, _running = 0, count;

void cdsl(void);
void not_supported(void);
void func_cd(void);
void func_date(void);
void func_dir(void);
void func_echo(void);
void func_help(void);
void func_mode(void);
void func_shutdown(void);
void func_time(void);
void func_version(void);

struct shell_command
{
	char command[20];
	void (*execute)(void);
} shell_commands[34] =
{
	{"break", not_supported},		{"buffers", not_supported},	
	{"call", not_supported},		{"chcp", not_supported},
	{"cd", func_cd},				{"chdir", func_cd},
	{"cdsl", cdsl},					{"clear", clrscr},
	{"cls", clrscr},				{"command", cdsl},
	{"copy", not_supported},		{"country", not_supported},
	{"ctty", not_supported},		{"date", func_date},
	{"del", not_supported},			{"dir", func_dir},
	{"echo", func_echo},			{"erase", not_supported},
	{"exit", func_shutdown},		{"fcbs", not_supported},
	{"for", not_supported},			{"goto", not_supported},
	{"help", func_help},			{"if", not_supported},
	{"md", not_supported},			{"mem", mem_walk},
	{"mode", func_mode},			{"path", not_supported},
	{"pause", not_supported},		{"prompt", not_supported},
	{"pstree", multi_show},			{"shutdown", func_shutdown},	
	{"ver", func_version},			{"version", func_version}
};


void not_supported(void)
{
	printf("cdsl: Befehl nicht unterstützt\n");
	return;	
}

void func_cd(void)
{
	if(args[1][0] == '\\')
	{
		//sprintf(path, "%c:%s", drive, args[1]);
		printf("Ungültiger Pfad: \"%c:%s\"\n", drive, strupr(args[1]));
	}
	else
	{
		//sprintf(path, "%s\\%s", path, args[1]);
		printf("Ungültiger Pfad: \"%s\\%s\"\n", path, strupr(args[1]));
	}
}

void func_date(void)
{
}

void func_dir(void)
{	
	struct find_t f;
	char tmp[256]; 

	if (args[1][0])
		strcpy(tmp, args[1]);
	else
		strcpy(tmp, "*.*");

	if ( !_fat_findfirst(tmp, _A_ARCH | _A_RDONLY, &f))
	{
  		do {
    		printf("%-14s %10ld %02d:%02d:%02d %02d/%02d/%04d\n",
           		f.name, f.size, (f.wr_time >> 11) & 0x1f,
				(f.wr_time >>  5) & 0x3f, (f.wr_time & 0x1f) * 2,
				(f.wr_date >>  5) & 0x0f, (f.wr_date & 0x1f),
				((f.wr_date >> 9) & 0x7f) + 1980);
		} while( !_fat_findnext(&f));
	}
}

void func_echo(void)
{
	int i;
	
	if(count == 1)
	{
		if(_echo)printf("ECHO ist an\n");
		else printf("ECHO ist aus\n");
	}
	else if((count == 2) && (!strcmp("ON", strupr(args[1]))))
	{
		_echo = 1;
	}
	else if((count == 2) && (!strcmp("OFF", strupr(args[1]))))
	{
		_echo = 0;
	}
	else
	{
		for(i = 1; i < count - 1; i++)
		{
			printf("%s ", args[i]);
		}
		printf("%s\n", args[count - 1]);
	}
}

void func_help(void)
{
	int i;
	
	for (i=0;i<sizeof(shell_commands)/sizeof(struct shell_command);i++)
	{		
		printf("%s", shell_commands[i].command);
		if (i % 2 != 0)
			gotoxy(20, wherey());
		else
			printf("\n");
	}
}

void func_mode(void)
{
	
	if (args[1][0])
	{
		printf("Setting mode %d...\n", (unsigned int)atol(args[1]));
		set_mode((unsigned int)atol(args[1]));
	}
}

void func_shutdown(void)
{
	printf("shutting down...System halted");
	halt(0);
}

void func_time(void)
{
}

void func_version(void)
{
		printf("%s", version);
}

static int analyze(void)
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

static void process(void)
{
	int i;
	
	count = 0;

	while(count == 0)
	{
		if(_echo)
		{
			if(path[2] == '\0')printf("%s\\>", path);
			else printf("%s>", path);
		}

		scanf("%s", cmd);
		printf("\n"); // scanf to be fixed...

		count = analyze();
	}
	
	//strcpy(args[0], strlwr(args[0]));

	if(count > 10)
	{
		printf("Syntaxfehler: Zuviele Parameter\n");
		return;
	}
	for (i=0;i<sizeof(shell_commands)/sizeof(struct shell_command);i++)
	{
		if (!strcmp(strlwr(shell_commands[i].command), strlwr(args[0])))
		{
			shell_commands[i].execute();
			return;
		}
	}
	printf("Befehl oder Dateiname nicht gefunden\n");
	cmd[0] = '\0';
}

void cdsl(void)
{	
	printf("ChaOS DOS Shell Light\n");
	if (_running)
	{
		printf("already running...\n");
		return;
	}
	_running = 1;	
	for(;;)
	{
		if(_echo)printf("\n");
		process();
	}
}

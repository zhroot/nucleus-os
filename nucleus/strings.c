
#include <strings.h>

char *strings_table[3][15] = 
{
	// Deutsch
	{
		"Nucleus Version ",
		" wird gestartet",
		"Das niedrige Debuglevel wurde gewaehlt",
		"Das mittlere Debuglevel wurde gewaehlt",
		"Das hohe Debuglevel wurde gewaehlt",
		"Das gewaehlte Debuglevel wird nicht unterstuetzt",
		"Suche CPU...",
		", mit einer Taktrate von ",
		" MHz gefunden\n",
		"Initalisiere die Interruptverwaltung...",
		"Programmiere PICs/APICs...",
		"Kein Initprozess verfuegbar!",
		"Kernel wurde beendet! Das sollte nie passieren!",
		"Sie muessen den PC manuell ausschalten!",
		"Ok\n"
	},
	// Englisch
	{
		"Nucleus version ",
		" is starting up",
		"The low debuglevel has been chosen",
		"The medium debuglevel has been chosen",
		"The high debuglevel has been chosen",
		"The chosen debuglevel is unsupported",
		"Searching for CPU...",
		", with a clockrate of ",
		" MHz found\n",
		"Initializing interrupt management...",
		"Programming PICs/APICs...",
		"No initprocess available!",
		"Kernel has been killed! That should never happen!",
		"You will have to power down your PC manually",
		"Ok\n"
	},
	// Latein
	{
		"Nucleus Version ",
		" wird gestartet",
		"Das niedrige Debuglevel wurde gewaehlt",
		"Das mittlere Debuglevel wurde gewaehlt",
		"Das hohe Debuglevel wurde gewaehlt",
		"Das gewaehlte Debuglevel wird nicht unterstuetzt",
		"Suche CPU...",
		", mit einer Taktrate von ",
		" MHz gefunden\n",
		"Initalisiere die Interruptverwaltung...",
		"Programmiere PICs/APICs...",
		"Kein Initprozess verfuegbar!",
		"Kernel wurde beendet! Das sollte nie passieren!",
		"Sie muessen den PC manuell ausschalten!",
		"Ok\n"
	},
};

int language = 0;

void strings_setlang(int langid)
{
	if(langid <= 2)language = langid;
}

char* strings_get(int stringid)
{
	return strings_table[language][stringid];
}

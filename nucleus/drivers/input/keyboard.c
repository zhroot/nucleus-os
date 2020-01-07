/* (C) Copyright TDS (Mathias Martin)                    */
/*                                                       */
/* keyboard.c                                            */
/*                                                       */
/* - main code taken from the pc_keyb.c and keyboard.c   */
/*   from the Linux source code                          */
/*   -> read the GNU GENERAL PUBLIC LICENSE              */
/*                                                       */
/* todo-list:                                            */
/*   - add more keys, like F13-F24 or alternative keys   */
/*                                                       */
/* - started rewriting of pascal code on                 */
/*   Tuesday, 10.04.2001                                 */
/* - first version on                                    */
/*   Friday, 13.04.2001 Easter ;-)                       */
/*   - readkey supports: - alt+key, ctrl+key + shift+key */
/*                       - same as the crt readkey       */
/*   - full support of all keys (including print, break  */
/*     and left+right ctrl/alt keys)                     */
/*   - 28.08.2004 - ported to NucleusKL					 */

#include <support.h>
#include <stdio.h>
#include <multi.h>
#include <drivers/timer.h>
#include <drivers/input/keyboard.h>
#include <drivers/input/keys_def.h>

#include <interrupts.h>
#include <drivers/pic.h>
#include <irqa.h>

//#define KBD_REPORT_UNKNOWN
#define KBD_REPORT_TIMEOUTS
//#define DEBUG_KEYBOARD
#define scan_key

#define reboot()	{}

static char CanReboot = 0; // careful
static char CanExit = 0;

// Each buffered key is stored in one of these structures
typedef struct 
{
	unsigned char keycode;
	unsigned char down;
} BufferEnt;

#define KEYB_BUFFER_LEN		128						// Must be power of two
#define KEYB_BUFFER_MASK	(KEYB_BUFFER_LEN - 1)	// For wraparound

static char reply_expected = 0;
static char acknowledge = 0;
static char resend = 0;
static unsigned char led_state = 0;
static unsigned char shift_state = 0;
static unsigned char shift_state2 = 0; // normal, w/o left and right
static unsigned int prev_scancode = 0;

// Keyboard buffer
static BufferEnt keyb_scbuffer[KEYB_BUFFER_LEN];
// Buffer head and tail pointers. When these are equal, buffer is empty.
static int keyb_sc_head;	// Pre-increment before inserting
static int keyb_sc_tail;	// Post-increment after removing

// Append a scancode to the keyboard buffer
static void keyb_insert(unsigned char keycode, unsigned char down)
{
	int newhead;

	// See where the head would be if I inserted a character
	newhead = (keyb_sc_head + 1) & KEYB_BUFFER_MASK;

	if (newhead == keyb_sc_tail) {
		// FIXME: click speaker
		return;		// Buffer is full, discard
	}

	// Insert the character in the queue
	keyb_scbuffer[newhead].keycode = keycode;
	keyb_scbuffer[newhead].down = down;
	keyb_sc_head = newhead;
}

// Get the next scancode in the keyboard buffer
// Returns true if the scancode was retrieved successfully
// Pass NULL pointer to just test buffer for a key
static int keyb_remove(BufferEnt *scancode)
{
	int ints_were_enabled = interrupts_disable();
	int ret;

	// See if there is something in the queue
	ret = 0;
	if (keyb_sc_head != keyb_sc_tail) {
		if (scancode) {
			*scancode = keyb_scbuffer[keyb_sc_tail++];
			keyb_sc_tail &= KEYB_BUFFER_MASK;
		}
		ret = 1;
	}

	if (ints_were_enabled)
		interrupts_enable();

	return ret;
}

char do_acknowledge( unsigned char scancode )
{
	if ( reply_expected )
	{
		if ( scancode == KBD_REPLY_ACK )
		{
			acknowledge = 1;
			reply_expected = 0;
			return 0;
		}
		else
		if ( scancode == KBD_REPLY_RESEND )
		{
			resend = 1;
			reply_expected = 0;
			return 0;
		}
#ifdef DEBUG_KEYBOARD
		printf( "Keyboard reply expected - got %02X\n", scancode );
#endif
	}
	return 1;
}

char kbd_translate( unsigned char scancode, unsigned char *keycode, char raw_mode )
{
	if ( ( scancode == 0xE0 ) || ( scancode == 0xE1 ) )
	{
		prev_scancode = scancode;
		return 0;
	}

	if ( ( scancode == 0x00 ) || ( scancode == 0xFF ) )
	{
		prev_scancode = 0;
		return 0;
	}

	scancode &= 0x7F;

	if ( prev_scancode )
	{
		if ( prev_scancode != 0xE0 )
		{
			if ( ( prev_scancode == 0xE1 ) && ( scancode == 0x1D ) )
			{
				prev_scancode = 0x100;
				return 0;
			}
			else
			if ( ( prev_scancode == 0x100 ) && ( scancode == 0x45 ) )
			{
				*keycode = E1_PAUSE;
				prev_scancode = 0;
			}
			else
			{ // unknown escape sequence
#ifdef KBD_REPORT_UNKNOWN
				if ( !raw_mode )
					printf( "Unknown E1 escape sequence.\n" );
#endif
				prev_scancode = 0;
				return 0;
			}
		}
		else
		{
			prev_scancode = 0;

			if ( ( scancode == 0x2A ) || ( scancode == 0x36 ) )
				return 0;

			if ( e0_keys[ scancode ] )
				* keycode = e0_keys[ scancode ];
			else
			{ // unknown scancode
#ifdef KBD_REPORT_UNKNOWN
				if ( !raw_mode )
					printf( "Unknown scancode E0 %02X.\n", scancode );
#endif
				return 0;
			}
		}
	} else if ( scancode >= SC_LIM )
	{
		*keycode = high_keys[ scancode - SC_LIM ];
		if ( !*keycode )
		{
			if ( !raw_mode )
			{ // unexpected scancode
#ifdef KBD_REPORT_UNKNOWN
				printf( "Unexpected scancode (%02X) ignored.\n", scancode );
#endif
			}
			return 0;
		}
	}
	else
		*keycode = scancode;
	return 1;
}

unsigned int pckbd_unexpected_up( unsigned char keycode )
{
	if ( ( keycode >= SC_LIM ) || ( keycode == 85 ) )
		return 0;

	return 200;
}

void Boot( void )
{
	delay( 500 );
	// if we have come here: reset CPU }
	outportb( 0x64, 0xFE ); // tell keyboard controller to reset CPU
	delay( 500 );           // wait until it has an effect
	// if all didn't work, try this: }
	reboot();
	// we had no luck, so go into an endless loop
	printf( "Can't reboot system, system halted..." );

	halt(0);
}

unsigned char handle_kbd_event( void );
void setleds( unsigned char _led_ );
void setshift( unsigned char _shift_, unsigned char on );

void handle_scancode( unsigned char scancode, unsigned char down )
{
	unsigned char keycode;

	if ( !kbd_translate( scancode, & keycode, 1 ) )
		return ;

	if ( keycode == SHIFT_LEFT )
		setshift( 0x01, down );

	if ( keycode == SHIFT_RIGHT )
		setshift( 0x02, down );

	if ( keycode == CTRL_LEFT )
		setshift( 0x04, down );

	if ( keycode == CTRL_RIGHT )
		setshift( 0x08, down );

	if ( keycode == ALT_LEFT )
		setshift( 0x10, down );

	if ( keycode == ALT_RIGHT )
		setshift( 0x20, down );

	if ( !down )
	{
		if ( keycode == SCROLL_LOCK )
			setleds( 0x01 );

		if ( keycode == NUM_LOCK )
			setleds( 0x02 );

		if ( keycode == CAPS_LOCK )
			setleds( 0x04 );
	}
	else
	{
		if ( CanReboot )
		{
			if ( keycode == 111 )             // Delete pressed
			{
				if ( ( shift_state2 & 2 ) &&   // Ctrl
			        ( shift_state2 & 4 ) )         // Alt
				Boot();
			}
		}
	}

	keyb_insert(scancode, down);
	//	keyb_scancode.keycode = scancode;
	//	keyb_scancode.down = down;
}

unsigned char handle_kbd_event( void )
{
	unsigned char status = inportb( KBD_STATUS_REG );
	unsigned char scancode;

	while ( status & KBD_STAT_OBF )
	{
		scancode = inportb( KBD_DATA_REG );

		if ( ( status & KBD_STAT_MOUSE_OBF ) == 0 )
		{
			if ( do_acknowledge( scancode ) )
					handle_scancode( scancode, ! ( scancode & 0x80 ) );
		}

		status = inportb( KBD_STATUS_REG );
	}
	return status;
}

void kb_wait( void )
{
	unsigned long timeout = KBC_TIMEOUT;

	do	{
		unsigned char status = handle_kbd_event();

		if ( !( status & KBD_STAT_IBF ) )
			return ;
	} while ( timeout-- );
#ifdef KBD_REPORT_TIMEOUTS
	printf( "Keyboard TimeOut! (1)\n" );
#endif
}

int KBD_NO_DATA = -1;  /* No data */
int KBD_BAD_DATA = -2; /* Parity or other error */

int kbd_read_input( void )
{
	int retval;
	unsigned char status, data;

	retval = KBD_NO_DATA;
	status = inportb( KBD_STATUS_REG );

	if ( status & KBD_STAT_OBF )
	{
		data = inportb( KBD_DATA_REG );
		retval = data;

		if ( status & ( KBD_STAT_GTO | KBD_STAT_PERR ) )
			retval = KBD_BAD_DATA;
	}

	return retval;
}

void kbd_clear_input( void )
{
	unsigned char maxread;

	maxread = 100;
	do {
		if ( kbd_read_input() == KBD_NO_DATA )
			break;
		maxread--;
	}
	while ( maxread > 0 );
}

char send_data( unsigned char data )
{
	int retries = 3;
	unsigned long timeout = KBD_TIMEOUT;

	do {
		acknowledge = 0;
		resend = 0;
		reply_expected = 1;
		kb_wait();
		outportb( KBD_DATA_REG, data );

		for ( ; ; )
		{
			if ( acknowledge )
				return 1;
			if ( resend )
				break;
			if ( timeout-- == 0 )
			{
#ifdef KBD_REPORT_TIMEOUTS
				printf( "Keyboard TimeOut! (2)\n" );
#endif
				return 0;
			}
		}
	}
	while ( retries-- );

#ifdef KBD_REPORT_TIMEOUTS
	printf( "Too many NACK's. Noisy keyboard cable ?\n" );
#endif
	return 0;
}

/* 1 = Scroll
2 = Num
4 = Caps */
void setleds( unsigned char _led_ )
{
	if ( led_state & _led_ )
		led_state &= ~_led_;
	else
		led_state |= _led_;

	send_data( KBD_CMD_SET_LEDS ); // besserer Weg, da sonst Abbruch nach
	// False-R?ckgabe
	send_data( led_state );
	send_data( KBD_CMD_ENABLE );
}

/*  1 = Shift links
2 = Shift rechts
4 = Strg  links
8 = Strg  rechts
16 = Alt   links
32 = Alt   rechts */
void setshift( unsigned char _shift_, unsigned char on )
{
	unsigned char shift;

	shift = shift_state;

	if ( on )
		shift |= _shift_;
	else
		shift &= ~_shift_;

	shift_state = shift;
	//Abfrage f?r Shift ohne links/rechts-Unterscheidung
	shift = shift_state2;

	if ( ( shift_state & 1 ) ||     // Shift links
	        ( shift_state & 2 ) )   // Shift rechts
		shift |= 1;
	else
		shift &= ~1;

	if ( ( shift_state & 4 ) ||     //Strg links
	        ( shift_state & 8 ) )   //Strg rechts
		shift |= 2;
	else
		shift &= ~2;

	if ( ( shift_state & 16 ) ||     //Alt links
	        ( shift_state & 32 ) )   //Alt rechts
		shift |= 4;
	else
		shift &= ~4;

	shift_state2 = shift;
}

char GetCurrentKey( unsigned char *keycode, unsigned char *down )
{
	int ints_were_enabled = interrupts_disable();
	int ret;
	BufferEnt tmp;

	ret = 0;
	if (keyb_remove(&tmp)) {
		*keycode = tmp.keycode;
		*down = tmp.down;
		ret = 1;
	}

	if (ints_were_enabled)
		interrupts_enable();

	return ret;
}

void ClearKey( void )
{
	//	keyb_scancode.keycode = 0;
	//	kbd_clear_input();
}

unsigned char SaveKey = 0x00;

unsigned char readkey( void )
{
	unsigned char KeyBuffer, keycode, down, result;
	BufferEnt tmp;

	result = 0x00;

	if ( SaveKey != 0x00 )
	{
		result = SaveKey;
		SaveKey = 0x00;
		return result;
	}

	do	{
		// wait till key is pressed
		while (!keyb_remove(&tmp))
			multi_yield(); // give time to other threads

		// save variables
		keycode = tmp.keycode - 1;
		down = tmp.down;

		if ( down )
		{
			if ( norm_map[ keycode ] == 255 )
				continue;

			// get third level keys
			if ( shift_state & 32 )   // Alt-Gr
			{
				KeyBuffer = alt_map[ keycode ];
				result = KeyBuffer;
				ClearKey();
				break;
			}

			if ( ( ( shift_state & 1 ) == 0 ) && ( ( led_state & 2 ) == 0 ) &&   // Numpad
			        ( ( keycode >= 70 ) && ( keycode <= 82 ) ) )
			{
				KeyBuffer = num_map[ keycode ];
				result = KeyBuffer;

				if ( KeyBuffer == 0x00 )   // extended key
					SaveKey = ext_num_map[ keycode ];

				ClearKey();
				break;
			}
			else
			{
				if ( shift_state2 & 1 )   // shift pressed
					KeyBuffer = shift_map[ keycode ];
				else
					KeyBuffer = norm_map[ keycode ];

				result = KeyBuffer;

				if ( KeyBuffer == 0x00 )   // extended key
					SaveKey = ext_map[ keycode ];

				ClearKey();
				break;
			}
		}
	} while ( 1 );	
	return result;
}

char keypressed( void )
{
	return keyb_remove(0);
}

unsigned char GetLEDStatus( void )
{
	return led_state;
}

void SetLEDStatus( unsigned char Status )
{
	setleds( Status );
}

unsigned char GetShiftStatus( void )
{
	return shift_state;
}

unsigned char GetShiftStatus2( void )
{
	return shift_state2;
}

void get_char( unsigned char ch, void * chardata )
{
	outportw( 0x3C4, 0x0402 );
	outportw( 0x3C4, 0x0704 );
	outportw( 0x3CE, 0x0204 );
	outportw( 0x3CE, 0x0005 );
	outportw( 0x3CE, 0x0006 );
	//  Move(Ptr(SegA000, Ord(ch) shl 5)^, Data, 16);
	outportw( 0x3C4, 0x0302 );
	outportw( 0x3C4, 0x0304 );
	outportw( 0x3CE, 0x0004 );
	outportw( 0x3CE, 0x1005 );
	outportw( 0x3CE, 0x0E06 );
}

void set_char( unsigned char ch, void * chardata )
{
	outportw( 0x3C4, 0x0402 );
	outportw( 0x3C4, 0x0704 );
	outportw( 0x3CE, 0x0204 );
	outportw( 0x3CE, 0x0005 );
	outportw( 0x3CE, 0x0006 );
	//  move(chardate, Ptr(segA000, Ord(ch) shl 5)^, 16);
	outportw( 0x3C4, 0x0302 );
	outportw( 0x3C4, 0x0304 );
	outportw( 0x3CE, 0x0004 );
	outportw( 0x3CE, 0x1005 );
	outportw( 0x3CE, 0x0E06 );
}

void kbd_reset( void )
{
	int status;

	outportb( KBD_DATA_REG, 0xFF ); /*reset keyboard   */

	do {
		status = inportb( KBD_STATUS_REG );
		//		printf( "%d", status & 2 );
	}
	while ( status & 0x02 );
	
	outportb( KBD_CNTL_REG, 0xAA ); /*selftest   */
	inportb( KBD_DATA_REG );
	outportb( KBD_CNTL_REG, 0xAB ); /*interface selftest   */
	inportb( KBD_DATA_REG );

	outportb( KBD_CNTL_REG, 0xAE ); /*enable keyboard   */

	outportb( KBD_DATA_REG, 0xFF ); /*reset keyboard   */
	
	inportb( KBD_DATA_REG );
	inportb( KBD_DATA_REG );

	outportb( KBD_CNTL_REG, 0xAD ); /*disable keyboard   */

	outportb( KBD_CNTL_REG, 0x60 );
	outportb( KBD_DATA_REG, 0x01 | 0x04 | 0x20 | 0x40 );

	outportb( KBD_CNTL_REG, 0xAE ); /*enable keyboard   */
}

void keyboard_handler( void )
{
	handle_kbd_event();
}

char keyboard_init( void )
{
	// 	unsigned char i;
	if ( !CanExit )
	{
		printf( "keyboard: Initialize keyboard functions (german layout)\n" );
		kbd_reset();

		kbd_clear_input();
		setleds( 0x00 ); // getting LED status not possible,
		// init with reset
		//		for (i=0;i<=255;i++)
		//			GetChar(chr(i), Save[i]); // save font
		//		Font = Save;
		CanReboot = 1;
		CanExit = 1;
	}
	else
		printf( "keyboard: Error: Keyboard driver already started\n" );

	interrupt_install(INT_HARDWARE, 1, isr_keyboard);
	irq_enable(1);

	return 1;
}

#ifdef DEBUG_KEYBOARD

int main( void )
{
	unsigned char a = 0;

#ifdef scan_key
	unsigned char b, save1, save2;
#endif

	if ( !StartKeyboard() )
		return 1;

#ifdef scan_key
	b = 0;
	save1 = 0;
	save2 = 0;
#endif

#ifdef scan_key
	do	{
		GetCurrentKey( & a, & b );

		if ( save1 != a || save2 != b )
		{
			printf( "%03d %s\n", a, b ? "pressed" : "released" );
			save1 = a;
			save2 = b;
		}
	}
	while ( a != 1 );
#else
	do {
		while ( keypressed() )
		{
			a = readkey();
			if ( a != 0x00 )
				printf( "%c", a );
		}
	}
	while ( a != 27 );
#endif
	EndKeyboard();
	return 0;
}

#endif

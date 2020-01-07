/* (C) Copyright TDS (Mathias Martin)                    */
/*                                                       */
/* keyboard.h                                            */
/*                                                       */
/* standard keyboard variables for accessing ports       */

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#define KBD_INIT_TIMEOUT  1000 // Timeout in ms for sending to keyboard controller
#define KBC_TIMEOUT       250  // Timeout in ms for keyboard command acknowledge
#define KBD_TIMEOUT       500

// Keyboard Controller Registers
#define KBD_STATUS_REG  0x64    // Status register (R)
#define KBD_CNTL_REG    0x64    // Controller command register (W)
#define KBD_DATA_REG    0x60    // Keyboard data register (R/W)

// Keyboard Controller Commands
#define KBD_CCMD_READ_MODE      0x20   // Read mode bits
#define KBD_CCMD_WRITE_MODE     0x60   // Write mode bits
#define KBD_CCMD_GET_VERSION    0xA1   // Get controller version
#define KBD_CCMD_MOUSE_DISABLE  0xA7   // Disable mouse interface
#define KBD_CCMD_MOUSE_ENABLE   0xA8   // Enable mouse interface
#define KBD_CCMD_TEST_MOUSE     0xA9   // Mouse interface test
#define KBD_CCMD_SELF_TEST      0xAA   // Controller self test
#define KBD_CCMD_KBD_TEST       0xAB   // Keyboard interface test
#define KBD_CCMD_KBD_DISABLE    0xAD   // Keyboard interface disable
#define KBD_CCMD_KBD_ENABLE     0xAE   // Keyboard interface enable
#define KBD_CCMD_WRITE_AUX_OBUF 0xD3   // Write to output buffer as if
                                       // initiated by the auxiliary device
#define KBD_CCMD_WRITE_MOUSE    0xD4   // Write the following byte to the mouse

// Keyboard Commands
#define KBD_CMD_SET_LEDS   0xED  // Set keyboard leds
#define KBD_CMD_SET_RATE   0xF3  // Set typematic rate
#define KBD_CMD_ENABLE     0xF4  // Enable scanning
#define KBD_CMD_DISABLE    0xF5  // Disable scanning
#define KBD_CMD_RESET      0xFF  // Reset

// Keyboard Replies
#define KBD_REPLY_POR    0xAA   // Power on reset
#define KBD_REPLY_ACK    0xFA   // Command ACK
#define KBD_REPLY_RESEND 0xFE   // Command NACK, send the cmd again

// Status Register Bits
#define KBD_STAT_OBF       0x01  // Keyboard output buffer full
#define KBD_STAT_IBF       0x02  // Keyboard input buffer full
#define KBD_STAT_SELFTEST  0x04  // Self test successful
#define KBD_STAT_CMD       0x08  // Last write was a command write (0data)
#define KBD_STAT_UNLOCKED  0x10  // Zero if keyboard locked
#define KBD_STAT_MOUSE_OBF 0x20  // Mouse output buffer full
#define KBD_STAT_GTO       0x40  // General receive/xmit timeout
#define KBD_STAT_PERR      0x80  // Parity error
#define AUX_STAT_OBF  (KBD_STAT_OBF | KBD_STAT_MOUSE_OBF)

// Controller Mode Register Bits
#define KBD_MODE_KBD_INT       0x01  // Keyboard data generate IRQ1
#define KBD_MODE_MOUSE_INT     0x02  // Mouse data generate IRQ12
#define KBD_MODE_SYS           0x04  // The system flag (?)
#define KBD_MODE_NO_KEYLOCK    0x08  // The keylock doesn't affect the keyboard if set
#define KBD_MODE_DISABLE_KBD   0x10  // Disable keyboard interface
#define KBD_MODE_DISABLE_MOUSE 0x20  // Disable mouse interface
#define KBD_MODE_KCC           0x40  // Scan code conversion to PC format
#define KBD_MODE_RFU           0x80

// Mouse Commands
#define AUX_SET_RES     0xE8  // Set resolution
#define AUX_SET_SCALE11 0xE6  // Set 1:1 scaling
#define AUX_SET_SCALE21 0xE7  // Set 2:1 scaling
#define AUX_GET_SCALE   0xE9  // Get scaling factor
#define AUX_SET_STREAM  0xEA  // Set stream mode
#define AUX_SET_SAMPLE  0xF3  // Set sample rate
#define AUX_ENABLE_DEV  0xF4  // Enable aux device
#define AUX_DISABLE_DEV 0xF5  // Enable aux device
#define AUX_RESET       0xFF  // Enable aux device
#define AUX_ACK         0xFA  // Enable aux device

// This might be better divisible by
// three to make overruns stay in sync
// but then the read function would need
// A lock etc - ick
#define AUX_BUF_SIZE  2048


#define CAPS_LOCK   58
#define NUM_LOCK    69
#define SCROLL_LOCK 70

#define SHIFT_LEFT  42
#define SHIFT_RIGHT 54
#define CTRL_LEFT   29
#define CTRL_RIGHT  97
#define ALT_LEFT    56
#define ALT_RIGHT  100

#define SC_LIM    89
#define FOCUS_PF1 85           // actual code!
#define FOCUS_PF2 89
#define FOCUS_PF3 90
#define FOCUS_PF4 91
#define FOCUS_PF5 92
#define FOCUS_PF6 93
#define FOCUS_PF7 94
#define FOCUS_PF8 95
#define FOCUS_PF9 120
#define FOCUS_PF10 121
#define FOCUS_PF11 122
#define FOCUS_PF12 123

#define JAP_86    124
/* The four keys are located over the numeric keypad, and are
   labelled A1-A4. It's an rc930 keyboard, from
   Regnecentralen/RC International, Now ICL.
   Scancodes: 59, 5A, 5B, 5C. */
#define RGN1     124
#define RGN2     125
#define RGN3     126
#define RGN4     127

#define E0_KPENTER  96

#define       E0_RCTRL    97
#define       E0_KPSLASH  98
#define       E0_PRSCR    99
#define       E0_RALT     100
#define       E0_BREAK    101  // Strg-Pause
#define       E0_HOME     102
#define       E0_UP       103
#define       E0_PGUP     104
#define       E0_LEFT     105
#define       E0_RIGHT    106
#define       E0_END      107
#define       E0_DOWN     108
#define       E0_PGDN     109
#define       E0_INS      110
#define       E0_DEL      111
#define       E1_PAUSE    119
      // BTC
#define       E0_MACRO    112
      // LK450
#define       E0_F13      113
#define       E0_F14      114
#define       E0_HELP     115
#define       E0_DO       116
#define       E0_F17      117
#define       E0_KPMINPLUS 118
      // OmniKey generates E0 4C for  the "OMNI" key and the
      // right alt key does nada.
#define       E0_OK	 124
      // New microsoft keyboard is rumoured to have
      //  E0 5D (left window button), e0 5c (right window button),
      //  E0 5D (menu button). [or: LBANNER, RBANNER, RMENU]
      //  [or: Windows_L, Windows_R, TaskMan] }
#define       E0_MSLW	 125
#define       E0_MSRW    126
#define       E0_MSTM	 127

static int high_keys[] =
{ RGN1, RGN2, RGN3, RGN4, 0, 0, 0,                   /* 0x59-0x5F */
  0, 0, 0, 0, 0, 0, 0, 0,                            /* 0x60-0x67 */
  0, 0, 0, 0, 0, FOCUS_PF11, 0, FOCUS_PF12,          /* 0x68-0x6F */
  0, 0, 0, FOCUS_PF2, FOCUS_PF9, 0, 0, FOCUS_PF3,    /* 0x70-0x77 */
  FOCUS_PF4, FOCUS_PF5, FOCUS_PF6, FOCUS_PF7,        /* 0x78-0x7B */
  FOCUS_PF8, JAP_86, FOCUS_PF10, 0                   /* 0x7C-0x7F */
};

static int e0_keys[] =
{ 0, 0, 0, 0, 0, 0, 0, 0,			      /* 0x00-0x07 */
  0, 0, 0, 0, 0, 0, 0, 0,			      /* 0x08-0x0F */
  0, 0, 0, 0, 0, 0, 0, 0,			      /* 0x10-0x17 */
  0, 0, 0, 0, E0_KPENTER, E0_RCTRL, 0, 0,	      /* 0x18-0x1F */
  0, 0, 0, 0, 0, 0, 0, 0,			      /* 0x20-0x27 */
  0, 0, 0, 0, 0, 0, 0, 0,			      /* 0x28-0x2F */
  0, 0, 0, 0, 0, E0_KPSLASH, 0, E0_PRSCR,	      /* 0x30-0x37 */
  E0_RALT, 0, 0, 0, 0, E0_F13, E0_F14, E0_HELP,	      /* 0x38-0x3F */
  E0_DO, E0_F17, 0, 0, 0, 0, E0_BREAK, E0_HOME,	      /* 0x40-0x47 */
  E0_UP, E0_PGUP, 0, E0_LEFT, E0_OK, E0_RIGHT, E0_KPMINPLUS, E0_END,
                                                      /* 0x48-0x4F */
  E0_DOWN, E0_PGDN, E0_INS, E0_DEL, 0, 0, 0, 0,	      /* 0x50-0x57 */
  0, 0, 0, E0_MSLW, E0_MSRW, E0_MSTM, 0, 0,	      /* 0x58-0x5F */
  0, 0, 0, 0, 0, 0, 0, 0,			      /* 0x60-0x67 */
  0, 0, 0, 0, 0, 0, 0, E0_MACRO,		      /* 0x68-0x6F */
  0, 0, 0, 0, 0, 0, 0, 0,			      /* 0x70-0x77 */
  0, 0, 0, 0, 0, 0, 0, 0			      /* 0x78-0x7F */
};

void Boot(void);
unsigned char readkey(void);
char keypressed(void);
char keyboard_init(void);
void kb_wait(void);
void keyboard_handler(void);

#define KEYBOARD_LAYOUT_US	0
#define KEYBOARD_LAYOUT_DE	1
int keyboard_setlayout(int layout);

#endif

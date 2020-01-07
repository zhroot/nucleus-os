/*
* CMOS Date/Time functions v0.2	*
* v0.1: JensMoppel				*
*       - basic routines		*
* v0.2: TDS (30.03.2004)		*
*       - bcd/bin check			*
*       - 12/24 hours check		*
* v0.3: Doug Gale (05.09.2004)						*
*		- Fixed wait for RTC update					*
*		- Disables interrupts during access			*
*		- Added function for getting time & date	*
*/

#include <support.h>
#include <stdio.h>
#include <drivers/cmos.h>

// If a time update never occurs, stop trying to wait
static int cmos_bad_rtc;

// Wait until a CMOS update has just completed
// WARNING: assumes interrupts are already disabled
static void cmos_wait_rtc(void)
{
	int longwait;

	if (cmos_bad_rtc)
		return;

	longwait = 2000;	// Timeout after 2 seconds

	// Wait for an update to start
	do {
		outb(0x70, 0x0A);
		if (--longwait == 0) {
			cmos_bad_rtc = 1;
			return;		// Abandon
		}
	} while ((inb(0x71) & 0x80) == 0);

	// Wait for the update to stop
		do {
		outb(0x70, 0x0A);
		if (--longwait == 0) {
			cmos_bad_rtc = 1;
			return;		// Abandon
		}
		} while ((inb(0x71) & 0x80) != 0);
}

// Read a CMOS register. Does not care if RTC update is in progress.
// Don't use this to read the clock, use cmos_get_time
int cmos_read_register(int address)
{
	int retval;
	int ints_were_enabled = interrupts_disable();

	outb(0x70, address);
	retval = inb(0x71);

	if (ints_were_enabled)
		interrupts_enable();

	return retval;
}

#define BCD_TO_BIN(val) (((val)&0x0f) + ((val)>>4)*10)
#define BIN_TO_BCD(val) ((((val)/10)<<4) + (val)%10)

// Read CMOS register, and convert from BCD to binary if necessary
static int cmos_read_register_special(int address)
{
	int tmp;
	
	tmp = cmos_read_register(address);
	if (!(cmos_read_register(CMOS_STATUS) & 0x04))
		return BCD_TO_BIN(tmp);
    return tmp;
}

// Properly get the current wallclock time and date
struct datetime cmos_get_time(void)
{
	struct datetime datetime1;
	unsigned char amp, pm;
	
	int ints_were_enabled = interrupts_disable();

	cmos_wait_rtc();

	datetime1.second = cmos_read_register_special(CMOS_SECOND);
	datetime1.alarm_second = cmos_read_register_special(CMOS_ALARMSECOND);
	datetime1.minute = cmos_read_register_special(CMOS_MINUTE);
	datetime1.alarm_minute = cmos_read_register_special(CMOS_ALARMMINUTE);
	amp = cmos_read_register_special(CMOS_HOUR);
	pm = amp & 0x80;
	amp &= 0x7F;
	datetime1.alarm_hour = cmos_read_register_special(CMOS_ALARMHOUR);
	datetime1.day_in_week = cmos_read_register_special(CMOS_DAYINWEEK);
	datetime1.day = cmos_read_register_special(CMOS_DAY);
	datetime1.month = cmos_read_register_special(CMOS_MONTH);
	datetime1.year = cmos_read_register_special(CMOS_YEAR);
	datetime1.century = cmos_read_register_special(CMOS_CENTURY);

	if (!(cmos_read_register(CMOS_STATUS) & 0x02))	// 12 hour (am/pm) mode
	{
		if (pm == 0x80)
		{
			datetime1.hour = (amp == 12) ? 12 : (amp + 12);
		}
		else
		{
			datetime1.hour = (amp == 12) ? 0 : amp;
		}
	}
	else
		datetime1.hour = amp;
	
	if (ints_were_enabled)
		interrupts_enable();

	return datetime1;
}

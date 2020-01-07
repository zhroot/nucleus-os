#define CMOS_SECOND			0x00
#define CMOS_ALARMSECOND	0x01
#define CMOS_MINUTE			0x02
#define CMOS_ALARMMINUTE	0x03
#define CMOS_HOUR			0x04
#define CMOS_ALARMHOUR		0x05
#define CMOS_DAYINWEEK		0x06
#define CMOS_DAY			0x07
#define CMOS_MONTH			0x08
#define CMOS_YEAR			0x09
#define CMOS_STATUS			0x0B
#define CMOS_CENTURY		0x2D

struct datetime
{
	byte second;
	byte alarm_second;
	byte minute;
	byte alarm_minute;
	byte hour;
	byte alarm_hour;
	byte day_in_week;
	byte day;
	byte month;
	byte year;
	byte century;
};

int cmos_read_register(int address);
struct datetime cmos_get_time(void);

#ifndef _CPUMHZ_H_
#define _CPUMHZ_H_

#define CLOCK_TICK_RATE	1193180
#define HZ 100
#define LATCH  ((CLOCK_TICK_RATE + HZ/2) / HZ)
#define CALIBRATE_LATCH	(5 * LATCH)
#define CALIBRATE_TIME	(5 * HZ)

unsigned long calibrate_tsc(void);
unsigned long get_clockrate(char tsc);

#endif

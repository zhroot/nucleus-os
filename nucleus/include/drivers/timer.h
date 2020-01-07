#ifndef _TIMER_H_
#define _TIMER_H_

#include <datatypes.h>

extern volatile long long timer_ticks;

struct datetime timer_get_time(void);
void timer_tick(void);
void delay(int x);
unsigned timer_get(int channel);
void timer_set(int channel, unsigned value);
void timer_init(void);

#endif

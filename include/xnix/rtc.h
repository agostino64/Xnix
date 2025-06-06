#ifndef _RTC_H
#define _RTC_H

#include "common.h"

typedef struct {
    u8 second;
    u8 minute;
    u8 hour;
    u8 day;
    u8 month;
    u16 year;
} rtc_time_t;

// Reads the current time from the RTC
rtc_time_t read_rtc_time(void);

#endif // _RTC_H

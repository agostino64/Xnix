#ifndef _RTC_H
#define _RTC_H

#include <stdint.h>
#include "common.h"

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

// Reads the current time from the RTC
rtc_time_t read_rtc_time(void);

#endif // _RTC_H

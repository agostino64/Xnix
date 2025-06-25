/*
 *  Xnix
 *
 *  Copyright (C) 2025  Agustin Gutierrez
 *
 *  Real-Time Clock (RTC)
 *  Provides functions to read the current date and time from the CMOS RTC.
 */

#include <stdint.h>
#include <xnix/common.h>
#include <xnix/rtc.h>
#include <xnix/log.h>

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

// CMOS register indices
#define CMOS_SECONDS   0x00
#define CMOS_MINUTES   0x02
#define CMOS_HOURS     0x04
#define CMOS_DAY       0x07
#define CMOS_MONTH     0x08
#define CMOS_YEAR      0x09
#define CMOS_STATUS_B  0x0B

/**
 * Reads a single byte from a CMOS register.
 *
 * @param reg The register index to read from.
 * @return The byte value stored in the specified CMOS register.
 */
static uint8_t read_rtc_register(uint8_t reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

/**
 * Converts a BCD (Binary-Coded Decimal) value to a binary value.
 *
 * @param bcd The BCD value to convert.
 * @return The equivalent binary value.
 */
static uint8_t bcd_to_bin(uint8_t bcd)
{
    return (bcd & 0x0F) + ((bcd / 16) * 10);
}

/**
 * Reads the current RTC time from CMOS and returns it as a structured time.
 *
 * This function reads the time in a consistent manner by checking for rollovers
 * during reads. It also handles conversion from BCD to binary format if needed.
 *
 * @return A populated rtc_time_t structure containing the current time.
 */
rtc_time_t read_rtc_time(void)
{
    rtc_time_t time;
    uint8_t century = 20; // CMOS typically lacks century storage; default to 20xx.

    // Read status register B to determine time format (binary vs BCD)
    uint8_t regB = read_rtc_register(CMOS_STATUS_B);
    bool binary = regB & 0x04;

    // Read values twice to ensure no time rollover between reads
    do {
        time.second = read_rtc_register(CMOS_SECONDS);
        time.minute = read_rtc_register(CMOS_MINUTES);
        time.hour   = read_rtc_register(CMOS_HOURS);
        time.day    = read_rtc_register(CMOS_DAY);
        time.month  = read_rtc_register(CMOS_MONTH);
        time.year   = read_rtc_register(CMOS_YEAR);
    } while (read_rtc_register(CMOS_SECONDS) != time.second);

    // Convert from BCD to binary if necessary
    if (!binary) {
        time.second = bcd_to_bin(time.second);
        time.minute = bcd_to_bin(time.minute);
        time.hour   = bcd_to_bin(time.hour);
        time.day    = bcd_to_bin(time.day);
        time.month  = bcd_to_bin(time.month);
        time.year   = bcd_to_bin(time.year);
    }

    // Expand 2-digit year into full 4-digit year
    time.year += century * 100;

    // Debug output for diagnostics
    //KLOG(LOG_LEVEL_INFO, "RTC Time Read: %d-%d-%d %d:%d:%d\n", time.year, time.month, time.day, time.hour, time.minute, time.second);

    return time;
}

const char* log_get_timestamp(void)
{
    static char timestamp[16];
    rtc_time_t time = read_rtc_time();

    // Format: [HH:MM:SS]
    timestamp[0]  = '[';
    timestamp[1]  = '0' + (time.hour / 10);
    timestamp[2]  = '0' + (time.hour % 10);
    timestamp[3]  = ':';
    timestamp[4]  = '0' + (time.minute / 10);
    timestamp[5]  = '0' + (time.minute % 10);
    timestamp[6]  = ':';
    timestamp[7]  = '0' + (time.second / 10);
    timestamp[8]  = '0' + (time.second % 10);
    timestamp[9]  = ']';
    timestamp[10] = ' ';
    timestamp[11] = '\0';

    return timestamp;
}


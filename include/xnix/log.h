#ifndef LOG_H
#define LOG_H

#include "common.h"  // for types
#include "drivers/serial.h"  // for printk_serial

// ---------------------------------------------------------------
// Logging system configuration
//
// LOG_LEVEL controls the verbosity of logging output.
// This affects KERN_INFO, KERN_WARN, and KERN_ERROR macros.
//
// Available levels (from most to least verbose):
//   LOG_LEVEL_DEBUG  (0) - Show all logs
//   LOG_LEVEL_INFO   (1) - Show INFO, WARN, and ERROR
//   LOG_LEVEL_WARN   (2) - Show only WARN and ERROR
//   LOG_LEVEL_ERROR  (3) - Show only ERROR
//   LOG_LEVEL_NONE   (4) - Disable all logging
//
// NOTE: KERN_DEBUG is controlled independently via the DEBUG macro.
//       Define DEBUG (e.g., -DDEBUG) to enable debug messages.
//
// Example: to show only warnings and errors, set:
//   #define LOG_LEVEL LOG_LEVEL_WARN
// ---------------------------------------------------------------
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_NONE  4

// Set the desired log level here:
// If LOG_LEVEL is not defined externally, default to INFO
#ifndef LOG_LEVEL
    #define LOG_LEVEL LOG_LEVEL_INFO
#endif

// --- INFO, WARN, ERROR logging controlled by LOG_LEVEL ---
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
  #define KERN_DEBUG(format, ...) \
    printk_serial_timetamp("D: %s [%s:%d]: " format, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#else
  #define KERN_DEBUG(format, ...) ((void)0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
  #define KERN_INFO(format, ...) \
    printk_serial_timetamp("I: " format, ##__VA_ARGS__)
#else
  #define KERN_INFO(format, ...) ((void)0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
  #define KERN_WARN(format, ...) \
    printk_serial_timetamp("W: %s [%s:%d]: " format, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#else
  #define KERN_WARN(format, ...) ((void)0)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
  #define KERN_ERROR(format, ...) \
    printk_serial_timetamp("E: %s [%s:%d]: " format, __FILE__, __func__, __LINE__, ##__VA_ARGS__)
#else
  #define KERN_ERROR(format, ...) ((void)0)
#endif

// --- Unified logging macro ---
#define KLOG(level, format, ...) \
    do { \
        if (level == LOG_LEVEL_DEBUG) KERN_DEBUG(format, ##__VA_ARGS__); \
        else if (level == LOG_LEVEL_INFO) KERN_INFO(format, ##__VA_ARGS__); \
        else if (level == LOG_LEVEL_WARN) KERN_WARN(format, ##__VA_ARGS__); \
        else if (level == LOG_LEVEL_ERROR) KERN_ERROR(format, ##__VA_ARGS__); \
    } while (0)

#endif // LOG_H


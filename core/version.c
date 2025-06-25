/*
 *  Get Xnix Kernel Version
 *
 *  Copyright (C) 2025  Agustin Gutierrez
 *
 *  Kernel version and build information banner for the XNIX shell.
 */

#include "../build_info.h"

/** @def STR(x)
 *  Stringifies a macro argument without expanding it.
 */
#define STR(x)   #x

/** @def XSTR(x)
 *  Expands and then stringifies a macro argument.
 */
#define XSTR(x)  STR(x)

/** @def XNIX_VERSION
 *  Kernel version number string.
 */
#define XNIX_VERSION "1.1.2-3"

/** @def BUILD_DATE
 *  Date of compilation
 */
#define BUILD_DATE __DATE__

/** @def BUILD_TIME
 *  Time of compilation
 */
#define BUILD_TIME __TIME__

#ifdef __clang__
    #define BUILD_COMPILER "Clang"
    #define BUILD_COMPILER_MAJOR __clang_major__
    #define BUILD_COMPILER_MINOR __clang_minor__
    #define BUILD_COMPILER_PATCH __clang_patchlevel__
#elif defined(__GNUC__)
    #define BUILD_COMPILER "GCC"
    #define BUILD_COMPILER_MAJOR __GNUC__
    #define BUILD_COMPILER_MINOR __GNUC_MINOR__
    #define BUILD_COMPILER_PATCH __GNUC_PATCHLEVEL__
#else
    #define BUILD_COMPILER "Unknown"
    #define BUILD_COMPILER_MAJOR 0
    #define BUILD_COMPILER_MINOR 0
    #define BUILD_COMPILER_PATCH 0
#endif

#ifndef BUILD_OS
    #define BUILD_OS "Unknown"
#endif

#ifndef BUILD_USER
    #define BUILD_USER "unknown"
#endif

#if IS_DEBUG
  #define BUILD_TYPE "debug"
#else
  #define BUILD_TYPE "release"
#endif

/**
 * @var xnix_proc_banner
 * @brief Kernel build banner string.
 *
 * This string includes kernel version, build metadata, and compilation environment.
 */
const char xnix_proc_banner[] =
    "xnix " XNIX_VERSION "-" BUILD_TYPE " #"
    BUILD_NUM             " "
    BUILD_DATE            " " 
    BUILD_TIME            " ("
    BUILD_COMPILER        " "
    XSTR(BUILD_COMPILER_MAJOR) "."
    XSTR(BUILD_COMPILER_MINOR) "."
    XSTR(BUILD_COMPILER_PATCH)
    " - " BUILD_USER "@" BUILD_OS ")";

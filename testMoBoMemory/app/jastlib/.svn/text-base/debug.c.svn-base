/*! 
 *\file debug.c
 *
 * Define show error&warning message
 *
 * Author: Maggie Tong
 *
 * Copyright (c) 2010 Jabil Circuit.
 *
 * This source code and any compilation or derivative thereof is the sole
 * property of Jabil Circuit and is provided pursuant to a Software License
 * Agreement. This code is the proprietary information of Jabil Circuit and
 * is confidential in nature. Its use and dissemination by any party other
 * than Jabil Circuit is strictly limited by the confidential information
 * provisions of Software License Agreement referenced above.
 
 *\par ChangeLog:
 * June 3, 2010      The initial version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>

#include "utils.h"

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#define inline
#endif

void __attribute__((noreturn))
die(char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    fputs("lspci: ", stderr);
    vfprintf(stderr, msg, args);
    fputc('\n', stderr);
    exit(1);
}

static void generic_error(char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    fputs("lib: ", stderr);
    vfprintf(stderr, msg, args);
    fputc('\n', stderr);
    exit(1);
}

static void generic_warn(char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    fputs("lib: ", stderr);
    vfprintf(stderr, msg, args);
    fputc('\n', stderr);
}

static generic_debug(char *msg, ...)
{
    va_list args;

    va_start(args, msg);
    vfprintf(stdout, msg, args);
    va_end(args);
}

static void null_debug(char * UNUSED msg, ...)
{
}
void tdebug(const char *fmt, ...)
{
#ifdef __DEBUG__
	va_list arg;

	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
#else
	return;
#endif
}

void tprintf(FILE *fp, const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vfprintf(fp, fmt, arg);
	va_end(arg);
}

void tprintt(FILE *fp, const char *fmt, ...)
{
	va_list arg;
	time_t ts = time(NULL);
	struct tm *tm = localtime(&ts);

	/* the month ranges from 0 to 11 and year start from 0 */
	fprintf(fp, "%d/%d/%d-%d:%d:%d: ", 
			tm->tm_mon + 1, tm->tm_mday, tm->tm_year + 1900,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	va_start(arg, fmt);
	vfprintf(fp, fmt, arg);
	va_end(arg);
}

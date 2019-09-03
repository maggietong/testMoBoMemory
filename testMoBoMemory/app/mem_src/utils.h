/* 
 * utils.h
 *
 * Author: Xiang-Yu Wang <rain_wang@jabil.com>
 *
 * Copyright (c) 2008 Jabil Circuit.
 *
 * This source code and any compilation or derivative thereof is the sole
 * property of Jabil Circuit and is provided pursuant to a Software License
 * Agreement. This code is the proprietary information of Jabil Circuit and
 * is confidential in nature. Its use and dissemination by any party other
 * than Jabil Circuit is strictly limited by the confidential information
 * provisions of Software License Agreement referenced above.
 *
 * 2008-01-17 made initial version
 * 2008-11-27 modified to make it a common utilis codes by removing 'sdtset' 
 *            specific definitions.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>


/* print debug messages */
extern void tdebug(const char *, ...);

/* use tpout & tperr to print messages */
extern void tprintf(FILE *, const char *, ...);
extern void tprintt(FILE *, const char *, ...);

#define tpout(fmt, arg...)	tprintf(stdout, fmt, ## arg)
#define tperr(fmt, arg...)	tprintf(stderr, fmt, ## arg)
#define tptout(fmt, arg...)	tprintt(stdout, fmt, ## arg)
#define tpterr(fmt, arg...)	tprintt(stderr, fmt, ## arg)

int CheckInstance(char *appName);
int CheckAllDigits(char *str);
#endif /* UTILS_H */

/*! 
 *\file log.c
 *
 * Define log function
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
 * June 9, 2010      The initial version
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "log.h"

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#define inline
#endif


struct logpriv_s {
    char *name;
    int daemon;
    int level;
};

struct logpriv_s *logpriv;

/* open connection to syslog if daemon  */
void log_init(const char *name, int isdaemon, int verbose)
{
    if (logpriv)
        return;

    logpriv = malloc(sizeof(*logpriv));
    if (!logpriv)
        return;

    if (name != NULL)
        logpriv->name = strdup(name);
    else
        logpriv->name = strdup(LOG_NAME_DEFAULT);
    
    if (logpriv->name == NULL)
        fprintf(stderr, "jedilib: malloc failure\r\n");

    logpriv->daemon = isdaemon;
//    logpriv->level = verbose + LOG_NOTICE;
    logpriv->level = verbose + LOG_WARN;
//    logpriv->level = verbose + LOG_ERR;

    if (logpriv->daemon)
        openlog(logpriv->name, LOG_CONS, LOG_LOCAL4);
}

/*
 * Stop syslog logging if daemon mode,
 * free used memory that stored log service
 */
void log_halt(void)
{
    if (!logpriv)
        return;

    if (logpriv->name)
        free(logpriv->name);
    
    if (logpriv->daemon)
        closelog();

    free(logpriv);
    logpriv = NULL;

}

int log_level_get(void)
{
    return logpriv->level;
}

void log_level_set(int level)
{
    logpriv->level = level;
}

static void log_reinit(void)
{
    log_init(NULL, 0, 0);
}

void lprintf_n(int level, const char *format, ...)
{
    static char logmsg[LOG_MSG_LENGTH];
    va_list vptr;

    if (!logpriv)
        log_reinit();
    if (logpriv->level < level)
        return;

    va_start(vptr, format);
    vsnprintf(logmsg, LOG_MSG_LENGTH, format, vptr);
    va_end(vptr);
    
    if (logpriv->daemon)
        syslog(level, "%s", logmsg);
    else
        fprintf(stdout, "%s\r", logmsg);

    return;

}

void lprintf(int level, const char *format, ...)
{
    static char logmsg[LOG_MSG_LENGTH];
    va_list vptr;

    if (!logpriv)
        log_reinit();
    if (logpriv->level < level)
        return;

    va_start(vptr, format);
    vsnprintf(logmsg, LOG_MSG_LENGTH, format, vptr);
    va_end(vptr);
    
    if (logpriv->daemon)
        syslog(level, "%s", logmsg);
    else
        fprintf(stdout, "%s\r\n", logmsg);

    return;

}

void lperror(int level, const char *format, ...)
{
    static char logmsg[LOG_MSG_LENGTH];
    va_list vptr;

    if (!logpriv)
        log_reinit();

    if (logpriv->level < level)
        return;

    va_start(vptr, format);
    vsnprintf(logmsg, LOG_MSG_LENGTH, format, vptr);
    va_end(vptr);
    
    if (logpriv->daemon)
        syslog(level, "%s: %s", logmsg, strerror(errno));
    else
        fprintf(stderr, "%s\r\n", logmsg);
//        fprintf(stderr, "%s: %s\r\n", logmsg, strerror(errno));

    return;
}


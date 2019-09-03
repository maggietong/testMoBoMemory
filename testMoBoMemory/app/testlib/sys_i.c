/*!
 * \file sys.c
 *
 * The local implementing code of this module.
 *
 * \author Pat Huang
 *
 * Copyright(c) 2009 Jabil Circuit.
 *
 * This source code and any compilation or derivative thereof is the sole property of 
 * Jabil Circuit and is provided pursuant to a Software License Agreement. This code 
 * is the proprietary information of Jabil Circuit and is confidential in nature. 
 * It's use and dissemination by any party other than Jabil Circuit is strictly 
 * limited by the confidential information provisions of Software License Agreement 
 * referenced above.
 *
 * Version history
 * Nov. 12, 2009    Change coding stytle
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define __USE_GNU
#include <sched.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "sys_i.h"
#include "jedi_comm.h"


long isys_isposix(void)
{
	unsigned int ret = ERR_SUCCESS;
#ifdef _SC_VERSION
	ret = sysconf(_SC_VERSION);
	if (ret < 198808L) 
	{
		ret = 0;
	}
#endif

	return ret;
}

long isys_pagesize(void)
{
	long ret = -1; 
#ifdef _SC_PAGE_SIZE
	ret = sysconf(_SC_PAGE_SIZE);
#else
	ret = 4096; /* or 8192? */
#endif
	return ret;
}

int isys_cpu_bind(const unsigned short cpu)
{
	cpu_set_t mask;
	int ret = ERR_SUCCESS;

	CPU_ZERO(&mask);
	CPU_SET((int)cpu, &mask);
	ret = sched_setaffinity(0, sizeof mask, &mask);
	
	return ret;
}

long isys_process_find(const char *name)
{
	DIR *dir;
	struct dirent *entry;
	char name_dir[256];
	char name_target[256];
	char exe_link[256];
	long sz;
	long ret = ERR_SUCCESS;
	ssize_t target_result;
	int cnt_err;

	cnt_err = 0;
	name_target[sizeof(name_target) - 1] = '\0';
	dir = opendir("/proc");
	if (NULL == dir) {
		return ret;
	}
	while(NULL != (entry = readdir(dir))) {
		if (strlen(entry->d_name) == strspn(entry->d_name, "1234567890")) {
			sz = snprintf(name_dir, 
				sizeof name_dir,
				"/proc/%s/",
				entry->d_name
				);
			if (sz > 0) {
				strcpy(exe_link, name_dir);
				strcpy(exe_link + sz, "exe");
				target_result = readlink(exe_link, name_target, sizeof(name_target) - 1);
				if (target_result > 0) {
					if (NULL != strstr(name_target, name)) {
						ret = (long)atoi(entry->d_name);
						break;
					}
				}
			} else {
				break;
			}
		}
	}
	closedir(dir);

	return ret;
}


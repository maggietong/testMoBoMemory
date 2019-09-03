/*!
 *\file jedi_comm.c
 *
 *\brief Define jedi common functions
 *
 *\author Maggie Tong <maggie_tong@jabil.com>
 *
 * Copyright(c) 2010 Jabil Circuit.
 *
 * This source code and any compilation or derivative thereof is the sole property of 
 * Jabil Circuit and is provided pursuant to a Software License Agreement. This code 
 * is the proprietary information of Jabil Circuit and is confidential in nature. 
 * It's use and dissemination by any party other than Jabil Circuit is strictly 
 * limited by the confidential information provisions of Software License Agreement 
 * referenced above.
 *
 *\version 
 *\par ChangeLog:
 * April 19, 2010    Create this file
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/io.h>

#include "log.h"
#include "jedi_comm.h"
#include "pcitool.h"

#define LOGDIR "/usr/local/diags/log/"

static inline int jedi_chkroot(const char *pname)
{
    if (0 != geteuid()) {
        lprintf(LOG_ERR, "You must have the root privileges. exit...\n\n");
        return -EPERM;
    }
    return 0;
}

/*!
 * \brief create the log file common function
 * \param input
 *      errcode     - the error code
 *      errinfo     - the error msg
 *
 *\return none
 *      0           - success
 *      !0          - error
 */
void jedi_create_log_file(int err_code, char *err_info)
{
    FILE *fp;
    char err_buf[LEN_256];
    char filename[LEN_256];

    if (access(LOGDIR, F_OK) != 0)
        mkdir(LOGDIR, 777);
    sprintf(filename, LOGDIR"%08x", err_code);
    memset(err_buf, 0x0, sizeof(err_buf));
    if (err_info != NULL)
        strcpy(err_buf, err_info);

    fp = fopen(filename, "w+");
    if (fp == NULL) {
        printf("[%s] %d: Can't open or create the log file:%s\n", __FILE__, __LINE__, filename);
        return;
    } else {
        fwrite(err_buf, LEN_256, 1, fp);
    }
    fclose(fp);
    return;
}


unsigned int efpci_cfg_pack_addr(efpci_addr_t *p, const unsigned short off)
{
	return (((unsigned int)p->bus << 16)
		| ((unsigned int)p->dev << 11)
		| ((unsigned int)p->fun << 8)
		| off
		| (unsigned int)0x80000000 /*!< enable bit */
		);
}

unsigned int efpci_cfg_read32(efpci_addr_t *p, const unsigned short off)
{
	PIO_OUTL_P(efpci_cfg_pack_addr(p, off), EFPCI_0_CFG_ADDR);
	return PIO_INL_P(EFPCI_0_CFG_DATA);
}

unsigned short efpci_cfg_read16(efpci_addr_t *p, const unsigned short off)
{
	PIO_OUTL_P(efpci_cfg_pack_addr(p, off), EFPCI_0_CFG_ADDR);
	return PIO_INW_P(EFPCI_0_CFG_DATA);
}

unsigned short efpci_cfg_read8(efpci_addr_t *p, const unsigned short off)
{
	PIO_OUTL_P(efpci_cfg_pack_addr(p, off), EFPCI_0_CFG_ADDR);
	return PIO_INB_P(EFPCI_0_CFG_DATA);
}

void efpci_cfg_write32(efpci_addr_t *p, const unsigned short off, const unsigned int data)
{
	PIO_OUTL_P(efpci_cfg_pack_addr(p, off), EFPCI_0_CFG_ADDR);
	PIO_OUTL_P(data, EFPCI_0_CFG_DATA);
}

void efpci_cfg_write16(efpci_addr_t *p, const unsigned short off, const unsigned short data)
{
	PIO_OUTL_P(efpci_cfg_pack_addr(p, off), EFPCI_0_CFG_ADDR);
	PIO_OUTW_P(data, EFPCI_0_CFG_DATA);
}

void efpci_cfg_write8(efpci_addr_t *p, const unsigned short off, const unsigned char data)
{
	PIO_OUTL_P(efpci_cfg_pack_addr(p, off), EFPCI_0_CFG_ADDR);
	PIO_OUTB_P(data, EFPCI_0_CFG_DATA);
}

int efpci_dev_read32(const unsigned char bus,
                    const unsigned char dev,
                    const unsigned char fun,
                    unsigned short off,
                    unsigned int *out,
                    const unsigned short cnt)
{
    efpci_addr_t addr;
    unsigned int *end = out + cnt;

    addr.bus = bus;
    addr.dev = dev;
    addr.fun = fun;

    while (out < end) {
        *out = efpci_cfg_read32(&addr, off);
        out ++;
        off += sizeof(*out);
    }

    return SUCCESS;
}

int efpci_dev_read32_addr(efpci_addr_t *p, unsigned short off, unsigned int *out, const unsigned short cnt)
{
    int ret_err = SUCCESS;
    int privilege = 0;
    unsigned int numperm = 1;
    unsigned short port = EFPCI_0_CFG_ADDR;

    if(port <= BASE_PORT) {
        ret_err = ioperm((unsigned long)port, numperm, IO_ENABLE);
        privilege = LOW_IO_PRIVILEGE;
    } else if (port > BASE_PORT) {
        ret_err = iopl(HIGH_IO_ENABLE);
        privilege = HIGH_IO_PRIVILEGE;
    } else
        return (FAIL);
    
    if (ret_err < 0) {
        fprintf(stderr, "%s(%d): Can't set the io port permission for reading !\n", __FILE__, __LINE__);
        return (FAIL);
    }
    
    efpci_dev_read32(p->bus, p->dev, p->fun, off, out, cnt);

    if (privilege == LOW_IO_PRIVILEGE) {
        ioperm(port, numperm, IO_DISABLE);
    } else if (privilege == HIGH_IO_PRIVILEGE) {
        iopl(IO_DISABLE);
    }
    IO_DELAY(RW_DELAY);

    return SUCCESS;
}



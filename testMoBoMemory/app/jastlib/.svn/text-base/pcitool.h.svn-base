/*!
*\file pcitool.h
* 
* Define the pci structure 
*
*\author Maggie Tong
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
*\par ChangeLog:
*
* July. 35, 2010    Create this file
*/

#ifndef PCITOOL_H_
#define PCITOOL_H_

#include "jedi_comm.h"

#define EFPCI_0_CFG_ADDR    0xcf8
#define EFPCI_0_CFG_DATA    0xcfc
#define EFPCI_1_CFG_ADDR    0xc78
#define EFPCI_1_CFG_DATA    0xc7c

#define MAX_EFPCI_BUS       255
#define efu64_t unsigned long long

#define PIO_INB         inb
#define PIO_INW         inw
#define PIO_INL         inl
#define PIO_INB_P       inb_p
#define PIO_INW_P       inw_p
#define PIO_INL_P       inl_p

#define PIO_OUTB        outb
#define PIO_OUTW        outw
#define PIO_OUTL        outl
#define PIO_OUTB_P      outb_p
#define PIO_OUTW_P      outw_p
#define PIO_OUTL_P      outl_p


/*
 * Copyright (c) 2008-2009 Micah Dowty
 */

typedef struct efpci_addr_t {
	unsigned char bus;
	unsigned char dev;
	unsigned char fun;
} efpci_addr_t;

int efpci_dev_read32_addr(efpci_addr_t *p, unsigned short off, unsigned int *out, const unsigned short cnt);




/*
 * For more PCI information, please look at
 *	http://www.pcisig.com.
 */

/*
 * Each device has 256 bytes configuration address space.
 * The first 64 bytes are common.
 * References: http://docsrv.sco.com/cgi-bin/man/man?pci+D5

   31                    16 15                     0
   -------------------------------------------------
   |       Device ID       |       Vendor ID       | 0x00
   -------------------------------------------------
   |       Status          |       Command         | 0x04
   -------------------------------------------------
   |            Class Code           |   Rev ID.   | 0x08
   -------------------------------------------------
   |    BIST   | Hdr_Type  | Lat_Timer |Cache_Ln_Sz| 0x0c
   -------------------------------------------------
   |                                               | 0x10
   -------------------------------------------------
   |                                               | 0x14
   -------------------------------------------------
   |                                               | 0x18
   -------------------------------------------------
   |                                               | 0x1c
   -------------------------------------------------
   |                                               | 0x20
   -------------------------------------------------
   |                                               | 0x24
   -------------------------------------------------
   |              Cardbus CIS Pointer**            | 0x28
   -------------------------------------------------
   |     Subsystem ID**    | Subsystem Vendor ID** | 0x2c
   -------------------------------------------------
   |       Expansion ROM base address              | 0x30
   -------------------------------------------------
   |               Reserved                        | 0x34
   -------------------------------------------------
   |               Reserved                        | 0x38
   -------------------------------------------------
   |  Max_Lat  | Min_Gnt   | Intr Pin. | Intr line | 0x3c
   -------------------------------------------------

** Field is reserved in PCI 2.0 specification 
*/

union efpci_cfg_t {
	efu64_t c64[8];
	unsigned int c32[16];
	unsigned short c16[32];
	unsigned char c8[64];
	struct {
		/*! 16 bits vendor id */
		unsigned short vendor_id; 
		/*! 16 bits device id */
		unsigned short device_id; 
		// 4B
		/*! 16 bits command */
		unsigned short command; 
		/*! 16 bits status. */
		unsigned short status;
		// 8B
		/*! High 24 bits are class, low 8 revision */
		unsigned char revision_id;
		unsigned char class_code[3]; 
		// 12B
		/*! cache line */
		unsigned char cache_lnsz;
		unsigned char lat_timer; 
		/*! header type */
		unsigned char hdr_type;
		unsigned char bist;
		// 16B
		/*! Base Addresses Registers */
		unsigned int bar[6];
		// 40B
		unsigned int cardbus_cis;
		// 44B
		unsigned short sub_sys_vendor_id;
		unsigned short sub_sys_id;
		// 48B
		unsigned int rom_addr;
		// 52B
		unsigned int resv[2];
		// 60B
		unsigned char intr_line;
		unsigned char intr_pin;
		unsigned char min_grant;
		unsigned char max_lat;
		// 64B
	};
};



#endif /* PCITOOL_H_  */

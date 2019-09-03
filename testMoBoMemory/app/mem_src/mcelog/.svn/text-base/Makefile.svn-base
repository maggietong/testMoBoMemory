#############################################################################################################
#file Makefile
# 
# Makefile 
#
#\author Maggie Tong
#
# Copyright(c) 2011 Jabil Circuit.
#
# This source code and any compilation or derivative thereof is the sole property of 
# Jabil Circuit and is provided pursuant to a Software License Agreement. This code 
# is the proprietary information of Jabil Circuit and is confidential in nature. 
# It's use and dissemination by any party other than Jabil Circuit is strictly 
# limited by the confidential information provisions of Software License Agreement 
# referenced above.
#
# \version
# May. 27, 2011    Create the Makefile
##########################################################################################################

CC= gcc
DFLAGS= -g -Wall -fPIC
WARNINGS := -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter \
	    -Wstrict-prototypes -Wformat-security -Wmissing-declarations \
	    -Wdeclaration-after-statement

#DEBUG= -D__DEBUG__
COMLIB=
INCLUDEDIR= -I. -I../../jastlib

#OBJS= mem_main.o task_generator.o mem_controller.o ../jastlib/log.o ../testlib/mem.o ./mcelog/mcelog_lib.o

OBJ := p4.o k8.o mcelog.o dmi.o tsc.o core2.o bitfield.o intel.o \
       nehalem.o dunnington.o tulsa.o config.o memutil.o msg.o   \
       eventloop.o leaky-bucket.o memdb.o server.o trigger.o 	 \
       client.o cache.o sysfs.o yellow.o page.o rbtree.o 	 \
       xeon75xx.o sandy-bridge.o ../../jastlib/log.o


SRC := $(OBJ:.o=.c)

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDEDIR) $(WARNINGS) $(ADD_DEFINES) -o $@ $<

.depend: ${SRC}
	${CC} -MM $(INCLUDEDIR) ${SRC} > .depend.X && mv .depend.X .depend



STATICTARGET= mcelog_lib.o

INCLUDEDIR+= -I. -I../../jastlib

.c.o:
	$(CC) $(DFLAGS) $(DEBUG) $(INCLUDEDIR) -c $@ $<

$(STATICTARGET):$(OBJ)
	$(AR) rcs $(STATICTARGET) $(OBJ)

install: $(STATICTARGET)

uninstall:clean

clean:
	rm -f $(OBJ) $(STATICTARGET)

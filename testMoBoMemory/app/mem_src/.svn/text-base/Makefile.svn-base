#############################################################################################################
#file Makefile
# 
# Makefile 
#
#\author Maggie Tong
#
# Copyright(c) 2009 Jabil Circuit.
#
# This source code and any compilation or derivative thereof is the sole property of 
# Jabil Circuit and is provided pursuant to a Software License Agreement. This code 
# is the proprietary information of Jabil Circuit and is confidential in nature. 
# It's use and dissemination by any party other than Jabil Circuit is strictly 
# limited by the confidential information provisions of Software License Agreement 
# referenced above.
#
# \version
# Nov. 15, 2009    Create the Makefile
##########################################################################################################

CC= gcc
DFLAGS= -g -Wall -fPIC

#DEBUG= -D__DEBUG__
COMLIB=
INTLLIBS += -lpthread
SOURCE= utils.c mem_main.c task_generator.c mem_controller.c fileops.c e820.c
OBJ= $(SOURCE:.c=.o)
#OBJS= utils.o memory_main.o ../testlib/memory.o ../testlib/absdiag_dev.o ../testlib/memory_err.o ../testlib/cpu_info.o ../testlib/sys_i.o ../testlib/test.o ../testlib/cache_walking.o ../testlib/cache_stuck.o ../testlib/cache_rdwr.o ../testlib/cache_randdata.o ../testlib/cache_randaddr.o ../testlib/cache_spill.o

OBJS= $(OBJ) ../jastlib/log.o ../testlib/mem.a ./mcelog/mcelog_lib.o

TARGET= testMoBoMemory
INCLUDEDIR+= -I. -I../../../include -I../testlib -I../jastlib -I./mcelog -I /usr/src/kernels/`uname -r`/include -I /lib/modules/`uname -r`/build/usr/include/
BINPATH= /usr/local/diags/bin

.c.o:
	$(CC) $(DFLAGS) $(DEBUG) $(INCLUDEDIR) -c $<

$(TARGET):$(OBJ)
	$(CC) $(DFLAGS) $(DEBUG) -o $@ $(OBJS) $(INTLLIBS)
	cp $(TARGET) ../../

install: $(TARGET)
	@echo "Installing testMemory excutable program..."
	/bin/mkdir -p $(BINPATH)
	/bin/cp -f $(TARGET) $(BINPATH)
	/bin/cp -f $(TARGET) ../../
uninstall:
	/bin/rm -f $(BINPATH)/$(TARGET)
	rm -f *.o $(TARGET)
	rm -f ../../$(TARGET)
clean:
	rm -f *.o $(TARGET)
	rm -f ../../$(TARGET)

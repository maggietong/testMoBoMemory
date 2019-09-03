#############################################################################################################
#\file Makefile
# 
# Makefile 
#
#\author Maggie Tong
#
# Copyright(c) 2010 Jabil Circuit.
#
# This source code and any compilation or derivative thereof is the sole property of 
# Jabil Circuit and is provided pursuant to a Software License Agreement. This code 
# is the proprietary information of Jabil Circuit and is confidential in nature. 
# It's use and dissemination by any party other than Jabil Circuit is strictly 
# limited by the confidential information provisions of Software License Agreement 
# referenced above.
#
# \version
# June 08, 2010    The initial created version
##########################################################################################################

CC= gcc

CFLAGS= 
DFLAGS= -g -Wall -rdynamic -fPIC

SHARE= -shared	
DEBUG= -D__DEBUG__
COMLIB=
INTLLIBS += 
SOURCE= jedi_comm.c log.c
OBJ= $(SOURCE:.c=.o)
OBJS= 
INCLUDEDIR+= -I. -I../include
TARGET= libjastlib.so

.c.o:
	$(CC) $(CFLAGS) $(DFLAGS) $(DEBUG) $(INCLUDEDIR) -c $<

$(TARGET):$(OBJ)
	$(CC) $(CFLAGS)$(DFLAGS) $(DEBUG) $(SHARE) -o $@ $(OBJ) $(INTLLIBS)

install:$(TARGET)

uninstall:clean

clean:
	rm -f *.o $(TARGET)

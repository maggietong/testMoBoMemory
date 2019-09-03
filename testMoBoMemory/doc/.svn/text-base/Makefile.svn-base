#---------------------------------------------------------------------------
#
# Copyright (c) 2009 Jabil Circuit, Inc., ALL RIGHTS RESERVED
#
# The information contained herein is confidential property of Jabil Circuit,
# Inc.. The use, copying, transfer, or disclosure of such information is 
# prohibited except by express written agreement with Jabil Circuit, Inc.
#
#---------------------------------------------------------------------------

#---------------------------------------------------------------------------
# \file Makefile
#
# \warning
# Modify carefully
#
# \author Maggie Tong
#
# \version 0.1.1
# \par ChangeLog:
# \verbatim
#  rev 0.1.1 - 
# \endverbatim
#---------------------------------------------------------------------------

SUBDIRS=latex
DOXYGEN=$(shell which doxygen 2>/dev/null)
INSTALL_DIR=/usr/local/diags/doc/
all:doc
#	cd latex; make

install:
	cp -f testMoBoMemory.pdf $(INSTALL_DIR)

uninstall:
	/bin/rm -f $(INSTALL_DIR)/testMoBoMemory.pdf

clean:
	@/bin/rm -rf latex
	@/bin/rm -rf  testMoBoMemory.pdf
.PHONY:doc
doc:
	@if [ "$(DOXYGEN)" != "" ]; then \
		if [ -f Doxyfile ]; then \
			doxygen Doxyfile;\
		fi \
	fi
	
	(cd latex && make && /bin/cp -f refman.pdf ../testMoBoMemory.pdf && cd ..)
	@/bin/rm -rf latex

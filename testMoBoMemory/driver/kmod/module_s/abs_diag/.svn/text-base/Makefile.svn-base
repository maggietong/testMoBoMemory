#
# \file Makefile
#
# \warning
# This Makefile was generated automatically from a template.
# 
# \author Pat Huang
#
# \version r0.1b03
# \par ChangeLog:
# \verbatim
#  rev 0.1-
#    b02, updated generated Makefile file header.
#    b03, 2009sep24, use new Jedi Core source tree.
# \endverbatim
#
#
KDEBUG = n

ifeq (${KDEBUG}, y)
	EXTRA_CFLAGS += -g -DDEBUG
else
	EXTRA_CFLAGS += -O4
endif

KMOD := $(PWD)/../../
#CORE := ${KMOD}/core
EXTRA_CFLAGS += -I${KMOD}/include

MODNAME := ${shell basename $(PWD);}
KDIR := /lib/modules/${shell uname -r}/build
#KDIR := /maggie/maggie/source_code/Absolut-linux-2.6.28.3
#KDIR := /root/rpmbuild/BUILD/kernel-2.6.33.fc13/linux-2.6.33.x86_64
TARGET := ${MODNAME}.ko

SRC := src

obj-m := ${MODNAME}.o 
${MODNAME}-y :=
${MODNAME}-y += ${SRC}/abs_diag_main.o \
	${SRC}/abs_diagdev_procfs.o \
	${SRC}/kthrd.o \
	${SRC}/abs_diag_thrd.o \
	${SRC}/mcamce.o \
	${SRC}/mecc_procfs.o \
	${SRC}/cecc_procfs.o \
	${SRC}/cachetest_procfs.o \
	${SRC}/cachetest.o

all:
	${MAKE} -C ${KDIR} M=$(PWD) modules

clean:
	rm -f ${SRC}/*.o ${SRC}/*.mod.* ${SRC}/.*.cmd
	rm -f *.ko ${TARGET} *.o *.mod.* *.symvers .*.cmd
	rm -rf .tmp_versions Module.* modules.*

install:
	./install.sh

uninstall:
	./uninstall.sh


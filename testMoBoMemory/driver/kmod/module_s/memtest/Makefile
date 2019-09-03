#
# \file Makefile
#
# \author Jackie Liu
#
# \par ChangeLog:
#
#
KDEBUG = n

ifeq (${KDEBUG}, y)
	EXTRA_CFLAGS += -g -DDEBUG
else
	EXTRA_CFLAGS += -O4
endif

MODNAME := memtest
KDIR := /lib/modules/${shell uname -r}/build
#KDIR := /usr/src/kernels/2.6.32-220.el6.x86_64/
#KDIR := /maggie/maggie/source_code/Absolut-linux-2.6.28.3
#KDIR := /root/rpmbuild/BUILD/kernel-2.6.33.fc13/linux-2.6.33.x86_64
TARGET := ${MODNAME}.ko
DRIVERPATH= /lib/modules/${shell uname -r}/extra/diags

SRC := src

obj-m := ${MODNAME}.o 
${MODNAME}-y :=
${MODNAME}-y += ${SRC}/MemoryModule.o $(SRC)/numa.o $(SRC)/e820_kernel.o

all:
	${MAKE} -C  ${KDIR} M=$(PWD) modules

install: all
#	cp -f ${MODNAME}.ko /lib/modules/`uname -r`/kernel/drivers/
#	depmod
	-rmmod ${MODNAME} >/dev/null 2>&1
	mkdir -p ${DRIVERPATH}
	cp -f ${MODNAME}.ko ${DRIVERPATH} 
#	modprobe -f ${MODNAME}
	insmod -f ${TARGET}

uninstall: clean
	@-rmmod ${MODNAME} >/dev/null 2>&1
	-rm -f ${DRIVERPATH}/${TARGET} 
clean:
	rm -f ${SRC}/*.o ${SRC}/*.mod.* ${SRC}/.*.cmd ./*.unsigned
	rm -f *.ko ${TARGET} *.o *.mod.* *.symvers .*.cmd
	rm -rf .tmp_versions Module.* modules.*


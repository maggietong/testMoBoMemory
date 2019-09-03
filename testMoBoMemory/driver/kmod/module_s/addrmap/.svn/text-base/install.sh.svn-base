#!/bin/bash

#
# \file install.sh
#
# \author Pat Huang
#
# \version r0.1b03
# \par ChangeLog:
# \verbatim
#  rev 0.1-
#    b02, 2009sep24, use basename(pwd); 
#      could be used for all modules.
#    b03, 2009oct30, fixed the incorrect order
#      of execution and PWD.
# \endverbatim
#

MODNAME=$(basename "$PWD")
DST=/lib/modules/`uname -r`/extra/diags/

echo "Module name ${MODNAME}"
echo "Destination ${DST}"

make clean
echo "Making module ${MODNAME}.ko..."
make

echo "Removing old module ${MODNAME}"
mkdir -p $(DST)
rm -f ${DST}/${MODNAME}.ko
rmmod ${MODNAME}

echo "Copying ${MODNAME}.ko to ${DST}"
cp -f ${MODNAME}.ko ${DST}
depmod

echo "Inserting module ${MODNAME}"
modprobe -f ${MODNAME}

echo "Make clean"
make clean

echo "Done!"
echo ""


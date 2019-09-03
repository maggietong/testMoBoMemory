#!/bin/bash

#
# \file uninstall.sh
#
# \author Pat Huang
#
# \version r0.1b03
# \par ChangeLog:
# \verbatim
#  rev 0.1-
#    b02, 2009sep24, use basename(pwd); 
#      could be used for all modules.
#    b03, 2009oct30, fixed PWD.
# \endverbatim
#

MODNAME=$(basename "$PWD")
DST=/lib/modules/`uname -r`/extra/diags/

echo "Module name ${MODNAME}"
echo "Destination ${DST}"

echo "Removing module ${MODNAME}"
rmmod ${MODNAME}
rm -f ${DST}${MODNAME}.ko
depmod

echo "Done!"
echo ""


#!/bin/bash

echo "==============================="
echo " REMOVE OLD BUILD"
echo "==============================="

rm -f envinfo

echo "[OK] Old executable removed."

echo "==============================="
echo " BUILD PROGRAM"
echo "==============================="

gcc envinfo.c -o envinfo

if [ $? -ne 0 ]; then
    echo "Compile failed!"
    exit 1
fi

echo "[OK] Build success."

echo "==============================="
echo " RUN PROGRAM"
echo "==============================="

./envinfo

status=$?

echo "Exit status:"
echo $status

echo "==============================="
echo " DONE"
echo "==============================="

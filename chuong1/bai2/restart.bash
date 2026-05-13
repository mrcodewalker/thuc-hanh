#!/bin/bash

echo "==============================="
echo " REMOVE OLD BUILD"
echo "==============================="

rm -f my_ls

echo "[OK] Old executable removed."

echo "==============================="
echo " BUILD PROGRAM"
echo "==============================="

gcc my_ls.c -o my_ls

if [ $? -ne 0 ]; then
    echo "Compile failed!"
    exit 1
fi

echo "[OK] Build success."

echo "==============================="
echo " TEST OPTION -l"
echo "==============================="

./my_ls -l

status=$?

echo "Exit status:"
echo $status

echo "==============================="
echo " TEST OPTION -a"
echo "==============================="

./my_ls -a

status=$?

echo "Exit status:"
echo $status

echo "==============================="
echo " TEST OPTION -la"
echo "==============================="

./my_ls -la

status=$?

echo "Exit status:"
echo $status

echo "==============================="
echo " TEST INVALID OPTION"
echo "==============================="

./my_ls -x

status=$?

echo "Exit status:"
echo $status

echo "==============================="
echo " DONE"
echo "==============================="

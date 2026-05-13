#!/bin/bash

echo "==============================="
echo " REMOVE OLD BUILD"
echo "==============================="

# Xóa executable cũ
rm -f openfile

# Xóa file test cũ
rm -f test.txt

echo "[OK] Old files removed."

echo "==============================="
echo " BUILD PROGRAM"
echo "==============================="

# Compile chương trình
gcc openfile.c -o openfile

if [ $? -ne 0 ]; then
    echo "Compile failed!"
    exit 1
fi

echo "[OK] Build success."

echo "==============================="
echo " CREATE TEST FILE"
echo "==============================="

# Tạo file tồn tại để test
touch test.txt

echo "[OK] test.txt created."

echo "==============================="
echo " TEST EXISTING FILE"
echo "==============================="

./openfile test.txt

echo "Exit status:"
echo $?

echo "==============================="
echo " TEST NON-EXISTING FILE"
echo "==============================="

./openfile abc.txt

status=$?

echo "Exit status:"
echo $status

echo "==============================="
echo " CLEAN FILES"
echo "==============================="

rm -f test.txt

echo "[OK] Cleanup completed."

echo "==============================="
echo " DONE"
echo "==============================="

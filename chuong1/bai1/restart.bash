#!/bin/bash

echo "==============================="
echo " REMOVE OLD SHARED LIBRARY"
echo "==============================="

# Xóa library cũ trong hệ thống
sudo rm -f /usr/lib/libmylib.so

# Cập nhật cache linker
sudo ldconfig

echo "[OK] Old shared library removed."

echo "==============================="
echo " CLEAN OLD FILES"
echo "==============================="

# Xóa file build cũ
rm -f *.o *.so app

echo "[OK] Old build files removed."

echo "==============================="
echo " BUILD SHARED LIBRARY"
echo "==============================="

# Compile object file
gcc -fPIC -c mylib.c

if [ $? -ne 0 ]; then
    echo "Compile mylib.c failed!"
    exit 1
fi

# Build shared library
gcc -shared -o libmylib.so mylib.o

if [ $? -ne 0 ]; then
    echo "Build shared library failed!"
    exit 1
fi

echo "[OK] Shared library created."

echo "==============================="
echo " COPY TO /usr/lib"
echo "==============================="

sudo cp libmylib.so /usr/lib/

# Update linker cache
sudo ldconfig

echo "[OK] Library copied to /usr/lib"

echo "==============================="
echo " BUILD MAIN PROGRAM"
echo "==============================="

gcc main.c -L/usr/lib -lmylib -o app

if [ $? -ne 0 ]; then
    echo "Build main program failed!"
    exit 1
fi

echo "[OK] Application built."

echo "==============================="
echo " RUN APPLICATION"
echo "==============================="

./app

echo "==============================="
echo " CHECK SHARED LIB"
echo "==============================="

ldd app | grep mylib

echo "==============================="
echo " DONE"
echo "==============================="

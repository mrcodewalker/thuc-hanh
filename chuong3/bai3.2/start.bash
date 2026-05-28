#!/bin/bash
# ============================================
# BAI 3.2 - MUTEX FILE WRITE
# 2 threads ghi vao chung 1 file voi mutex
# Su dung wrapper function
# ============================================

clear
echo "┌──────────────────────────────────────────┐"
echo "│   COMPILING BAI 3.2 - MUTEX FILE WRITE   │"
echo "└──────────────────────────────────────────┘"
echo ""

# Kill process cu neu con chay
pkill -f mutex_file 2>/dev/null

# Xoa file output cu
rm -f output.txt

# Compile
gcc -o mutex_file mutex_file.c -lpthread
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile that bai!"
    exit 1
fi

echo "[OK] Compile thanh cong!"
echo ""
echo "─────────────────────────────────────────────"
echo ""

# Chay chuong trinh
./mutex_file

echo ""
echo "─────────────────────────────────────────────"
echo "[DONE] Chuong trinh ket thuc."

#!/bin/bash
# ============================================
# BAI 3.4 - SEMAPHORE FILE SYNC
# 3 threads + semaphore dong bo ghi 2 file
# ============================================

clear
echo "┌──────────────────────────────────────────────┐"
echo "│  COMPILING BAI 3.4 - SEMAPHORE FILE SYNC     │"
echo "└──────────────────────────────────────────────┘"
echo ""

# Kill process cu
pkill -f semaphore_file 2>/dev/null

# Xoa file output cu
rm -f output1.txt output2.txt

# Compile
gcc -o semaphore_file semaphore_file.c -lpthread
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile that bai!"
    exit 1
fi

echo "[OK] Compile thanh cong!"
echo ""
echo "─────────────────────────────────────────────────"
echo ""

# Chay
./semaphore_file

echo ""
echo "─────────────────────────────────────────────────"
echo "[DONE] Chuong trinh ket thuc."

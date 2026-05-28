#!/bin/bash
# ============================================
# BAI 3.1 - MUTEX COUNTER
# 2 threads dong bo tang bien dem bang mutex
# ============================================

clear
echo "┌──────────────────────────────────────────┐"
echo "│     COMPILING BAI 3.1 - MUTEX COUNTER    │"
echo "└──────────────────────────────────────────┘"
echo ""

# Kill process cu neu con chay
pkill -f mutex_counter 2>/dev/null

# Compile
gcc -o mutex_counter mutex_counter.c -lpthread
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile that bai!"
    exit 1
fi

echo "[OK] Compile thanh cong!"
echo ""
echo "─────────────────────────────────────────────"
echo ""

# Chay chuong trinh
./mutex_counter

echo ""
echo "─────────────────────────────────────────────"
echo "[DONE] Chuong trinh ket thuc."

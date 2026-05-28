#!/bin/bash
# ============================================
# BAI 3.6 - DEM SO LE TU 1 DEN 100 TY
# So sanh multithread vs single thread
# ============================================

clear
echo "┌──────────────────────────────────────────────────────┐"
echo "│  COMPILING BAI 3.6 - COUNT ODD NUMBERS (1 to 100B)   │"
echo "└──────────────────────────────────────────────────────┘"
echo ""

# Kill process cu
pkill -f count_odd 2>/dev/null

# Compile
gcc -O2 -o count_odd count_odd.c -lpthread
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile that bai!"
    exit 1
fi

echo "[OK] Compile thanh cong!"
echo ""
echo "─────────────────────────────────────────────────────────"
echo ""

# Chay
./count_odd

echo ""
echo "─────────────────────────────────────────────────────────"
echo "[DONE] Chuong trinh ket thuc."

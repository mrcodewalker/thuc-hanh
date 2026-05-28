#!/bin/bash
# ============================================
# BAI 3.3 - MUTEX COMBINED (Counter + File)
# Ket hop tang counter va ghi file voi mutex
# ============================================

clear
echo "┌──────────────────────────────────────────┐"
echo "│  COMPILING BAI 3.3 - MUTEX COMBINED      │"
echo "└──────────────────────────────────────────┘"
echo ""

# Kill process cu neu con chay
pkill -f mutex_combined 2>/dev/null

# Xoa file output cu
rm -f result.txt

# Compile
gcc -o mutex_combined mutex_combined.c -lpthread
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile that bai!"
    exit 1
fi

echo "[OK] Compile thanh cong!"
echo ""
echo "─────────────────────────────────────────────"
echo ""

# Chay chuong trinh
./mutex_combined

echo ""
echo "─────────────────────────────────────────────"
echo "[DONE] Chuong trinh ket thuc."

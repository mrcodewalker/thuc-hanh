#!/bin/bash
# ============================================
# BAI 3.5 - MATRIX MULTIPLICATION (MULTITHREAD)
# Nhan 10 ma tran 200x200 voi multi thread
# ============================================

clear
echo "┌──────────────────────────────────────────────────┐"
echo "│  COMPILING BAI 3.5 - MATRIX MULTIPLY (PARALLEL)  │"
echo "└──────────────────────────────────────────────────┘"
echo ""

# Kill process cu
pkill -f matrix_multiply 2>/dev/null

# Compile (them -O2 de toi uu toc do)
gcc -O2 -o matrix_multiply matrix_multiply.c -lpthread -lm
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile that bai!"
    exit 1
fi

echo "[OK] Compile thanh cong!"
echo ""
echo "─────────────────────────────────────────────────────"
echo ""

# Chay
./matrix_multiply

echo ""
echo "─────────────────────────────────────────────────────"
echo "[DONE] Chuong trinh ket thuc."

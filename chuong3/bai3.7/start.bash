#!/bin/bash
# ============================================
# BAI 3.7 - TAO FILE SO NGAU NHIEN
# 10 files x 5 trieu so, so sanh thoi gian
# ============================================

clear
echo "┌──────────────────────────────────────────────────────────┐"
echo "│  COMPILING BAI 3.7 - RANDOM FILE GENERATOR (MULTITHREAD) │"
echo "└──────────────────────────────────────────────────────────┘"
echo ""

# Kill process cu
pkill -f random_files 2>/dev/null

# Tao thu muc data
mkdir -p data

# Xoa file cu
rm -f data/random_*.txt

# Compile
gcc -O2 -o random_files random_files.c -lpthread
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile that bai!"
    exit 1
fi

echo "[OK] Compile thanh cong!"
echo ""
echo "─────────────────────────────────────────────────────────────"
echo ""

# Chay
./random_files

echo ""
echo "─────────────────────────────────────────────────────────────"
echo ""

# Hien thi kich thuoc file
echo "Kich thuoc cac file da tao:"
echo ""
ls -lh data/random_*.txt 2>/dev/null | awk '{printf "  %s  %s\n", $5, $9}'

echo ""
echo "─────────────────────────────────────────────────────────────"
echo "[DONE] Chuong trinh ket thuc."

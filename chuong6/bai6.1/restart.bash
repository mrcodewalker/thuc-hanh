#!/bin/bash
# ============================================
# BAI 6.1 - LSEEK FILE I/O (Localhost)
# Biên dịch và chạy thử ứng dụng lseek đọc ghi file
# ============================================

clear
echo "┌──────────────────────────────────────────────────┐"
echo "│         BIÊN DỊCH BÀI 6.1 - LSEEK FILE I/O       │"
echo "└──────────────────────────────────────────────────┘"
echo ""

# Dọn dẹp chương trình cũ
rm -f read_anywhere sample.txt

# Biên dịch chương trình C
gcc -o read_anywhere read_anywhere.c
if [ $? -ne 0 ]; then
    echo "[ERROR] Biên dịch thất bại!"
    exit 1
fi
echo "[OK] Biên dịch thành công: read_anywhere"
echo "─────────────────────────────────────────────────────"
echo ""

# Chạy thử chương trình lần 1 (Mặc định: Offset = 14, Length = 20)
echo "=== LẦN 1: Chạy với thông số mặc định (Offset = 14, Length = 20) ==="
./read_anywhere
echo ""
echo "─────────────────────────────────────────────────────"
echo ""

# Chạy thử chương trình lần 2 (Tùy chỉnh: Offset = 22, Length = 15)
echo "=== LẦN 2: Chạy thử với tham số tuỳ chọn (Offset = 22, Length = 15) ==="
./read_anywhere 22 15
echo ""
echo "─────────────────────────────────────────────────────"
echo "[HƯỚNG DẪN] Bạn có thể chạy thủ công bằng cách nhập: ./read_anywhere <offset> <length>"

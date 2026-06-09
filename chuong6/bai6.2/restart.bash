#!/bin/bash
# ============================================
# BAI 6.2 - DIRECTORY LISTING (Localhost)
# Biên dịch và chạy thử ứng dụng liệt kê folder
# ============================================

clear
echo "┌──────────────────────────────────────────────────┐"
echo "│      BIÊN DỊCH BÀI 6.2 - DIRECTORY LISTING       │"
echo "└──────────────────────────────────────────────────┘"
echo ""

# Dọn dẹp chương trình cũ
rm -f list_dir

# Biên dịch chương trình C
gcc -o list_dir list_dir.c
if [ $? -ne 0 ]; then
    echo "[ERROR] Biên dịch thất bại!"
    exit 1
fi
echo "[OK] Biên dịch thành công: list_dir"
echo "─────────────────────────────────────────────────────"
echo ""

# Chạy thử chương trình lần 1 (Mặc định: thư mục '.' được khai báo trong code)
echo "=== LẦN 1: Chạy quét thư mục mặc định trong source code ( FIXED_DIR_PATH = \".\" ) ==="
./list_dir
echo ""
echo "─────────────────────────────────────────────────────"
echo ""

# Chạy thử chương trình lần 2 (Quét thư mục cha '..')
echo "=== LẦN 2: Chạy thử quét thư mục cha ( .. ) ==="
./list_dir ..
echo ""
echo "─────────────────────────────────────────────────────"
echo "[HƯỚNG DẪN] Bạn có thể chạy thủ công bằng cách nhập: ./list_dir <đường_dẫn_folder>"

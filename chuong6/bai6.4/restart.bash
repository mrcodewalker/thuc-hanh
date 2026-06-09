#!/bin/bash
# ============================================
# BAI 6.4 - CREATE DIRECTORY & READ-ONLY SCAN
# Biên dịch và chạy thử kiểm tra quyền truy cập file
# ============================================

clear
echo "┌──────────────────────────────────────────────────┐"
echo "│         BIÊN DỊCH BÀI 6.4 - READ-ONLY SCAN       │"
echo "└──────────────────────────────────────────────────┘"
echo ""

# Dọn dẹp cấu trúc thư mục cũ để tạo mới hoàn toàn sạch sẽ
rm -rf manage_dir test_structure

# Biên dịch chương trình C
gcc -o manage_dir manage_dir.c
if [ $? -ne 0 ]; then
    echo "[ERROR] Biên dịch thất bại!"
    exit 1
fi
echo "[OK] Biên dịch thành công: manage_dir"
echo "─────────────────────────────────────────────────────"
echo ""

# Chạy thử chương trình để tạo và quét
./manage_dir
echo ""
echo "─────────────────────────────────────────────────────"
echo ""

# In ra cấu trúc thư mục thực tế bằng hệ thống để đối chứng
echo "[XÁC MINH HỆ THỐNG] Hiển thị chi tiết quyền truy cập thực tế (ls -laR):"
echo ""
ls -laR test_structure
echo ""
echo "─────────────────────────────────────────────────────"
echo "[HƯỚNG DẪN] Các file read-only được lọc dựa trên điều kiện: có quyền đọc, không có quyền ghi."

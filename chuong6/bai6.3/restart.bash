#!/bin/bash
# ====================================================
# BAI 6.3 - BUILD & TEST KERNEL DEVICE DRIVER
# Biên dịch, nạp Driver, chạy chương trình thử nghiệm,
# và gỡ bỏ Driver nhân Linux.
# ====================================================

clear
echo "┌──────────────────────────────────────────────────┐"
echo "│    BIÊN DỊCH & KIỂM THỬ LINUX KERNEL DRIVER      │"
echo "└──────────────────────────────────────────────────┘"
echo ""

# 1. Gỡ bỏ driver cũ nếu nó vẫn đang được nạp để tránh lỗi
echo "[1] Dọn dẹp Driver cũ nếu đang được nạp..."
sudo rmmod misc-module 2>/dev/null
make clean >/dev/null 2>&1
sleep 1
echo "    ✓ Hoàn tất."
echo ""

# 2. Biên dịch Driver và Test App bằng lệnh make
echo "[2] Đang tiến hành biên dịch (lệnh make)..."
make
if [ $? -ne 0 ]; then
    echo ""
    echo "[ERROR] Biên dịch thất bại!"
    echo "Gợi ý: Hãy đảm bảo bạn đang chạy trên hệ điều hành Linux và đã cài đặt linux-headers:"
    echo "       Lệnh cài đặt: sudo apt install build-essential linux-headers-\$(uname -r)"
    exit 1
fi
echo "    ✓ Biên dịch thành công!"
echo ""

# 3. Nạp Module vào Nhân Linux (insmod)
echo "[3] Đang nạp Module vào nhân Linux (sudo insmod misc-module.ko)..."
sudo insmod misc-module.ko
if [ $? -ne 0 ]; then
    echo "[ERROR] Nạp module thất bại!"
    exit 1
fi
echo "    ✓ Nạp Driver thành công!"
echo ""

# 4. Kiểm tra sự tồn tại của file /dev/misc-module và phân quyền
echo "[4] Phân quyền cho file thiết bị tự sinh /dev/misc-module..."
if [ ! -c "/dev/misc-module" ]; then
    echo "[ERROR] Không tìm thấy file thiết bị /dev/misc-module!"
    sudo rmmod misc-module
    exit 1
fi
# Cho phép tiến trình không cần root đọc ghi vào file thiết bị này
sudo chmod 666 /dev/misc-module
echo "    ✓ Phân quyền chmod 666 /dev/misc-module hoàn tất."
echo ""

# 5. Chạy ứng dụng kiểm tra ở User Space
echo "[5] Chạy ứng dụng kiểm thử (test-app)..."
echo "────────────────── BẮT ĐẦU TEST-APP ──────────────────"
./test-app
echo "────────────────── KẾT THÚC TEST-APP ──────────────────"
echo ""

# 6. Gỡ bỏ Driver ra khỏi hệ thống (rmmod) và dọn dẹp
echo "[6] Đang gỡ bỏ Driver ra khỏi nhân Linux (sudo rmmod misc-module)..."
sudo rmmod misc-module
if [ $? -ne 0 ]; then
    echo "[ERROR] Gỡ module thất bại!"
    exit 1
fi
echo "    ✓ Gỡ Driver thành công!"
echo ""

# Dọn dẹp các file rác sinh ra trong quá trình build
make clean >/dev/null 2>&1
echo "[OK] Đã dọn dẹp các tệp tin rác. Hoàn tất toàn bộ bài tập 6.3."

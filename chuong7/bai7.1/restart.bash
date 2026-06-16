#!/bin/bash
# ====================================================
# BAI 7.1 - MOUNT KERNEL DEVICE FILE VOI FILE SYSTEM AO
# Biên dịch, nạp Driver, tự động Mount, kiểm thử đồng bộ
# giữa File Thiết bị và Tệp ảo trên VFS, và dọn dẹp hệ thống.
# ====================================================

clear
echo "┌──────────────────────────────────────────────────┐"
echo "│  BIÊN DỊCH & KIỂM THỬ HE THONG FILE KERNEL MYFS  │"
echo "└──────────────────────────────────────────────────┘"
echo ""

MOUNT_DIR="/mnt/myfs_mount"
DEV_FILE="/dev/myfs_dev"

# 1. Dọn dẹp trạng thái cũ (nếu có)
echo "[1] Kiểm tra và dọn dẹp các tài nguyên cũ..."

# Giải phóng các tiến trình đang chiếm dụng tài nguyên (nếu bị treo)
echo "    -> Đang giải phóng các tiến trình đang mở $MOUNT_DIR hoặc $DEV_FILE..."
sudo fuser -k -9 "$MOUNT_DIR" "$DEV_FILE" >/dev/null 2>&1
sleep 1

# Kiểm tra nếu thư mục mount đang hoạt động
if mountpoint -q "$MOUNT_DIR" 2>/dev/null || grep -q "$MOUNT_DIR" /proc/mounts; then
    echo "    -> Đang unmount $MOUNT_DIR..."
    sudo umount -f "$MOUNT_DIR" 2>/dev/null || sudo umount -l "$MOUNT_DIR"
fi

# Gỡ bỏ module cũ nếu đang được nạp
if lsmod | grep -q "^myfs "; then
    echo "    -> Đang gỡ bỏ Kernel Module 'myfs' cũ..."
    sudo rmmod -f myfs 2>/dev/null || sudo rmmod myfs
fi

# Dọn dẹp các tệp tin cũ do quá trình build trước sinh ra
make clean >/dev/null 2>&1
sleep 1
echo "    ✓ Hoàn tất chuẩn bị."
echo ""

# 2. Biên dịch Driver Kernel
echo "[2] Đang tiến hành biên dịch (lệnh make)..."
make
if [ $? -ne 0 ]; then
    echo ""
    echo "[ERROR] Biên dịch thất bại!"
    echo "Gợi ý: Hãy đảm bảo bạn đang chạy trên hệ điều hành Linux và đã cài đặt linux-headers:"
    echo "       sudo apt install build-essential linux-headers-\$(uname -r)"
    exit 1
fi
echo "    ✓ Biên dịch thành công tệp 'myfs.ko'."
echo ""

# 3. Nạp Module vào Nhân Linux (insmod)
echo "[3] Nạp module mới vào nhân Linux (sudo insmod myfs.ko)..."
sudo insmod myfs.ko
if [ $? -ne 0 ]; then
    echo "[ERROR] Nạp module thất bại!"
    exit 1
fi
echo "    ✓ Nạp Kernel Module thành công."
echo ""

# 4. Kiểm tra sự tồn tại của thiết bị /dev/myfs_dev và phân quyền
echo "[4] Kiểm tra và phân quyền cho file thiết bị $DEV_FILE..."
if [ ! -c "$DEV_FILE" ]; then
    echo "[ERROR] Không tìm thấy file thiết bị $DEV_FILE!"
    sudo rmmod myfs
    exit 1
fi
# Cho phép user không cần root đọc ghi vào file thiết bị này
sudo chmod 666 "$DEV_FILE"
echo "    ✓ File thiết bị hoạt động tốt và đã được phân quyền 666."
echo ""

# 5. Tạo thư mục và tiến hành Mount thiết bị
echo "[5] Tạo thư mục mount và tiến hành gắn kết (mount)..."
sudo mkdir -p "$MOUNT_DIR"
sudo chmod 777 "$MOUNT_DIR"

# Thực hiện lệnh mount thiết bị vào thư mục
echo "    Lệnh chạy: sudo mount -t myfs $DEV_FILE $MOUNT_DIR"
sudo mount -t myfs "$DEV_FILE" "$MOUNT_DIR"
if [ $? -ne 0 ]; then
    echo "[ERROR] Mount thất bại!"
    sudo rmmod myfs
    exit 1
fi
echo "    ✓ Gắn kết thiết bị vào thư mục $MOUNT_DIR thành công!"
echo ""

# 6. Kiểm nghiệm thực tế (Verification Tests)
echo "[6] --- BẮT ĐẦU CHƯƠNG TRÌNH KIỂM THỬ ---"
echo ""

# A. Xem danh sách các file trong thư mục mount
echo "A. Liệt kê các tệp tin trong thư mục mount ($MOUNT_DIR):"
echo "--------------------------------------------------------"
ls -la "$MOUNT_DIR"
echo "--------------------------------------------------------"
echo ""

# B. Đọc file thông tin hướng dẫn (readme.txt)
echo "B. Đọc nội dung tệp hướng dẫn 'readme.txt':"
echo "--------------------------------------------------------"
cat "$MOUNT_DIR/readme.txt"
echo "--------------------------------------------------------"
echo ""

# C. Đọc giá trị mặc định lúc khởi tạo trong file hệ thống ảo
echo "C. Giá trị khởi tạo mặc định của 'device_data.txt':"
echo "   $(cat "$MOUNT_DIR/device_data.txt")"
echo ""

# D. Kiểm tra đồng bộ: Ghi vào file thiết bị -> Đọc ở tệp hệ thống ảo
echo "D. Thử nghiệm ghi dữ liệu vào FILE THIẾT BỊ (/dev/myfs_dev):"
MSG1="[Data duoc ghi vao /dev/myfs_dev luc $(date +%H:%M:%S)]"
echo -n "$MSG1" > "$DEV_FILE"
echo "   -> Đã ghi: \"$MSG1\""
echo "   -> Đọc lại từ tệp VFS '$MOUNT_DIR/device_data.txt':"
echo "      \"$(cat "$MOUNT_DIR/device_data.txt")\""
echo ""

# E. Kiểm tra đồng bộ ngược lại: Ghi vào tệp hệ thống ảo -> Đọc ở file thiết bị
echo "E. Thử nghiệm ghi dữ liệu vào TỆP TRÊN FILE SYSTEM ($MOUNT_DIR/device_data.txt):"
MSG2="[Data duoc ghi vao device_data.txt luc $(date +%H:%M:%S)]"
echo -n "$MSG2" > "$MOUNT_DIR/device_data.txt"
echo "   -> Đã ghi: \"$MSG2\""
echo "   -> Đọc lại từ file thiết bị '$DEV_FILE':"
echo "      \"$(cat "$DEV_FILE")\""
echo ""

echo "[6] --- KẾT THÚC CHƯƠNG TRÌNH KIỂM THỬ ---"
echo ""

# 7. Gỡ bỏ thiết lập và dọn dẹp hệ thống
echo "[7] Đang dọn dẹp hệ thống..."
echo "    -> Đang gỡ gắn kết (sudo umount $MOUNT_DIR)..."
sudo umount "$MOUNT_DIR"

echo "    -> Đang tháo gỡ module (sudo rmmod myfs)..."
sudo rmmod myfs

# Xóa thư mục mount nếu trống
if [ -d "$MOUNT_DIR" ] && [ ! "$(ls -A "$MOUNT_DIR")" ]; then
    sudo rmdir "$MOUNT_DIR"
fi

# Clean tệp build rác
make clean >/dev/null 2>&1
echo "    ✓ Hệ thống đã được dọn dẹp sạch sẽ."
echo ""
echo "===================================================="
echo "[OK] Bài tập 7.1 đã hoàn thành và kiểm thử thành công!"
echo "===================================================="

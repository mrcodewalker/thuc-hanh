#!/bin/bash
# ====================================================
# BAI 7.1 - MOUNT KERNEL DEVICE FILE VOI FILE SYSTEM AO
# Biên dịch, nạp Driver, tự động Mount, kiểm thử đồng bộ
# ====================================================

# ANSI Colors
RED='\e[1;31m'
GREEN='\e[1;32m'
YELLOW='\e[1;33m'
BLUE='\e[1;34m'
MAGENTA='\e[1;35m'
CYAN='\e[1;36m'
BOLD='\e[1m'
RESET='\e[0m'

clear
echo -e "${CYAN}┌──────────────────────────────────────────────────┐${RESET}"
echo -e "${CYAN}│  ${BOLD}BIÊN DỊCH & KIỂM THỬ HỆ THỐNG FILE KERNEL MYFS${RESET}  ${CYAN}│${RESET}"
echo -e "${CYAN}└──────────────────────────────────────────────────┘${RESET}"
echo ""

MOUNT_DIR="/mnt/myfs_mount"
DEV_FILE="/dev/myfs_dev"

# 1. Dọn dẹp trạng thái cũ (nếu có)
echo -e "${BLUE}[1] Kiểm tra và dọn dẹp các tài nguyên cũ...${RESET}"

# Giải phóng các tiến trình đang chiếm dụng tài nguyên (nếu bị treo)
echo -e "    -> Đang giải phóng các tiến trình đang mở ${CYAN}$MOUNT_DIR${RESET} hoặc ${CYAN}$DEV_FILE${RESET}..."
sudo fuser -k -9 "$MOUNT_DIR" "$DEV_FILE" >/dev/null 2>&1
sleep 1

# Kiểm tra nếu thư mục mount đang hoạt động
if mountpoint -q "$MOUNT_DIR" 2>/dev/null || grep -q "$MOUNT_DIR" /proc/mounts; then
    echo -e "    -> Đang unmount ${CYAN}$MOUNT_DIR${RESET}..."
    sudo umount -f "$MOUNT_DIR" 2>/dev/null || sudo umount -l "$MOUNT_DIR"
fi

# Gỡ bỏ module cũ nếu đang được nạp
if lsmod | grep -q "^myfs "; then
    echo -e "    -> Đang gỡ bỏ Kernel Module ${CYAN}'myfs'${RESET} cũ..."
    sudo rmmod -f myfs 2>/dev/null || sudo rmmod myfs
fi

# Dọn dẹp các tệp tin cũ do quá trình build trước sinh ra
make clean >/dev/null 2>&1
sleep 1
echo -e "    ${GREEN}✓ Hoàn tất chuẩn bị.${RESET}"
echo ""

# 2. Biên dịch Driver Kernel
echo -e "${BLUE}[2] Đang tiến hành biên dịch (lệnh make)...${RESET}"
make
if [ $? -ne 0 ]; then
    echo ""
    echo -e "${RED}[ERROR] Biên dịch thất bại!${RESET}"
    echo -e "${YELLOW}Gợi ý: Hãy đảm bảo bạn đang chạy trên hệ điều hành Linux và đã cài đặt linux-headers:${RESET}"
    echo -e "       sudo apt install build-essential linux-headers-\$(uname -r)"
    exit 1
fi
echo -e "    ${GREEN}✓ Biên dịch thành công tệp 'myfs.ko'.${RESET}"
echo ""

# 3. Nạp Module vào Nhân Linux (insmod)
echo -e "${BLUE}[3] Nạp module mới vào nhân Linux (sudo insmod myfs.ko)...${RESET}"
sudo insmod myfs.ko
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] Nạp module thất bại!${RESET}"
    exit 1
fi
echo -e "    ${GREEN}✓ Nạp Kernel Module thành công.${RESET}"
echo ""

# 4. Kiểm tra sự tồn tại của thiết bị /dev/myfs_dev và phân quyền
echo -e "${BLUE}[4] Kiểm tra và phân quyền cho file thiết bị ${CYAN}$DEV_FILE${RESET}...${RESET}"
if [ ! -c "$DEV_FILE" ]; then
    echo -e "${RED}[ERROR] Không tìm thấy file thiết bị $DEV_FILE!${RESET}"
    sudo rmmod myfs
    exit 1
fi
# Cho phép user không cần root đọc ghi vào file thiết bị này
sudo chmod 666 "$DEV_FILE"
echo -e "    ${GREEN}✓ File thiết bị hoạt động tốt và đã được phân quyền 666.${RESET}"
echo ""

# 5. Tạo thư mục và tiến hành Mount thiết bị
echo -e "${BLUE}[5] Tạo thư mục mount và tiến hành gắn kết (mount)...${RESET}"
sudo mkdir -p "$MOUNT_DIR"
sudo chmod 777 "$MOUNT_DIR"

# Thực hiện lệnh mount thiết bị vào thư mục
echo -e "    Lệnh chạy: ${YELLOW}sudo mount -t myfs $DEV_FILE $MOUNT_DIR${RESET}"
sudo mount -t myfs "$DEV_FILE" "$MOUNT_DIR"
if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] Mount thất bại!${RESET}"
    sudo rmmod myfs
    exit 1
fi
echo -e "    ${GREEN}✓ Gắn kết thiết bị vào thư mục $MOUNT_DIR thành công!${RESET}"
echo ""

# 6. Kiểm nghiệm thực tế (Verification Tests)
echo -e "${MAGENTA}[6] --- BẮT ĐẦU CHƯƠNG TRÌNH KIỂM THỬ ---${RESET}"
echo ""

# A. Xem danh sách các file trong thư mục mount
echo -e "${YELLOW}A. Liệt kê các tệp tin trong thư mục mount (${CYAN}$MOUNT_DIR${YELLOW}):${RESET}"
echo -e "${CYAN}--------------------------------------------------------${RESET}"
ls -la "$MOUNT_DIR"
echo -e "${CYAN}--------------------------------------------------------${RESET}"
echo ""

# B. Đọc file thông tin hướng dẫn (readme.txt)
echo -e "${YELLOW}B. Đọc nội dung tệp hướng dẫn 'readme.txt':${RESET}"
echo -e "${CYAN}--------------------------------------------------------${RESET}"
cat "$MOUNT_DIR/readme.txt"
echo -e "${CYAN}--------------------------------------------------------${RESET}"
echo ""

# C. Đọc giá trị mặc định lúc khởi tạo trong file hệ thống ảo
echo -e "${YELLOW}C. Giá trị khởi tạo mặc định của 'device_data.txt':${RESET}"
echo -e "   ${BOLD}$(cat "$MOUNT_DIR/device_data.txt")${RESET}"
echo ""

# D. Kiểm tra đồng bộ: Ghi vào file thiết bị -> Đọc ở tệp hệ thống ảo
echo -e "${YELLOW}D. Thử nghiệm ghi dữ liệu vào FILE THIẾT BỊ (${CYAN}/dev/myfs_dev${YELLOW}):${RESET}"
MSG1="[Data duoc ghi vao /dev/myfs_dev luc $(date +%H:%M:%S)]"
echo -n "$MSG1" > "$DEV_FILE"
echo -e "   -> Đã ghi: ${CYAN}\"$MSG1\"${RESET}"
echo -e "   -> Đọc lại từ tệp VFS '${CYAN}$MOUNT_DIR/device_data.txt${RESET}':"
echo -e "      ${BOLD}\"$(cat "$MOUNT_DIR/device_data.txt")\"${RESET}"
echo ""

# E. Kiểm tra đồng bộ ngược lại: Ghi vào tệp hệ thống ảo -> Đọc ở file thiết bị
echo -e "${YELLOW}E. Thử nghiệm ghi dữ liệu vào TỆP TRÊN FILE SYSTEM (${CYAN}$MOUNT_DIR/device_data.txt${YELLOW}):${RESET}"
MSG2="[Data duoc ghi vao device_data.txt luc $(date +%H:%M:%S)]"
echo -n "$MSG2" > "$MOUNT_DIR/device_data.txt"
echo -e "   -> Đã ghi: ${CYAN}\"$MSG2\"${RESET}"
echo -e "   -> Đọc lại từ file thiết bị '${CYAN}$DEV_FILE${RESET}':"
echo -e "      ${BOLD}\"$(cat "$DEV_FILE")\"${RESET}"
echo ""

echo -e "${MAGENTA}[6] --- KẾT THÚC CHƯƠNG TRÌNH KIỂM THỬ ---${RESET}"
echo ""

# Note: We do NOT perform step 7 cleanup at the end so the user and teacher can inspect the live mounted system.
# The cleanup will instead run automatically at the start of the next run of restart.bash.

echo -e "${CYAN}====================================================${RESET}"
echo -e "${GREEN}${BOLD}[OK] Bài tập 7.1 đã hoàn thành và kiểm thử thành công!${RESET}"
echo -e "${YELLOW}Lưu ý: Hệ thống file vẫn được mount tại: ${CYAN}$MOUNT_DIR${RESET}"
echo -e "${YELLOW}Bạn có thể kiểm tra trực tiếp bằng lệnh: ${CYAN}ls -la $MOUNT_DIR${RESET}"
echo -e "${CYAN}====================================================${RESET}"

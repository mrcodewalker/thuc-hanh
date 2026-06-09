#!/bin/bash
# ====================================================
# BAI 6.5 - MAKE BUILD PACKAGING & C SETUP/UNINSTALL
# Biên dịch và đóng gói ứng dụng chat. Hướng dẫn cài đặt.
# ====================================================

clear
echo "┌──────────────────────────────────────────────────┐"
echo "│    BUILD PACKAGING & C INSTALLER/UNINSTALLER     │"
echo "└──────────────────────────────────────────────────┘"
echo ""

# 1. Dọn dẹp bản build cũ
echo "[1] Dang don dep ban build cu..."
make clean >/dev/null 2>&1
echo "    ✓ Da don dep."
echo ""

# 2. Biên dịch và đóng gói dự án bằng Makefile
echo "[2] Dang tien hanh bien dich va dong goi (make)..."
make
if [ $? -ne 0 ]; then
    echo "[ERROR] Bien dich that bai! Hay chac chan rang ban dang o tren Linux."
    exit 1
fi
echo ""
echo "    ✓ Da dong goi tat ca cac file vao thu muc: packet/"
echo "    ├── packet/bin/chat-server     (File Binary Server)"
echo "    ├── packet/bin/chat-client     (File Binary Client)"
echo "    ├── packet/lib/libchat.so      (File Shared Library)"
echo "    └── packet/config/chat.conf    (File Config)"
echo "    ✓ Da bien dich file setup va uninstall."
echo ""
echo "─────────────────────────────────────────────────────"
echo ""

# 3. Huong dan su dung
echo "HƯỚNG DẪN CÀI ĐẶT VÀ CHẠY THỬ NGHIỆM:"
echo ""
echo "  Bước 1: Chạy chương trình cài đặt viết bằng C dưới quyền root:"
echo "          sudo ./setup"
echo ""
echo "  Bước 2: Mở 2 terminal độc lập và chạy chat từ bất kỳ đâu:"
echo "          - Terminal 1: chat-server"
echo "          - Terminal 2: chat-client"
echo ""
echo "  Bước 3: Xem tệp tin cấu hình tại:"
echo "          cat /etc/chat.conf"
echo ""
echo "  Bước 4: Xem tệp tin nhật ký cuộc trò chuyện (Log) được ghi tự động tại:"
echo "          cat /var/log/chat_app.log"
echo ""
echo "  Bước 5: Gỡ bỏ chương trình ra khỏi hệ thống:"
echo "          sudo ./uninstall"
echo ""
echo "─────────────────────────────────────────────────────"
echo "[GỢI Ý] Nếu muốn chạy thử cục bộ tại thư mục hiện tại mà không cần cài đặt:"
echo "        Mở terminal 1: ./chat-server"
echo "        Mở terminal 2: ./chat-client"

#!/bin/bash
# ============================================
# BAI 4.3 - GUI/NHAN FILE (Nha <-> Cong ty)
# File transfer qua socket TCP
# ============================================

clear
echo "┌──────────────────────────────────────────────────┐"
echo "│  COMPILING BAI 4.3 - FILE TRANSFER (Nha<->CongTy)│"
echo "└──────────────────────────────────────────────────┘"
echo ""

# Kill process cu
pkill -f "./file_server" 2>/dev/null
pkill -f "./file_client" 2>/dev/null
sleep 1

# Compile
gcc -o file_server file_server.c
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile file_server that bai!"
    exit 1
fi

gcc -o file_client file_client.c
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile file_client that bai!"
    exit 1
fi

echo "[OK] Compile thanh cong: file_server + file_client"
echo ""

# Tao file test mau de demo
if [ ! -f test_sample.txt ]; then
    echo "Day la file mau de test gui/nhan file." > test_sample.txt
    echo "Bai 4.3 - Chuong trinh gui nhan file qua socket." >> test_sample.txt
    echo "Tao luc: $(date)" >> test_sample.txt
    for i in $(seq 1 100); do
        echo "Dong du lieu thu $i - noi dung mau de kiem tra truyen file." >> test_sample.txt
    done
    echo "[OK] Da tao file mau: test_sample.txt"
fi

mkdir -p received

echo ""
echo "─────────────────────────────────────────────────────"
echo "  DIA CHI IP CUA MAY NAY (may NHAN / CONG TY):"
echo "─────────────────────────────────────────────────────"
if command -v hostname &> /dev/null; then
    hostname -I 2>/dev/null | tr ' ' '\n' | grep -v '^$' | sed 's/^/    /'
fi
echo "─────────────────────────────────────────────────────"
echo ""
echo "  CACH CHAY:"
echo ""
echo "  [May NHAN - \"Cong ty\"]:"
echo "      ./file_server"
echo "      (File nhan duoc luu vao thu muc received/)"
echo ""
echo "  [May GUI - \"Nha\"]:"
echo "      ./file_client <IP_CONG_TY> <file>"
echo "      Vi du: ./file_client 192.168.1.10 baocao.pdf"
echo ""
echo "  [Test cung 1 may]:"
echo "      Terminal 1:  ./file_server"
echo "      Terminal 2:  ./file_client 127.0.0.1 test_sample.txt"
echo ""
echo "─────────────────────────────────────────────────────"

# Tu dong demo cung may
open_demo() {
    if command -v gnome-terminal &> /dev/null; then
        gnome-terminal --title="FILE-SERVER (Cong ty)" -- bash -c "./file_server; exec bash"
        sleep 2
        gnome-terminal --title="FILE-CLIENT (Nha)" -- bash -c "./file_client 127.0.0.1 test_sample.txt; exec bash"
        echo "[OK] Da mo demo: server nhan + client gui test_sample.txt"
        return 0
    elif command -v xterm &> /dev/null; then
        xterm -title "FILE-SERVER" -e "./file_server" &
        sleep 2
        xterm -title "FILE-CLIENT" -e "bash -c './file_client 127.0.0.1 test_sample.txt; read'" &
        echo "[OK] Da mo demo (xterm)"
        return 0
    fi
    return 1
}

echo ""
read -p "Mo demo tu dong tren may nay (gui test_sample.txt)? [y/N]: " ans
if [ "$ans" = "y" ] || [ "$ans" = "Y" ]; then
    if ! open_demo; then
        echo "[!] Khong tim thay terminal emulator. Hay chay thu cong nhu huong dan tren."
    fi
fi

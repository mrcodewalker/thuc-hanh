#!/bin/bash
# ============================================
# BAI 4.2 - CHAT CLIENT-SERVER (LAN)
# Multi-client chat qua mang LAN
# ============================================

clear
echo "┌──────────────────────────────────────────────────┐"
echo "│  COMPILING BAI 4.2 - CHAT LAN (Multi-client)     │"
echo "└──────────────────────────────────────────────────┘"
echo ""

# Kill process cu
pkill -f "./server" 2>/dev/null
pkill -f "./client" 2>/dev/null
sleep 1

# Compile
gcc -o server server.c -lpthread
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile server that bai!"
    exit 1
fi

gcc -o client client.c -lpthread
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile client that bai!"
    exit 1
fi

echo "[OK] Compile thanh cong: server + client"
echo ""

# Hien thi IP cua may (de client khac trong LAN ket noi)
echo "─────────────────────────────────────────────────────"
echo "  DIA CHI IP CUA MAY NAY (dung cho client trong LAN):"
echo "─────────────────────────────────────────────────────"
if command -v hostname &> /dev/null; then
    hostname -I 2>/dev/null | tr ' ' '\n' | grep -v '^$' | sed 's/^/    /'
fi
echo "─────────────────────────────────────────────────────"
echo ""
echo "  CACH CHAY:"
echo ""
echo "  [Tren may SERVER]:"
echo "      ./server"
echo ""
echo "  [Tren may CLIENT trong LAN]:"
echo "      ./client <IP_SERVER> <TEN>"
echo "      Vi du: ./client 192.168.1.10 Alice"
echo ""
echo "  [Test cung 1 may]:"
echo "      Terminal 1:  ./server"
echo "      Terminal 2:  ./client 127.0.0.1 Bob"
echo "      Terminal 3:  ./client 127.0.0.1 Carol"
echo ""
echo "─────────────────────────────────────────────────────"

# Tu dong mo server + 2 client de demo cung may
open_demo() {
    if command -v gnome-terminal &> /dev/null; then
        gnome-terminal --title="SERVER" -- bash -c "./server; exec bash"
        sleep 1
        gnome-terminal --title="CLIENT-Alice" -- bash -c "./client 127.0.0.1 Alice; exec bash"
        sleep 1
        gnome-terminal --title="CLIENT-Bob" -- bash -c "./client 127.0.0.1 Bob; exec bash"
        echo "[OK] Da mo demo: 1 server + 2 client (Alice, Bob)"
        return 0
    elif command -v xterm &> /dev/null; then
        xterm -title "SERVER" -e "./server" &
        sleep 1
        xterm -title "CLIENT-Alice" -e "./client 127.0.0.1 Alice" &
        sleep 1
        xterm -title "CLIENT-Bob" -e "./client 127.0.0.1 Bob" &
        echo "[OK] Da mo demo: 1 server + 2 client (Alice, Bob)"
        return 0
    fi
    return 1
}

echo ""
read -p "Mo demo tu dong (server + 2 client) tren may nay? [y/N]: " ans
if [ "$ans" = "y" ] || [ "$ans" = "Y" ]; then
    if ! open_demo; then
        echo "[!] Khong tim thay terminal emulator. Hay chay thu cong nhu huong dan tren."
    fi
fi

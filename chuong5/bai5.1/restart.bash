#!/bin/bash
# ============================================
# BAI 5.1 - CHAT CLIENT-SERVER (Shared Memory)
# Compile va chay server + client tren cung 1 may
# ============================================

clear
echo "┌──────────────────────────────────────────────────┐"
echo "│  COMPILING BAI 5.1 - CHAT SHARED MEMORY (LOCAL) │"
echo "└──────────────────────────────────────────────────┘"
echo ""

# Kill process cu de giai phong tai nguyen
pkill -f "./server" 2>/dev/null
pkill -f "./client" 2>/dev/null

# Xoa phan vung shared memory cu o cap he thong (Linux thuong map vao /dev/shm)
rm -f /dev/shm/shm_chat_5_1 2>/dev/null
sleep 1

# Compile server
gcc -o server server.c -lpthread -lrt
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile server that bai!"
    exit 1
fi

# Compile client
gcc -o client client.c -lpthread -lrt
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile client that bai!"
    exit 1
fi

echo "[OK] Compile thanh cong: server + client"
echo ""
echo "─────────────────────────────────────────────────────"
echo ""

# Tu dong mo 2 terminal neu co the
open_terminals() {
    if command -v gnome-terminal &> /dev/null; then
        gnome-terminal --title="SERVER (Shared Memory)" -- bash -c "./server; exec bash"
        sleep 1
        gnome-terminal --title="CLIENT (Shared Memory)" -- bash -c "./client; exec bash"
        echo "[OK] Da mo 2 cua so terminal (SERVER + CLIENT)"
        return 0
    elif command -v xterm &> /dev/null; then
        xterm -title "SERVER (Shared Memory)" -e "./server" &
        sleep 1
        xterm -title "CLIENT (Shared Memory)" -e "./client" &
        echo "[OK] Da mo 2 cua so xterm (SERVER + CLIENT)"
        return 0
    elif command -v konsole &> /dev/null; then
        konsole --new-tab -e "./server" &
        sleep 1
        konsole --new-tab -e "./client" &
        echo "[OK] Da mo 2 tab konsole (SERVER + CLIENT)"
        return 0
    else
        return 1
    fi
}

if ! open_terminals; then
    echo "[!] Khong tim thay terminal emulator de tu dong mo."
    echo ""
    echo "  Hay mo 2 terminal va chay thu cong:"
    echo ""
    echo "    Terminal 1 (Server):   ./server"
    echo "    Terminal 2 (Client):   ./client"
    echo ""
fi

echo ""
echo "─────────────────────────────────────────────────────"
echo "[HUONG DAN] Go tin nhan + Enter de chat. Go '/quit' de thoat."

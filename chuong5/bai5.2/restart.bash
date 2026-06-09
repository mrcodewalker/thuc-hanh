#!/bin/bash
# ====================================================
# BAI 5.2 - SIMULATION CLIENT-SERVER (SHM & OS Signals)
# Compile va chay server + client tren cung 1 may
# ====================================================

clear
echo "┌──────────────────────────────────────────────────┐"
echo "│   COMPILING BAI 5.2 - SHM & SIGNAL SIMULATION    │"
echo "└──────────────────────────────────────────────────┘"
echo ""

# Kill process cu
pkill -f "./server" 2>/dev/null
pkill -f "./client" 2>/dev/null

# Xoa shm va log cu neu co
rm -f /dev/shm/shm_data_5_2 2>/dev/null
# Luu y: Khong tu dong xoa file server.log de giu lai lich su, tru khi muon lam sach tu dau
# rm -f server.log 2>/dev/null
sleep 1

# Compile server (dung -lrt cho POSIX Shared Memory)
gcc -o server server.c -lrt
if [ $? -ne 0 ]; then
    echo "[ERROR] Compile server that bai!"
    exit 1
fi

# Compile client
gcc -o client client.c -lrt
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
        gnome-terminal --title="SERVER (Sleep/Wakeup)" -- bash -c "./server; exec bash"
        sleep 1
        gnome-terminal --title="CLIENT (Signal sender)" -- bash -c "./client; exec bash"
        echo "[OK] Da mo 2 cua so terminal (SERVER + CLIENT)"
        return 0
    elif command -v xterm &> /dev/null; then
        xterm -title "SERVER (Sleep/Wakeup)" -e "./server" &
        sleep 1
        xterm -title "CLIENT (Signal sender)" -e "./client" &
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
echo "[HUONG DAN] Nhap Ten, Tuoi o Client de gui tin hieu danh thuc Server."

#!/bin/bash

clear

# =========================
# COLORS
# =========================
RED='\033[1;31m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
CYAN='\033[1;36m'
MAGENTA='\033[1;35m'
WHITE='\033[1;37m'
NC='\033[0m'

echo -e "${CYAN}"
echo "╔══════════════════════════════════════════════╗"
echo "║            FORK() PROCESS DEMO              ║"
echo "║        Linux Kernel Programming Lab         ║"
echo "╚══════════════════════════════════════════════╝"
echo -e "${NC}"

sleep 1

# =========================
# CLEAN
# =========================
echo -e "${YELLOW}[1] Cleaning old files...${NC}"

rm -f fork_demo
rm -f fork_output.txt

echo -e "${GREEN}[OK] Cleanup completed.${NC}"

sleep 1

# =========================
# BUILD
# =========================
echo
echo -e "${BLUE}[2] Building source code...${NC}"

gcc fork_demo.c -o fork_demo

if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] Compile failed!${NC}"
    exit 1
fi

echo -e "${GREEN}[OK] Build success.${NC}"

sleep 1

# =========================
# RUN
# =========================
echo
echo -e "${MAGENTA}========================================${NC}"
echo -e "${MAGENTA} Running fork_demo program${NC}"
echo -e "${MAGENTA}========================================${NC}"

./fork_demo &

MAIN_PID=$!

sleep 2

# =========================
# PROCESS LIST
# =========================
echo
echo -e "${CYAN}[3] Process list using ps${NC}"

echo -e "${WHITE}----------------------------------------${NC}"

ps -ef | grep fork_demo | grep -v grep

echo -e "${WHITE}----------------------------------------${NC}"

sleep 1

# =========================
# FILE CONTENT
# =========================
echo
echo -e "${CYAN}[4] Content of fork_output.txt${NC}"

echo -e "${WHITE}----------------------------------------${NC}"

cat fork_output.txt

echo -e "${WHITE}----------------------------------------${NC}"

sleep 1

# =========================
# FIND CHILD
# =========================
echo
echo -e "${YELLOW}[5] Finding child process...${NC}"

CHILD_PID=$(pgrep -P $MAIN_PID)

if [ ! -z "$CHILD_PID" ]; then
    echo -e "${GREEN}[OK] Child PID found: ${CHILD_PID}${NC}"
else
    echo -e "${RED}[ERROR] Child process not found.${NC}"
fi

sleep 1

# =========================
# KILL CHILD
# =========================
echo
echo -e "${RED}[6] Killing child process...${NC}"

if [ ! -z "$CHILD_PID" ]; then
    kill $CHILD_PID
    echo -e "${GREEN}[OK] Child process killed.${NC}"
fi

sleep 1

# =========================
# CHECK REMAINING
# =========================
echo
echo -e "${CYAN}[7] Remaining processes${NC}"

echo -e "${WHITE}----------------------------------------${NC}"

ps -ef | grep fork_demo | grep -v grep

echo -e "${WHITE}----------------------------------------${NC}"

sleep 1

# =========================
# CLEAN ALL
# =========================
echo
echo -e "${YELLOW}[8] Cleaning remaining processes...${NC}"

pkill fork_demo

echo -e "${GREEN}[OK] All processes cleaned.${NC}"

sleep 1

# =========================
# FINISH
# =========================
echo
echo -e "${GREEN}"
echo "╔══════════════════════════════════════════════╗"
echo "║                 FINISHED                    ║"
echo "║         Fork() demo completed successfully  ║"
echo "╚══════════════════════════════════════════════╝"
echo -e "${NC}"

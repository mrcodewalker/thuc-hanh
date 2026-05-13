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
echo "║         PARENT - CHILD PROCESS DEMO         ║"
echo "║          Linux Process Programming          ║"
echo "╚══════════════════════════════════════════════╝"
echo -e "${NC}"

sleep 1

# =========================
# CLEAN
# =========================
echo -e "${YELLOW}[1] Cleaning old files...${NC}"

rm -f parent
rm -f child
rm -f output.txt

echo -e "${GREEN}[OK] Cleanup completed.${NC}"

sleep 1

# =========================
# BUILD CHILD
# =========================
echo
echo -e "${BLUE}[2] Building child.c ...${NC}"

gcc child.c -o child

if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] child.c compile failed!${NC}"
    exit 1
fi

echo -e "${GREEN}[OK] child build success.${NC}"

sleep 1

# =========================
# BUILD PARENT
# =========================
echo
echo -e "${BLUE}[3] Building parent.c ...${NC}"

gcc parent.c -o parent

if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] parent.c compile failed!${NC}"
    exit 1
fi

echo -e "${GREEN}[OK] parent build success.${NC}"

sleep 1

# =========================
# RUN PROGRAM
# =========================
echo
echo -e "${MAGENTA}========================================${NC}"
echo -e "${MAGENTA} Running parent process${NC}"
echo -e "${MAGENTA}========================================${NC}"

./parent

sleep 1

# =========================
# SHOW FILE CONTENT
# =========================
echo
echo -e "${CYAN}[4] Content of output.txt${NC}"

echo -e "${WHITE}----------------------------------------${NC}"

cat output.txt

echo -e "${WHITE}----------------------------------------${NC}"

sleep 1

# =========================
# SHOW PROCESS CHECK
# =========================
echo
echo -e "${CYAN}[5] Process check${NC}"

ps -ef | grep parent | grep -v grep
ps -ef | grep child | grep -v grep

sleep 1

# =========================
# FINISH
# =========================
echo
echo -e "${GREEN}"
echo "╔══════════════════════════════════════════════╗"
echo "║                 FINISHED                    ║"
echo "║      Parent/Child demo completed OK        ║"
echo "╚══════════════════════════════════════════════╝"
echo -e "${NC}"

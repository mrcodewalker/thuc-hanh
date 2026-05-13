#!/bin/bash

clear

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
echo "║            FORK + LS DEMO                   ║"
echo "║         Process & File Management           ║"
echo "╚══════════════════════════════════════════════╝"
echo -e "${NC}"

sleep 1

# CLEAN
echo -e "${YELLOW}[1] Cleaning old files...${NC}"

rm -f fork_ls
rm -f output.txt

echo -e "${GREEN}[OK] Cleanup completed.${NC}"

sleep 1

# BUILD
echo
echo -e "${BLUE}[2] Building source code...${NC}"

gcc fork_ls.c -o fork_ls

if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] Compile failed!${NC}"
    exit 1
fi

echo -e "${GREEN}[OK] Build success.${NC}"

sleep 1

# RUN
echo
echo -e "${MAGENTA}========================================${NC}"
echo -e "${MAGENTA} Running fork_ls program${NC}"
echo -e "${MAGENTA}========================================${NC}"

./fork_ls

sleep 1

# SHOW OUTPUT FILE
echo
echo -e "${CYAN}[3] Raw output.txt content${NC}"

echo -e "${WHITE}----------------------------------------${NC}"

cat output.txt

echo -e "${WHITE}----------------------------------------${NC}"

sleep 1

# FINISH
echo
echo -e "${GREEN}"
echo "╔══════════════════════════════════════════════╗"
echo "║                 FINISHED                    ║"
echo "║         fork() demo completed OK            ║"
echo "╚══════════════════════════════════════════════╝"
echo -e "${NC}"

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

TARGET_USER="testuser"

echo -e "${CYAN}"
echo "╔══════════════════════════════════════════════╗"
echo "║          USER SWITCHING DEMO                ║"
echo "║        Linux Permission Management          ║"
echo "╚══════════════════════════════════════════════╝"
echo -e "${NC}"

sleep 1

# =========================
# CHECK USER
# =========================
echo -e "${YELLOW}[1] Checking target user...${NC}"

id $TARGET_USER > /dev/null 2>&1

if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] User '$TARGET_USER' does not exist.${NC}"
    echo -e "${YELLOW}Create user with:${NC}"
    echo "sudo useradd -m $TARGET_USER"
    exit 1
fi

echo -e "${GREEN}[OK] User exists.${NC}"

sleep 1

# =========================
# CLEAN
# =========================
echo
echo -e "${YELLOW}[2] Cleaning old files...${NC}"

rm -f switch_user
sudo rm -f /home/$TARGET_USER/userB_file.txt

echo -e "${GREEN}[OK] Cleanup completed.${NC}"

sleep 1

# =========================
# BUILD
# =========================
echo
echo -e "${BLUE}[3] Building source code...${NC}"

gcc switch_user.c -o switch_user

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
echo -e "${MAGENTA} Running switch_user program${NC}"
echo -e "${MAGENTA}========================================${NC}"

sudo ./switch_user

sleep 1

# =========================
# CHECK FILE OWNER
# =========================
echo
echo -e "${CYAN}[4] Checking file ownership${NC}"

echo -e "${WHITE}----------------------------------------${NC}"

ls -l /home/$TARGET_USER/userB_file.txt

echo -e "${WHITE}----------------------------------------${NC}"

sleep 1

# =========================
# SHOW FILE CONTENT
# =========================
echo
echo -e "${CYAN}[5] File content${NC}"

echo -e "${WHITE}----------------------------------------${NC}"

cat /home/$TARGET_USER/userB_file.txt

echo -e "${WHITE}----------------------------------------${NC}"

sleep 1

# =========================
# SHOW CURRENT USER
# =========================
echo
echo -e "${CYAN}[6] Current user info${NC}"

id

sleep 1

# =========================
# FINISH
# =========================
echo
echo -e "${GREEN}"
echo "╔══════════════════════════════════════════════╗"
echo "║                 FINISHED                    ║"
echo "║         User switching demo success         ║"
echo "╚══════════════════════════════════════════════╝"
echo -e "${NC}"

#!/bin/bash

clear

RED='\033[1;31m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
CYAN='\033[1;36m'
NC='\033[0m'

echo -e "${CYAN}"
echo "╔══════════════════════════════════════════════╗"
echo "║         NETWORK CONFIGURATION DEMO          ║"
echo "║         system() + Linux Networking         ║"
echo "╚══════════════════════════════════════════════╝"
echo -e "${NC}"

sleep 1

echo -e "${YELLOW}[1] Cleaning old build...${NC}"

rm -f network_config

sleep 1

echo -e "${BLUE}[2] Building source code...${NC}"

gcc network_config.c -o network_config

if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] Compile failed!${NC}"
    exit 1
fi

echo -e "${GREEN}[OK] Build success.${NC}"

sleep 1

echo
echo -e "${CYAN}[3] Current interfaces${NC}"

ip a

sleep 1

echo
echo -e "${YELLOW}IMPORTANT:${NC}"
echo "Edit interface name in network_config.c"
echo "Example: eth0 / ens33 / wlan0"

sleep 1

echo
echo -e "${BLUE}[4] Running program...${NC}"

sudo ./network_config

echo
echo -e "${GREEN}[DONE] Network demo completed.${NC}"

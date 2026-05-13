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
echo "║            HocLinux_sh DEMO                 ║"
echo "║          Custom Shell Interpreter           ║"
echo "╚══════════════════════════════════════════════╝"
echo -e "${NC}"

sleep 1

# =========================
# CLEAN
# =========================
echo -e "${YELLOW}[1] Cleaning old files...${NC}"

rm -f HocLinux_sh
rm -f hello.sh

echo -e "${GREEN}[OK] Cleanup completed.${NC}"

sleep 1

# =========================
# CREATE SCRIPT
# =========================
echo
echo -e "${BLUE}[2] Creating hello.sh ...${NC}"

cat > hello.sh << 'EOF'
#!/bin/bash

echo "Hello world I am $1"
EOF

chmod +x hello.sh

echo -e "${GREEN}[OK] hello.sh created.${NC}"

sleep 1

# =========================
# BUILD PROGRAM
# =========================
echo
echo -e "${BLUE}[3] Building HocLinux_sh.c ...${NC}"

gcc HocLinux_sh.c -o HocLinux_sh

if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] Compile failed!${NC}"
    exit 1
fi

echo -e "${GREEN}[OK] Build success.${NC}"

sleep 1

# =========================
# SHOW SCRIPT
# =========================
echo
echo -e "${CYAN}[4] Script content${NC}"

echo -e "${WHITE}----------------------------------------${NC}"

cat hello.sh

echo -e "${WHITE}----------------------------------------${NC}"

sleep 1

# =========================
# RUN INTERPRETER
# =========================
echo
echo -e "${MAGENTA}========================================${NC}"
echo -e "${MAGENTA} Running HocLinux_sh${NC}"
echo -e "${MAGENTA}========================================${NC}"

./HocLinux_sh hello.sh CodeWalker

sleep 1

# =========================
# FINISH
# =========================
echo
echo -e "${GREEN}"
echo "╔══════════════════════════════════════════════╗"
echo "║                 FINISHED                    ║"
echo "║      HocLinux_sh demo completed OK          ║"
echo "╚══════════════════════════════════════════════╝"
echo -e "${NC}"

# ====================================================
# MAKEFILE FOR BAI 6.5 - CHAT APP PACKAGING
# Biên dịch thư viện chia sẻ, ứng dụng chat, và đóng gói
# ====================================================

CC = gcc
CFLAGS = -Wall -Wextra

# Các tệp đích
LIB_TARGET = libchat.so
SERVER_TARGET = chat-server
CLIENT_TARGET = chat-client
SETUP_TARGET = setup
UNINSTALL_TARGET = uninstall

all: lib server client installer package

# 1. Biên dịch thư viện chia sẻ (Shared Library)
lib: src/chat_lib.c src/chat_lib.h
	$(CC) $(CFLAGS) -fPIC -shared -o $(LIB_TARGET) src/chat_lib.c

# 2. Biên dịch Server liên kết với libchat.so
# Sử dụng rpath để chỉ đường dẫn tìm thư viện động lúc chạy:
# - rpath=/usr/local/lib: Vị trí cài đặt chính thức sau khi setup
# - rpath=.: Vị trí thư mục hiện tại khi test nội bộ
server: src/server.c lib
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) src/server.c -L. -lchat -lpthread -lrt -Wl,-rpath,/usr/local/lib -Wl,-rpath,.

# 3. Biên dịch Client liên kết với libchat.so
client: src/client.c lib
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) src/client.c -L. -lchat -lpthread -lrt -Wl,-rpath,/usr/local/lib -Wl,-rpath,.

# 4. Biên dịch chương trình setup và uninstall
installer: setup.c uninstall.c
	$(CC) $(CFLAGS) -o $(SETUP_TARGET) setup.c
	$(CC) $(CFLAGS) -o $(UNINSTALL_TARGET) uninstall.c

# 5. Đóng gói bộ cài đặt (Packaging) thành cấu trúc tệp tin sạch sẽ
package:
	@echo "Đang tạo gói cài đặt trong thư mục packet/..."
	mkdir -p packet/bin
	mkdir -p packet/lib
	mkdir -p packet/config
	cp $(SERVER_TARGET) $(CLIENT_TARGET) packet/bin/
	cp $(LIB_TARGET) packet/lib/
	cp config/chat.conf packet/config/
	@echo "✓ Tạo gói packet/ thành công!"

# 6. Dọn dẹp các tệp tin biên dịch
clean:
	@echo "Đang dọn dẹp các file build..."
	rm -rf $(LIB_TARGET) $(SERVER_TARGET) $(CLIENT_TARGET) $(SETUP_TARGET) $(UNINSTALL_TARGET) packet/
	@echo "✓ Đã dọn dẹp xong."

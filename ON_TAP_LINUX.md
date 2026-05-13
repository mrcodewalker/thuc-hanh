# 📚 ÔN TẬP LẬP TRÌNH NHÂN LINUX — PROCESS MANAGEMENT

> Tổng hợp từ các bài thực hành Chương 1

---

## 🔑 KHÁI NIỆM CỐT LÕI

### Process là gì?
- Một **process** là một chương trình **đang chạy** trong hệ thống
- Mỗi process có:
  - **PID** (Process ID): số định danh duy nhất
  - **PPID** (Parent PID): PID của process cha
  - **UID/GID**: user/group sở hữu process
  - Không gian bộ nhớ riêng (code, stack, heap, data)

### Fork là gì?
- `fork()` tạo ra một **bản sao** của process hiện tại
- Sau khi fork:
  - Process **cha** nhận về **PID của con** (> 0)
  - Process **con** nhận về **0**
  - Nếu **lỗi** trả về **-1**
- Hai process chạy **song song**, độc lập nhau về bộ nhớ

### Exec là gì?
- `execl()` / `execv()` **thay thế toàn bộ** image của process hiện tại bằng chương trình mới
- Thường dùng trong process con sau `fork()`
- Nếu thành công → **không bao giờ return**

### Zombie & Orphan
| Loại | Giải thích |
|---|---|
| **Zombie** | Con đã chết nhưng cha chưa gọi `wait()` → entry vẫn còn trong process table |
| **Orphan** | Cha chết trước con → con được `init` (PID 1) nhận nuôi |

---

## 📦 BÀI 1 — Shared Library

**Yêu cầu:** Viết thư viện, build thành `.so`, copy vào `/usr/lib`, dùng trong chương trình C.

### Các bước build:

```bash
# 1. Compile thành object file — flag -fPIC bắt buộc
gcc -fPIC -c mylib.c

# 2. Build thành shared library (.so)
gcc -shared -o libmylib.so mylib.o

# 3. Copy vào /usr/lib để hệ thống tìm thấy
sudo cp libmylib.so /usr/lib/

# 4. Cập nhật cache của dynamic linker
sudo ldconfig

# 5. Compile chương trình chính, link với thư viện
gcc main.c -L/usr/lib -lmylib -o app

# 6. Kiểm tra shared lib nào được dùng
ldd app | grep mylib
```

### Code mẫu:

```c
// mylib.h
#ifndef MYLIB_H
#define MYLIB_H
void hello();
#endif

// mylib.c
#include <stdio.h>
#include "mylib.h"
void hello() {
    printf("Hello from shared library!\n");
}

// main.c
#include "mylib.h"
int main() {
    hello();
    return 0;
}
```

### Khái niệm cần nhớ:
| Khái niệm | Giải thích |
|---|---|
| `-fPIC` | Position Independent Code — code load được ở bất kỳ địa chỉ RAM nào |
| `-shared` | Tạo file `.so` thay vì executable |
| `ldconfig` | Cập nhật cache `/etc/ld.so.cache` để linker tìm được lib |
| `ldd` | List dynamic dependencies của executable |
| `-lmylib` | Link với `libmylib.so` (bỏ prefix `lib` và suffix `.so`) |

---

## 📦 BÀI 1.2 — Mở File, Return Exit Code

**Yêu cầu:** Mở file, return 0 nếu thành công, -1 nếu lỗi. Đọc kết quả bằng `$?`.

### Code mẫu:

```c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int fd;
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

    fd = open(argv[1], O_RDONLY);  // Mở file chỉ đọc

    if (fd < 0) {
        printf("Open file failed!\n");
        return -1;                 // Exit code lỗi
    }

    printf("Open file success!\n");
    close(fd);
    return 0;                      // Exit code thành công
}
```

### Chạy và đọc kết quả:

```bash
./openfile test.txt
echo $?          # 0 = thành công

./openfile abc.txt
echo $?          # 255 = lỗi (return -1 → unsigned = 255)
```

### Khái niệm cần nhớ:
| Khái niệm | Giải thích |
|---|---|
| `open()` | System call mở file, trả về file descriptor (int ≥ 0) hoặc -1 nếu lỗi |
| `O_RDONLY` | Flag mở file chỉ đọc |
| `close(fd)` | Giải phóng file descriptor |
| `$?` | Biến shell chứa exit code của lệnh/chương trình vừa chạy |

---

## 📦 BÀI 2 — Xử lý tham số dòng lệnh (giống `ls`)

**Yêu cầu:** Xử lý `-l`, `-a`, `-la` như lệnh `ls`.

### Code mẫu:

```c
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [-l | -a | -la]\n", argv[0]);
        return -1;
    }

    if (strcmp(argv[1], "-l") == 0)
        printf("Display full property\n");

    else if (strcmp(argv[1], "-a") == 0)
        printf("Display hidden files\n");

    else if (strcmp(argv[1], "-la") == 0 || strcmp(argv[1], "-al") == 0) {
        printf("Display full property\n");
        printf("Display hidden files\n");
    }
    else {
        printf("Invalid option!\n");
        return -1;
    }
    return 0;
}
```

### Chạy:

```bash
./my_ls -l     # Display full property
./my_ls -a     # Display hidden files
./my_ls -la    # Cả 2
./my_ls -x     # Invalid option
```

### Khái niệm cần nhớ:
| Khái niệm | Giải thích |
|---|---|
| `argc` | Argument count — số lượng tham số (bao gồm tên chương trình) |
| `argv` | Argument vector — mảng chuỗi chứa các tham số |
| `argv[0]` | Tên chương trình |
| `argv[1]` | Tham số đầu tiên người dùng truyền vào |
| `strcmp()` | So sánh 2 chuỗi, trả về 0 nếu giống nhau |

---

## 📦 BÀI 2.2 — Environment Variables

**Yêu cầu:** Đọc biến môi trường HOME, USER, đường dẫn hiện tại.

### Code mẫu:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    char cwd[1024];

    char *home = getenv("HOME");   // Lấy biến HOME
    char *user = getenv("USER");   // Lấy biến USER
    getcwd(cwd, sizeof(cwd));      // Lấy thư mục hiện tại

    printf("HOME Directory : %s\n", home);
    printf("User Name      : %s\n", user);
    printf("Current Path   : %s\n", cwd);

    return 0;
}
```

### Khái niệm cần nhớ:
| Khái niệm | Giải thích |
|---|---|
| Environment variable | Biến toàn cục của shell, process con kế thừa từ cha |
| `getenv()` | Đọc biến môi trường, trả về NULL nếu không tồn tại |
| `getcwd()` | Lấy đường dẫn thư mục hiện tại, tương đương lệnh `pwd` |

---

## 📦 BÀI 3 — Fork Demo: PID, File Descriptor, Biến, Kill

**Yêu cầu:** Fork cha/con, in PID, cùng ghi vào file fd, thử biến thông thường, dùng `kill`.

### Code mẫu:

```c
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

int global_var = 100;

int main() {
    int fd;
    pid_t pid;

    // Mở file TRƯỚC khi fork → cha và con dùng chung fd
    fd = open("fork_output.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);

    pid = fork();

    if (pid == 0) {
        // CON
        global_var += 50;   // Chỉ thay đổi bản sao của con
        printf("[CHILD] PID: %d, PPID: %d\n", getpid(), getppid());
        printf("[CHILD] global_var: %d\n", global_var);  // 150
        write(fd, "Child writes to file\n", 21);
        sleep(30);
    } else {
        // CHA
        global_var += 10;   // Chỉ thay đổi bản sao của cha
        printf("[PARENT] PID: %d, Child PID: %d\n", getpid(), pid);
        printf("[PARENT] global_var: %d\n", global_var);  // 110
        write(fd, "Parent writes to file\n", 22);
        sleep(30);
    }

    close(fd);
    return 0;
}
```

### Lệnh shell quan trọng:

```bash
./fork_demo &              # Chạy nền
ps -ef | grep fork_demo    # Xem process đang chạy
pgrep -P <PPID>            # Tìm PID con theo PID cha
kill <PID>                 # Gửi SIGTERM để kill process
pkill fork_demo            # Kill tất cả process tên fork_demo
```

### Kết quả quan sát quan trọng:

| Thí nghiệm | Kết quả |
|---|---|
| **File descriptor** mở trước fork | Cha và con **dùng chung** fd, cùng offset → cả hai ghi được vào file |
| **Biến thông thường** (`global_var`) | Sau fork, cha và con có **bản sao riêng** → thay đổi không ảnh hưởng nhau |
| **Kill con** | Cha vẫn sống bình thường |
| **Kill cha** | Con thành **orphan**, được `init` (PID 1) nhận nuôi |

### Khái niệm cần nhớ:
| Khái niệm | Giải thích |
|---|---|
| `fork()` | Tạo process con là bản sao của cha |
| `getpid()` | Lấy PID của process hiện tại |
| `getppid()` | Lấy PID của process cha |
| Copy-on-write | Bộ nhớ chỉ thực sự copy khi một bên ghi vào |
| File descriptor sharing | fd mở trước fork được chia sẻ, cùng offset |
| `kill` | Gửi signal đến process (mặc định SIGTERM = 15) |
| `ps -ef` | Hiển thị toàn bộ process đang chạy |

---

## 📦 BÀI 4 — Cha gọi Con bằng execl(), Con ghi file, Cha nhận kết quả

**Yêu cầu:** Cha fork → con dùng `execl()` chạy chương trình `child`, con ghi "Hello World" vào file, cha dùng `wait()` nhận exit code.

### Code mẫu — parent.c:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    int status;

    pid = fork();

    if (pid == 0) {
        // CON: thay thế bằng chương trình child
        execl("./child", "child", "output.txt", NULL);
        perror("execl");   // Chỉ chạy nếu execl thất bại
        exit(-1);
    } else {
        // CHA: chờ con kết thúc
        wait(&status);

        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            printf("[PARENT] Child exit code: %d\n", code);
            if (code == 0)
                printf("[PARENT] Child success.\n");
            else
                printf("[PARENT] Child failed.\n");
        }
    }
    return 0;
}
```

### Code mẫu — child.c:

```c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) return -1;

    int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { perror("open"); return -1; }

    write(fd, "Hello World\n", 12);
    close(fd);

    printf("[CHILD] Write file success.\n");
    return 0;
}
```

### Khái niệm cần nhớ:
| Khái niệm | Giải thích |
|---|---|
| `execl(path, arg0, arg1, ..., NULL)` | arg0 là tên chương trình (argv[0]), kết thúc bằng NULL |
| `wait(&status)` | Cha chờ con kết thúc, nhận status |
| `WIFEXITED(status)` | Kiểm tra con thoát bình thường (không bị signal) |
| `WEXITSTATUS(status)` | Lấy exit code thực sự của con |
| `O_WRONLY \| O_CREAT \| O_TRUNC` | Mở file ghi, tạo mới nếu chưa có, xóa nội dung cũ |

---

## 📦 BÀI 5 — Chuyển User (setuid/setgid)

**Yêu cầu:** Chạy bằng user A, trong chương trình chuyển sang user B, tạo file thuộc sở hữu user B.

### Code mẫu:

```c
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>

int main() {
    struct passwd *pw;

    printf("Before - UID: %d, GID: %d\n", getuid(), getgid());

    // Lấy thông tin user B từ /etc/passwd
    pw = getpwnam("testuser");
    if (pw == NULL) { perror("getpwnam"); return -1; }

    // PHẢI đổi GID trước, vì sau setuid mất quyền root
    if (setgid(pw->pw_gid) != 0) { perror("setgid"); return -1; }
    if (setuid(pw->pw_uid) != 0) { perror("setuid"); return -1; }

    printf("After  - UID: %d, GID: %d\n", getuid(), getgid());

    // Tạo file — file sẽ thuộc sở hữu testuser
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "/home/%s/userB_file.txt", pw->pw_name);

    int fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { perror("open"); return -1; }

    write(fd, "This file belongs to user B\n", 29);
    close(fd);

    printf("[OK] File created: %s\n", filepath);
    return 0;
}
```

### Chạy:

```bash
sudo ./switch_user                          # Cần sudo vì setuid cần quyền root
ls -l /home/testuser/userB_file.txt         # Kiểm tra owner
```

### Khái niệm cần nhớ:
| Khái niệm | Giải thích |
|---|---|
| UID/GID | User ID / Group ID — định danh user trong Linux |
| `setuid()` | Thay đổi UID của process (cần quyền root) |
| `setgid()` | Thay đổi GID — **phải set trước setuid** |
| `getpwnam()` | Tra cứu thông tin user trong `/etc/passwd` |
| `struct passwd` | Chứa: pw_uid, pw_gid, pw_name, pw_dir... |
| File ownership | File tạo ra thuộc sở hữu UID/GID hiện tại của process |

---

## 📦 BÀI 6 — Fork + execl() chạy Shell Script

**Yêu cầu:** Chương trình C fork ra con, con dùng `execl()` chạy bash script, cha nhận exit code.

### Code mẫu:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    pid_t pid;
    int status;

    if (argc < 2) {
        printf("Usage: %s <script.sh> [argument]\n", argv[0]);
        return -1;
    }

    pid = fork();

    if (pid == 0) {
        // CON: chạy bash script
        if (argc == 3)
            execl("/bin/bash", "bash", argv[1], argv[2], NULL);
        else
            execl("/bin/bash", "bash", argv[1], NULL);

        perror("execl");
        exit(-1);
    } else {
        // CHA: chờ và nhận kết quả
        wait(&status);
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            printf("[PARENT] Exit code: %d\n", code);
        }
    }
    return 0;
}
```

### hello.sh:

```bash
#!/bin/bash
echo "Hello world I am $1"
```

### Chạy:

```bash
./HocLinux_sh hello.sh CodeWalker
# Output: Hello world I am CodeWalker
```

### Khái niệm cần nhớ:
| Khái niệm | Giải thích |
|---|---|
| `execl("/bin/bash", "bash", script, arg, NULL)` | Chạy bash với script và tham số |
| `$1` trong shell script | Tham số đầu tiên truyền vào script |
| `chmod +x` | Cấp quyền thực thi cho script |

---

## 📦 BÀI 7 — system() để cấu hình mạng

**Yêu cầu:** Dùng `system()` để up/down interface và set IP.

### Code mẫu:

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Down interface
    system("sudo ip link set eth0 down");

    // Set IP
    system("sudo ip addr add 192.168.1.100/24 dev eth0");

    // Up interface
    system("sudo ip link set eth0 up");

    // Hiển thị kết quả
    system("ip addr show eth0");

    return 0;
}
```

### Khái niệm cần nhớ:
| Khái niệm | Giải thích |
|---|---|
| `system(cmd)` | Chạy lệnh shell từ C, tương đương fork + exec + wait |
| `ip link set <if> up/down` | Bật/tắt network interface |
| `ip addr add <ip/mask> dev <if>` | Gán địa chỉ IP cho interface |
| `ip addr show` | Hiển thị thông tin IP |
| **Nhược điểm system()** | Tạo thêm shell process trung gian `/bin/sh -c`, kém hiệu quả hơn fork+exec |

---

## 📦 BÀI 7.2 — Fork + Con chạy `ls`, Cha đọc file kết quả

**Yêu cầu:** Con dùng `system("ls -l > output.txt")`, cha `wait()` rồi đọc file in ra màn hình.

### Code mẫu:

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    int status;
    FILE *fp;
    char buffer[256];

    pid = fork();

    if (pid == 0) {
        // CON: chạy ls và ghi vào file
        int ret = system("ls -l > output.txt");
        if (ret != 0) {
            printf("[CHILD] ls failed.\n");
            exit(-1);
        }
        printf("[CHILD] Output written to output.txt\n");
        exit(0);
    } else {
        // CHA: chờ con xong rồi đọc file
        wait(&status);

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            fp = fopen("output.txt", "r");
            if (fp == NULL) { perror("fopen"); return -1; }

            while (fgets(buffer, sizeof(buffer), fp) != NULL)
                printf("%s", buffer);

            fclose(fp);
        } else {
            printf("[PARENT] Child failed.\n");
        }
    }
    return 0;
}
```

### Khái niệm cần nhớ:
| Khái niệm | Giải thích |
|---|---|
| `>` (redirect) | Chuyển stdout của lệnh vào file |
| `fopen()` | Mở file theo kiểu C standard, trả về FILE* |
| `fgets()` | Đọc từng dòng từ file |
| `fclose()` | Đóng FILE* |
| `exit(0)` | Kết thúc process con với exit code 0 |

---

## 🧠 BẢNG TÓM TẮT HÀM QUAN TRỌNG

| Hàm | Header | Chức năng |
|---|---|---|
| `fork()` | `unistd.h` | Tạo process con |
| `getpid()` | `unistd.h` | Lấy PID hiện tại |
| `getppid()` | `unistd.h` | Lấy PID cha |
| `execl()` | `unistd.h` | Thay thế process bằng chương trình mới |
| `wait()` | `sys/wait.h` | Chờ process con kết thúc |
| `WIFEXITED()` | `sys/wait.h` | Kiểm tra con thoát bình thường |
| `WEXITSTATUS()` | `sys/wait.h` | Lấy exit code của con |
| `system()` | `stdlib.h` | Chạy lệnh shell |
| `open()` | `fcntl.h` | Mở file (system call), trả về fd |
| `close()` | `unistd.h` | Đóng file descriptor |
| `write()` | `unistd.h` | Ghi vào file descriptor |
| `read()` | `unistd.h` | Đọc từ file descriptor |
| `fopen()` | `stdio.h` | Mở file (C library), trả về FILE* |
| `fgets()` | `stdio.h` | Đọc từng dòng từ FILE* |
| `fclose()` | `stdio.h` | Đóng FILE* |
| `getenv()` | `stdlib.h` | Đọc biến môi trường |
| `getcwd()` | `unistd.h` | Lấy thư mục hiện tại |
| `setuid()` | `unistd.h` | Đổi UID của process |
| `setgid()` | `unistd.h` | Đổi GID của process |
| `getuid()` | `unistd.h` | Lấy UID hiện tại |
| `getgid()` | `unistd.h` | Lấy GID hiện tại |
| `getpwnam()` | `pwd.h` | Lấy thông tin user từ /etc/passwd |

---

## ⚡ CÂU HỎI HAY BỊ HỎI

**Q: Sau fork(), ai chạy trước — cha hay con?**
> Không xác định, do scheduler của kernel quyết định.

**Q: Tại sao phải setgid trước setuid?**
> Vì sau khi setuid sang user thường, process mất quyền root và không thể setgid nữa.

**Q: Khác nhau giữa `open()` và `fopen()`?**
> `open()` là system call trả về file descriptor (int). `fopen()` là C library trả về FILE* (có buffer). `fopen` dùng `open` bên dưới.

**Q: Khác nhau giữa `execl()` và `system()`?**
> `execl()` thay thế process hiện tại, không return. `system()` tạo thêm shell process trung gian (`/bin/sh -c`), cha vẫn tiếp tục sau khi lệnh xong.

**Q: File descriptor có được kế thừa qua fork không?**
> Có. fd mở trước fork được chia sẻ giữa cha và con, cùng offset — nên cả hai ghi vào cùng vị trí trong file.

**Q: Biến thông thường có được chia sẻ qua fork không?**
> Không. Sau fork, cha và con có bản sao riêng. Thay đổi ở con không ảnh hưởng cha và ngược lại (copy-on-write).

**Q: Zombie process là gì và cách tránh?**
> Con đã chết nhưng cha chưa gọi `wait()`. Tránh bằng cách luôn gọi `wait()` hoặc `waitpid()` trong cha.

**Q: `$?` trong shell là gì?**
> Biến chứa exit code của lệnh/chương trình vừa chạy. 0 = thành công, khác 0 = lỗi.

**Q: Tại sao `return -1` trong main() lại thành 255 khi đọc `$?`?**
> Vì exit code là unsigned 8-bit (0–255). `-1` theo unsigned = 255.

---

## 🔧 LỆNH SHELL THƯỜNG DÙNG

```bash
# Compile
gcc file.c -o output
gcc -fPIC -c mylib.c
gcc -shared -o libmylib.so mylib.o
gcc main.c -L/usr/lib -lmylib -o app

# Process
ps -ef                    # Xem tất cả process
ps -ef | grep <tên>       # Tìm process theo tên
pgrep -P <PPID>           # Tìm PID con theo PID cha
kill <PID>                # Gửi SIGTERM
kill -9 <PID>             # Gửi SIGKILL (buộc kill)
pkill <tên>               # Kill theo tên

# File
ldd <executable>          # Xem shared lib dependencies
ls -l <file>              # Xem quyền và owner của file
cat <file>                # Đọc nội dung file

# User
id                        # Xem UID, GID hiện tại
sudo useradd -m testuser  # Tạo user mới

# Network
ip a                      # Xem tất cả interface
ip link set eth0 up/down  # Bật/tắt interface
ip addr add <ip/mask> dev <if>  # Gán IP
```

---

*Chúc học tốt và trả lời bài tốt! 🚀*

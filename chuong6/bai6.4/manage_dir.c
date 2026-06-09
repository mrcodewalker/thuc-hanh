/*
 * Bai 6.4: Tao thu muc cau truc va loc ra cac file co che do Read-only.
 * Su dung mkdir, open, chmod de tao cau truc.
 * Su dung opendir, readdir, stat de quet de quy va loc file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define ROOT_DIR "test_structure"
#define SUB_DIR "test_structure/sub_folder"

/* Ham phu tao file va thiet lap quyen chmod chinh xac */
void create_file(const char *path, const char *content, mode_t mode) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("[ERROR] open file");
        return;
    }
    if (content != NULL) {
        write(fd, content, strlen(content));
    }
    close(fd);

    // Dung chmod de thiet lap quyen chinh xac, bo qua umask cua he thong
    if (chmod(path, mode) == -1) {
        perror("[ERROR] chmod");
    }
}

/* Giai doan 1: Tao cau truc thu muc va file nhu thiet ke */
void create_directory_structure() {
    printf("[1] Dang khoi tao cau truc thu muc...\n");

    // Tao thu muc goc
    if (mkdir(ROOT_DIR, 0755) == -1) {
        // Neu da ton tai thi thong bao, khong sao ca
        printf("    (Thu muc '%s' da co san)\n", ROOT_DIR);
    } else {
        printf("    ✓ Tao thu muc goc '%s' thanh cong.\n", ROOT_DIR);
    }

    // Tao cac file o thu muc goc voi quyen khac nhau
    printf("    -> Dang tao file_read_write.txt (Mode: 0644 - Read/Write for owner)...\n");
    create_file(ROOT_DIR "/file_read_write.txt", 
                "Day la file co quyen Doc va Ghi cho Owner (0644).\n", 
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    printf("    -> Dang tao file_read_only.txt (Mode: 0444 - Read-only for all)...\n");
    create_file(ROOT_DIR "/file_read_only.txt", 
                "Day la file chi doc (Read-only) cho tat ca moi nguoi (0444).\n", 
                S_IRUSR | S_IRGRP | S_IROTH);

    printf("    -> Dang tao file_write_only.txt (Mode: 0222 - Write-only for all)...\n");
    create_file(ROOT_DIR "/file_write_only.txt", 
                "Day la file chi ghi (Write-only) cho tat ca moi nguoi (0222).\n", 
                S_IWUSR | S_IWGRP | S_IWOTH);

    // Tao thu muc con
    if (mkdir(SUB_DIR, 0755) == -1) {
        printf("    (Thu muc con '%s' da co san)\n", SUB_DIR);
    } else {
        printf("    ✓ Tao thu muc con '%s' thanh cong.\n", SUB_DIR);
    }

    // Tao cac file trong thu muc con
    printf("    -> Dang tao file_ro_owner.txt (Mode: 0400 - Read-only for Owner)...\n");
    create_file(SUB_DIR "/file_ro_owner.txt", 
                "Day la file chi doc cho rieng chu so huu (0400).\n", 
                S_IRUSR);

    printf("    -> Dang tao file_executable.sh (Mode: 0755 - Read/Write/Execute)...\n");
    create_file(SUB_DIR "/file_executable.sh", 
                "#!/bin/bash\necho \"Hello from executable file!\"\n", 
                S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

    printf("✓ Hoan tat tao cau truc thu muc.\n\n");
}

/* Giai doan 2: Quet de quy va hien thi cac file Read-only */
void scan_for_readonly_files(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    struct dirent *entry;
    struct stat st;
    char full_path[1024];

    if (dir == NULL) {
        perror("[ERROR] opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Bo qua thu muc dac biet . va ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Ghep duong dan day du
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (stat(full_path, &st) == -1) {
            continue;
        }

        // Neu la thu muc con thi goi de quy quet tiep
        if (S_ISDIR(st.st_mode)) {
            scan_for_readonly_files(full_path);
        }
        // Neu la file thuong thi kiem tra quyen truy cap
        else if (S_ISREG(st.st_mode)) {
            // Dieu kien cua File Read-Only:
            // 1. Co it nhat mot quyen doc duoc mo (Owner/Group/Others)
            int is_read_enabled = ((st.st_mode & (S_IRUSR | S_IRGRP | S_IROTH)) != 0);
            
            // 2. KHONG co quyen ghi (write) nao duoc bat o bat ky nhom nao
            int is_write_disabled = ((st.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0);

            if (is_read_enabled && is_write_disabled) {
                // In thong tin file tim thay duoi dang dep mat
                printf("│ %-45.45s │   %04o    │ %8ld bytes │\n", 
                       full_path, (unsigned int)(st.st_mode & 0777), (long)st.st_size);
            }
        }
    }

    closedir(dir);
}

int main() {
    printf("╔══════════════════════════════════════════════╗\n");
    printf("║    BAI 6.4 - CREATE DIRECTORY & RO SCANNER   ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // 1. Tao cau truc file nhu yeu cau
    create_directory_structure();

    // 2. Quet va hien thi file Read-only
    printf("[2] Dang quet de quy va tim kiem cac file co che do Read-only...\n");
    printf("┌──────────────────────────────────────────────┬──────────┬────────────────┐\n");
    printf("│ Duong Dan File Read-Only                     │ Quyen    │ Kich Thuoc     │\n");
    printf("├──────────────────────────────────────────────┼──────────┼────────────────┤\n");

    scan_for_readonly_files(ROOT_DIR);

    printf("└──────────────────────────────────────────────┴──────────┴────────────────┘\n");
    printf("[OK] Hoan thanh chuong trinh Bai 6.4.\n");

    return 0;
}

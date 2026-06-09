/*
 * Bai 6.2: Liet ke thong tin cac file trong 1 folder co duong dan co dinh.
 * Dung opendir, readdir, closedir va stat de lay ten, size, modify time.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

// Duong dan thu muc co dinh trong source code
#define FIXED_DIR_PATH "."

int main(int argc, char *argv[]) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[1024];
    const char *dir_path = FIXED_DIR_PATH;

    printf("╔══════════════════════════════════════════════╗\n");
    printf("║    BAI 6.2 - DIRECTORY LISTING & FILE STATS  ║\n");
    printf("╚══════════════════════════════════════════════╝\n\n");

    // Neu nguoi dung truyen tham so tu dong lenh, cho phep ghi de duong dan co dinh de tang tinh linh hoat
    if (argc >= 2) {
        dir_path = argv[1];
    }

    printf("[i] Dang quet thu muc: %s\n\n", dir_path);

    // 1. Mo thu muc bang opendir()
    dir = opendir(dir_path);
    if (dir == NULL) {
        fprintf(stderr, "[ERROR] Khong the mo thu muc '%s': %s\n", dir_path, strerror(errno));
        return 1;
    }

    // In tieu de bang
    printf("┌──────┬──────────────────────────────┬──────────────┬─────────────────────┐\n");
    printf("│ Loai │ Ten File                     │ Size (Bytes) │ Sua Doi Lan Cuoi    │\n");
    printf("├──────┼──────────────────────────────┼──────────────┼─────────────────────┤\n");

    // 2. Lap qua tung entry bang readdir()
    while ((entry = readdir(dir)) != NULL) {
        // Ghep duong dan day du de goi stat()
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        // 3. Lay thong tin chi tiet bang stat()
        if (stat(full_path, &file_stat) == -1) {
            // Neu loi stat, bỏ qua file do va tiep tuc
            continue;
        }

        // Phan loai kieu file
        char file_type[8];
        if (S_ISDIR(file_stat.st_mode)) {
            strcpy(file_type, "FOLDER");
        } else if (S_ISREG(file_stat.st_mode)) {
            strcpy(file_type, "FILE");
        } else if (S_ISLNK(file_stat.st_mode)) {
            strcpy(file_type, "LINK");
        } else {
            strcpy(file_type, "OTHER");
        }

        // Dinh dang thoi gian sua doi (Modify Time)
        struct tm *tm_info = localtime(&file_stat.st_mtime);
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

        // In thong tin file (cat bot ten file neu ten file qua dai de giu vung layout bang)
        char temp_name[31];
        if (strlen(entry->d_name) > 28) {
            strncpy(temp_name, entry->d_name, 25);
            temp_name[25] = '\0';
            strcat(temp_name, "...");
        } else {
            strcpy(temp_name, entry->d_name);
        }

        printf("│ %-4.4s │ %-28.28s │ %12ld │ %s │\n", 
               file_type, temp_name, (long)file_stat.st_size, time_str);
    }

    printf("└──────┴──────────────────────────────┴──────────────┴─────────────────────┘\n");

    // 4. Dong thu muc bang closedir()
    closedir(dir);

    printf("\n[OK] Hoan thanh quet thu muc.\n");
    return 0;
}

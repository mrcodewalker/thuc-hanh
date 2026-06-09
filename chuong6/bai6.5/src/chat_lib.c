#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "chat_lib.h"

/* Hàm loại bỏ khoảng trắng thừa đầu và cuối chuỗi */
static void trim(char *str) {
    char *end;
    // Trim leading space
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end >= str && (isspace((unsigned char)*end) || *end == '\r' || *end == '\n')) {
        *end = '\0';
        end--;
    }
}

int read_config(const char *config_path, char *shm_name, char *log_path) {
    FILE *file = fopen(config_path, "r");
    if (file == NULL) {
        return -1;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        // Bỏ qua dòng bình luận hoặc dòng trống
        if (line[0] == '#' || line[0] == ';' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        char *eq = strchr(line, '=');
        if (eq == NULL) {
            continue;
        }

        *eq = '\0';
        char *key = line;
        char *val = eq + 1;

        trim(key);
        trim(val);

        if (strcmp(key, "SHM_NAME") == 0) {
            strncpy(shm_name, val, MAX_SHM_LEN - 1);
            shm_name[MAX_SHM_LEN - 1] = '\0';
        } else if (strcmp(key, "LOG_PATH") == 0) {
            strncpy(log_path, val, MAX_PATH_LEN - 1);
            log_path[MAX_PATH_LEN - 1] = '\0';
        }
    }

    fclose(file);
    return 0;
}

void write_log(const char *log_path, const char *sender, const char *message) {
    FILE *log_file = fopen(log_path, "a");
    if (log_file == NULL) {
        // Nếu không mở được log (ví dụ thiếu quyền write ở /var/log khi chạy user), ghi tạm vào /tmp
        log_file = fopen("/tmp/chat_app_fallback.log", "a");
        if (log_file == NULL) return;
    }

    // Lấy thời gian thực
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(log_file, "[%s] [%s]: %s\n", time_str, sender, message);
    fclose(log_file);
}

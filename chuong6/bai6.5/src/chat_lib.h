#ifndef CHAT_LIB_H
#define CHAT_LIB_H

#define MAX_PATH_LEN 256
#define MAX_SHM_LEN 64

/* 
 * Đọc file cấu hình từ config_path.
 * Trả về 0 nếu thành công, -1 nếu thất bại.
 */
int read_config(const char *config_path, char *shm_name, char *log_path);

/* 
 * Ghi log tin nhắn chat kèm theo nhãn thời gian thực.
 */
void write_log(const char *log_path, const char *sender, const char *message);

#endif /* CHAT_LIB_H */

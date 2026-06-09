/*
 * Bai 6.3: Driver nhan Linux - misc-module
 * Dang ky 1 thiet bi misc character device, tu dong tao /dev/misc-module khi load
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "misc-module"
#define BUFFER_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux Programming Student");
MODULE_DESCRIPTION("A simple Linux misc device driver with Read/Write support");
MODULE_VERSION("1.0");

// Bo dem luu tru du lieu trong nhan
static char kernel_buffer[BUFFER_SIZE] = "Xin chao! Day la thong diep mac dinh tu ben trong Driver Kernel.";
static size_t data_size = 64; // Kich thuoc thong diep ban dau

/* Ham mo thiet bi */
static int misc_open(struct inode *inode, struct file *file) {
    pr_info("misc-module: Thiet bi da duoc mo\n");
    return 0;
}

/* Ham dong thiet bi */
static int misc_release(struct inode *inode, struct file *file) {
    pr_info("misc-module: Thiet bi da duoc dong\n");
    return 0;
}

/* Ham doc thiet bi: copy_to_user gui du lieu ve User Space */
static ssize_t misc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    // Neu vi tri hien tai vuot qua kich thuoc du lieu, bao EOF (0)
    if (*ppos >= data_size) {
        return 0;
    }

    // Dieu chinh so luong byte can doc
    if (*ppos + count > data_size) {
        count = data_size - *ppos;
    }

    // Copy du lieu tu kernel buffer sang user space buffer
    if (copy_to_user(buf, kernel_buffer + *ppos, count) != 0) {
        pr_err("misc-module: copy_to_user that bai\n");
        return -EFAULT;
    }

    *ppos += count;
    pr_info("misc-module: Da gui %zu bytes ve User Space (f_pos = %lld)\n", count, *ppos);
    return count;
}

/* Ham ghi vao thiet bi: copy_from_user luu du lieu tu User Space vao Kernel */
static ssize_t misc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    // Gioi han kich thuoc ghi de tranh tran bo dem
    if (count > BUFFER_SIZE - 1) {
        count = BUFFER_SIZE - 1;
    }

    // Copy du lieu tu user space buffer vao kernel buffer
    if (copy_from_user(kernel_buffer, buf, count) != 0) {
        pr_err("misc-module: copy_from_user that bai\n");
        return -EFAULT;
    }

    kernel_buffer[count] = '\0';
    data_size = count;
    *ppos = count; // Cap nhat con tro ghi
    
    pr_info("misc-module: Da nhan %zu bytes tu User Space\n", count);
    return count;
}

/* Ham di chuyen con tro file (lseek) */
static loff_t misc_llseek(struct file *file, loff_t offset, int whence) {
    loff_t new_pos = 0;

    switch (whence) {
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = file->f_pos + offset;
            break;
        case SEEK_END:
            new_pos = data_size + offset;
            break;
        default:
            return -EINVAL;
    }

    if (new_pos < 0 || new_pos > BUFFER_SIZE) {
        return -EINVAL;
    }

    file->f_pos = new_pos;
    pr_info("misc-module: Thuc hien lseek ve vi tri: %lld\n", new_pos);
    return new_pos;
}

/* Gan cac ham callback cho file_operations */
static const struct file_operations misc_fops = {
    .owner = THIS_MODULE,
    .open = misc_open,
    .release = misc_release,
    .read = misc_read,
    .write = misc_write,
    .llseek = misc_llseek,
};

/* Khai bao cau truc misc device */
static struct miscdevice my_misc_device = {
    .minor = MISC_DYNAMIC_MINOR, // Cap phat minor number dong
    .name = DEVICE_NAME,         // Ten cua file trong /dev
    .fops = &misc_fops,
};

/* Ham khoi tao Driver */
static int __init misc_init(void) {
    int error;

    error = misc_register(&my_misc_device);
    if (error) {
        pr_err("misc-module: Dang ky misc device that bai (%d)\n", error);
        return error;
    }

    pr_info("misc-module: Driver da duoc load thanh cong vao nhan.\n");
    return 0;
}

/* Ham go Driver */
static void __exit misc_exit(void) {
    misc_deregister(&my_misc_device);
    pr_info("misc-module: Driver da duoc unload khoi nhan.\n");
}

module_init(misc_init);
module_exit(misc_exit);

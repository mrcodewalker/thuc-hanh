/*
 * Chuong 7 - Bai 7.1: He thong file tuy bien dung Kernel Module (myfs)
 * Tich hop thiet bi misc character device (/dev/myfs_dev)
 * 
 * Dac diem:
 * 1. Tu dong tao file thiet bi /dev/myfs_dev.
 * 2. Dang ky loai he thong file 'myfs'.
 * 3. Cho phep mount /dev/myfs_dev vao mot thu muc bat ky.
 * 4. Thu muc mount tu dong hien thi 2 file:
 *    - readme.txt (read-only): thong tin he thong file.
 *    - device_data.txt (read-write): du lieu chia se voi thiet bi /dev/myfs_dev.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/statfs.h>
#include <linux/mutex.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 18, 0)
#include <linux/fs_context.h>
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linux Programming Student");
MODULE_DESCRIPTION("A custom Linux virtual filesystem (myfs) integrated with a misc device file");
MODULE_VERSION("1.0");

#define DEVICE_NAME "myfs_dev"
#define BUFFER_SIZE 1024

/* Bo dem toan cuc luu tru du lieu duoc chia se */
static char device_buffer[BUFFER_SIZE] = "Xin chao! Day la du lieu ban dau duoc luu trong Kernel Space.";
static size_t device_data_size = 61; // Do dai chuoi ban dau
static DEFINE_MUTEX(myfs_lock);      // Bao ve truy cap dong thoi vao bo dem

/* Khai bao truoc cac cau truc operations */
static const struct file_operations myfs_file_operations;
static const struct file_operations myfs_dir_operations;
static const struct inode_operations myfs_dir_inode_operations;
static const struct super_operations myfs_super_ops;

/* Helper ham tao inode phu hop cho file/directory trong he thong file */
static struct inode *myfs_get_inode(struct super_block *sb, const struct inode *dir, umode_t mode, int ino) {
    struct inode *inode = new_inode(sb);
    if (!inode)
        return NULL;

    inode->i_ino = ino;
    inode->i_mode = mode;
    inode->i_uid = current_fsuid();
    inode->i_gid = current_fsgid();

    /* Thiet lap thoi gian tuong thich cho cac phien ban Kernel */
    {
        struct timespec64 ts = current_time(inode);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
        inode_set_atime_to_ts(inode, ts);
        inode_set_mtime_to_ts(inode, ts);
        inode_set_ctime_to_ts(inode, ts);
#else
        inode->i_atime = ts;
        inode->i_mtime = ts;
        inode->i_ctime = ts;
#endif
    }

    if (S_ISDIR(mode)) {
        inode->i_op = &myfs_dir_inode_operations;
        inode->i_fop = &myfs_dir_operations;
        inc_nlink(inode); /* Thu muc co it nhat 2 link: "." va dentry trong cha */
    } else if (S_ISREG(mode)) {
        inode->i_fop = &myfs_file_operations;
    }

    return inode;
}

/* ====================================================
 * 1. CAC HOAT DONG QUAN LY SUPERBLOCK (FILESYSTEM SUPER_OPERATIONS)
 * ====================================================
 */
static int myfs_statfs(struct dentry *dentry, struct kstatfs *buf) {
    buf->f_type = dentry->d_sb->s_magic;
    buf->f_bsize = dentry->d_sb->s_blocksize;
    buf->f_namelen = NAME_MAX;
    return 0;
}

static const struct super_operations myfs_super_ops = {
    .statfs = myfs_statfs,
};

/* ====================================================
 * 2. CAC HOAT DONG THU MUC (DIRECTORY INODE & FILE OPERATIONS)
 * ====================================================
 */

/* Ham tim kiem file trong thu muc (lookup) */
static struct dentry *myfs_lookup(struct inode *parent, struct dentry *dentry, unsigned int flags) {
    struct inode *inode = NULL;
    const char *name = dentry->d_name.name;

    pr_info("myfs: Truy van tim tệp '%s' trong thu muc mount.\n", name);

    if (strcmp(name, "readme.txt") == 0) {
        inode = myfs_get_inode(parent->i_sb, parent, S_IFREG | 0444, 101); /* Chi doc */
    } else if (strcmp(name, "device_data.txt") == 0) {
        inode = myfs_get_inode(parent->i_sb, parent, S_IFREG | 0666, 102); /* Doc/Ghi */
    }

    /* Neu file hop le hoac NULL (de tao negative dentry), d_splice_alias se tu dong xu ly */
    return d_splice_alias(inode, dentry);
}

static const struct inode_operations myfs_dir_inode_operations = {
    .lookup = myfs_lookup,
};

/* Ham duyet danh sach file trong thu muc (readdir) */
static int myfs_readdir(struct file *file, struct dir_context *ctx) {
    pr_info("myfs: Duyet danh sach file tai vi tri ctx->pos: %lld\n", ctx->pos);

    /* Hien thi thu muc goc va cha (. va ..) */
    if (!dir_emit_dots(file, ctx))
        return 0;

    /* Pos = 2: readme.txt */
    if (ctx->pos == 2) {
        if (!dir_emit(ctx, "readme.txt", 10, 101, DT_REG))
            return 0;
        ctx->pos++;
    }

    /* Pos = 3: device_data.txt */
    if (ctx->pos == 3) {
        if (!dir_emit(ctx, "device_data.txt", 15, 102, DT_REG))
            return 0;
        ctx->pos++;
    }

    return 0;
}

static const struct file_operations myfs_dir_operations = {
    .read           = generic_read_dir,
    .iterate_shared = myfs_readdir,
    .llseek         = generic_file_llseek,
};

/* ====================================================
 * 3. HOAT DONG THAO TAC VOI FILE TRONG FILESYSTEM
 * ====================================================
 */
static int myfs_open(struct inode *inode, struct file *file) {
    pr_info("myfs: Mo file tren filesystem ao (inode = %lu)\n", inode->i_ino);
    return 0;
}

static int myfs_release(struct inode *inode, struct file *file) {
    pr_info("myfs: Dong file tren filesystem ao (inode = %lu)\n", inode->i_ino);
    return 0;
}

/* Ham doc file cua filesystem */
static ssize_t myfs_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    struct inode *inode = file_inode(file);
    unsigned long ino = inode->i_ino;
    char local_buffer[512];
    const char *data_ptr = NULL;
    size_t size = 0;
    ssize_t ret = 0;

    mutex_lock(&myfs_lock);

    if (ino == 101) {
        /* Noi dung cua readme.txt */
        snprintf(local_buffer, sizeof(local_buffer),
                 "==================================================\n"
                 "        MYFS - HE THONG FILE KERNEL MODULE        \n"
                 "==================================================\n"
                 "Thiet bi phan cung ao ho tro: /dev/%s\n"
                 "  - De doc tu thiet bi:  cat /dev/%s\n"
                 "  - De ghi vao thiet bi: echo '...' > /dev/%s\n"
                 "  - Giao tiep qua file:  /mnt/myfs_mount/device_data.txt\n"
                 "\n"
                 "Bo dem se luu tru dong thoi ca tren thiet bi va file.\n"
                 "==================================================\n",
                 DEVICE_NAME, DEVICE_NAME, DEVICE_NAME);
        data_ptr = local_buffer;
        size = strlen(local_buffer);
    } else if (ino == 102) {
        /* Doc tu bo dem toan cuc cho device_data.txt */
        data_ptr = device_buffer;
        size = device_data_size;
    } else {
        ret = -EINVAL;
        goto out;
    }

    ret = simple_read_from_buffer(buf, count, ppos, data_ptr, size);

out:
    mutex_unlock(&myfs_lock);
    return ret;
}

/* Ham ghi file vao filesystem */
static ssize_t myfs_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    struct inode *inode = file_inode(file);
    unsigned long ino = inode->i_ino;
    ssize_t ret = 0;

    if (ino == 101) {
        return -EACCES; /* readme.txt la read-only */
    }

    if (ino != 102) {
        return -EINVAL;
    }

    mutex_lock(&myfs_lock);

    if (count > BUFFER_SIZE - 1) {
        count = BUFFER_SIZE - 1;
    }

    if (copy_from_user(device_buffer, buf, count)) {
        ret = -EFAULT;
        goto out;
    }

    device_buffer[count] = '\0';
    device_data_size = count;
    *ppos = count;

    /* Cap nhat thoi gian sua doi file */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
    inode_set_mtime_to_ts(inode, current_time(inode));
    inode_set_ctime_to_ts(inode, current_time(inode));
#else
    inode->i_mtime = inode->i_ctime = current_time(inode);
#endif

    ret = count;
    pr_info("myfs: Ghi thanh cong %zu bytes vao file 'device_data.txt' tren VFS\n", count);

out:
    mutex_unlock(&myfs_lock);
    return ret;
}

static const struct file_operations myfs_file_operations = {
    .open    = myfs_open,
    .release = myfs_release,
    .read    = myfs_read,
    .write   = myfs_write,
    .llseek  = generic_file_llseek,
};

/* ====================================================
 * 4. KHOI TAO VA GAN MOUNTING CHO FILESYSTEM
 * ====================================================
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 18, 0)
static int myfs_fill_super(struct super_block *sb, struct fs_context *fc) {
#else
static int myfs_fill_super(struct super_block *sb, void *data, int silent) {
#endif
    struct inode *root_inode;

    sb->s_blocksize = 1024;
    sb->s_blocksize_bits = 10;
    sb->s_magic = 0x13072026; /* Magic number ngau nhien cho he thong file */
    sb->s_op = &myfs_super_ops;

    /* Tao inode cho thu muc goc (inode = 100) */
    root_inode = myfs_get_inode(sb, NULL, S_IFDIR | 0755, 100);
    if (!root_inode) {
        return -ENOMEM;
    }

    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) {
        return -ENOMEM;
    }

    pr_info("myfs: Hoan tat khoi dung thong tin superblock.\n");
    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 18, 0)
static int myfs_get_tree(struct fs_context *fc) {
    return get_tree_nodev(fc, myfs_fill_super);
}

static const struct fs_context_operations myfs_context_ops = {
    .get_tree = myfs_get_tree,
};

static int myfs_init_fs_context(struct fs_context *fc) {
    fc->ops = &myfs_context_ops;
    return 0;
}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
static struct dentry *myfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data) {
    pr_info("myfs: Yeu cau mount thiet bi thong qua duong dan: %s\n", dev_name);
    /* Su dung co che mount khong gian ao, bo qua device block vat ly thuc te */
    return mount_nodev(fs_type, flags, data, myfs_fill_super);
}
#else
static int myfs_get_sb(struct file_system_type *fs_type, int flags, const char *dev_name, void *data, struct vfsmount *mnt) {
    return get_sb_nodev(fs_type, flags, data, myfs_fill_super, mnt);
}
#endif

static void myfs_kill_sb(struct super_block *sb) {
    pr_info("myfs: He thong dang unmount, thu hoi cac superblock.\n");
    kill_litter_super(sb);
}

static struct file_system_type myfs_type = {
    .owner   = THIS_MODULE,
    .name    = "myfs",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 18, 0)
    .init_fs_context = myfs_init_fs_context,
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
    .mount   = myfs_mount,
#else
    .get_sb  = myfs_get_sb,
#endif
    .kill_sb = myfs_kill_sb,
};

/* ====================================================
 * 5. CAC HOAT DONG CUA DEVICE FILE (/dev/myfs_dev)
 * ====================================================
 */
static int dev_open(struct inode *inode, struct file *file) {
    pr_info("myfs_dev: Mo thanh cong file thiet bi /dev/%s\n", DEVICE_NAME);
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    pr_info("myfs_dev: Dong file thiet bi /dev/%s\n", DEVICE_NAME);
    return 0;
}

/* Doc tu thiet bi: tra ve noi dung trong bo dem toan cuc */
static ssize_t dev_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    ssize_t ret;
    mutex_lock(&myfs_lock);
    ret = simple_read_from_buffer(buf, count, ppos, device_buffer, device_data_size);
    if (ret > 0) {
        pr_info("myfs_dev: Da doc %zd bytes tu thiet bi gui ve user space.\n", ret);
    }
    mutex_unlock(&myfs_lock);
    return ret;
}

/* Ghi vao thiet bi: Cap nhat bo dem toan cuc */
static ssize_t dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    ssize_t ret = 0;
    mutex_lock(&myfs_lock);

    if (count > BUFFER_SIZE - 1) {
        count = BUFFER_SIZE - 1;
    }

    if (copy_from_user(device_buffer, buf, count)) {
        ret = -EFAULT;
        goto out;
    }

    device_buffer[count] = '\0';
    device_data_size = count;
    *ppos = count;

    ret = count;
    pr_info("myfs_dev: Nhap thanh cong %zu bytes tu user space ghi vao thiet bi.\n", count);

out:
    mutex_unlock(&myfs_lock);
    return ret;
}

static const struct file_operations dev_fops = {
    .owner   = THIS_MODULE,
    .open    = dev_open,
    .release = dev_release,
    .read    = dev_read,
    .write   = dev_write,
    .llseek  = generic_file_llseek,
};

static struct miscdevice myfs_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = DEVICE_NAME,
    .fops  = &dev_fops,
};

/* ====================================================
 * 6. PHAN DANG KY VA THAO GO DRIVER KERNEL MODULE
 * ====================================================
 */
static int __init myfs_init(void) {
    int ret;

    /* 1. Dang ky thiet bi kieu character device (misc device) */
    ret = misc_register(&myfs_misc_device);
    if (ret) {
        pr_err("myfs: Dang ky thiet bi /dev/%s that bai! (loi = %d)\n", DEVICE_NAME, ret);
        return ret;
    }
    pr_info("myfs: Thiet bi /dev/%s dang ky thanh cong.\n", DEVICE_NAME);

    /* 2. Dang ky he thong file voi nhan Linux */
    ret = register_filesystem(&myfs_type);
    if (ret) {
        pr_err("myfs: Dang ky he thong file 'myfs' that bai! (loi = %d)\n", ret);
        misc_deregister(&myfs_misc_device);
        return ret;
    }
    pr_info("myfs: Dang ky he thong file 'myfs' thanh cong.\n");

    return 0;
}

static void __exit myfs_exit(void) {
    /* 1. Huy dang ky he thong file */
    unregister_filesystem(&myfs_type);
    pr_info("myfs: Da huy dang ky he thong file 'myfs'.\n");

    /* 2. Huy dang ky thiet bi misc */
    misc_deregister(&myfs_misc_device);
    pr_info("myfs: Da go thiet bi /dev/%s ra khoi nhan.\n", DEVICE_NAME);
}

module_init(myfs_init);
module_exit(myfs_exit);

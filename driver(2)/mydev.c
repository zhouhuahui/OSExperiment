#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>

#define SUCCESS 0
#define DEVICE_NAME "geekz_char_dev"
#define BUF_LEN 128

static  int Device_Open =0;
static char tmp[BUF_LEN] = "mydev from kernel!";
static int Major;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("geek-zhu");
MODULE_DESCRIPTION("A Simple Character Device driver module");



static int 
mydev_open(struct inode *inodep, struct file *filep){
    if(Device_Open)
        return -EBUSY;
    Device_Open=1;
    try_module_get(THIS_MODULE);
    printk( KERN_EMERG "open\n");
    return 0;
}

static int
mydev_release(struct inode *inodep, struct file *filep)
{
    Device_Open=0;
    module_put(THIS_MODULE);
    printk( KERN_EMERG "release\n");
    return 0;
}

static ssize_t
mydev_read(struct file *filep, char __user *buf, size_t count, loff_t *offset)
{
    
    size_t  avail;

    printk( KERN_EMERG "read\n");

    avail = sizeof(tmp) - *offset;

    size_t len=(count <= avail)?count:avail;
    
    if(!access_ok(VERIFY_WRITE,buf,len)) 
        return -EFAULT;  

    size_t ret=copy_to_user(buf, tmp + *offset, len);        

    if (ret != 0) {
        printk( KERN_EMERG "read err!\n");
        return -EFAULT;
    }
     *offset += len;
    return len;
}

static ssize_t
mydev_write(struct file *filep, const char __user *buf, size_t count,
            loff_t *offset)
{
    size_t  avail;

    printk( KERN_EMERG "write\n");

    avail = sizeof(tmp) - *offset;
    size_t len=(count <= avail)?count:avail;

    if(!access_ok(VERIFY_READ,buf,len)) 
        return -EFAULT;
    memset(tmp + *offset, 0, avail);

    size_t ret=copy_from_user(tmp + *offset, buf, len);  
    printk( KERN_EMERG "write %s\n",tmp);
    if(ret != 0)
        return -EFAULT;
    *offset += avail;
    return len;

}

static loff_t
mydev_llseek(struct file *filep, loff_t off, int whence)
{
    loff_t  newpos;

    switch (whence) {
    case 0: /* SEEK_SET */
        newpos = off;
        break;
    case 1: /* SEEK_CUR */
        newpos = filep->f_pos + off;
        break;
    case 2: /* SEEK_END */
        newpos = sizeof(tmp) + off;
        break;
    default:
        return -EINVAL;
    }

    if (newpos < 0) {
        return -EINVAL;
    }

    filep->f_pos = newpos;
    return newpos;
}

static const struct file_operations  fops = {
    .owner = THIS_MODULE,
    .open = mydev_open,
    .release = mydev_release,
    .read = mydev_read,
    .llseek = mydev_llseek,
    .write = mydev_write,
};

static int __init mydev_init(void) {

    printk( KERN_EMERG "Load mydev\n");
    
    Major = register_chrdev(0,DEVICE_NAME,&fops);

    if (Major < 0) {
        printk( KERN_EMERG "Registering character device failed with %d\n",Major);
        return Major;
    }
    printk( KERN_EMERG "Registration success with Major device number %d\n",Major);

    return 0;
}

static void __exit mydev_cleanup(void) {

    printk( KERN_EMERG "cleanup mydev\n");
    //设备注销
    unregister_chrdev(Major,DEVICE_NAME);

}

module_init(mydev_init);
module_exit(mydev_cleanup);

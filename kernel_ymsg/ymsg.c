/*
 *  linux/fs/proc/ymsg.c
 *
 *  Copyright (C) 1992  by Linus Torvalds
 *
 */

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/syslog.h>

#include <asm/uaccess.h>
#include <asm/io.h>

extern wait_queue_head_t log_wait;
char* page = NULL;

static int ymsg_open(struct inode * inode, struct file * file)
{
	return do_syslog(SYSLOG_ACTION_OPEN, NULL, 0, SYSLOG_FROM_PROC);
}

static int ymsg_release(struct inode * inode, struct file * file)
{
	(void) do_syslog(SYSLOG_ACTION_CLOSE, NULL, 0, SYSLOG_FROM_PROC);
    if(page) free_page((unsigned long)page);
    
	return 0;
}

static ssize_t ymsg_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	int val = 0;        

	/*将字符串转换成数字*/        
	val = simple_strtol(page, NULL, 10);

    printk(KERN_ALERT"ymsg_read: %d[%s].\n", val, page);

    return snprintf(buf, PAGE_SIZE, "%d\n", val);
}

/*把缓冲区的值buff保存到设备寄存器val中去*/
//ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
static ssize_t ymsg_proc_write(struct file* filp, const char __user *buff, size_t len, loff_t* data) {
	int err = 0;

    printk(KERN_ALERT"ymsg_proc_write: %d.\n", len);

	if(len > PAGE_SIZE) {
		printk(KERN_ALERT"The buff is too large: %d.\n", len);
		return -EFAULT;
	}

	if(!page) {                
		printk(KERN_ALERT"Failed to alloc page.\n");
		return -ENOMEM;
	}        

	/*先把用户提供的缓冲区值拷贝到内核缓冲区中去*/
	if(copy_from_user(page, buff, len)) {
		printk(KERN_ALERT"Failed to copy buff from user.\n");                
		err = -EFAULT;
		goto out;
	}

    err = len;
    
out:	
	return err;
}


static unsigned int ymsg_poll(struct file *file, poll_table *wait)
{
	poll_wait(file, &log_wait, wait);
	if (do_syslog(SYSLOG_ACTION_SIZE_UNREAD, NULL, 0, SYSLOG_FROM_PROC))
		return POLLIN | POLLRDNORM;
	return 0;
}


static const struct file_operations proc_ymsg_operations = {
	.read		= ymsg_read,
    .write      = ymsg_proc_write,
	.poll		= ymsg_poll,
	.open		= ymsg_open,
	.release	= ymsg_release,
	.llseek		= generic_file_llseek,
};

static int __init proc_ymsg_init(void)
{
	proc_create("ymsg", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, NULL, &proc_ymsg_operations);
	page = (char*)__get_free_page(GFP_KERNEL);
    
	return 0;
}
fs_initcall(proc_ymsg_init);

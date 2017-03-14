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

static int ymsg_open(struct inode * inode, struct file * file)
{
	return do_syslog(SYSLOG_ACTION_OPEN, NULL, 0, SYSLOG_FROM_PROC);
}

static int ymsg_release(struct inode * inode, struct file * file)
{
	(void) do_syslog(SYSLOG_ACTION_CLOSE, NULL, 0, SYSLOG_FROM_PROC);
	return 0;
}

static ssize_t ymsg_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	if ((file->f_flags & O_NONBLOCK) &&
	    !do_syslog(SYSLOG_ACTION_SIZE_UNREAD, NULL, 0, SYSLOG_FROM_PROC))
		return -EAGAIN;
	return do_syslog(SYSLOG_ACTION_READ, buf, count, SYSLOG_FROM_PROC);
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
	.poll		= ymsg_poll,
	.open		= ymsg_open,
	.release	= ymsg_release,
	.llseek		= generic_file_llseek,
};

static int __init proc_ymsg_init(void)
{
	proc_create("ymsg", S_IRUSR, NULL, &proc_ymsg_operations);
	return 0;
}
fs_initcall(proc_ymsg_init);

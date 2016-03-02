/*
 * Copyright (C) 1998 Alessandro Rubini (rubini@linux.it)
 * 
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/module.h>

#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#include <asm/segment.h>

static struct miscdevice pipe_dev;


/* =================================== a simple delayer */

static void delay_tenths(int tenths)
{
    unsigned long j = jiffies;

    /* delay tenths tenths of a second */
    j+= tenths * HZ / 10;
    while (j > jiffies)
        schedule();
}

/* =================================== open and close */

//int pipe_open (struct inode *inode, struct file *filp)
int pipe_open (struct inode *inode, struct file *file)
{
    //MOD_INC_USE_COUNT;
    printk(KERN_ALERT "pipe_drv: open called %s %d \n",__FUNCTION__,__LINE__);
    return 0;
}

void pipe_close (struct inode *inode, struct file *filp)
{
    //MOD_DEC_USE_COUNT;
    printk(KERN_ALERT "pipe_drv: close called %s %d \n",__FUNCTION__,__LINE__);
}

/* =================================== read and write */

static DECLARE_WAIT_QUEUE_HEAD(wq);
static int flag = 0;
/*
 * The read() method just returns the current status of the leds.
 * Try "while true; do dd bs=1 if=/dev/kiss count=1; sleep 1; done"
 */
ssize_t pipe_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    printk(KERN_DEBUG "process %i (%s) going to sleep\n",
            current->pid, current->comm);
    wait_event_interruptible(wq, flag != 0);
    flag = 0;
    printk(KERN_DEBUG "awoken %i (%s)\n", current->pid, current->comm);
    return 0; /* EOF */
}

/*
 * Write understands a few textual commands
 */ 
ssize_t pipe_write (struct file *filp, const char __user *buf, size_t count,
                        loff_t *f_pos)
{
    printk(KERN_DEBUG "process %i (%s) awakening the readers...\n",
            current->pid, current->comm);
    flag = 1;
    wake_up_interruptible(&wq);
    return count; /* succeed, to avoid retrial */

}



/* =================================== all the operations */
struct file_operations pipe_fops = {
	.owner =	THIS_MODULE,
//	.llseek =	no_llseek,
	.read =		pipe_read,
 	.write =	pipe_write,
//	.poll =		scull_p_poll,
//	.unlocked_ioctl = scull_ioctl,
	.open =		pipe_open,
//	.release =	scull_p_release,
//	.fasync =	scull_p_fasync,
};
#if 0
/*
struct file_operations pipe_fops = {
    NULL,
    pipe_read,
    pipe_write,
    NULL,          /* pipe_readdir */
    NULL,           /* pipe_select */
    NULL,          /* pipe_ioctl */
    NULL,          /* pipe_mmap */
    pipe_open,
    pipe_close,
    NULL,          /* pipe_fsync */
    NULL,          /* pipe_fasync */
                   /* nothing more, fill with NULLs */
    NULL,
};
#endif

/* =================================== init and cleanup */

int init_module(void)
{
    int retval;
    pipe_dev.minor = MISC_DYNAMIC_MINOR;
    pipe_dev.name = "pipe";
    pipe_dev.fops = &pipe_fops;
    retval = misc_register(&pipe_dev);
    printk(KERN_ALERT "pipe_drv: init called %s %d \n",__FUNCTION__,__LINE__);
    if (retval) {
        printk(KERN_ALERT "pipe_drv: register error %s %d \n",__FUNCTION__,__LINE__);
        return retval;
    }
    return 0;
}

void cleanup_module(void)
{
    misc_deregister(&pipe_dev);
    printk(KERN_ALERT "pipe_drv: cleanup  called %s %d \n",__FUNCTION__,__LINE__);
}


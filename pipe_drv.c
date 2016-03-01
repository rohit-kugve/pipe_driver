#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");

static int pipe_init(void)
{
	printk(KERN_ALERT "Pipe Driver Init\n");
	return 0;
}
static void pipe_exit(void)
{
	printk(KERN_ALERT "Pipe exit\n");
}

module_init(pipe_init);
module_exit(pipe_exit);

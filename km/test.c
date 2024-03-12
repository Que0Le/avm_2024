#include <linux/init.h>
#include <linux/module.h>

#include "./common.h"
#include "./proc.h"

#define PROC_FILENAME "avm_proc_file"

MODULE_LICENSE("Dual BSD/GPL");


static unsigned char *internal_storage;


static int my_init(void)
{
    printk(KERN_ALERT "Module loaded!\n");

    internal_storage =
	    (unsigned char *)kmalloc(BUFFER_SIZE * MAX_PKT, GFP_KERNEL);

    /* Create proc file */
	proc_create(PROC_FILENAME, 0, NULL, &pops);
    return 0;
}
static void my_exit(void)
{
    printk(KERN_ALERT "Module is being unloaded ...!\n");
    remove_proc_entry(PROC_FILENAME, NULL);
    kfree(internal_storage);
    printk(KERN_ALERT "Module unloaded!\n");

}






module_init(my_init);
module_exit(my_exit);
#include <linux/init.h>
#include <linux/module.h>
#include <linux/timekeeping.h>
#include <linux/timer.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/list.h>

#include "./common.h"
#include "./proc.h"

#define PROC_FILENAME "avm_proc_file"

static s64 ms_since_boot(void)
{
	ktime_t boottime;
	s64 boottime_ns;
	unsigned long long boottime_ms;

	boottime = ktime_get_boottime();
	boottime_ns = ktime_to_ns(boottime);
	boottime_ms = boottime_ns / 1000000ULL;

	return boottime_ms;
}

static struct timer_list my_timer;

static void my_timer_callback(struct timer_list *t)
{
	printk(KERN_INFO "Timer callback: Running task every %d miliseconds\n",
	       BACKGROUND_SLEEP_INTERVAL);

	mod_timer(&my_timer,
		  jiffies + msecs_to_jiffies(BACKGROUND_SLEEP_INTERVAL));
}



static int my_init(void)
{
	printk(KERN_ALERT "Module loaded!\n");
	////
	printk(KERN_INFO "Boot time since system boot: %llu ms\n",
	       ms_since_boot());
	////

	printk(KERN_INFO "The process is \"%s\" (pid %i)\n", current->comm,
	       current->pid);
	current_storage_pos = 0;

	// alternatively, a larger memory area can be allocated with
	// (unsigned long *) __get_free_pages(GFP_KERNEL, PAGES_ORDER);
	internal_storage = (unsigned char *)kmalloc(BUFFER_SIZE, GFP_KERNEL);

	sema_init(&my_semaphore, 1);

	/* Create proc file */
	proc_create(PROC_FILENAME, 0, NULL, &pops);

	// Initialize the timer
	timer_setup(&my_timer, my_timer_callback, 0);
	mod_timer(&my_timer,
		  jiffies + msecs_to_jiffies(BACKGROUND_SLEEP_INTERVAL));

	printk(KERN_INFO "Timer initialized\n");
	return 0;
}

static void my_exit(void)
{
	// TODO: lock
	printk(KERN_ALERT "Module is being unloaded ...!\n");

	remove_proc_entry(PROC_FILENAME, NULL);

	del_timer(&my_timer);
	kfree(internal_storage);

	printk(KERN_ALERT "Module unloaded!\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Que0Le");
MODULE_DESCRIPTION("Demo Module: an example module built for demonstrating purpose");




// #include <linux/slab.h>
// #include <linux/module.h>
// #include <linux/init.h>
// #include <linux/list.h>
// #include <linux/kernel.h>

// struct my_node {
//     int data;
//     struct list_head list;
// };

// static LIST_HEAD(my_list);

// static int __init linked_list_init(void)
// {
//     struct my_node *node;
//     int i;

//     printk(KERN_INFO "Initializing linked list module\n");

//     // Add elements to the list
//     for (i = 0; i < 5; i++) {
//         node = kmalloc(sizeof(*node), GFP_KERNEL);
//         if (!node) {
//             printk(KERN_ERR "Failed to allocate memory\n");
//             return -ENOMEM;
//         }

//         node->data = i;
//         INIT_LIST_HEAD(&node->list); // Initialize the list head

//         // Add node to the end of the list
//         list_add_tail(&node->list, &my_list);
//     }

//     // Traverse the list and print the data
//     struct list_head *pos;
//     struct my_node *entry;

//     printk(KERN_INFO "Linked list elements:\n");
//     list_for_each(pos, &my_list) {
//         entry = list_entry(pos, struct my_node, list);
//         printk(KERN_INFO "Data: %d\n", entry->data);
//     }

//     return 0;
// }

// static void __exit linked_list_exit(void)
// {
//     struct my_node *entry, *tmp;

//     // Free allocated memory and delete list nodes
//     list_for_each_entry_safe(entry, tmp, &my_list, list) {
//         list_del(&entry->list);
//         kfree(entry);
//     }

//     printk(KERN_INFO "Exiting linked list module\n");
// }

// module_init(linked_list_init);
// module_exit(linked_list_exit);

// MODULE_LICENSE("GPL");
// MODULE_DESCRIPTION("Example of Linked List Usage in Kernel Module");



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
	struct storage_node *entry;
	int i = 0;

	if (down_interruptible(&my_semaphore)) {
		printk(KERN_ALERT "Interrupted down_interruptible");
		// We couldn't acquire access to data.
		// No big deal, just skip it this time.
		goto set_new_timer;
	}
	if (list_empty(&storage_list)) {
		printk(KERN_INFO "Timer callback (%d ms): <empty storage>",
		       BACKGROUND_SLEEP_INTERVAL);
	}
	list_for_each_entry(entry, &storage_list, list) {
		if (i == word_index_to_read) {
			word_index_to_read =
				(word_index_to_read + 1) % total_word_count;
			printk(KERN_INFO "Timer callback (%d ms): "
					 "Word_th (%d) = ### %s ###",
			       BACKGROUND_SLEEP_INTERVAL, entry->word_th,
			       entry->word);
			break;
		}
		i++;
	}
	up(&my_semaphore);

set_new_timer:
	// Setup timer for next round.
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
	storage_len = 0;

	// alternatively, a larger memory area can be allocated with
	// (unsigned long *) __get_free_pages(GFP_KERNEL, PAGES_ORDER);

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
	printk(KERN_ALERT "Module is being unloaded ...!\n");
	int ret;
	for (int i = 0; i < MAX_TRY_ACQUIRE_LOCK; i++) {
		ret = down_interruptible(&my_semaphore);
		if (ret) {
			if (i + 1 == MAX_TRY_ACQUIRE_LOCK) {
				printk(KERN_INFO
				       "... Forcing clear resources ...");
			} else {
				printk(KERN_INFO
				       "... Waiting for clearing resources ...");
			}
		} else {
			printk(KERN_INFO "... Clearing resources ...");
			break;
		}
	}

	remove_proc_entry(PROC_FILENAME, NULL);

	free_storage_nodes(&storage_list);

	del_timer(&my_timer);
	kfree(internal_storage);
	up(&my_semaphore);

	printk(KERN_ALERT "Module unloaded!\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Que0Le");
MODULE_DESCRIPTION("Demo Module: built for demonstrating purpose");

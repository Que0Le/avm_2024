
#ifndef COMMON_H__
#define COMMON_H__

#include <linux/semaphore.h>
#include <linux/slab.h>

#define BACKGROUND_SLEEP_INTERVAL 1000
#define BUFFER_SIZE 128
#define DEBUG_READ_FROM_US 1
#define LOG_PROC_FILE_PREFIX "AVM: Proc-file"

#define MAX_TRY_ACQUIRE_LOCK 5

static unsigned char *internal_storage;
// the current text's length that is stored internally
static size_t storage_len = 0;
static size_t total_word_count = 0;
static int word_index_to_read = 0;

static struct semaphore my_semaphore;

struct storage_node {
	char *word;
	int word_th;
	struct list_head list;
};

static LIST_HEAD(storage_list);

static int free_storage_nodes(struct list_head *lh) {
    struct storage_node *entry, *tmp;

    // Free allocated memory and delete list nodes
    list_for_each_entry_safe(entry, tmp, lh, list) {
		if (entry->word) {
			kfree(entry->word);
		}
        list_del(&entry->list);
        kfree(entry);
    }
	return 0;
}

static void print_all_nodes(struct list_head *lh) {
	// Traverse the list and print the data
	struct storage_node *entry;
	printk(KERN_INFO "Linked list elements:\n");
	list_for_each(lh, &storage_list) {
		entry = list_entry(lh, struct storage_node, list);
		printk(KERN_INFO "Word_th: %d, word: %s\n", entry->word_th,
		       entry->word);
	}
}

// static void pop_content_last_node(struct list_head *lh)
// {
// 	// Pop the last node
// 	if (!list_empty(lh)) {
// 		struct storage_list *last_node =
// 			list_last_entry(lh, struct storage_list, list);
// 		printk(KERN_INFO "Popped node (word_th %d): %s\n",
// 		       last_node->word_th, last_node->word);
// 		list_del(&last_node->list);
// 		kfree(last_node->word);
// 		kfree(last_node);
// 	}
// }

static int str_to_linked_list(struct list_head *lh, char *a, size_t n)
{
	char temp[128];
	int word_head = -1, word_tail = -1;
	int slice_now = 0;
	int count = 0;
	int word_count = 0;
	for (int i = 0; i < n; i++) {
		if (word_head == -1)
			word_head = i;
		if ((a[i] == ' ') | (a[i] == ',') | (a[i] == ';') |
		    (a[i] == ':') | (a[i] == '\n')) {
			// if we read a delimeter we reached the possible end of a word
			if (word_tail == -1) {
				// ... except multiple non-word chars
				word_head = -1;
				continue;
			} else {
				// ... signal slicing
				slice_now = 1;
			}
		} else {
			word_tail = i;
		}
		// handle slicing in case of being signaled,
		// or we reached the end of string
		if (slice_now || (i + 1 == n)) {
			word_count += 1;
			size_t len = min(127, word_tail - word_head + 1);
			printk(KERN_INFO "head %d tail %d\n", word_head,
			       word_tail);
			memcpy(&temp, &a[word_head], len);
			temp[len] = '\0';
			printk(KERN_INFO "%s\n", temp);
			//
			struct storage_node *node = kmalloc(sizeof(*node), GFP_KERNEL);
			if (!node) {
				printk(KERN_ERR "Failed to allocate memory for node!\n");
				return -ENOMEM;
			}
			node->word = kmalloc(len + 1, GFP_KERNEL);
			if (!node->word) {
				printk(KERN_ERR "Failed to allocate memory for word '%s'!\n", temp);
				return -ENOMEM;
			}
			memcpy(node->word, &a[word_head], len);
			node->word[len] = '\0';
			node->word_th = count++;

			INIT_LIST_HEAD(&node->list); // Initialize the list head

			// Add node to the end of the list
			list_add_tail(&node->list, lh);
			//
			word_tail = -1;
			word_head = -1;
			slice_now = 0;
		}
	}
	return word_count;
}

#endif
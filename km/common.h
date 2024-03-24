
#ifndef COMMON_H__
#define COMMON_H__

#include <linux/semaphore.h>
#include <linux/slab.h>

#define BACKGROUND_SLEEP_INTERVAL 1000
#define MAX_TRY_ACQUIRE_LOCK 5
// Including null char
#define MAX_WORD_BUFF_LEN 8

static unsigned char *internal_storage;
// the current text's length including null char
static size_t storage_len = 0;
// number of extracted words
static size_t total_word_count = 0;
// the word-th to read from the list
static int word_index_to_read = 0;

// static struct semaphore my_semaphore;
// static struct mutex mut_proc;
// static struct mutex mut_storage;
static struct mutex mut_all;


struct storage_node {
	char *word;
	int word_th;
	struct list_head list;
};

// Linked list where we store words
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
	printk(KERN_INFO "Linked list elements:");
	list_for_each(lh, &storage_list) {
		entry = list_entry(lh, struct storage_node, list);
		printk(KERN_INFO "... word_th: %d, word: %s", entry->word_th,
		       entry->word);
	}
}

// static void pop_content_last_node(struct list_head *lh)
// {
// 	// Pop the last node
// 	if (!list_empty(lh)) {
// 		struct storage_list *last_node =
// 			list_last_entry(lh, struct storage_list, list);
// 		printk(KERN_INFO "Popped node (word_th %d): %s",
// 		       last_node->word_th, last_node->word);
// 		list_del(&last_node->list);
// 		kfree(last_node->word);
// 		kfree(last_node);
// 	}
// }


/**
 * @brief 
 * Extract words from a given string and store them in the linked list.
 * Word longer than (MAX_WORD_BUFF_LEN - 1) will be truncated.
 * This function expects no special char in the string, i.e a null char.
 * The algorithm is simple: go through the string, mark the beginning and
 * ending of word and copy the memory between those marks.
 * Non-word is defined as space, comma, semi-colon, ...
 * 
 * @param lh struct list_head Linked list target.
 * @param a Raw string
 * @param n Size of string
 * @return int Number of word extracted
 */
static int str_to_linked_list(struct list_head *lh, char *a, size_t n)
{
	char temp[MAX_WORD_BUFF_LEN];	// debug purpose
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
			// minus 1 to make space for the null char
			size_t len = min(MAX_WORD_BUFF_LEN - 1, word_tail - word_head + 1);
			memset(&temp, '\0', MAX_WORD_BUFF_LEN);
			memcpy(&temp, &a[word_head], len);
			// temp[len - 1] = '\0';

			// store data
			struct storage_node *node = kmalloc(sizeof(*node), GFP_KERNEL);
			if (!node) {
				printk(KERN_ERR "Failed to allocate memory for node!");
				return -ENOMEM;
			}
			node->word = kmalloc(MAX_WORD_BUFF_LEN, GFP_KERNEL);
			if (!node->word) {
				printk(KERN_ERR "Failed to allocate memory for word '%s'!", temp);
				return -ENOMEM;
			}
			memcpy(node->word, &a[word_head], len);
			node->word[len] = '\0';
			node->word_th = count++;

			INIT_LIST_HEAD(&node->list); // Initialize the list head

			// Add node to the end of the list
			list_add_tail(&node->list, lh);

			// Get ready for next round
			word_tail = -1;
			word_head = -1;
			slice_now = 0;
		}
	}
	return word_count;
}

#endif
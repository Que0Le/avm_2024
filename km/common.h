
#ifndef COMMON_H__
#define COMMON_H__

#define BACKGROUND_SLEEP_INTERVAL 1000
#define BUFFER_SIZE 128
#define DEBUG_READ_FROM_US 1
#define LOG_PROC_FILE_PREFIX "AVM: Proc-file"

static unsigned char *internal_storage;
// the current text's length that is stored internally
static size_t current_storage_pos;

#include <linux/semaphore.h>

static struct semaphore my_semaphore;

// struct storage_node {
//     char* word;
//     struct list_head list;
// };

// static LIST_HEAD(storage_list);

static int str_to_linked_list(char *a, size_t n)
{
	char temp[128];
	int word_head = -1, word_tail = -1;
	int slice_now = 0;
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
		if (slice_now | (i + 1 == n)) {
			printk(KERN_INFO "head %d tail %d\n", word_head, word_tail);
			memset(&temp, '\0', 128);
			memcpy(&temp, &a[word_head], word_tail - word_head + 1);
			printk(KERN_INFO "%s\n", temp);
			word_tail = -1;
			word_head = -1;
			slice_now = 0;
		}
	}
	return 0;
}

#endif
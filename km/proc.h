#include <linux/fs.h>
#include <linux/kernel.h> /* min */
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h> /* copy_from_user, copy_to_user */
#include <linux/slab.h>
#include <linux/getcpu.h>
#include <linux/timekeeping.h>
#include <linux/semaphore.h>

#include <linux/string.h>
#include "./common.h"

// static unsigned long *log_buffs[NUM_LOG_BUFF];

struct mmap_info {
	char *data;
};

// static unsigned char *buff_from_here;
// static unsigned char *buff_temp;
// unsigned int current_index = 0; // index of (should be) next free cell
// int status[MAX_PKT] = { 0 };

// unsigned long count_pkt = 0;
// unsigned long count_pkt_overflow = 0;

/* After unmap. */
static void vm_close(struct vm_area_struct *vma)
{
	pr_info("vm_close\n");
}

/* First page access. */
static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct page *page;
	struct mmap_info *info;

	pr_info("vm_fault\n");
	info = (struct mmap_info *)vmf->vma->vm_private_data;
	if (info->data) {
		page = virt_to_page(info->data);
		get_page(page);
		vmf->page = page;
	}
	return 0;
}

/* After mmap. TODO vs mmap, when can this happen at a different time than mmap? */
static void vm_open(struct vm_area_struct *vma)
{
	pr_info("vm_open\n");
}

static struct vm_operations_struct vm_ops = {
	.close = vm_close,
	.fault = vm_fault,
	.open = vm_open,
};

static int mmap(struct file *filp, struct vm_area_struct *vma)
{
	pr_info("mmap\n");
	vma->vm_ops = &vm_ops;
	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
	vma->vm_private_data = filp->private_data;
	vm_open(vma);
	return 0;
}

static int open(struct inode *inode, struct file *filp)
{
	struct mmap_info *info;

	pr_info("%s opened\n", LOG_PROC_FILE_PREFIX);
	info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);
	// pr_info("virt_to_phys = 0x%llx\n",
	// 	(unsigned long long)virt_to_phys((void *)info));
	info->data = (char *)get_zeroed_page(GFP_KERNEL);
	// info->data = buff_from_here;
	memcpy(info->data, "asdf", 4);
	filp->private_data = info;
	return 0;
}

static ssize_t read(struct file *filp, char __user *buf, size_t len,
		    loff_t *off)
{
	// TODO: cat cmd now keeps reading the file forever.
	// TODO: use wait_for_completion(&comp) to wait for reading the list
	ssize_t ret;
	ret = min(len, (size_t)BUFFER_SIZE - (size_t)*off);
	pr_info("read: min(len, (size_t)BUFFER_SIZE - (size_t)*off) = %ld", ret);
	
	// Traverse the list and print the data
	struct list_head *pos;
	struct storage_node *entry;
	printk(KERN_INFO "Linked list elements:\n");
	list_for_each(pos, &storage_list) {
		entry = list_entry(pos, struct storage_node, list);
		printk(KERN_INFO "Word_th: %d, word: %s\n", entry->word_th,
		       entry->word);
	}
	//

	if (copy_to_user(buf, internal_storage, ret)) {
		// unsigned long __copy_to_user (void __user * to,const void * from,unsigned long n);
		// Returns number of bytes that could not be copied. On success, this will be zero.
		printk(KERN_ERR "copy_to_user failed!!!\n");
		ret = -EFAULT;
	} else {
		printk(KERN_INFO "copy_to_user done!!!\n");
	}

	return ret;
}


static ssize_t write(struct file *filp, const char __user *buf, size_t len,
		     loff_t *off)
{
	struct mmap_info *info;

	pr_info("Handling write-to-proc-file\n");
	info = filp->private_data;

	// if (copy_from_user(info->data, buf, min(len, (size_t)BUFFER_SIZE))) {
	// 	return -EFAULT;
	// } else {
	// 	return len;
	// }

	// TODO: lock internal_storage and current_storage_pos
	size_t temp_len = min(len, (size_t)BUFFER_SIZE);
	// down_interruptible(&my_semaphore);
	if (copy_from_user(internal_storage, buf, temp_len)) {
		return -EFAULT;
	} else {
		current_storage_pos = temp_len;
		free_storage_nodes(&storage_list);
		str_to_linked_list(&storage_list, internal_storage,
				   current_storage_pos);
	}
		return len;
	}
	// up(&my_semaphore);

static int release(struct inode *inode, struct file *filp)
{
	struct mmap_info *info;

	pr_info("%s released\n", LOG_PROC_FILE_PREFIX);
	info = filp->private_data;
	free_page((unsigned long)info->data);
	kfree(info);
	filp->private_data = NULL;
	return 0;
}

static const struct proc_ops pops = {
	.proc_mmap = mmap,
	.proc_open = open,
	.proc_release = release,
	.proc_read = read,
	.proc_write = write,
};
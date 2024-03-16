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

struct mmap_info {
	char *data;
};

/* 
* These vm_ functions were copied from other project. 
* Not sure which purpose they serve
*/
static void vm_close(struct vm_area_struct *vma)
{
	pr_info("vm_close");
}

/* First page access. */
static vm_fault_t vm_fault(struct vm_fault *vmf)
{
	struct page *page;
	struct mmap_info *info;

	pr_info("vm_fault");
	info = (struct mmap_info *)vmf->vma->vm_private_data;
	if (info->data) {
		page = virt_to_page(info->data);
		get_page(page);
		vmf->page = page;
	}
	return 0;
}

static void vm_open(struct vm_area_struct *vma)
{
	pr_info("vm_open");
}

static struct vm_operations_struct vm_ops = {
	.close = vm_close,
	.fault = vm_fault,
	.open = vm_open,
};

static int mmap(struct file *filp, struct vm_area_struct *vma)
{
	pr_info("mmap");
	vma->vm_ops = &vm_ops;
	vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;
	vma->vm_private_data = filp->private_data;
	vm_open(vma);
	return 0;
}

static int open(struct inode *inode, struct file *filp)
{
	// struct mmap_info *info;
	// info = kmalloc(sizeof(struct mmap_info), GFP_KERNEL);
	// info->data = (char *)get_zeroed_page(GFP_KERNEL);
	// filp->private_data = info;
	return 0;
}

/**
 * Executed when proc file is read from by userspace
*/
static ssize_t read(struct file *filp, char __user *buf, size_t len,
		    loff_t *off)
{
	pr_info("READ: User is reading %ld bytes", len);

	if (!internal_storage || storage_len == 0) {
		printk(KERN_INFO "... storage empty");
		return 0;
	}
	// TODO: cat cmd now keeps reading the file forever.
	ssize_t ret;

	if (down_interruptible(&my_semaphore)) {
		printk(KERN_ALERT "Interrupted down_interruptible");
		return (-EINTR);
	}
	ret = min(len, storage_len - (size_t)*off);
	pr_info("... Calculate bytes to read %ld: len (%ld) "
		"(storage_len - (size_t)*off) (%ld) ",
		ret, len, storage_len - (size_t)*off);

	if (copy_to_user(buf, internal_storage, ret)) {
		printk(KERN_ERR "copy_to_user failed!!!");
		ret = -EFAULT;
	} else {
		printk(KERN_INFO "copy_to_user done!!!");
	}
	up(&my_semaphore);

	return ret;
}

/**
 * Executed when proc file is written to from userspace.
 * "len" includes null char
*/
static ssize_t write(struct file *filp, const char __user *buf, size_t len,
		     loff_t *off)
{
	// struct mmap_info *info;
	// info = filp->private_data;

	// TODO: we assume that no malicious char is entered
	pr_info("WRITE: User has written %ld bytes: %s", len - 1, buf);

	if (down_interruptible(&my_semaphore)) {
		printk(KERN_ALERT "Failed acquiring lock for writing!!!");
		return(-EINTR); 
	}
	// Clean up data from last write
	kfree(internal_storage);
	storage_len = 0;
	word_index_to_read = 0;
	internal_storage = (unsigned char *)kmalloc(len, GFP_KERNEL);
	if (!internal_storage) {
		printk(KERN_ERR
		       "Failed allocate %ld bytes for user string!!!",
		       len);
		return -ENOMEM;
	}
	// Copy data from user and stored in storage
	if (copy_from_user(internal_storage, buf, len)) {
		printk(KERN_ERR "Failed copy_from_user!!!");
		kfree(internal_storage);
		up(&my_semaphore);
		return -EFAULT;
	} else {
		storage_len = len;
		free_storage_nodes(&storage_list);
		total_word_count = str_to_linked_list(
			&storage_list, internal_storage, storage_len);
		// Visual check
		print_all_nodes(&storage_list);
	}
	up(&my_semaphore);

	return len;
}

static int release(struct inode *inode, struct file *filp)
{
	// struct mmap_info *info;
	// info = filp->private_data;
	// free_page((unsigned long)info->data);
	// kfree(info);
	// filp->private_data = NULL;
	return 0;
}

static const struct proc_ops pops = {
	.proc_mmap = mmap,
	.proc_open = open,
	.proc_release = release,
	.proc_read = read,
	.proc_write = write,
};
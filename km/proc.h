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
	ssize_t ret;
	ret = min(len, (size_t)BUFFER_SIZE - (size_t)*off);
	pr_info("read: min(len, (size_t)BUFFER_SIZE - (size_t)*off) = %ld", ret);
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

// static ssize_t read2(struct file *filp, char __user *buf, size_t len,
// 		    loff_t *off)
// {
// 	struct mmap_info *info;
// 	ssize_t ret;
// #ifdef DEBUG_READ_FROM_US
// 	printk(KERN_INFO "read: len[%zu] off[%zu]\n", len, (size_t)*off);
// #endif
// 	if ((size_t)BUFFER_SIZE <= *off) {
// 		ret = 0;
// 	} else {
// 		// This 2 lines work (not neccessary) so let it be here.
// 		info = filp->private_data;
// 		ret = min(len, (size_t)BUFFER_SIZE - (size_t)*off);

// 		// /* We return buff_temp (BUFFER_SIZE) data to userspace */
// 		// unsigned int ci =
// 		// 	current_index; // remember last read index from last read() access
// 		// memset(buff_temp, '\0', BUFFER_SIZE);
// 		// int copied_pkts =
// 		// 	0; // how many packets did we grab to be copied to user space
// 		// int copied_index[PKTS_PER_BUFFER]; // index of those pkt in KM buffer
// 		// for (int i = 0; i < PKTS_PER_BUFFER; i++)
// 		// 	copied_index[i] = -1;
// 		// int try =
// 		// 	0; // keep track on how many times we search for pkt in buffer
// 		// /* We search for <PKTS_PER_BUFFER> available pkts in buffer, put it in buff_temp*/
// 		// while (copied_pkts < PKTS_PER_BUFFER && try < MAX_PKT) {
// 		// 	if (status[ci] != 0) {
// 		// 		memcpy(buff_temp +
// 		// 			       copied_pkts * PKT_BUFFER_SIZE,
// 		// 		       buff_from_here + ci * PKT_BUFFER_SIZE,
// 		// 		       PKT_BUFFER_SIZE);
// 		// 		copied_index[copied_pkts] = ci;
// 		// 		copied_pkts += 1;
// 		// 	}
// 		// 	ci = (ci + 1) % MAX_PKT;
// 		// 	try += 1;
// 		// }
// 		// /* If no pkts found in buffer, return */
// 		// // if (copied_pkts==0) {
// 		// //     return 0;
// 		// // }
// #ifdef DEBUG_READ_FROM_US
// 		// printk(KERN_INFO "copied [%d] pkts\n", copied_pkts);
// #endif
// 		/* Else, deliver to userspace and cleanup on success */
// 		if (copy_to_user(buf, internal_storage, 7)) {
// 			// unsigned long __copy_to_user (void __user * to,const void * from,unsigned long n);
// 			// Returns number of bytes that could not be copied. On success, this will be zero.
// 			printk(KERN_ERR "copy_to_user failed!!!\n");
// 			ret = -EFAULT;
// 		} else {
//             printk(KERN_INFO "copy_to_user done!!!\n");
// 			// Copy success. Clean up the KM bufffer slot(s)
// 			// for (int i = 0; i < PKTS_PER_BUFFER; i++) {
// 			// 	if (copied_index[i] != -1) {
// 			// 		memset(buff_from_here +
// 			// 			       copied_index[i] *
// 			// 				       PKT_BUFFER_SIZE,
// 			// 		       '\0', PKT_BUFFER_SIZE);
// 			// 		status[copied_index[i]] = 0;
// 			// 	}
// 			// }
// 		}
// 	}
// 	return BUFFER_SIZE;
// }

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
	if (copy_from_user(internal_storage, buf, temp_len)) {
		return -EFAULT;
	} else {
		current_storage_pos = temp_len;
		return len;
	}
}

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
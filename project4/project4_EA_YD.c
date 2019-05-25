/**
 * CS342 Spring 2019 - Project 4
 * A Kernel module to access and print some file system and file
 * related information.
 * @author Yusuf Dalva - 21602867
 * @author Efe Acer - 21602217
 */

// Necessary includes
#include <linux/module.h>		// needed for all modules
#include <linux/kernel.h>		// KERN_INFO
#include <linux/init.h>			// macros
#include <linux/moduleparam.h>	// passing command line arguments
#include <linux/pid.h>			// pid_task and find_get_pid functions
#include <linux/sched.h>		// task_struct
#include <linux/fs.h>			// files_struct and file
#include <linux/fdtable.h>		// files_fdtable
#include <linux/path.h>			// f_path
#include <linux/dcache.h>		// dentry
#include <linux/fs_struct.h>	// fs_struct
#include <linux/uidgid.h>		// kuid_t
#include <linux/pagemap.h>		// find_get_page
#include <linux/mm_types.h> 	// page
#include <linux/buffer_head.h> 	// buffer_head

#define AUTHORS "Efe Acer & Yusuf Dalva"
#define DESCRIPTION "Project 4 Module"

// Code to receive argument from command line
static int processid;
module_param(processid, int, 0000);
MODULE_PARM_DESC(processid, "The input process identifier");

// Variables
struct task_struct* 	task;
struct fs_struct* 		p_fs;
struct files_struct* 	p_files;
struct fdtable* 		p_fdtable;
struct file* 			curr_file;
struct address_space* 	curr_mapping;
struct page* 			curr_page;
struct buffer_head* 	curr_buffer_head;
int i;
int j;
int num_pages;

static int __init project4_EA_YD_init(void) {
	printk(KERN_INFO "project4_EA_YD module is started.\n");

	printk(KERN_INFO "\nInput process identifier: %d\n", processid);

	// Getting the task struct (PCB) of the process with processid
	task = pid_task(find_get_pid(processid), PIDTYPE_PID);

	if (task == NULL) {
		printk(KERN_INFO "There is no process with the given identifier.\n");
		return 0;
	}
	printk(KERN_INFO "Information about the file descriptors and open files of the process:\n");

	p_fs = task->fs;
	p_files = task->files;
	p_fdtable = files_fdtable(p_files);

	if (p_fs == NULL || p_files == NULL || p_fdtable == NULL) {
		printk(KERN_INFO "One of the kernel structures is null.\n");
		return 0;
	}
	
	printk(KERN_INFO "\n");
	i = 0;
	while (p_fdtable->fd[i] != NULL) {
		curr_file = p_fdtable->fd[i]; // current file
		printk(KERN_INFO "==============================\n");
		printk(KERN_INFO "Descriptor Number: %d\n", i);
		printk(KERN_INFO "Current File Position Pointer: %lld\n", curr_file->f_pos);
		printk(KERN_INFO "User's ID: %d\n", (int) curr_file->f_owner.euid.val);
		printk(KERN_INFO "Process Access Mode: %d\n", curr_file->f_mode);
		printk(KERN_INFO "Name of the File: %s\n", curr_file->f_path.dentry->d_name.name);
		printk(KERN_INFO "inode Number of the File: %ld\n", curr_file->f_path.dentry->d_inode->i_ino);
		printk(KERN_INFO "File Length in Bytes: %d\n", curr_file->f_inode->i_size);
		printk(KERN_INFO "Number of Blocks Allocated to the File: %ld\n", curr_file->f_inode->i_blocks);
		
		curr_mapping = curr_file->f_path.dentry->d_inode->i_mapping;
		if (curr_mapping == NULL) {
			continue;
		}
		num_pages = curr_mapping->nrpages;
		printk(KERN_INFO "Number of Blocks (Pages) Cached: %d", num_pages);
		j = 0;
		while ((curr_page = find_get_page(curr_mapping, j)) != NULL && j < curr_mapping->nrpages && j < 100) {
			printk(KERN_INFO "------------------------------\n");
			curr_buffer_head = (struct buffer_head*) curr_page->private; // needs casting
			printk(KERN_INFO "Storage Device (Search Key): %d", curr_buffer_head->b_bdev->bd_dev);
			printk(KERN_INFO "Block Number: %ld", curr_buffer_head->b_blocknr);
			printk(KERN_INFO "Use Count: %ld", curr_page->_refcount);
			j++;
		}
		printk(KERN_INFO "------------------------------\n");
		i++;
	}
	printk(KERN_INFO "==============================\n\n");
	printk(KERN_INFO "Current Directory of the Process: %s\n", p_fs->pwd.dentry->d_name.name);
	return 0;
}

static void __exit project4_EA_YD_exit(void) {
	printk(KERN_INFO "\nproject4_EA_YD module is finished.\n");
}

// Code to specify the initializer and desctructor of the module
module_init(project4_EA_YD_init);
module_exit(project4_EA_YD_exit);

// Code to license the module
MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHORS);
MODULE_DESCRIPTION(DESCRIPTION);

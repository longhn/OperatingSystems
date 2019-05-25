/**
 * CS342 Spring 2019 - Project 4
 * A simple Kernel module to print "Hello, World!"
 * @author Yusuf Dalva - 21602867
 * @author Efe Acer - 21602217
 */

// Necessary include(s)
#include <linux/module.h> // Needed for all modules
#include <linux/kernel.h> // Needed for KERN_INFO

int init_module(void) {
	printk(KERN_INFO "Hello, World!\n");
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "Goodbye, World.\n");
}



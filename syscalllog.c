#include <linux/module.h>       /* Needed by all modules */
#include <linux/kernel.h>       /* Needed for KERN_INFO */
#include <linux/init.h>         /* Needed for the macros */
#include <linux/syscalls.h>		/* Needed for getting syscall reference */
#include <linux/time.h>			/* Needed for getting microseconds from Epoch */
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <asm/current.h>
#include <linux/unistd.h>
#include <linux/namei.h>


#define MODULE_VERS "0.01"
#define MODULE_NAME "syscalllog"

static unsigned long *sys_call_table;
static bool replaced = false;

asmlinkage long (*original_sys_open) (const char __user * filename, int
flags, int mode);

asmlinkage long our_fake_open_function(const char __user *filename, int
flags, int mode)
{
	struct timeval tv;
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		printk(KERN_INFO "SyscallLog: Uid: %d open %s at time %ld %llu\n",current->uid,filename,tv.tv_sec, tv.tv_usec);
	}
	return original_sys_open(filename,flags,mode);
}

/* From other source: https://bbs.archlinux.org/viewtopic.php?id=139406 */
static void disable_page_protection(void) 
{
	unsigned long value;
	asm volatile("mov %%cr0, %0" : "=r" (value));

	if(!(value & 0x00010000))
		return;

	asm volatile("mov %0, %%cr0" : : "r" (value & ~0x00010000));
}

static void enable_page_protection(void) 
{
	unsigned long value;
	asm volatile("mov %%cr0, %0" : "=r" (value));

	if((value & 0x00010000))
		return;

	asm volatile("mov %0, %%cr0" : : "r" (value | 0x00010000));
}

/* End of page protection */

// init process
static int __init logger_init(void)
{
	
	int i=1024;
	unsigned long *sys_table;
	int flag = 0;
	sys_table = (unsigned long *)simple_strtoul("0xffffffff804fbb80",NULL,16);
	
	printk(KERN_INFO "%lu\n", *sys_table);
	
	flag = 1;
	sys_call_table = sys_table;
	if(flag) {
		disable_page_protection();
		original_sys_open =(void * )xchg(&(sys_call_table[__NR_open]), our_fake_open_function);
		enable_page_protection();
		printk(KERN_INFO "SyscallLog: Syscall open found, replacing it...\n");
		replaced = true;
	}
	else {
		printk(KERN_INFO "SyscallLog: Syscall open not found, nothing to do...\n");
	}
	/*
	*/
	printk(KERN_INFO "SyscallLog: Everything loaded, good to go!\n");
	printk(KERN_INFO "SyscallLog: All logs will be recorded from now...\n");
	return 0;
}

// close process
static void __exit logger_exit(void)
{
	// unlink the file
	if (replaced) {
		disable_page_protection();
		xchg(&(sys_call_table[__NR_open]), original_sys_open);
		enable_page_protection();
	}
	printk(KERN_INFO "SyscallLog: Warning: You have turned off the logger.\n");
}

module_init(logger_init);
module_exit(logger_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("System calls interposer");

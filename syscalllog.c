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

/**
These syscall will be recorded:

fork
read
open
creat
execve
mount
access
readlink
mmap
ioperm

setuid
setreuid
//mmap2
vfork
pread
setresuid
mremap
fdatasync
fsync
readv
setfsuid

**/


#define MODULE_VERS "0.1"
#define MODULE_NAME "syscalllog"

static unsigned long *sys_call_table;
static bool replaced = false;

static void log_action(unsigned long uid, struct timeval tv, const char *sys_call_name) {
	printk(KERN_INFO "SyscallLog: Uid: %d %s at time %ld.%.6ld\n",current->uid,sys_call_name,tv.tv_sec, tv.tv_usec);
}

/** fork **/

asmlinkage int (*original_sys_fork) (struct pt_regs regs);
	
asmlinkage int our_fake_fork_function(struct pt_regs regs)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "fork");
	}
	
	return original_sys_fork(regs);
}

/** read **/
asmlinkage ssize_t (*original_sys_read) (unsigned int fd, char __user * buf, size_t count);

asmlinkage ssize_t our_fake_read_function(unsigned int fd, char __user * buf, size_t count)
{
	struct timeval tv;
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		log_action(current->uid, tv, "read");
	}
	return original_sys_read(fd, buf, count);
}

/** open **/

asmlinkage long (*original_sys_open) (const char __user * filename, int flags, int mode);

asmlinkage long our_fake_open_function(const char __user * filename, int flags, int mode)
{
	struct timeval tv;
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		//printk(KERN_INFO "SyscallLog: Uid: %d open %s at time %ld.%.6ld\n",current->uid,filename,tv.tv_sec, tv.tv_usec);
		log_action(current->uid, tv, "open");
	}
	return original_sys_open(filename,flags,mode);
}

/** creat **/

asmlinkage long (*original_sys_creat) (const char __user * pathname, int mode);

asmlinkage long our_fake_creat_function(const char __user * pathname, int mode)
{
	struct timeval tv;
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		log_action(current->uid, tv, "creat");
	}
	return original_sys_creat(pathname,mode);	
}

/** execve **/

asmlinkage int (*original_sys_execve) (struct pt_regs regs);

asmlinkage int our_fake_execve_function(struct pt_regs regs)
{
	struct timeval tv;
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		log_action(current->uid, tv, "execve");
	}
	return original_sys_execve(regs);
}

/** mount **/

asmlinkage long (*original_sys_mount) (char __user * dev_name, char __user * dir_name, char __user * type, unsigned long flags,
			void __user * data);
						
asmlinkage long our_fake_mount_function(char __user * dev_name, char __user * dir_name, char __user * type, unsigned long flags,
			void __user * data)
{
	struct timeval tv;
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		log_action(current->uid, tv, "mount");
	}
	return original_sys_mount(dev_name, dir_name, type, flags, data);
}

/** access **/

asmlinkage long (*original_sys_access)(const char __user * filename, int mode);

asmlinkage long our_fake_access_function(const char __user * filename, int mode)
{
	struct timeval tv;
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		log_action(current->uid, tv, "access");
	}
	return original_sys_access(filename,mode);
}

/** readlink **/

asmlinkage long (*original_sys_readlink)(const char __user * path, char __user * buf, int bufsiz);

asmlinkage long our_fake_readlink_function(const char __user * path, char __user * buf, int bufsiz)
{
	struct timeval tv;
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		log_action(current->uid, tv, "readlink");
	}
	return original_sys_readlink(path, buf, bufsiz);
}

/** mmap **/

asmlinkage long (*original_old_mmap) (unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, 
	unsigned long fd, unsigned long offset);

asmlinkage long our_fake_mmap_function(unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, 
	unsigned long fd, unsigned long offset)
{
	struct timeval tv;
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		log_action(current->uid, tv, "mmap");
	}
	return original_old_mmap(addr, len, prot, flags, fd, offset);	
}

/** ioperm **/

asmlinkage long (*original_sys_ioperm) (unsigned long from, unsigned long num, int turn_on);

asmlinkage long our_fake_ioperm_function(unsigned long from, unsigned long num, int turn_on)
{
	struct timeval tv;
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		log_action(current->uid, tv, "ioperm");
	}
	return original_sys_ioperm(from,num,turn_on);
}

/** setfsuid **/

asmlinkage int (*original_sys_setfsuid)(uid_t fsuid); 
	
asmlinkage int our_fake_setfsuid_function(uid_t fsuid)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "setfsuid");
	}
	
	return original_sys_setfsuid(fsuid);
}

/** readv **/

asmlinkage ssize_t (*original_sys_readv)(int fd, const struct iovec *iov, int iovcnt); 
	
asmlinkage ssize_t our_fake_readv_function(int fd, const struct iovec *iov, int iovcnt)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "readv");
	}
	
	return original_sys_readv(fd, iov, iovcnt); 
}

/** fsync **/

asmlinkage int (*original_sys_fsync)(int fd); 
	
asmlinkage int our_fake_fsync_function(int fd)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "fsync");
	}
	
	return original_sys_fsync(fd); 
}

/** fdatasync **/

asmlinkage int (*original_sys_fdatasync)(unsigned int fd); 
	
asmlinkage int our_fake_fdatasync_function(unsigned int fd)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "fdatasync");
	}
	
	return original_sys_fdatasync(fd); 
}

/** mremap **/

asmlinkage unsigned long (*original_sys_mremap) (unsigned long addr, unsigned long old_len, unsigned long new_len,
	unsigned long flags, unsigned long new_addr);
	
asmlinkage unsigned long our_fake_mremap_function (unsigned long addr, unsigned long old_len, unsigned long new_len,
        unsigned long flags, unsigned long new_addr)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "mremap");
	}
	
	return original_sys_mremap(addr,old_len,new_len,flags,new_addr); 
}

/** setresuid **/

asmlinkage long (*original_sys_setresuid)(uid_t ruid, uid_t euid, uid_t suid);
	
asmlinkage long our_fake_setresuid_function(uid_t ruid, uid_t euid, uid_t suid)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "setresuid");
	}
	
	return original_sys_setresuid(ruid, euid, suid); 
}

/** pread **/

asmlinkage ssize_t (*original_sys_pread)(unsigned int fd, char __user *buf, size_t count, loff_t pos);
	
asmlinkage ssize_t our_fake_pread_function(unsigned int fd, char __user *buf, size_t count, loff_t pos)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "pread64");
	}
	
	return original_sys_pread(fd, buf, count, pos); 
}

/** setregid **/

asmlinkage int (*original_sys_setregid) (gid_t rgid, gid_t egid);
	
asmlinkage int our_fake_setregid_function(gid_t rgid, gid_t egid)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "setregid");
	}
	
	return original_sys_setregid(rgid, egid);
}

/** setreuid **/

asmlinkage int (*original_sys_setreuid) (uid_t ruid, uid_t euid);
	
asmlinkage int our_fake_setreuid_function(uid_t ruid, uid_t euid)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "setreuid");
	}
	
	return original_sys_setreuid(ruid,euid);
}

/** setuid **/

asmlinkage int (*original_sys_setuid)(uid_t uid);
	
asmlinkage int our_fake_setuid_function(uid_t uid)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "setuid");
	}
	
	return original_sys_setuid(uid);
}

/** mmap2 **/
/*
asmlinkage long(*original_sys_mmap2) (unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags,
	unsigned long fd, unsigned long pgoff);

asmlinkage long our_fake_mmap2_function(unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags,
	unsigned long fd, unsigned long pgoff)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "mmap2");
	}
	
	return original_sys_mmap2(addr, len, prot, flags, fd, pgoff);
}
*/
/** vfork **/

asmlinkage pid_t (*original_sys_vfork)(struct pt_regs regs);

asmlinkage pid_t our_fake_vfork_function(struct pt_regs regs)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "vfork");
	}
	
	return original_sys_vfork(regs);
}


/************************************************************
				End of hijack syscall function
*************************************************************/

/* From other source: https://bbs.archlinux.org/viewtopic.php?id=139406 */

static unsigned long **aquire_sys_call_table(void)
{
	unsigned long int offset = PAGE_OFFSET;
	unsigned long **sct;
	printk(KERN_INFO "Start offset: %lu\n", offset);
	printk(KERN_INFO "close: %lu %lu\n", (unsigned long) sys_close, sizeof(void *));
	while (offset < ULLONG_MAX) {
		sct = (unsigned long **)offset;

		if ( sct[__NR_close] ==  (unsigned long *)sys_close) 
			return &sct[0];

		offset += sizeof(void *);
	}

	return NULL;
}

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

/* End of page protection source */

// init process
static int __init logger_init(void)
{
	unsigned long *sys_table;
	unsigned long *sys_table2;
	int flag = 0;
	printk(KERN_INFO "%lu\n", simple_strtoul("0xffffffff804fbb80",NULL,16));
	printk(KERN_INFO "%lu\n", simple_strtoul("0xffffffff804ff148",NULL,16));
		
	sys_table = (unsigned long *)aquire_sys_call_table();
	//sys_table = (unsigned long *) simple_strtoul("0xffffffff804fbb80",NULL,16);
	
	if (sys_table) {
		flag = 1;
		printk(KERN_INFO "%lu %lu\n", sys_table[__NR_open], (unsigned long )sys_open);
	}
	/*
	if (sys_table2) {
		printk(KERN_INFO "table2: %lu\n", sys_table[__NR_open]);
	}
	*/	
	if(flag) {
		sys_call_table = sys_table;
		printk(KERN_INFO "SyscallLog: Syscall table found, replacing selected functions with our own...\n");
		disable_page_protection();
		original_sys_fork =(void * )xchg(&(sys_call_table[__NR_fork]), our_fake_fork_function);
		original_sys_read =(void * )xchg(&(sys_call_table[__NR_read]), our_fake_read_function);
		original_sys_open =(void * )xchg(&(sys_call_table[__NR_open]), our_fake_open_function);
		original_sys_creat =(void * )xchg(&(sys_call_table[__NR_creat]), our_fake_creat_function);
		// 	original_sys_execve =(void * )xchg(&(sys_call_table[__NR_execve]), our_fake_execve_function);
		// 	original_sys_mount =(void * )xchg(&(sys_call_table[__NR_mount]), our_fake_mount_function);
		// 	original_sys_access =(void * )xchg(&(sys_call_table[__NR_access]), our_fake_access_function);
		// 	original_sys_readlink =(void * )xchg(&(sys_call_table[__NR_readlink]), our_fake_readlink_function);
		// 	original_old_mmap =(void * )xchg(&(sys_call_table[__NR_mmap]), our_fake_mmap_function);
		// 	original_sys_ioperm =(void * )xchg(&(sys_call_table[__NR_ioperm]), our_fake_ioperm_function);
		// 	//
		// 	original_sys_setuid =(void * )xchg(&(sys_call_table[__NR_setuid]), our_fake_setuid_function);
		// 	original_sys_setreuid =(void * )xchg(&(sys_call_table[__NR_setreuid]), our_fake_setreuid_function);
		// 	//original_sys_mmap2 =(void * )xchg(&(sys_call_table[__NR_mmap2]), our_fake_mmap2_function);
		// 	original_sys_vfork =(void * )xchg(&(sys_call_table[__NR_vfork]), our_fake_vfork_function);
		// 	original_sys_pread =(void * )xchg(&(sys_call_table[__NR_pread64]), our_fake_pread_function);
		// 	original_sys_setresuid =(void * )xchg(&(sys_call_table[__NR_setresuid]), our_fake_setresuid_function);
		// 	original_sys_mremap =(void * )xchg(&(sys_call_table[__NR_mremap]), our_fake_mremap_function);
		// 	original_sys_fdatasync =(void * )xchg(&(sys_call_table[__NR_fdatasync]), our_fake_fdatasync_function);
		// 	original_sys_fsync=(void * )xchg(&(sys_call_table[__NR_fsync]), our_fake_fsync_function);
		// 	original_sys_readv =(void * )xchg(&(sys_call_table[__NR_readv]), our_fake_readv_function);
		// 	original_sys_setfsuid =(void * )xchg(&(sys_call_table[__NR_setfsuid]), our_fake_setfsuid_function);
		// 
		enable_page_protection();
		
		replaced = true;
	}
	else {
		printk(KERN_INFO "SyscallLog: Syscall open not found, nothing to do...\nSyscallLog: Leaving...\n");
		return -1;
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
	// restore the syscall function before hijacking
	
	if (replaced) {
		disable_page_protection();
		xchg(&(sys_call_table[__NR_fork]), original_sys_fork);
		xchg(&(sys_call_table[__NR_read]), original_sys_read);
		xchg(&(sys_call_table[__NR_open]), original_sys_open);
		xchg(&(sys_call_table[__NR_creat]), original_sys_creat);
		// 	xchg(&(sys_call_table[__NR_execve]), original_sys_execve);
		// 	xchg(&(sys_call_table[__NR_mount]), original_sys_mount);
		// 	xchg(&(sys_call_table[__NR_access]), original_sys_access);
		// 	xchg(&(sys_call_table[__NR_readlink]), original_sys_readlink);
		// 	xchg(&(sys_call_table[__NR_mmap]), original_old_mmap);
		// 	xchg(&(sys_call_table[__NR_ioperm]), original_sys_ioperm);
		// 	//
		// 	
		// 	xchg(&(sys_call_table[__NR_setuid]), original_sys_setuid);
		// 	xchg(&(sys_call_table[__NR_setreuid]), original_sys_setreuid);
		// 	//xchg(&(sys_call_table[__NR_mmap2]), original_sys_mmap2);
		// 	xchg(&(sys_call_table[__NR_vfork]), original_sys_vfork);
		// 	xchg(&(sys_call_table[__NR_pread64]), original_sys_pread);
		// 	xchg(&(sys_call_table[__NR_setresuid]), original_sys_setresuid);
		// 	xchg(&(sys_call_table[__NR_mremap]), original_sys_mremap);
		// 	xchg(&(sys_call_table[__NR_fdatasync]), original_sys_fdatasync);
		// 	xchg(&(sys_call_table[__NR_fsync]), original_sys_fsync);
		// 	xchg(&(sys_call_table[__NR_readv]), original_sys_readv);
		// 	xchg(&(sys_call_table[__NR_setfsuid]), original_sys_setfsuid);
		// 	
		enable_page_protection();
	}
	printk(KERN_INFO "SyscallLog: Warning: You have turned off the logger.\n");
}

module_init(logger_init);
module_exit(logger_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("System calls interposer");

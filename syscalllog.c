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
#include <linux/seq_file.h>		/* for seq_file */

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
vfork
pread
setresuid
mremap
fdatasync
fsync
readv
setfsuid

**/


#define MODULE_VERS "0.5"
#define MODULE_NAME "syscalllog"
#define MAX_LOG_LENGTH 1024
#define procfs_name "syslog"


static unsigned long *sys_call_table;
static bool replaced = false;

struct logMsg {
	char *msg;
	struct logMsg *next;
};

static struct logMsg *msg_head;
static struct logMsg *msg_tail;

static struct proc_dir_entry *syslog_file;

DEFINE_MUTEX(msg_mutex);

// <pid>  <syscall number>    <timestamp>     <arg values>
static void add_msg(const char *msg, int len) {
	struct logMsg *new_msg;
	int i;
	new_msg = vmalloc(sizeof(struct logMsg));
	new_msg->msg = vmalloc(sizeof(char) * len);
	new_msg->next = NULL;
	for (i = 0 ; i<len; i++) {
		new_msg->msg[i] = msg[i];
	}
	
	mutex_lock(&msg_mutex);
	
	if (!msg_head) {
		msg_head = new_msg;
		new_msg->next = NULL;
		msg_tail = new_msg;
	} else {
		msg_tail->next = new_msg;
		msg_tail = new_msg;
	}
	mutex_unlock(&msg_mutex);
}

static void remove_head_msg(void)
{
	struct logMsg *cur;
	mutex_lock(&msg_mutex);
	if (msg_head) {
		if (msg_head == msg_tail) {
			cur = msg_tail;
			msg_tail = NULL;
			msg_head = NULL;
			vfree(cur->msg);
			vfree(cur);
		} else {
			cur = msg_head;
			msg_head = msg_head->next;
			vfree(cur->msg);
			vfree(cur);
		}
	}
	mutex_unlock(&msg_mutex);
}

/* seq_printf: reference: http://linux.die.net/lkmpg/x861.html */

/**
 * This function is called at the beginning of a sequence.
 * ie, when:
 *	- the /proc file is read (first time)
 *	- after the function stop (end of sequence)
 *
 */
static void *my_seq_start(struct seq_file *s, loff_t *pos)
{
	static unsigned long counter = 0;

	/* beginning a new sequence ? */	
	if (msg_head)
	{	
		/* yes => return a non null value to begin the sequence */
		return &counter;
	}
	else
	{
		/* no => it's the end of the sequence, return end to stop reading */
		*pos = 0;
		return NULL;
	}
}

/**
 * This function is called after the beginning of a sequence.
 * It's called until the return is NULL (this ends the sequence).
 *
 */
static void *my_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	loff_t *spos = (loff_t *) v;
	*pos = ++(*spos);
	mutex_lock(&msg_mutex);
	if (!msg_head) {
		mutex_unlock(&msg_mutex);
		return NULL;
	} else {
		mutex_unlock(&msg_mutex);
		return spos;
	}
	
}

/**
 * This function is called at the end of a sequence
 * 
 */
static void my_seq_stop(struct seq_file *s, void *v)
{
	/* nothing to do, we use a static value in start() */
}

/**
 * This function is called for each "step" of a sequence
 *
 */
static int my_seq_show(struct seq_file *s, void *v)
{
	//loff_t *spos = (loff_t *) v;
	
	if (msg_head)
		seq_printf(s, "%s\n", msg_head->msg);
		
	remove_head_msg();
	return 0;
}

/**
 * This structure gather "function" to manage the sequence
 *
 */
static struct seq_operations my_seq_ops = {
	.start = my_seq_start,
	.next  = my_seq_next,
	.stop  = my_seq_stop,
	.show  = my_seq_show
};

/**
 * This function is called when the /proc file is open.
 *
 */
static int my_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &my_seq_ops);
};

/**
 * This structure gather "function" that manage the /proc file
 *
 */
static struct file_operations my_file_ops = {
	.owner   = THIS_MODULE,
	.open    = my_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};

/* end of seq_printf */

static void log_action(unsigned long pid, struct timeval tv, unsigned long sys_call_number, 
const char *additional_info) {

	char *str = vmalloc(sizeof(char) * MAX_LOG_LENGTH);
	int len;
	len = sprintf(str, "%-6ld\t%-14ld\t%ld.%.6ld\t%s", pid, sys_call_number, tv.tv_sec, tv.tv_usec, additional_info);
	add_msg(str,len+1);
	vfree(str);
}

/***************************syscall method *******************************/
/** fork **/

asmlinkage int (*original_sys_fork) (struct pt_regs regs);
	
asmlinkage int our_fake_fork_function(struct pt_regs regs)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->pid, tv, __NR_fork, "arg1(regs)");
	}
	
	return original_sys_fork(regs);
}

/** read **/
asmlinkage ssize_t (*original_sys_read) (unsigned int fd, char __user * buf, size_t count);

asmlinkage ssize_t our_fake_read_function(unsigned int fd, char __user * buf, size_t count)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		sprintf(argument, "arg1(fd): %u", fd);
		log_action(current->pid, tv, __NR_read, argument);
		sprintf(argument, "arg3(count): %ld", count);
		log_action(current->pid, tv, __NR_read, argument);
	}
	return original_sys_read(fd, buf, count);
}

/** open **/

asmlinkage long (*original_sys_open) (const char __user * filename, int flags, int mode);

asmlinkage long our_fake_open_function(const char __user * filename, int flags, int mode)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		sprintf(argument, "arg1(filename): %s", filename);
		log_action(current->pid, tv, __NR_open, argument);
		sprintf(argument, "arg2(flags): %d", flags);
		log_action(current->pid, tv, __NR_open, argument);
		sprintf(argument, "arg3(mode): %d", mode);
		log_action(current->pid, tv, __NR_open, argument);
	}
	return original_sys_open(filename,flags,mode);
}

/** creat **/

asmlinkage long (*original_sys_creat) (const char __user * pathname, int mode);

asmlinkage long our_fake_creat_function(const char __user * pathname, int mode)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		sprintf(argument, "arg1(pathname): %s", pathname);
		log_action(current->pid, tv, __NR_creat, argument);
		sprintf(argument, "arg2(fd): %d", mode);
		log_action(current->pid, tv, __NR_creat, argument);
	}
	return original_sys_creat(pathname,mode);	
}

/** execve **/

asmlinkage long (*original_sys_execve) (char __user *filename, char __user * __user *argv, char __user * __user *envp, 
	struct pt_regs *regs);

asmlinkage int our_fake_execve_function(char __user *filename, char __user * __user *argv, char __user * __user *envp, 
	struct pt_regs *regs)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		sprintf(argument, "arg1(pathname): %s", filename);
		log_action(current->pid, tv, __NR_execve, argument);
	}
	return original_sys_execve(filename, argv, envp, regs);
}

/** mount **/

asmlinkage long (*original_sys_mount) (char __user * dev_name, char __user * dir_name, char __user * type, unsigned long flags,
			void __user * data);
						
asmlinkage long our_fake_mount_function(char __user * dev_name, char __user * dir_name, char __user * type, unsigned long flags,
			void __user * data)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		sprintf(argument, "arg1(dev_name): %s", dev_name);
		log_action(current->pid, tv, __NR_mount, argument);
		
		sprintf(argument, "arg2(dir_name): %s", dir_name);
		log_action(current->pid, tv, __NR_mount, argument);
		
		sprintf(argument, "arg3(type): %s", type);
		log_action(current->pid, tv, __NR_mount, argument);
		
		sprintf(argument, "arg4(flags): %lu", flags);
		log_action(current->pid, tv, __NR_mount, argument);
	}
	return original_sys_mount(dev_name, dir_name, type, flags, data);
}

/** access **/

asmlinkage long (*original_sys_access)(const char __user * filename, int mode);

asmlinkage long our_fake_access_function(const char __user * filename, int mode)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		sprintf(argument, "arg1(filename): %s", filename);
		log_action(current->pid, tv, __NR_access, argument);
		sprintf(argument, "arg2(mode): %d", mode);
		log_action(current->pid, tv, __NR_access, argument);
	}
	return original_sys_access(filename,mode);
}

/** readlink **/

asmlinkage long (*original_sys_readlink)(const char __user * path, char __user * buf, int bufsiz);

asmlinkage long our_fake_readlink_function(const char __user * path, char __user * buf, int bufsiz)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		sprintf(argument, "arg1(path): %s", path);
		log_action(current->pid, tv, __NR_readlink, argument);
		sprintf(argument, "arg3(bufsiz): %d", bufsiz);
		log_action(current->pid, tv, __NR_readlink, argument);
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
	char argument[256];
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		sprintf(argument, "arg1(addr): %lu", addr);
		log_action(current->pid, tv, __NR_mmap, argument);
		sprintf(argument, "arg2(len): %lu", len);
		log_action(current->pid, tv, __NR_mmap, argument);
		sprintf(argument, "arg3(prot): %lu", prot);
		log_action(current->pid, tv, __NR_mmap, argument);
		sprintf(argument, "arg4(flags): %lu", flags);
		log_action(current->pid, tv, __NR_mmap, argument);
		sprintf(argument, "arg5(fd): %lu", fd);
		log_action(current->pid, tv, __NR_mmap, argument);
		sprintf(argument, "arg6(offset): %lu", offset);
		log_action(current->pid, tv, __NR_mmap, argument);
	}
	return original_old_mmap(addr, len, prot, flags, fd, offset);	
}

/** ioperm **/

asmlinkage long (*original_sys_ioperm) (unsigned long from, unsigned long num, int turn_on);

asmlinkage long our_fake_ioperm_function(unsigned long from, unsigned long num, int turn_on)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);// = current_kernel_time();
	// try to get the current user id, timestamp and filename
	if (current->uid) {
		sprintf(argument, "arg1(from): %lu", from);
		log_action(current->pid, tv, __NR_ioperm, argument);
		sprintf(argument, "arg2(num): %lu", num);
		log_action(current->pid, tv, __NR_ioperm, argument);
		sprintf(argument, "arg1(turn_on): %d", turn_on);
		log_action(current->pid, tv, __NR_ioperm, argument);
	}
	return original_sys_ioperm(from,num,turn_on);
}

/** setfsuid **/

asmlinkage int (*original_sys_setfsuid)(uid_t fsuid); 
	
asmlinkage int our_fake_setfsuid_function(uid_t fsuid)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);
	if (current->uid) {
		sprintf(argument, "arg1(fsuid): %u", fsuid);
		log_action(current->pid, tv, __NR_setfsuid, argument);
	}
	
	return original_sys_setfsuid(fsuid);
}

/** readv **/

asmlinkage ssize_t (*original_sys_readv)(int fd, const struct iovec *iov, int iovcnt); 
	
asmlinkage ssize_t our_fake_readv_function(int fd, const struct iovec *iov, int iovcnt)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);
	if (current->uid) {
		sprintf(argument, "arg1(fd): %d", fd);
		log_action(current->pid, tv, __NR_readv, argument);
		sprintf(argument, "arg1(iovcnt): %d", iovcnt);
		log_action(current->pid, tv, __NR_readv, argument);
	}
	
	return original_sys_readv(fd, iov, iovcnt); 
}

/** fsync **/

asmlinkage int (*original_sys_fsync)(int fd); 
	
asmlinkage int our_fake_fsync_function(int fd)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);
	if (current->uid) {
		sprintf(argument, "arg1(fd): %d", fd);
		log_action(current->pid, tv, __NR_fsync, argument);
	}
	
	return original_sys_fsync(fd); 
}

/** fdatasync **/

asmlinkage int (*original_sys_fdatasync)(unsigned int fd); 
	
asmlinkage int our_fake_fdatasync_function(unsigned int fd)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);
	if (current->uid) {
		sprintf(argument, "arg1(fd): %u", fd);
		log_action(current->pid, tv, __NR_fdatasync, argument);
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
	char argument[256];
	do_gettimeofday(&tv);
	if (current->uid) {
		sprintf(argument, "arg1(addr): %lu", addr);
		log_action(current->pid, tv, __NR_mremap, argument);
		sprintf(argument, "arg2(old_len): %lu", old_len);
		log_action(current->pid, tv, __NR_mremap, argument);
		sprintf(argument, "arg3(new_len): %lu", new_len);
		log_action(current->pid, tv, __NR_mremap, argument);
		sprintf(argument, "arg4(flags): %lu", flags);
		log_action(current->pid, tv, __NR_mremap, argument);
		sprintf(argument, "arg5(new_addr): %lu", new_addr);
		log_action(current->pid, tv, __NR_mremap, argument);
	}
	
	return original_sys_mremap(addr,old_len,new_len,flags,new_addr); 
}

/** setresuid **/

asmlinkage long (*original_sys_setresuid)(uid_t ruid, uid_t euid, uid_t suid);
	
asmlinkage long our_fake_setresuid_function(uid_t ruid, uid_t euid, uid_t suid)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);
	if (current->uid) {
		sprintf(argument, "arg1(ruid): %u", ruid);
		log_action(current->pid, tv, __NR_setresuid, argument);
		sprintf(argument, "arg2(euid): %u", euid);
		log_action(current->pid, tv, __NR_setresuid, argument);
		sprintf(argument, "arg3(suid): %u", suid);
		log_action(current->pid, tv, __NR_setresuid, argument);
	}
	
	return original_sys_setresuid(ruid, euid, suid); 
}

/** pread **/

asmlinkage ssize_t (*original_sys_pread)(unsigned int fd, char __user *buf, size_t count, loff_t pos);
	
asmlinkage ssize_t our_fake_pread_function(unsigned int fd, char __user *buf, size_t count, loff_t pos)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);
	if (current->uid) {
		sprintf(argument, "arg1(fd): %u", fd);
		log_action(current->pid, tv, __NR_pread64, argument);
		sprintf(argument, "arg3(pos): %lld", pos);
		log_action(current->pid, tv, __NR_pread64, argument);
	}
	
	return original_sys_pread(fd, buf, count, pos); 
}

/** setregid **/

asmlinkage int (*original_sys_setregid) (gid_t rgid, gid_t egid);
	
asmlinkage int our_fake_setregid_function(gid_t rgid, gid_t egid)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);
	if (current->uid) {
		sprintf(argument, "arg1(rgid): %u", rgid);
		log_action(current->pid, tv, __NR_setregid, argument);
		sprintf(argument, "arg1(egid): %u", egid);
		log_action(current->pid, tv, __NR_setregid, argument);
	}
	
	return original_sys_setregid(rgid, egid);
}

/** setreuid **/

asmlinkage int (*original_sys_setreuid) (uid_t ruid, uid_t euid);
	
asmlinkage int our_fake_setreuid_function(uid_t ruid, uid_t euid)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);
	if (current->uid) {
		sprintf(argument, "arg1(ruid): %u", ruid);
		log_action(current->pid, tv, __NR_setreuid, argument);
		sprintf(argument, "arg1(euid): %u", euid);
		log_action(current->pid, tv, __NR_setreuid, argument);
	}
	
	return original_sys_setreuid(ruid,euid);
}

/** setuid **/

asmlinkage int (*original_sys_setuid)(uid_t uid);
	
asmlinkage int our_fake_setuid_function(uid_t uid)
{
	struct timeval tv;
	char argument[256];
	do_gettimeofday(&tv);
	if (current->uid) {
		sprintf(argument, "arg1(uid): %u", uid);
		log_action(current->pid, tv, __NR_setuid, argument);
	}
	
	return original_sys_setuid(uid);
}

/** vfork **/

asmlinkage pid_t (*original_sys_vfork)(struct pt_regs regs);

asmlinkage pid_t our_fake_vfork_function(struct pt_regs regs)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->pid, tv, __NR_vfork, "arg1(regs)");
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
	//unsigned long *sys_table2;
	int flag = 0;
	printk(KERN_INFO "%lu\n", simple_strtoul("0xffffffff804fbb80",NULL,16));
	printk(KERN_INFO "%lu\n", simple_strtoul("0xffffffff804ff148",NULL,16));
		
	sys_table = (unsigned long *)aquire_sys_call_table();
	//sys_table = (unsigned long *) simple_strtoul("0xffffffff804fbb80",NULL,16);
	
	if (sys_table) {
		flag = 1;
		printk(KERN_INFO "%lu %lu\n", sys_table[__NR_open], (unsigned long )sys_open);
	}

	if(flag) {
		// link up the loggerFile and set permission
		syslog_file = create_proc_entry(procfs_name, 0400, NULL); // read only file
		if (syslog_file == NULL) {
		        remove_proc_entry(procfs_name, NULL);
		        printk(KERN_INFO "SyscallLog: Error: Could not initialize /proc/%s\n",procfs_name);
		        return -ENOMEM;
		}

		//syslog_file->read_proc = syslog_read;
		syslog_file->proc_fops = &my_file_ops;
		
		// switch sys_call definition
		sys_call_table = sys_table;
		printk(KERN_INFO "SyscallLog: Syscall table found, replacing selected functions with our own...\n");
		disable_page_protection();
		original_sys_fork =(void * )xchg(&(sys_call_table[__NR_fork]), our_fake_fork_function);
		original_sys_read =(void * )xchg(&(sys_call_table[__NR_read]), our_fake_read_function);
		original_sys_open =(void * )xchg(&(sys_call_table[__NR_open]), our_fake_open_function);
		original_sys_creat =(void * )xchg(&(sys_call_table[__NR_creat]), our_fake_creat_function);
		original_sys_execve =(void * )xchg(&(sys_call_table[__NR_execve]), our_fake_execve_function);
		original_sys_mount =(void * )xchg(&(sys_call_table[__NR_mount]), our_fake_mount_function);
		original_sys_access =(void * )xchg(&(sys_call_table[__NR_access]), our_fake_access_function);
		original_sys_readlink =(void * )xchg(&(sys_call_table[__NR_readlink]), our_fake_readlink_function);
		original_old_mmap =(void * )xchg(&(sys_call_table[__NR_mmap]), our_fake_mmap_function);
		original_sys_ioperm =(void * )xchg(&(sys_call_table[__NR_ioperm]), our_fake_ioperm_function);
		//
		original_sys_setuid =(void * )xchg(&(sys_call_table[__NR_setuid]), our_fake_setuid_function);
		original_sys_setreuid =(void * )xchg(&(sys_call_table[__NR_setreuid]), our_fake_setreuid_function);
		original_sys_vfork =(void * )xchg(&(sys_call_table[__NR_vfork]), our_fake_vfork_function);
		original_sys_pread =(void * )xchg(&(sys_call_table[__NR_pread64]), our_fake_pread_function);
		original_sys_setresuid =(void * )xchg(&(sys_call_table[__NR_setresuid]), our_fake_setresuid_function);
		original_sys_mremap =(void * )xchg(&(sys_call_table[__NR_mremap]), our_fake_mremap_function);
		original_sys_fdatasync =(void * )xchg(&(sys_call_table[__NR_fdatasync]), our_fake_fdatasync_function);
		original_sys_fsync=(void * )xchg(&(sys_call_table[__NR_fsync]), our_fake_fsync_function);
		original_sys_readv =(void * )xchg(&(sys_call_table[__NR_readv]), our_fake_readv_function);
		original_sys_setfsuid =(void * )xchg(&(sys_call_table[__NR_setfsuid]), our_fake_setfsuid_function);
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
	int i=0;
	if (replaced) {
		disable_page_protection();
		xchg(&(sys_call_table[__NR_fork]), original_sys_fork);
		xchg(&(sys_call_table[__NR_read]), original_sys_read);
		xchg(&(sys_call_table[__NR_open]), original_sys_open);
		xchg(&(sys_call_table[__NR_creat]), original_sys_creat);
		xchg(&(sys_call_table[__NR_execve]), original_sys_execve);
		xchg(&(sys_call_table[__NR_mount]), original_sys_mount);
		xchg(&(sys_call_table[__NR_access]), original_sys_access);
		xchg(&(sys_call_table[__NR_readlink]), original_sys_readlink);
		xchg(&(sys_call_table[__NR_mmap]), original_old_mmap);
		xchg(&(sys_call_table[__NR_ioperm]), original_sys_ioperm);
		//
		xchg(&(sys_call_table[__NR_setuid]), original_sys_setuid);
		xchg(&(sys_call_table[__NR_setreuid]), original_sys_setreuid);
		xchg(&(sys_call_table[__NR_vfork]), original_sys_vfork);
		xchg(&(sys_call_table[__NR_pread64]), original_sys_pread);
		xchg(&(sys_call_table[__NR_setresuid]), original_sys_setresuid);
		xchg(&(sys_call_table[__NR_mremap]), original_sys_mremap);
		xchg(&(sys_call_table[__NR_fdatasync]), original_sys_fdatasync);
		xchg(&(sys_call_table[__NR_fsync]), original_sys_fsync);
		xchg(&(sys_call_table[__NR_readv]), original_sys_readv);
		xchg(&(sys_call_table[__NR_setfsuid]), original_sys_setfsuid);
		// 			
		enable_page_protection();
		if (msg_head) {
			printk("SyscallLog: You have messages without getting export...\n");
		}
		while (msg_head) {
			// clear the msg buffer first
			i++;
			remove_head_msg();
		}
		printk("SyscallLog: You lose %d messages.\n", i);
		remove_proc_entry(procfs_name, NULL);
	}
	printk(KERN_INFO "SyscallLog: Warning: You have turned off the logger.\n");
}

module_init(logger_init);
module_exit(logger_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("System calls interposer");

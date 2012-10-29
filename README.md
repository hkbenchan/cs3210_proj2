CS3210 Project 2 -- A syscall interposer to detect malicious employee behavior

-------------------------------------

Overview:

As this is to be done via the LKM, we can alter the system call table and insert monitoring command to gather information from the syscall.
Among nearly 300 syscall currently available inside the linux kernel, we have picked the following syscalls (attached below) to monitor as:
(1) Monitoring all syscalls will produce too many noise message, reduce the chance to locate suspicious behavior.
(2) Picked syscalls can monitor most of the possible ways to copy out files, whether valuable or not.

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
mmap2
vfork
pread
setresuid
mremap
fdatasync
fsync
readv
setfsuid

Flow of the LKM:

	Inserting the module (Init):
	(1) Searching through the memory page to locate sys_call_table (as it may different memory addresses on different machine even with the same kernel version)
	(2) Disable memory page protection to overwrite original syscall.
	(3) Exchange original syscall with our own implemented syscall and keep the old one for calling and restoring later.
	(4) Enable memory page protection to prevent others screwing up the memory page. Done.
	
	During any modified syscall is called:
	(1) Users called the syscall and get captured by the system.
	(2) System will switch to kernel mode and interupt.
	(3) System will look up the sys_call_table to locate the corresponding syscall fundtion, which some of them will be our own implementation.
	(4) System called that function and that function will capture the timestamp, user id and arguments for that syscall.
	(5) The function will call original one to continue and return.
	(6) Once finished, system switch back to user mode and continue to run the user porgram.
	
	Removing the module (Exit):
	(1) Check if the table is modified (by a boolean value defined inside the module). If not, done. Otherwise, continue.
	(2) Replaced the all modified syscall with original one (saved before). Done.
	

Cons:
Syscall definition may change when kernel keeps evolving, this module may not be as portability as it should be. But with constant effort on upgrading this module, the effect of this can be minimized.
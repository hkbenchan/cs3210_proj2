/** setfsuid **/

asmlinkage int (*original_sys_setfsuid)(uid_t fsuid); 
	
asmlinkage int *our_fake_setfsuid(uid_t fsuid)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "setfsuid");
	}
	
	return original_sys_setfsuid(fsuid); 
}

/** preadv **/

asmlinkage ssize_t (original_sys_preadv)(int fd, const struct iovec *iov, int iovcnt,
                      off_t offset); 
	
asmlinkage ssize_t *our_fake_preadv(int fd, const struct iovec *iov, int iovcnt,
                      off_t offset)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "preadv");
	}
	
	return original_sys_preadv(fd, iov, iovcnt, offset); 
}

/** readv **/

asmlinkage ssize_t (original_sys_readv)(int fd, const struct iovec *iov, int iovcnt); 
	
asmlinkage ssize_t *our_fake_readv(int fd, const struct iovec *iov, int iovcnt)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "readv");
	}
	
	return original_sys_readv(fd, iov, iovcnt); 
}

/** fsync **/

asmlinkage int (original_sys_fsync)(int fd); 
	
asmlinkage int *our_fake_fsync(int fd)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "fsync");
	}
	
	return original_sys_fsync(fd); 
}

/** fdatasync **/

asmlinkage int (original_sys_fdatasync)(int fd); 
	
asmlinkage int *our_fake_fdatasync(int fd)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "fdatasync");
	}
	
	return original_sys_fdatasync(fd); 
}

/** mremap **/

asmlinkage void * (original_sys_mremap)(void *old_address, size_t old_size,
	size_t new_size, int flags, ... /* void *new_address */);
	
asmlinkage void *our_fake_mremap(void *old_address, size_t old_size,
	size_t new_size, int flags, ... /* void *new_address */)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "mremap");
	}
	
	return original_sys_mremap(old_address, old_size); 
}

/** setresuid **/

asmlinkage int (*original_sys_setresuid)(uid_t ruid, uid_t euid, uid_t suid);
	
asmlinkage int our_fake_setresuid(uid_t ruid, uid_t euid, uid_t suid)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "setresuid");
	}
	
	return original_sys_setresuid(ruid, euid, suid); 
}

/** pread **/

asmlinkage ssize_t (*original_sys_pread)(int fd, void *buf, size_t count, off_t offset);
	
asmlinkage ssize_t our_fake_pread(int fd, void *buf, size_t count, off_t offset)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "pread");
	}
	
	return original_sys_pread(fd, *buf, count, offset); 
}

/** setregid **/

asmlinkage int (*original_sys_setregid) (gid_t rgid, gid_t egid);
	
asmlinkage int our_fake_setreuid(gid_t rgid, gid_t egid)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "setregid");
	}
	
	return original_sys_setregid(gid);
}

/** setreuid **/

asmlinkage int (*original_sys_setreuid) (uid_t ruid, uid_t euid);
	
asmlinkage int our_fake_setreuid(uid_t ruid, uid_t euid)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "setreuid");
	}
	
	return original_sys_setreuid(uid);
}

/** setuid **/

asmlinkage int (*original_sys_setuid)(uid_t uid);
	
asmlinkage int our_fake_setuid (uid_t uid)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "setuid");
	}
	
	return original_sys_setuid(uid);
}

/** mmap2 **/

asmlinkage  void * (*orginal_sys_mmap2) (void *start, size_t length, int prot, int flags, int fd, off_t pgoffset);

asmlinkage void * our_fake_mmap2 (void *start, size_t length, int prot, int flags, int fd, off_t pgoffset)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "mmap2");
	}
	
	return original_sys_mmap2(start, length, prot, flags, fd, pgoffset);
}

/** vfork **/

asmlinkage pid_t (*original_sys_vfork)(void);

asmlinkage pid_t our_fake_vfork(void)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	if (current->uid) {
		log_action(current->uid, tv, "vfork");
	}
	
	return original_sys_vfork(void);
}

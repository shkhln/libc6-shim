# encoding: utf-8

def split_prototype(prototype)

  parts = []

  depth = 0
  decl  = ''

  prototype.gsub(/\/\*.*?\*\//, '').chars.each_with_index do |char, i|
    case char
      when '('
        if depth == 0
          parts << decl
          decl = ''
        else
          decl += '('
        end
        depth += 1
      when ')'
        depth -= 1
        if depth == 0
          parts << decl
        else
          decl += ')'
        end
      when ','
        if depth == 1
          parts << decl
          decl = ''
        else
          decl += ','
        end
      when /[\w\s\*\^\[\]\.\+]/i
        decl += char
      else
        if char.ord == 0xa0
          decl += ' '
        else
          raise "Unexpected character '#{char}' (0x#{'%x' % char.ord}) at pos #{i + 1} in #{prototype.inspect}"
        end
    end
  end

  parts.map(&:strip)
end

FUNCTION_POINTER_DECL = /^(.+)?\([\*\^](\w+)\)\s*(\([^\)]+\))$/

def parse_prototype(prototype)

  fn_type_name, *args = split_prototype(prototype).select{|p| !p.empty?}.to_enum.with_index.map do |decl, i|
    name = nil
    type = nil

    case decl
      when /\*$/, /\*\srestrict$/
        name = "_arg_#{i}"
        type = decl
      when 'void'
        type = 'void'
      when '...'
        name = '...'
      when /^[^\s]+$/
        name = "_arg_#{i}"
        type = decl
      when FUNCTION_POINTER_DECL
        name = $2
        type = $1 + '(*)' + $3
      when /^(.+?)(\w+)$/
        name = $2
        type = $1.strip
      when /^(.+?)(\w+)(\[(\d*|restrict)\])$/
        name = $2
        type = $1 + $3
      else
        raise "Unknown decl #{decl.inspect} in #{prototype.inspect}"
    end

    {name: name, type: type}
  end

  {prototype: prototype, name: fn_type_name[:name], type: fn_type_name[:type], args: args}
end

$functions = {}

def define(headers, prototypes, lsb = false)
  for prototype in prototypes
    fn = parse_prototype(prototype)
    raise "Duplicate definition #{prototype.inspect}" if $functions[fn[:name]]
    fn[:includes] = headers.map{|h| '#include <' + h + '>'}
    fn[:lsb]      = lsb
    $functions[fn[:name]] = fn
  end
end

def lsb_define(headers, prototypes)
  define(headers, prototypes, true)
end

# SYSCALL(2)
define(["sys/syscall.h", "unistd.h"], [
  "long syscall(long number, ...)"
])

# EXIT(2)
define(["unistd.h"], [
  "void _exit(int status)"
])

# ACCEPT(2)
define(["sys/types.h", "sys/socket.h"], [
  "int accept(int s, struct sockaddr* restrict addr, socklen_t* restrict addrlen)",
  "int accept4(int s, struct sockaddr* restrict addr, socklen_t* restrict addrlen, int flags)"
])

# ACCESS(2)
define(["unistd.h"], [
  "int access(const char* path, int mode)",
  "int eaccess(const char* path, int mode)",
  "int faccessat(int fd, const char* path, int mode, int flag)"
])

# ACCT(2)
define(["unistd.h"], [
  "int acct(const char* file)"
])

# ADJTIME(2)
define(["sys/time.h"], [
  "int adjtime(const struct timeval* delta, struct timeval* olddelta)"
])

# AIO_CANCEL(2)
define(["aio.h"], [
  "int aio_cancel(int fildes, struct aiocb* iocb)"
])

# AIO_ERROR(2)
define(["aio.h"], [
  "int aio_error(const struct aiocb* iocb)"
])

# AIO_FSYNC(2)
define(["aio.h"], [
  "int aio_fsync(int op, struct aiocb* iocb)"
])

# AIO_READ(2)
define(["aio.h"], [
  "int aio_read(struct aiocb* iocb)"
])

# AIO_RETURN(2)
define(["aio.h"], [
  "ssize_t aio_return(struct aiocb* iocb)"
])

# AIO_SUSPEND(2)
define(["aio.h"], [
  "int aio_suspend(const struct aiocb* const iocbs[], int niocb, const struct timespec* timeout)"
])

# AIO_WRITE(2)
define(["aio.h"], [
  "int aio_write(struct aiocb* iocb)"
])

# BASENAME(3)
define(["libgen.h"], [
  "char* basename(const char* path)"
])

# BIND(2)
define(["sys/types.h", "sys/socket.h"], [
  "int bind(int s, const struct sockaddr* addr, socklen_t addrlen)"
])

# BRK(2)
define(["unistd.h"], [
  "int brk(void* addr)",
  "void* sbrk(intptr_t incr)"
])

# CHDIR(2)
define(["unistd.h"], [
  "int chdir(const char* path)",
  "int fchdir(int fd)"
])

# CHFLAGS(2)
define(["sys/stat.h", "unistd.h"], [
  "int chflags(const char* path, unsigned long flags)",
  "int fchflags(int fd, unsigned long flags)"
])

# CHMOD(2)
define(["sys/stat.h"], [
  "int chmod(const char* path, mode_t mode)",
  "int fchmod(int fd, mode_t mode)",
  "int lchmod(const char* path, mode_t mode)",
  "int fchmodat(int fd, const char* path, mode_t mode, int flag)"
])

# CHOWN(2)
define(["unistd.h"], [
  "int chown(const char* path, uid_t owner, gid_t group)",
  "int fchown(int fd, uid_t owner, gid_t group)",
  "int lchown(const char* path, uid_t owner, gid_t group)",
  "int fchownat(int fd, const char* path, uid_t owner, gid_t group, int flag)"
])

# CHROOT(2)
define(["unistd.h"], [
  "int chroot(const char* dirname)"
])

# CLOCK_GETTIME(2)
define(["time.h"], [
  "int clock_gettime(clockid_t clock_id, struct timespec* tp)",
  "int clock_settime(clockid_t clock_id, const struct timespec* tp)",
  "int clock_getres(clockid_t clock_id, struct timespec* tp)"
])

# NANOSLEEP(2)
define(["time.h"], [
  "int clock_nanosleep(clockid_t clock_id, int flags, const struct timespec* rqtp, struct timespec* rmtp)",
  "int nanosleep(const struct timespec* rqtp, struct timespec* rmtp)"
])

# CLOSE(2)
define(["unistd.h"], [
  "int close(int fd)"
])

# CONNECT(2)
define(["sys/types.h", "sys/socket.h"], [
  "int connect(int s, const struct sockaddr* name, socklen_t namelen)"
])

# CREAT(2)
define(["fcntl.h"], [
  "int creat(const char* path, mode_t mode)"
])

# DUP(2)
define(["unistd.h"], [
  "int dup(int oldd)",
  "int dup2(int oldd, int newd)"
])

# EXECVE(2)
define(["unistd.h"], [
  "int execve(const char* path, char* const argv[], char* const envp[])",
  "int fexecve(int fd, char* const argv[], char* const envp[])"
])

# FCNTL(2)
define(["fcntl.h"], [
  "int fcntl(int fd, int cmd, ...)"
])

# FSYNC(2)
define(["unistd.h"], [
  "int fdatasync(int fd)",
  "int fsync(int fd)"
])

# FLOCK(2)
define(["sys/file.h"], [
  "int flock(int fd, int operation)"
])

# FORK(2)
define(["unistd.h"], [
  "pid_t fork(void)"
])

# PATHCONF(2)
define(["unistd.h"], [
  "long pathconf(const char* path, int name)",
  "long fpathconf(int fd, int name)"
])

# STATFS(2)
define(["sys/param.h", "sys/mount.h", ("sys/statfs.h" if LINUX)].compact, [
  "int statfs(const char* path, struct statfs* buf)",
  "int fstatfs(int fd, struct statfs* buf)"
])

# TRUNCATE(2)
define(["unistd.h"], [
  "int truncate(const char* path, off_t length)",
  "int ftruncate(int fd, off_t length)"
])

# UTIMENSAT(2)
define(["sys/stat.h"], [
  "int futimens(int fd, const struct timespec times[2])",
  "int utimensat(int fd, const char* path, const struct timespec times[2], int flag)"
])

# UTIMES(2)
define(["sys/time.h"], [
  "int utimes(const char* path, const struct timeval* times)",
  "int lutimes(const char* path, const struct timeval* times)",
  "int futimes(int fd, const struct timeval* times)",
  "int futimesat(int fd, const char* path, const struct timeval times[2])"
])

# GETDIRENTRIES(2)
define(["sys/types.h", "dirent.h"], [
  #~ "int getdirentries(int fd, char* buf, int nbytes, long* basep)"
])

# GETDTABLESIZE(2)
define(["unistd.h"], [
  "int getdtablesize(void)"
])

# GETGID(2)
define(["unistd.h"], [
  "gid_t getgid(void)",
  "gid_t getegid(void)"
])

# GETUID(2)
define(["unistd.h"], [
  "uid_t getuid(void)",
  "uid_t geteuid(void)"
])

# GETGROUPS(2)
define(["unistd.h"], [
  "int getgroups(int gidsetlen, gid_t* gidset)"
])

# GETITIMER(2)
define(["sys/time.h"], [
  "int getitimer(int which, struct itimerval* value)",
  "int setitimer(int which, const struct itimerval* value, struct itimerval* ovalue)"
])

# GETLOGIN(2)
define(["unistd.h", "sys/param.h"], [
  "char* getlogin(void)",
  "int getlogin_r(char* name, size_t len)",
  "int setlogin(const char* name)"
])

# GETPEERNAME(2)
define(["sys/types.h", "sys/socket.h"], [
  "int getpeername(int s, struct sockaddr* restrict name, socklen_t* restrict namelen)"
])

# GETPGRP(2)
define(["unistd.h"], [
  "pid_t getpgrp(void)",
  "pid_t getpgid(pid_t pid)"
])

# GETPID(2)
define(["unistd.h"], [
  "pid_t getpid(void)",
  "pid_t getppid(void)"
])

# GETPRIORITY(2)
define(["sys/time.h", "sys/resource.h"], [
  "int getpriority(int which, int who)",
  "int setpriority(int which, int who, int prio)"
])

# SETRESUID(2)
define(["sys/types.h", "unistd.h"], [
  "int getresgid(gid_t* rgid, gid_t* egid, gid_t* sgid)",
  "int getresuid(uid_t* ruid, uid_t* euid, uid_t* suid)",
  "int setresgid(gid_t rgid, gid_t egid, gid_t sgid)",
  "int setresuid(uid_t ruid, uid_t euid, uid_t suid)"
])

# GETRLIMIT(2)
define(["sys/types.h", "sys/time.h", "sys/resource.h"], [
  "int getrlimit(int resource, struct rlimit* rlp)",
  "int setrlimit(int resource, const struct rlimit* rlp)"
])

# GETRUSAGE(2)
define(["sys/types.h", "sys/time.h", "sys/resource.h"], [
  "int getrusage(int who, struct rusage* rusage)"
])

# GETSID(2)
define(["unistd.h"], [
  "pid_t getsid(pid_t pid)"
])

# GETSOCKNAME(2)
define(["sys/types.h", "sys/socket.h"], [
  "int getsockname(int s, struct sockaddr* restrict name, socklen_t* restrict namelen)"
])

# GETSOCKOPT(2)
define(["sys/types.h", "sys/socket.h"], [
  "int getsockopt(int s, int level, int optname, void* restrict optval, socklen_t* restrict optlen)",
  "int setsockopt(int s, int level, int optname, const void* optval, socklen_t optlen)"
])

# GETTIMEOFDAY(2)
define(["sys/time.h"], [
  "int gettimeofday(struct timeval* tp, struct timezone* tzp)",
  "int settimeofday(const struct timeval* tp, const struct timezone* tzp)"
])

# IOCTL(2)
define(["sys/ioctl.h"], [
  "int ioctl(int fd, unsigned long request, ...)"
])

# KILL(2)
define(["sys/types.h", "signal.h"], [
  "int kill(pid_t pid, int sig)"
])

# KILLPG(2)
define(["sys/types.h", "signal.h"], [
  "int killpg(pid_t pgrp, int sig)"
])

# LINK(2)
define(["unistd.h"], [
  "int link(const char* name1, const char* name2)",
  "int linkat(int fd1, const char* name1, int fd2, const char* name2, int flag)"
])

# LIO_LISTIO(2)
define(["aio.h"], [
  "int lio_listio(int mode, struct aiocb* const list[], int nent, struct sigevent* sig)"
])

# LISTEN(2)
define(["sys/types.h", "sys/socket.h"], [
  "int listen(int s, int backlog)"
])

# LSEEK(2)
define(["unistd.h"], [
  "off_t lseek(int fildes, off_t offset, int whence)"
])

# MADVISE(2)
define(["sys/mman.h"], [
  "int madvise(void* addr, size_t len, int behav)",
  "int posix_madvise(void* addr, size_t len, int behav)"
])

# MINCORE(2)
define(["sys/mman.h"], [
  #~ "int mincore(void* addr, size_t len, unsigned char* vec)"
])

# MKDIR(2)
define(["sys/stat.h"], [
  "int mkdir(const char* path, mode_t mode)",
  "int mkdirat(int fd, const char* path, mode_t mode)"
])

# MKFIFO(2)
define(["sys/types.h", "sys/stat.h"], [
  "int mkfifo(const char* path, mode_t mode)",
  "int mkfifoat(int fd, const char* path, mode_t mode)"
])

# MLOCK(2)
define(["sys/mman.h"], [
  "int mlock(const void* addr, size_t len)",
  "int munlock(const void* addr, size_t len)"
])

# MLOCKALL(2)
define(["sys/mman.h"], [
  "int mlockall(int flags)",
  "int munlockall(void)"
])

# MMAP(2)
define(["sys/mman.h"], [
  "void* mmap(void* addr, size_t len, int prot, int flags, int fd, off_t offset)"
])

# MOUNT(2)
define(["sys/param.h", "sys/mount.h"], [
  #~ "int mount(const char* type, const char* dir, int flags, void* data)"
])

# MPROTECT(2)
define(["sys/mman.h"], [
  "int mprotect(void* addr, size_t len, int prot)"
])

# MSGCTL(2)
define(["sys/types.h", "sys/ipc.h", "sys/msg.h"], [
  "int msgctl(int msqid, int cmd, struct msqid_ds* buf)"
])

# MSGGET(2)
define(["sys/msg.h"], [
  "int msgget(key_t key, int msgflg)"
])

# MSGRCV(2)
define(["sys/types.h", "sys/ipc.h", "sys/msg.h"], [
  "ssize_t msgrcv(int msqid, void* msgp, size_t msgsz, long msgtyp, int msgflg)"
])

# MSGSND(2)
define(["sys/types.h", "sys/ipc.h", "sys/msg.h"], [
  "int msgsnd(int msqid, const void* msgp, size_t msgsz, int msgflg)"
])

# MSYNC(2)
define(["sys/mman.h"], [
  "int msync(void* addr, size_t len, int flags)"
])

# MUNMAP(2)
define(["sys/mman.h"], [
  "int munmap(void* addr, size_t len)"
])

# OPEN(2)
define(["fcntl.h"], [
  "int open(const char* path, int flags, ...)",
  #~ "int openat(int fd, const char* path, int flags, ...)"
])

# PIPE(2)
define(["unistd.h"], [
  "int pipe(int fildes[2])",
  "int pipe2(int fildes[2], int flags)"
])

# POLL(2)
define(["poll.h"], [
  "int poll(struct pollfd fds[], nfds_t nfds, int timeout)",
  "int ppoll(struct pollfd fds[], nfds_t nfds, const struct timespec* restrict timeout, const sigset_t* restrict newsigmask)"
])

# POSIX_FADVISE(2)
define(["fcntl.h"], [
  "int posix_fadvise(int fd, off_t offset, off_t len, int advice)"
])

# POSIX_FALLOCATE(2)
define(["fcntl.h"], [
  "int posix_fallocate(int fd, off_t offset, off_t len)"
])

# POSIX_OPENPT(2)
define(["stdlib.h", "fcntl.h"], [
  "int posix_openpt(int oflag)"
])

# READ(2)
define(["unistd.h", "sys/uio.h"], [
  "ssize_t read(int fd, void* buf, size_t nbytes)",
  "ssize_t pread(int fd, void* buf, size_t nbytes, off_t offset)",
  "ssize_t readv(int fd, const struct iovec* iov, int iovcnt)",
  "ssize_t preadv(int fd, const struct iovec* iov, int iovcnt, off_t offset)"
])

# PROFIL(2)
define(["unistd.h"], [
  "int profil(char* samples, size_t size, vm_offset_t offset, int scale)"
])

# PSELECT(2)
define(["sys/select.h"], [
  "int pselect(int nfds, fd_set* restrict readfds, fd_set* restrict writefds, fd_set* restrict exceptfds, const struct timespec* restrict timeout, const sigset_t* restrict newsigmask)"
])

# PTRACE(2)
define(["sys/types.h", "sys/ptrace.h"], [
  #~ "int ptrace(int request, pid_t pid, caddr_t addr, int data)"
])

# WRITE(2)
define(["unistd.h", "sys/uio.h"], [
  "ssize_t write(int fd, const void* buf, size_t nbytes)",
  "ssize_t pwrite(int fd, const void* buf, size_t nbytes, off_t offset)",
  "ssize_t writev(int fd, const struct iovec* iov, int iovcnt)",
  "ssize_t pwritev(int fd, const struct iovec* iov, int iovcnt, off_t offset)"
])

# QUOTACTL(2)
define(["sys/types.h", ("ufs/ufs/quota.h" if not LINUX)].compact, [
  "int quotactl(const char* path, int cmd, int id, void* addr)"
])

# READLINK(2)
define(["unistd.h"], [
  "ssize_t readlink(const char* restrict path, char* restrict buf, size_t bufsiz)",
  "ssize_t readlinkat(int fd, const char* restrict path, char* restrict buf, size_t bufsize)"
])

# REBOOT(2)
define(["unistd.h", "sys/reboot.h"], [
  "int reboot(int howto)"
])

# RECV(2)
define(["sys/types.h", "sys/socket.h"], [
  "ssize_t recv(int s, void* buf, size_t len, int flags)",
  "ssize_t recvfrom(int s, void* buf, size_t len, int flags, struct sockaddr* restrict from, socklen_t* restrict fromlen)",
  "ssize_t recvmsg(int s, struct msghdr* msg, int flags)",
  "int recvmmsg(int s, struct mmsghdr* msgvec, unsigned int vlen, int flags, const struct timespec* timeout)"
])

# RENAME(2)
define(["stdio.h"], [
  "int rename(const char* from, const char* to)",
  "int renameat(int fromfd, const char* from, int tofd, const char* to)"
])

# REVOKE(2)
define(["unistd.h"], [
  "int revoke(const char* path)"
])

# RMDIR(2)
define(["unistd.h"], [
  "int rmdir(const char* path)"
])

# SCHED_GET_PRIORITY_MAX(2)
define(["sched.h"], [
  "int sched_get_priority_max(int policy)",
  "int sched_get_priority_min(int policy)",
  "int sched_rr_get_interval(pid_t pid, struct timespec* interval)"
])

# SCHED_SETPARAM(2)
define(["sched.h"], [
  "int sched_setparam(pid_t pid, const struct sched_param* param)",
  "int sched_getparam(pid_t pid, struct sched_param* param)"
])

# SCHED_SETSCHEDULER(2)
define(["sched.h"], [
  "int sched_setscheduler(pid_t pid, int policy, const struct sched_param* param)",
  "int sched_getscheduler(pid_t pid)"
])

# SCHED_YIELD(2)
define(["sched.h"], [
  "int sched_yield(void)"
])

# SELECT(2)
define(["sys/select.h"], [
  "int select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout)"
])

# SEMCTL(2)
define(["sys/types.h", "sys/ipc.h", "sys/sem.h"], [
  "int semctl(int semid, int semnum, int cmd, ...)"
])

# SEMGET(2)
define(["sys/sem.h"], [
  "int semget(key_t key, int nsems, int flag)"
])

# SEMOP(2)
define(["sys/types.h", "sys/ipc.h", "sys/sem.h"], [
  "int semop(int semid, struct sembuf* array, size_t nops)"
])

# SEND(2)
define(["sys/types.h", "sys/socket.h"], [
  "ssize_t send(int s, const void* msg, size_t len, int flags)",
  "ssize_t sendto(int s, const void* msg, size_t len, int flags, const struct sockaddr* to, socklen_t tolen)",
  "ssize_t sendmsg(int s, const struct msghdr* msg, int flags)",
  "int sendmmsg(int s, struct mmsghdr* restrict msgvec, unsigned int vlen, int flags)"
])

# SENDFILE(2)
define(["sys/types.h", "sys/socket.h", "sys/uio.h"], [
  #~ "int sendfile(int fd, int s, off_t offset, size_t nbytes, struct sf_hdtr* hdtr, off_t* sbytes, int flags)"
])

# SETUID(2)
define(["unistd.h"], [
  "int setuid(uid_t uid)",
  "int seteuid(uid_t euid)",
  "int setgid(gid_t gid)",
  "int setegid(gid_t egid)"
])

# SETGROUPS(2)
define(["sys/param.h", "unistd.h"], [
  "int setgroups(size_t ngroups, const gid_t* gidset)"
])

# SETPGID(2)
define(["unistd.h"], [
  "int setpgid(pid_t pid, pid_t pgrp)",
  #~ "int setpgrp(pid_t pid, pid_t pgrp)"
])

# SETREGID(2)
define(["unistd.h"], [
  "int setregid(gid_t rgid, gid_t egid)"
])

# SETREUID(2)
define(["unistd.h"], [
  "int setreuid(uid_t ruid, uid_t euid)"
])

# SETSID(2)
define(["unistd.h"], [
  "pid_t setsid(void)"
])

# SHM_OPEN(2)
define(["sys/types.h", "sys/mman.h", "fcntl.h"], [
  "int shm_open(const char* path, int flags, mode_t mode)",
  "int shm_unlink(const char* path)"
])

# SHMAT(2)
define(["sys/types.h", "sys/ipc.h", "sys/shm.h"], [
  "void* shmat(int shmid, const void* addr, int flag)",
  "int shmdt(const void* addr)"
])

# SHMCTL(2)
define(["sys/types.h", "sys/ipc.h", "sys/shm.h"], [
  "int shmctl(int shmid, int cmd, struct shmid_ds* buf)"
])

# SHMGET(2)
define(["sys/shm.h"], [
  "int shmget(key_t key, size_t size, int flag)"
])

# SHUTDOWN(2)
define(["sys/types.h", "sys/socket.h"], [
  "int shutdown(int s, int how)"
])

# SIGALTSTACK(2)
define(["signal.h"], [
  "int sigaltstack(const stack_t* restrict ss, stack_t* restrict oss)"
])

# SIGSETMASK(2)
define(["signal.h"], [
  "int sigsetmask(int mask)",
  "int sigblock(int mask)"
])

# SIGPENDING(2)
define(["signal.h"], [
  "int sigpending(sigset_t* set)"
])

# SIGPROCMASK(2)
define(["signal.h"], [
  "int sigprocmask(int how, const sigset_t* restrict set, sigset_t* restrict oset)"
])

# SIGQUEUE(2)
define(["signal.h"], [
  "int sigqueue(pid_t pid, int signo, const union sigval value)"
])

# SIGRETURN(2)
define(["signal.h"], [
  #~ "int sigreturn(const ucontext_t* scp)"
])

# SIGSUSPEND(2)
define(["signal.h"], [
  "int sigsuspend(const sigset_t* sigmask)"
])

# SIGTIMEDWAIT(2)
define(["signal.h"], [
  "int sigtimedwait(const sigset_t* restrict set, siginfo_t* restrict info, const struct timespec* restrict timeout)",
  "int sigwaitinfo(const sigset_t* restrict set, siginfo_t* restrict info)"
])

# SIGWAIT(2)
define(["signal.h"], [
  "int sigwait(const sigset_t* restrict set, int* restrict sig)"
])

# SOCKET(2)
define(["sys/types.h", "sys/socket.h"], [
  "int socket(int domain, int type, int protocol)"
])

# SOCKETPAIR(2)
define(["sys/types.h", "sys/socket.h"], [
  "int socketpair(int domain, int type, int protocol, int* sv)"
])

# SWAPON(2)
define(["unistd.h"], [
  "int swapon(const char* special)",
  "int swapoff(const char* special)"
])

# SYMLINK(2)
define(["unistd.h"], [
  "int symlink(const char* name1, const char* name2)",
  "int symlinkat(const char* name1, int fd, const char* name2)"
])

# SYNC(2)
define(["unistd.h"], [
  "void sync(void)"
])

# UMASK(2)
define(["sys/stat.h"], [
  "mode_t umask(mode_t numask)"
])

# UNLINK(2)
define(["unistd.h"], [
  "int unlink(const char* path)",
  "int unlinkat(int fd, const char* path, int flag)"
])

# VFORK(2)
define(["unistd.h"], [
  "pid_t vfork(void)"
])

# WAIT(2)
define(["sys/types.h", "sys/wait.h", "signal.h", "sys/time.h", "sys/resource.h"], [
  "pid_t wait(int* status)",
  "pid_t waitpid(pid_t wpid, int* status, int options)",
  "int waitid(idtype_t idtype, id_t id, siginfo_t* info, int options)",
  "pid_t wait3(int* status, int options, struct rusage* rusage)",
  "pid_t wait4(pid_t wpid, int* status, int options, struct rusage* rusage)"
])

# EXIT(3)
define(["stdlib.h"], [
  "void exit(int status)",
  "void _Exit(int status)"
])

# ICONV(3)
#~ define(["iconv.h"], [
  #~ "iconv_t iconv_open(const char* dstname, const char* srcname)",
  #~ "int iconv_close(iconv_t cd)",
  #~ "size_t iconv(iconv_t cd, char** restrict src, size_t* restrict srcleft, char** restrict dst, size_t* restrict dstleft)"
#~ ])

# RPC_SVC_CALLS(3)
define(["rpc/rpc.h"], [
  "void svc_exit(void)",
  "void svc_getreq_common(const int fd)",
  "void svc_getreq_poll(struct pollfd* pfdp, const int pollretval)",
  "void svc_getreqset(fd_set* rdfds)",
  "void svc_run(void)",
  "bool_t svc_sendreply(SVCXPRT* xprt, xdrproc_t outproc, void* out)"
])

# SETJMP(3)
define(["setjmp.h"], [
  "void siglongjmp(sigjmp_buf env, int val)",
  "int setjmp(jmp_buf env)",
  "void longjmp(jmp_buf env, int val)",
  "int _setjmp(jmp_buf env)",
  "void _longjmp(jmp_buf env, int val)"
])

# RAND48(3)
define(["stdlib.h"], [
  "double drand48(void)",
  "double erand48(unsigned short xseed[3])",
  "long lrand48(void)",
  "long nrand48(unsigned short xseed[3])",
  "long mrand48(void)",
  "long jrand48(unsigned short xseed[3])",
  "void srand48(long seed)",
  "unsigned short* seed48(unsigned short xseed[3])",
  "void lcong48(unsigned short p[7])"
])

# A64L(3)
define(["stdlib.h"], [
  "long a64l(const char* s)",
  "char* l64a(long int l)"
])

# ABORT(3)
define(["stdlib.h"], [
  "void abort(void)"
])

# ABS(3)
define(["stdlib.h"], [
  "int abs(int j)"
])

# ACOS(3)
define(["math.h"], [
  "double acos(double x)",
  "float acosf(float x)",
  "long double acosl(long double x)"
])

# ACOSH(3)
define(["math.h"], [
  "double acosh(double x)",
  "float acoshf(float x)",
  "long double acoshl(long double x)"
])

# INET(3)
define(["sys/types.h", "sys/socket.h", "netinet/in.h", "arpa/inet.h"], [
  "int inet_aton(const char* cp, struct in_addr* pin)",
  "in_addr_t inet_addr(const char* cp)",
  "in_addr_t inet_network(const char* cp)",
  "char* inet_ntoa(struct in_addr in)",
  "const char* inet_ntop(int af, const void* restrict src, char* restrict dst, socklen_t size)",
  "int inet_pton(int af, const char* restrict src, void* restrict dst)",
  "struct in_addr inet_makeaddr(in_addr_t net, in_addr_t lna)",
  "in_addr_t inet_lnaof(struct in_addr in)",
  "in_addr_t inet_netof(struct in_addr in)"
])

# ALARM(3)
define(["unistd.h"], [
  "unsigned int alarm(unsigned int seconds)"
])

# JEMALLOC(3)
define(["stdlib.h", ("malloc_np.h" if not LINUX)].compact, [
  "void* malloc(size_t size)",
  "void* calloc(size_t number, size_t size)",
  "int posix_memalign(void** ptr, size_t alignment, size_t size)",
  "void* aligned_alloc(size_t alignment, size_t size)",
  "void* realloc(void* ptr, size_t size)",
  "void free(void* ptr)",
  "size_t malloc_usable_size(const void* ptr)"
])

# SCANDIR(3)
define(["dirent.h"], [
  "int scandir(const char* dirname, struct dirent*** namelist, int (*select)(const struct dirent*), int (*compar)(const struct dirent**, const struct dirent**))",
  "int alphasort(const struct dirent** d1, const struct dirent** d2)"
])

# CTIME(3)
define(["time.h"], [
  "char* ctime(const time_t* clock)",
  "double difftime(time_t time1, time_t time0)",
  "char* asctime(const struct tm* tm)",
  "struct tm* localtime(const time_t* clock)",
  "struct tm* gmtime(const time_t* clock)",
  "time_t mktime(struct tm* tm)",
  "time_t timegm(struct tm* tm)",
  "char* ctime_r(const time_t* clock, char* buf)",
  "struct tm* localtime_r(const time_t* clock, struct tm* result)",
  "struct tm* gmtime_r(const time_t* clock, struct tm* result)",
  "char* asctime_r(const struct tm* tm, char* buf)"
])

# ASIN(3)
define(["math.h"], [
  "double asin(double x)",
  "float asinf(float x)",
  "long double asinl(long double x)"
])

# ASINH(3)
define(["math.h"], [
  "double asinh(double x)",
  "float asinhf(float x)",
  "long double asinhl(long double x)"
])

# PRINTF(3)
define(["stdio.h", "stdarg.h"], [
  "int printf(const char* restrict format, ...)",
  "int fprintf(FILE* restrict stream, const char* restrict format, ...)",
  "int sprintf(char* restrict str, const char* restrict format, ...)",
  "int snprintf(char* restrict str, size_t size, const char* restrict format, ...)",
  "int asprintf(char** ret, const char* format, ...)",
  "int dprintf(int fd, const char* restrict format, ...)",
  "int vprintf(const char* restrict format, va_list ap)",
  "int vfprintf(FILE* restrict stream, const char* restrict format, va_list ap)",
  "int vsprintf(char* restrict str, const char* restrict format, va_list ap)",
  "int vsnprintf(char* restrict str, size_t size, const char* restrict format, va_list ap)",
  "int vasprintf(char** ret, const char* format, va_list ap)",
  "int vdprintf(int fd, const char* restrict format, va_list ap)"
])

# ATAN(3)
define(["math.h"], [
  "double atan(double x)",
  "float atanf(float x)",
  "long double atanl(long double x)"
])

# ATAN2(3)
define(["math.h", "complex.h"], [
  "double atan2(double y, double x)",
  "float atan2f(float y, float x)",
  "long double atan2l(long double y, long double x)",
  "double carg(double complex z)",
  "float cargf(float complex z)",
  "long double cargl(long double complex z)"
])

# ATANH(3)
define(["math.h"], [
  "double atanh(double x)",
  "float atanhf(float x)",
  "long double atanhl(long double x)"
])

# ATEXIT(3)
define(["stdlib.h"], [
  "int atexit(void (*function)(void))"
])

# ATOF(3)
define(["stdlib.h"], [
  "double atof(const char* nptr)"
])

# ATOI(3)
define(["stdlib.h"], [
  "int atoi(const char* nptr)"
])

# ATOL(3)
define(["stdlib.h"], [
  "long atol(const char* nptr)",
  "long long atoll(const char* nptr)"
])

# RPC_CLNT_AUTH(3)
define(["rpc/rpc.h"], [
  "AUTH* authnone_create(void)"
])

# BCMP(3)
define(["strings.h"], [
  "int bcmp(const void* b1, const void* b2, size_t len)"
])

# BCOPY(3)
define(["strings.h"], [
  "void bcopy(const void* src, void* dst, size_t len)"
])

# BINDRESVPORT(3)
define(["sys/types.h", "rpc/rpc.h"], [
  "int bindresvport(int sd, struct sockaddr_in* sin)"
])

# BSEARCH(3)
define(["stdlib.h"], [
  "void* bsearch(const void* key, const void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*))"
])

# BTOWC(3)
define(["wchar.h"], [
  "wint_t btowc(int c)",
  "int wctob(wint_t c)"
])

# BYTEORDER(3)
define(["arpa/inet.h", "netinet/in.h"], [
  "uint32_t htonl(uint32_t hostlong)",
  "uint16_t htons(uint16_t hostshort)",
  "uint32_t ntohl(uint32_t netlong)",
  "uint16_t ntohs(uint16_t netshort)"
])

# BZERO(3)
define(["strings.h"], [
  "void bzero(void* b, size_t len)"
])

# WCRTOMB(3)
define(["wchar.h", "uchar.h"], [
  "size_t wcrtomb(char* restrict s, wchar_t c, mbstate_t* restrict ps)",
  "size_t c16rtomb(char* restrict s, char16_t c, mbstate_t* restrict ps)",
  "size_t c32rtomb(char* restrict s, char32_t c, mbstate_t* restrict ps)"
])

# HYPOT(3)
define(["math.h", "complex.h"], [
  "double hypot(double x, double y)",
  "float hypotf(float x, float y)",
  "long double hypotl(long double x, long double y)",
  "double cabs(double complex z)",
  "float cabsf(float complex z)",
  "long double cabsl(long double complex z)"
])

# CACOS(3)
define(["complex.h"], [
  "double complex cacos(double complex z)",
  "float complex cacosf(float complex z)",
  "long double complex cacosl(long double complex z)",
  "double complex cacosh(double complex z)",
  "float complex cacoshf(float complex z)",
  "long double complex cacoshl(long double complex z)",
  "double complex casin(double complex z)",
  "float complex casinf(float complex z)",
  "long double complex casinl(long double complex z)",
  "double complex casinh(double complex z)",
  "float complex casinhf(float complex z)",
  "long double complex casinhl(long double complex z)",
  "double complex catan(double complex z)",
  "float complex catanf(float complex z)",
  "long double complex catanl(long double complex z)",
  "double complex catanh(double complex z)",
  "float complex catanhf(float complex z)",
  "long double complex catanhl(long double complex z)"
])

# CATCLOSE(3)
define(["nl_types.h"], [
  "int catclose(nl_catd catd)"
])

# CATGETS(3)
define(["nl_types.h"], [
  "char* catgets(nl_catd catd, int set_id, int msg_id, const char* s)"
])

# CATOPEN(3)
define(["nl_types.h"], [
  "nl_catd catopen(const char* name, int oflag)"
])

# DES_CRYPT(3)
define(["rpc/des_crypt.h"], [
  "int ecb_crypt(char* key, char* data, unsigned datalen, unsigned mode)",
  "int cbc_crypt(char* key, char* data, unsigned datalen, unsigned mode, char* ivec)",
  "void des_setparity(char* key)"
])

# SQRT(3)
define(["math.h"], [
  "double cbrt(double x)",
  "float cbrtf(float x)",
  "long double cbrtl(long double x)",
  "double sqrt(double x)",
  "float sqrtf(float x)",
  "long double sqrtl(long double x)"
])

# CCOS(3)
define(["complex.h"], [
  "double complex ccos(double complex z)",
  "float complex ccosf(float complex z)",
  "double complex csin(double complex z)",
  "float complex csinf(float complex z)",
  "double complex ctan(double complex z)",
  "float complex ctanf(float complex z)"
])

# CCOSH(3)
define(["complex.h"], [
  "double complex ccosh(double complex z)",
  "float complex ccoshf(float complex z)",
  "double complex csinh(double complex z)",
  "float complex csinhf(float complex z)",
  "double complex ctanh(double complex z)",
  "float complex ctanhf(float complex z)"
])

# CEIL(3)
define(["math.h"], [
  "double ceil(double x)",
  "float ceilf(float x)",
  "long double ceill(long double x)"
])

# CEXP(3)
define(["complex.h"], [
  "double complex cexp(double complex z)",
  "float complex cexpf(float complex z)"
])

# TCSETATTR(3)
define(["termios.h"], [
  "speed_t cfgetispeed(const struct termios* t)",
  "int cfsetispeed(struct termios* t, speed_t speed)",
  "speed_t cfgetospeed(const struct termios* t)",
  "int cfsetospeed(struct termios* t, speed_t speed)",
  "int cfsetspeed(struct termios* t, speed_t speed)",
  "void cfmakeraw(struct termios* t)",
  "int tcgetattr(int fd, struct termios* t)",
  "int tcsetattr(int fd, int action, const struct termios* t)"
])

# CIMAG(3)
define(["complex.h"], [
  "double cimag(double complex z)",
  "float cimagf(float complex z)",
  "long double cimagl(long double complex z)",
  "double complex conj(double complex z)",
  "float complex conjf(float complex z)",
  "long double complex conjl(long double complex z)",
  "double complex cproj(double complex z)",
  "float complex cprojf(float complex z)",
  "long double complex cprojl(long double complex z)",
  "double creal(double complex z)",
  "float crealf(float complex z)",
  "long double creall(long double complex z)"
])

# FERROR(3)
define(["stdio.h"], [
  "void clearerr(FILE* stream)",
  "void clearerr_unlocked(FILE* stream)",
  "int feof(FILE* stream)",
  "int feof_unlocked(FILE* stream)",
  "int ferror(FILE* stream)",
  "int ferror_unlocked(FILE* stream)",
  "int fileno(FILE* stream)",
  "int fileno_unlocked(FILE* stream)"
])

# RPC_CLNT_CALLS(3)
define(["rpc/rpc.h"], [
  "void clnt_perrno(const enum clnt_stat stat)",
  "void clnt_perror(CLIENT* clnt, const char* s)",
  "char* clnt_sperrno(const enum clnt_stat stat)",
  "char* clnt_sperror(CLIENT* clnt, const char* s)"
])

# RPC_CLNT_CREATE(3)
define(["rpc/rpc.h"], [
  "CLIENT* clnt_create(const char* host, const rpcprog_t prognum, const rpcvers_t versnum, const char* nettype)",
  "void clnt_pcreateerror(const char* s)",
  "char* clnt_spcreateerror(const char* s)"
])

# CLOCK(3)
define(["time.h"], [
  "clock_t clock(void)"
])

# CLOCK_GETCPUCLOCKID(3)
define(["time.h"], [
  "int clock_getcpuclockid(pid_t pid, clockid_t* clock_id)"
])

# DIRECTORY(3)
define(["dirent.h"], [
  "DIR* opendir(const char* filename)",
  "DIR* fdopendir(int fd)",
  "struct dirent* readdir(DIR* dirp)",
  "int readdir_r(DIR* dirp, struct dirent* entry, struct dirent** result)",
  "long telldir(DIR* dirp)",
  "void seekdir(DIR* dirp, long loc)",
  "void rewinddir(DIR* dirp)",
  "int closedir(DIR* dirp)",
  "int dirfd(DIR* dirp)"
])

# SYSLOG(3)
define(["syslog.h", "stdarg.h"], [
  "void syslog(int priority, const char* message, ...)",
  "void vsyslog(int priority, const char* message, va_list args)",
  "void openlog(const char* ident, int logopt, int facility)",
  "void closelog(void)",
  "int setlogmask(int maskpri)"
])

# CONFSTR(3)
define(["unistd.h"], [
  "size_t confstr(int name, char* buf, size_t len)"
])

# COPYSIGN(3)
define(["math.h"], [
  "double copysign(double x, double y)",
  "float copysignf(float x, float y)",
  "long double copysignl(long double x, long double y)"
])

# COS(3)
define(["math.h"], [
  "double cos(double x)",
  "float cosf(float x)",
  "long double cosl(long double x)"
])

# COSH(3)
define(["math.h"], [
  "double cosh(double x)",
  "float coshf(float x)",
  "long double coshl(long double x)"
])

# CSQRT(3)
define(["complex.h"], [
  "double complex csqrt(double complex z)",
  "float complex csqrtf(float complex z)",
  "long double complex csqrtl(long double complex z)"
])

# CTERMID(3)
define(["stdio.h"], [
  "char* ctermid(char* buf)"
])

# CTYPE(3)
define(["ctype.h"], [
  "int isalnum(int c)",
  "int isalpha(int c)",
  "int isascii(int c)",
  "int iscntrl(int c)",
  "int isdigit(int c)",
  "int isgraph(int c)",
  "int islower(int c)",
  "int isprint(int c)",
  "int ispunct(int c)",
  "int isspace(int c)",
  "int isupper(int c)",
  "int isxdigit(int c)",
  "int toascii(int c)",
  "int tolower(int c)",
  "int toupper(int c)"
])

# CTYPE_L(3)
define(["ctype.h"], [
  "int isalnum_l(int c, locale_t loc)",
  "int isalpha_l(int c, locale_t loc)",
  "int iscntrl_l(int c, locale_t loc)",
  "int isdigit_l(int c, locale_t loc)",
  "int isgraph_l(int c, locale_t loc)",
  "int islower_l(int c, locale_t loc)",
  "int isprint_l(int c, locale_t loc)",
  "int ispunct_l(int c, locale_t loc)",
  "int isspace_l(int c, locale_t loc)",
  "int isupper_l(int c, locale_t loc)",
  "int isxdigit_l(int c, locale_t loc)",
  "int tolower_l(int c, locale_t loc)",
  "int toupper_l(int c, locale_t loc)"
])

# DAEMON(3)
define(["stdlib.h"], [
  "int daemon(int nochdir, int noclose)"
])

# DIRNAME(3)
define(["libgen.h"], [
  "char* dirname(char* path)"
])

# DIV(3)
define(["stdlib.h"], [
  "div_t div(int num, int denom)"
])

# DLADDR(3)
define(["dlfcn.h"], [
  "int dladdr(const void* addr, Dl_info* info)"
])

# DLOPEN(3)
define(["dlfcn.h"], [
  "void* dlopen(const char* path, int mode)",
  "void* dlsym(void* restrict handle, const char* restrict symbol)",
  "char* dlerror(void)",
  "int dlclose(void* handle)"
])

# DLINFO(3)
define(["link.h", "dlfcn.h"], [
  "int dlinfo(void* restrict handle, int request, void* restrict p)"
])

# RESOLVER(3)
define(["sys/types.h", "netinet/in.h", "arpa/nameser.h", "resolv.h"], [
  "int res_init(void)"
])

# DUP3(3)
define(["fcntl.h", "unistd.h"], [
  "int dup3(int oldd, int newd, int flags)"
])

# DUPLOCALE(3)
define(["locale.h"], [
  "locale_t duplocale(locale_t locale)"
])

# GETFSENT(3)
define(["fstab.h"], [
  "struct fstab* getfsent(void)",
  "struct fstab* getfsspec(const char* spec)",
  "struct fstab* getfsfile(const char* file)",
  "int setfsent(void)",
  "void endfsent(void)"
])

# GETGRENT(3)
define(["grp.h"], [
  "struct group* getgrent(void)",
  "int getgrent_r(struct group* grp, char* buffer, size_t bufsize, struct group** result)",
  "struct group* getgrnam(const char* name)",
  "int getgrnam_r(const char* name, struct group* grp, char* buffer, size_t bufsize, struct group** result)",
  "struct group* getgrgid(gid_t gid)",
  "int getgrgid_r(gid_t gid, struct group* grp, char* buffer, size_t bufsize, struct group** result)",
  "void setgrent(void)",
  "void endgrent(void)"
])

# GETHOSTBYNAME(3)
define(["netdb.h"], [
  "struct hostent* gethostbyname(const char* name)",
  "struct hostent* gethostbyname2(const char* name, int af)",
  "struct hostent* gethostbyaddr(const void* addr, socklen_t len, int af)",
  "struct hostent* gethostent(void)",
  "void sethostent(int stayopen)",
  "void endhostent(void)",
  "void herror(const char* string)",
  "const char* hstrerror(int err)"
])

# GETNETENT(3)
define(["netdb.h"], [
  "struct netent* getnetent(void)",
  "struct netent* getnetbyname(const char* name)",
  "struct netent* getnetbyaddr(uint32_t net, int type)",
  "void setnetent(int stayopen)",
  "void endnetent(void)"
])

# GETNETGRENT(3)
define(["netdb.h"], [
  "int getnetgrent(char** host, char** user, char** domain)",
  "int getnetgrent_r(char** host, char** user, char** domain, char* buf, size_t bufsize)",
  "int innetgr(const char* netgroup, const char* host, const char* user, const char* domain)",
  #~ "int setnetgrent(const char* netgroup)",
  "void endnetgrent(void)"
])

# GETPROTOENT(3)
define(["netdb.h"], [
  "struct protoent* getprotoent(void)",
  "struct protoent* getprotobyname(const char* name)",
  "struct protoent* getprotobynumber(int proto)",
  "void setprotoent(int stayopen)",
  "void endprotoent(void)"
])

# GETPWENT(3)
define(["sys/types.h", "pwd.h"], [
  "struct passwd* getpwent(void)",
  "int getpwent_r(struct passwd* pwd, char* buffer, size_t bufsize, struct passwd** result)",
  "struct passwd* getpwnam(const char* login)",
  "int getpwnam_r(const char* name, struct passwd* pwd, char* buffer, size_t bufsize, struct passwd** result)",
  "struct passwd* getpwuid(uid_t uid)",
  "int getpwuid_r(uid_t uid, struct passwd* pwd, char* buffer, size_t bufsize, struct passwd** result)",
  "void setpwent(void)",
  "void endpwent(void)"
])

# GETRPCENT(3)
define(["rpc/rpc.h"], [
  "struct rpcent* getrpcent(void)",
  "struct rpcent* getrpcbyname(const char* name)",
  "struct rpcent* getrpcbynumber(int number)",
  "void setrpcent(int stayopen)",
  "void endrpcent(void)"
])

# GETSERVENT(3)
define(["netdb.h"], [
  "struct servent* getservent()",
  "struct servent* getservbyname(const char* name, const char* proto)",
  "struct servent* getservbyport(int port, const char* proto)",
  "void setservent(int stayopen)",
  "void endservent(void)"
])

# GETTTYENT(3)
define(["ttyent.h"], [
  "struct ttyent* getttyent(void)",
  "struct ttyent* getttynam(const char* name)",
  "int setttyent(void)",
  "int endttyent(void)"
])

# GETUSERSHELL(3)
define(["unistd.h"], [
  "char* getusershell(void)",
  "void setusershell(void)",
  "void endusershell(void)"
])

# GETUTXENT(3)
define(["utmpx.h"], [
  "void endutxent(void)",
  "struct utmpx* getutxent(void)",
  "struct utmpx* getutxid(const struct utmpx* id)",
  "struct utmpx* getutxline(const struct utmpx* line)",
  "struct utmpx* pututxline(const struct utmpx* utmpx)",
  "void setutxent(void)"
])

# ERF(3)
define(["math.h"], [
  "double erf(double x)",
  "float erff(float x)",
  "long double erfl(long double x)",
  "double erfc(double x)",
  "float erfcf(float x)",
  "long double erfcl(long double x)"
])

# ERR(3)
define(["err.h", "stdarg.h"], [
  "void err(int eval, const char* fmt, ...)",
  "void errx(int eval, const char* fmt, ...)",
  "void warn(const char* fmt, ...)",
  "void warnx(const char* fmt, ...)",
  "void verr(int eval, const char* fmt, va_list args)",
  "void verrx(int eval, const char* fmt, va_list args)",
  "void vwarn(const char* fmt, va_list args)",
  "void vwarnx(const char* fmt, va_list args)"
])

# ETHERS(3)
define(["sys/types.h", "sys/socket.h", "net/ethernet.h"], [
  "int ether_line(const char* l, struct ether_addr* e, char* hostname)",
  "struct ether_addr* ether_aton(const char* a)",
  "struct ether_addr* ether_aton_r(const char* a, struct ether_addr* e)",
  "char* ether_ntoa(const struct ether_addr* n)",
  "char* ether_ntoa_r(const struct ether_addr* n, char* buf)",
  "int ether_ntohost(char* hostname, const struct ether_addr* e)",
  "int ether_hostton(const char* hostname, struct ether_addr* e)"
])

# EXEC(3)
define(["unistd.h"], [
  #~ "int execl(const char* path, const char* arg, ..., /* , (char* )0,* /)",
  #~ "int execlp(const char* file, const char* arg, ..., /* , (char* )0,* /)",
  #~ "int execle(const char* path, const char* arg, ..., /* (char* )0 char* const envp[]* /)",
  "int execv(const char* path, char* const argv[])",
  "int execvp(const char* file, char* const argv[])"
])

# EXP(3)
define(["math.h"], [
  "double exp(double x)",
  "float expf(float x)",
  "long double expl(long double x)",
  "double exp2(double x)",
  "float exp2f(float x)",
  "long double exp2l(long double x)",
  "double expm1(double x)",
  "float expm1f(float x)",
  "long double expm1l(long double x)",
  "double pow(double x, double y)",
  "float powf(float x, float y)",
  "long double powl(long double x, long double y)"
])

# FABS(3)
define(["math.h"], [
  "double fabs(double x)",
  "float fabsf(float x)",
  "long double fabsl(long double x)"
])

# FCLOSE(3)
define(["stdio.h"], [
  "int fclose(FILE* stream)",
  #~ "int fcloseall(void)"
])

# FDIM(3)
define(["math.h"], [
  "double fdim(double x, double y)",
  "float fdimf(float x, float y)",
  "long double fdiml(long double x, long double y)"
])

# FOPEN(3)
define(["stdio.h"], [
  "FILE* fopen(const char* restrict path, const char* restrict mode)",
  "FILE* fdopen(int fildes, const char* mode)",
  "FILE* freopen(const char* path, const char* mode, FILE* stream)",
  "FILE* fmemopen(void* buf, size_t size, const char* restrict mode)"
])

# FENV(3)
define(["fenv.h"], [
  "int feclearexcept(int excepts)",
  "int fegetexceptflag(fexcept_t* flagp, int excepts)",
  "int feraiseexcept(int excepts)",
  "int fesetexceptflag(const fexcept_t* flagp, int excepts)",
  "int fetestexcept(int excepts)",
  "int fegetround(void)",
  "int fesetround(int round)",
  "int fegetenv(fenv_t* envp)",
  "int feholdexcept(fenv_t* envp)",
  "int fesetenv(const fenv_t* envp)",
  "int feupdateenv(const fenv_t* envp)",
  "int feenableexcept(int excepts)",
  "int fedisableexcept(int excepts)",
  "int fegetexcept(void)"
])

# FFLUSH(3)
define(["stdio.h"], [
  "int fflush(FILE* stream)"
])

# FFS(3)
define(["strings.h"], [
  "int ffs(int value)",
  "int ffsl(long value)",
  "int ffsll(long long value)"
])

# GETC(3)
define(["stdio.h"], [
  "int fgetc(FILE* stream)",
  "int getc(FILE* stream)",
  "int getc_unlocked(FILE* stream)",
  "int getchar(void)",
  "int getchar_unlocked(void)",
  "int getw(FILE* stream)"
])

# FSEEK(3)
define(["stdio.h", "sys/types.h"], [
  "int fseek(FILE* stream, long offset, int whence)",
  "long ftell(FILE* stream)",
  "void rewind(FILE* stream)",
  "int fgetpos(FILE* restrict stream, fpos_t* restrict pos)",
  "int fsetpos(FILE* stream, const fpos_t* pos)",
  "int fseeko(FILE* stream, off_t offset, int whence)",
  "off_t ftello(FILE* stream)"
])

# FGETS(3)
define(["stdio.h"], [
  "char* fgets(char* restrict str, int size, FILE* restrict stream)",
  #~ "char* gets(char* str)"
])

# GETWC(3)
define(["stdio.h", "wchar.h"], [
  "wint_t fgetwc(FILE* stream)",
  "wint_t getwc(FILE* stream)",
  "wint_t getwchar(void)"
])

# FGETWS(3)
define(["stdio.h", "wchar.h"], [
  "wchar_t* fgetws(wchar_t* restrict ws, int n, FILE* restrict fp)"
])

# FLOCKFILE(3)
define(["stdio.h"], [
  "void flockfile(FILE* stream)",
  "int ftrylockfile(FILE* stream)",
  "void funlockfile(FILE* stream)"
])

# FLOOR(3)
define(["math.h"], [
  "double floor(double x)",
  "float floorf(float x)",
  "long double floorl(long double x)"
])

# FMA(3)
define(["math.h"], [
  "double fma(double x, double y, double z)",
  "float fmaf(float x, float y, float z)",
  "long double fmal(long double x, long double y, long double z)"
])

# FMAX(3)
define(["math.h"], [
  "double fmax(double x, double y)",
  "float fmaxf(float x, float y)",
  "long double fmaxl(long double x, long double y)",
  "double fmin(double x, double y)",
  "float fminf(float x, float y)",
  "long double fminl(long double x, long double y)"
])

# FMOD(3)
define(["math.h"], [
  "double fmod(double x, double y)",
  "float fmodf(float x, float y)",
  "long double fmodl(long double x, long double y)"
])

# FMTMSG(3)
define(["fmtmsg.h"], [
  "int fmtmsg(long classification, const char* label, int severity, const char* text, const char* action, const char* tag)"
])

# FNMATCH(3)
define(["fnmatch.h"], [
  "int fnmatch(const char* pattern, const char* string, int flags)"
])

# FOPENCOOKIE(3)
define(["stdio.h"], [
  "FILE* fopencookie(void* cookie, const char* mode, cookie_io_functions_t io_funcs)"
])

# PUTC(3)
define(["stdio.h"], [
  "int fputc(int c, FILE* stream)",
  "int putc(int c, FILE* stream)",
  "int putc_unlocked(int c, FILE* stream)",
  "int putchar(int c)",
  "int putchar_unlocked(int c)",
  "int putw(int w, FILE* stream)"
])

# FPUTS(3)
define(["stdio.h"], [
  "int fputs(const char* str, FILE* stream)",
  "int puts(const char* str)"
])

# PUTWC(3)
define(["stdio.h", "wchar.h"], [
  "wint_t fputwc(wchar_t wc, FILE* stream)",
  "wint_t putwc(wchar_t wc, FILE* stream)",
  "wint_t putwchar(wchar_t wc)"
])

# FPUTWS(3)
define(["stdio.h", "wchar.h"], [
  "int fputws(const wchar_t* restrict ws, FILE* restrict fp)"
])

# FREAD(3)
define(["stdio.h"], [
  "size_t fread(void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream)",
  "size_t fwrite(const void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream)"
])

# FREELOCALE(3)
define(["locale.h"], [
  "void freelocale(locale_t locale)"
])

# FREXP(3)
define(["math.h"], [
  "double frexp(double value, int* exp)",
  "float frexpf(float value, int* exp)",
  "long double frexpl(long double value, int* exp)"
])

# SCANF(3)
define(["stdio.h", "stdarg.h"], [
  "int scanf(const char* restrict format, ...)",
  "int fscanf(FILE* restrict stream, const char* restrict format, ...)",
  "int sscanf(const char* restrict str, const char* restrict format, ...)",
  "int vscanf(const char* restrict format, va_list ap)",
  "int vsscanf(const char* restrict str, const char* restrict format, va_list ap)",
  "int vfscanf(FILE* restrict stream, const char* restrict format, va_list ap)"
])

# STATVFS(3)
define(["sys/statvfs.h"], [
  "int statvfs(const char* restrict path, struct statvfs* restrict buf)",
  "int fstatvfs(int fd, struct statvfs* buf)"
])

# FTOK(3)
define(["sys/types.h", "sys/ipc.h"], [
  "key_t ftok(const char* path, int id)"
])

# FTS(3)
define(["fts.h"], [
  #~ "FTS* fts_open(char* const* path_argv, int options, int (*compar)(const FTSENT**, const FTSENT**))",
  "FTSENT* fts_read(FTS* ftsp)",
  "FTSENT* fts_children(FTS* ftsp, int options)",
  "int fts_set(FTS* ftsp, FTSENT* f, int options)",
  "int fts_close(FTS* ftsp)"
])

# FWIDE(3)
define(["stdio.h", "wchar.h"], [
  "int fwide(FILE* stream, int mode)"
])

# WPRINTF(3)
define(["stdio.h", "wchar.h", "stdarg.h"], [
  "int fwprintf(FILE* restrict stream, const wchar_t* restrict format, ...)",
  "int swprintf(wchar_t* restrict ws, size_t n, const wchar_t* restrict format, ...)",
  "int wprintf(const wchar_t* restrict format, ...)",
  "int vfwprintf(FILE* restrict stream, const wchar_t* restrict, va_list ap)",
  "int vswprintf(wchar_t* restrict ws, size_t n, const wchar_t* restrict format, va_list ap)",
  "int vwprintf(const wchar_t* restrict format, va_list ap)"
])

# WSCANF(3)
define(["stdio.h", "wchar.h", "stdarg.h"], [
  "int wscanf(const wchar_t* restrict format, ...)",
  "int fwscanf(FILE* restrict stream, const wchar_t* restrict format, ...)",
  "int swscanf(const wchar_t* restrict str, const wchar_t* restrict format, ...)",
  "int vwscanf(const wchar_t* restrict format, va_list ap)",
  "int vswscanf(const wchar_t* restrict str, const wchar_t* restrict format, va_list ap)",
  "int vfwscanf(FILE* restrict stream, const wchar_t* restrict format, va_list ap)"
])

# LGAMMA(3)
define(["math.h"], [
  "double lgamma(double x)",
  "double lgamma_r(double x, int* signgamp)",
  "float lgammaf(float x)",
  "float lgammaf_r(float x, int* signgamp)",
  "long double lgammal(long double x)",
  "long double lgammal_r(long double x, int* signgamp)",
  "double gamma(double x)",
  "float gammaf(float x)",
  "double tgamma(double x)",
  "float tgammaf(float x)",
  "long double tgammal(long double x)"
])

# GETCONTEXT(3)
define(["ucontext.h"], [
  "int getcontext(ucontext_t* ucp)",
  "int setcontext(const ucontext_t* ucp)"
])

# GETCWD(3)
define(["unistd.h"], [
  "char* getcwd(char* buf, size_t size)",
  "char* getwd(char* buf)"
])

# GETLINE(3)
define(["stdio.h"], [
  "ssize_t getdelim(char** restrict linep, size_t* restrict linecapp, int delimiter, FILE* restrict stream)",
  "ssize_t getline(char** restrict linep, size_t* restrict linecapp, FILE* restrict stream)"
])

# GETDOMAINNAME(3)
define(["unistd.h"], [
  "int getdomainname(char* name, size_t namelen)",
  "int setdomainname(const char* name, size_t namelen)"
])

# GETENV(3)
define(["stdlib.h"], [
  "char* getenv(const char* name)",
  "int setenv(const char* name, const char* value, int overwrite)",
  "int putenv(char* string)",
  "int unsetenv(const char* name)"
])

# GETGROUPLIST(3)
define(["unistd.h"], [
  "int getgrouplist(const char* name, gid_t basegid, gid_t* groups, int* ngroups)"
])

# GETHOSTID(3)
define(["unistd.h"], [
  "long gethostid(void)",
  #~ "int sethostid(long hostid)"
])

# GETHOSTNAME(3)
define(["unistd.h"], [
  "int gethostname(char* name, size_t namelen)",
  "int sethostname(const char* name, size_t namelen)"
])

# GETLOADAVG(3)
define(["stdlib.h"], [
  "int getloadavg(double loadavg[], int nelem)"
])

# GETOPT(3)
define(["unistd.h"], [
  "int getopt(int argc, char* const argv[], const char* optstring)"
])

# GETOPT_LONG(3)
define(["getopt.h"], [
  "int getopt_long(int argc, char* const* argv, const char* optstring, const struct option* longopts, int* longindex)",
  "int getopt_long_only(int argc, char* const* argv, const char* optstring, const struct option* longopts, int* longindex)"
])

# GETPAGESIZE(3)
define(["unistd.h"], [
  "int getpagesize(void)"
])

# GETPASS(3)
define(["pwd.h", "unistd.h"], [
  "char* getpass(const char* prompt)"
])

# GETRPCPORT(3)
define([], [
  "int getrpcport(const char* host, unsigned long prognum, unsigned long versnum, unsigned int proto)"
])

# GETSUBOPT(3)
define(["stdlib.h"], [
  "int getsubopt(char** optionp, char* const* tokens, char** valuep)"
])

# GLOB(3)
define(["glob.h"], [
  "int glob(const char* restrict pattern, int flags, int (*errfunc)(const char*, int), glob_t* restrict pglob)",
  "void globfree(glob_t* pglob)"
])

# PTSNAME(3)
define(["stdlib.h"], [
  "int grantpt(int fildes)",
  "char* ptsname(int fildes)",
  "int unlockpt(int fildes)"
])

# HCREATE(3)
define(["search.h"], [
  "int hcreate(size_t nel)",
  "int hcreate_r(size_t nel, struct hsearch_data* table)",
  "void hdestroy(void)",
  "void hdestroy_r(struct hsearch_data* table)",
  "ENTRY* hsearch(ENTRY item, ACTION action)",
  "int hsearch_r(ENTRY item, ACTION action, ENTRY** itemp, struct hsearch_data* table)"
])

# QSORT(3)
define(["stdlib.h"], [
  "void qsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*))",
  #~ "void qsort_r(void* base, size_t nmemb, size_t size, void* thunk, int (*compar)(void*, const void*, const void*))"
])

# IEEE_TEST(3)
define(["math.h"], [
  "double scalb(double x, double n)",
  "float scalbf(float x, float n)",
  "double significand(double x)",
  "float significandf(float x)"
])

# ILOGB(3)
define(["math.h"], [
  "int ilogb(double x)",
  "int ilogbf(float x)",
  "int ilogbl(long double x)",
  "double logb(double x)",
  "float logbf(float x)",
  "long double logbl(long double x)"
])

# IMAXABS(3)
define(["inttypes.h"], [
  "intmax_t imaxabs(intmax_t j)"
])

# IMAXDIV(3)
define(["inttypes.h"], [
  "imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom)"
])

# INDEX(3)
define(["strings.h"], [
  "char* index(const char* s, int c)",
  "char* rindex(const char* s, int c)"
])

# INITGROUPS(3)
define(["unistd.h"], [
  "int initgroups(const char* name, gid_t basegid)"
])

# RANDOM(3)
define(["stdlib.h"], [
  "long random(void)",
  "void srandom(unsigned int seed)",
  "char* initstate(unsigned int seed, char* state, size_t n)",
  "char* setstate(char* state)"
])

# INSQUE(3)
define(["search.h"], [
  "void insque(void* element1, void* pred)",
  "void remque(void* element)"
])

# RCMD(3)
define(["unistd.h"], [
  "int rcmd(char** ahost, unsigned short int inport, const char* locuser, const char* remuser, const char* cmd, int* fd2p)",
  "int rresvport(int* port)",
  "int iruserok(uint32_t raddr, int superuser, const char* ruser, const char* luser)",
  "int ruserok(const char* rhost, int superuser, const char* ruser, const char* luser)",
  "int rcmd_af(char** ahost, unsigned short int inport, const char* locuser, const char* remuser, const char* cmd, int* fd2p, int af)",
  "int rresvport_af(int* port, int af)"
])

# TTYNAME(3)
define(["unistd.h"], [
  "char* ttyname(int fd)",
  "int ttyname_r(int fd, char* buf, size_t len)",
  "int isatty(int fd)"
])

# ISBLANK(3)
define(["ctype.h"], [
  "int isblank(int c)",
  "int isblank_l(int c, locale_t loc)"
])

# ISWALNUM(3)
define(["wctype.h"], [
  "int iswalnum(wint_t wc)",
  "int iswalpha(wint_t wc)",
  "int iswblank(wint_t wc)",
  "int iswcntrl(wint_t wc)",
  "int iswdigit(wint_t wc)",
  "int iswgraph(wint_t wc)",
  "int iswlower(wint_t wc)",
  "int iswprint(wint_t wc)",
  "int iswpunct(wint_t wc)",
  "int iswspace(wint_t wc)",
  "int iswupper(wint_t wc)",
  "int iswxdigit(wint_t wc)"
])

# ISWALNUM_L(3)
define(["wctype.h"], [
  "int iswalnum_l(wint_t wc, locale_t loc)",
  "int iswalpha_l(wint_t wc, locale_t loc)",
  "int iswcntrl_l(wint_t wc, locale_t loc)",
  #~ "int iswctype_l(wint_t wc, locale_t loc)",
  "int iswdigit_l(wint_t wc, locale_t loc)",
  "int iswgraph_l(wint_t wc, locale_t loc)",
  "int iswlower_l(wint_t wc, locale_t loc)",
  "int iswprint_l(wint_t wc, locale_t loc)",
  "int iswpunct_l(wint_t wc, locale_t loc)",
  "int iswspace_l(wint_t wc, locale_t loc)",
  "int iswupper_l(wint_t wc, locale_t loc)",
  "int iswxdigit_l(wint_t wc, locale_t loc)",
  "wint_t towlower_l(wint_t wc, locale_t loc)",
  "wint_t towupper_l(wint_t wc, locale_t loc)",
  #~ "wctype_t wctype_l(wint_t wc, locale_t loc)",
  "int iswblank_l(wint_t wc, locale_t loc)",
  "wint_t towctrans_l(wint_t wc, wctrans_t, locale_t loc)",
  "wctrans_t wctrans_l(const char*, locale_t loc)"
])

# WCTYPE(3)
define(["wctype.h"], [
  "int iswctype(wint_t wc, wctype_t charclass)",
  "wctype_t wctype(const char* property)"
])

# J0(3)
define(["math.h"], [
  "double j0(double x)",
  "float j0f(float x)",
  "double j1(double x)",
  "float j1f(float x)",
  "double jn(int n, double x)",
  "float jnf(int n, float x)",
  "double y0(double x)",
  "float y0f(float x)",
  "double y1(double x)",
  "float y1f(float x)",
  "double yn(int n, double x)",
  "float ynf(int n, float x)"
])

# LABS(3)
define(["stdlib.h"], [
  "long labs(long j)"
])

# LDEXP(3)
define(["math.h"], [
  "double ldexp(double x, int exp)",
  "float ldexpf(float x, int exp)",
  "long double ldexpl(long double x, int exp)"
])

# LDIV(3)
define(["stdlib.h"], [
  "ldiv_t ldiv(long num, long denom)"
])

# LSEARCH(3)
define(["search.h"], [
  "void* lsearch(const void* key, void* base, size_t* nelp, size_t width, int (*compar)(const void*, const void*))",
  "void* lfind(const void* key, const void* base, size_t* nelp, size_t width, int (*compar)(const void*, const void*))"
])

# LLABS(3)
define(["stdlib.h"], [
  "long long llabs(long long j)"
])

# LLDIV(3)
define(["stdlib.h"], [
  "lldiv_t lldiv(long long numer, long long denom)"
])

# LRINT(3)
define(["math.h"], [
  "long long llrint(double x)",
  "long long llrintf(float x)",
  "long long llrintl(long double x)",
  "long lrint(double x)",
  "long lrintf(float x)",
  "long lrintl(long double x)"
])

# LROUND(3)
define(["math.h"], [
  "long long llround(double x)",
  "long long llroundf(float x)",
  "long long llroundl(long double x)",
  "long lround(double x)",
  "long lroundf(float x)",
  "long lroundl(long double x)"
])

# LOCALECONV(3)
define(["locale.h"], [
  "struct lconv* localeconv(void)"
])

# LOCKF(3)
define(["unistd.h"], [
  "int lockf(int fd, int function, off_t size)"
])

# LOG(3)
define(["math.h"], [
  "double log(double x)",
  "float logf(float x)",
  "long double logl(long double x)",
  "double log10(double x)",
  "float log10f(float x)",
  "long double log10l(long double x)",
  "double log2(double x)",
  "float log2f(float x)",
  "long double log2l(long double x)",
  "double log1p(double x)",
  "float log1pf(float x)",
  "long double log1pl(long double x)"
])

# MAKECONTEXT(3)
define(["ucontext.h"], [
  #~ "void makecontext(ucontext_t* ucp, void (*func)(void), int argc, ...)",
  #~ "int swapcontext(ucontext_t* oucp, const ucontext_t* ucp)"
])

# MBLEN(3)
define(["stdlib.h"], [
  "int mblen(const char* mbchar, size_t nbytes)"
])

# MBRLEN(3)
define(["wchar.h"], [
  "size_t mbrlen(const char* restrict s, size_t n, mbstate_t* restrict ps)"
])

# MBRTOWC(3)
define(["wchar.h", "uchar.h"], [
  "size_t mbrtowc(wchar_t* restrict pc, const char* restrict s, size_t n, mbstate_t* restrict ps)",
  "size_t mbrtoc16(char16_t* restrict pc, const char* restrict s, size_t n, mbstate_t* restrict ps)",
  "size_t mbrtoc32(char32_t* restrict pc, const char* restrict s, size_t n, mbstate_t* restrict ps)"
])

# MBSINIT(3)
define(["wchar.h"], [
  "int mbsinit(const mbstate_t* ps)"
])

# MBSRTOWCS(3)
define(["wchar.h"], [
  "size_t mbsrtowcs(wchar_t* restrict dst, const char** restrict src, size_t len, mbstate_t* restrict ps)",
  "size_t mbsnrtowcs(wchar_t* restrict dst, const char** restrict src, size_t nms, size_t len, mbstate_t* restrict ps)"
])

# MBSTOWCS(3)
define(["stdlib.h"], [
  "size_t mbstowcs(wchar_t* restrict wcstring, const char* restrict mbstring, size_t nwchars)"
])

# MBTOWC(3)
define(["stdlib.h"], [
  "int mbtowc(wchar_t* restrict wcharp, const char* restrict mbchar, size_t nbytes)"
])

# MEMCCPY(3)
define(["string.h"], [
  "void* memccpy(void* dst, const void* src, int c, size_t len)"
])

# MEMCHR(3)
define(["string.h"], [
  "void* memchr(const void* b, int c, size_t len)",
  "void* memrchr(const void* b, int c, size_t len)"
])

# MEMCMP(3)
define(["string.h"], [
  "int memcmp(const void* b1, const void* b2, size_t len)"
])

# MEMCPY(3)
define(["string.h"], [
  "void* memcpy(void* dst, const void* src, size_t len)"
])

# MEMMEM(3)
define(["string.h"], [
  "void* memmem(const void* big, size_t big_len, const void* little, size_t little_len)"
])

# MEMMOVE(3)
define(["string.h"], [
  "void* memmove(void* dst, const void* src, size_t len)"
])

# MEMSET(3)
define(["string.h"], [
  "void* memset(void* dest, int c, size_t len)"
])

# MKTEMP(3)
define(["stdlib.h", "unistd.h"], [
  "char* mktemp(char* template)",
  "int mkstemp(char* template)",
  "int mkostemp(char* template, int oflags)",
  "int mkostemps(char* template, int suffixlen, int oflags)",
  "char* mkdtemp(char* template)",
  "int mkstemps(char* template, int suffixlen)"
])

# MODF(3)
define(["math.h"], [
  "double modf(double value, double* iptr)",
  "float modff(float value, float* iptr)",
  "long double modfl(long double value, long double* iptr)"
])

# MONCONTROL(3)
define(["sys/types.h", "sys/gmon.h"], [
  "void moncontrol(int mode)",
  "void monstartup(u_long lowpc, u_long highpc)"
])

# NAN(3)
define(["math.h"], [
  "double nan(const char* s)",
  "float nanf(const char* s)",
  "long double nanl(const char* s)"
])

# RINT(3)
define(["math.h"], [
  "double nearbyint(double x)",
  "float nearbyintf(float x)",
  "long double nearbyintl(long double x)",
  "double rint(double x)",
  "float rintf(float x)",
  "long double rintl(long double x)"
])

# NEWLOCALE(3)
define(["locale.h"], [
  "locale_t newlocale(int mask, const char* locale, locale_t base)"
])

# NEXTAFTER(3)
define(["math.h"], [
  "double nextafter(double x, double y)",
  "float nextafterf(float x, float y)",
  "long double nextafterl(long double x, long double y)",
  "double nexttoward(double x, long double y)",
  "float nexttowardf(float x, long double y)",
  "long double nexttowardl(long double x, long double y)"
])

# NICE(3)
define(["unistd.h"], [
  "int nice(int incr)"
])

# NL_LANGINFO(3)
define(["langinfo.h"], [
  "char* nl_langinfo(nl_item item)",
  "char* nl_langinfo_l(nl_item item, locale_t loc)"
])

# OPEN_MEMSTREAM(3)
define(["stdio.h", "wchar.h"], [
  "FILE* open_memstream(char** bufp, size_t* sizep)",
  "FILE* open_wmemstream(wchar_t** bufp, size_t* sizep)"
])

# PAUSE(3)
define(["unistd.h"], [
  "int pause(void)"
])

# POPEN(3)
define(["stdio.h"], [
  "FILE* popen(const char* command, const char* type)",
  "int pclose(FILE* stream)"
])

# STRERROR(3)
define(["stdio.h", "string.h"], [
  "void perror(const char* string)",
  "char* strerror(int errnum)",
  "char* strerror_r(int errnum, char* strerrbuf, size_t buflen)"
])

# POSIX_SPAWN(3)
define(["spawn.h"], [
  "int posix_spawn(pid_t* restrict pid, const char* restrict path, const posix_spawn_file_actions_t* file_actions, const posix_spawnattr_t* restrict attrp, char* const argv[restrict], char* const envp[restrict])",
  "int posix_spawnp(pid_t* restrict pid, const char* restrict file, const posix_spawn_file_actions_t* file_actions, const posix_spawnattr_t* restrict attrp, char* const argv[restrict], char* const envp[restrict])"
])

# POSIX_SPAWN_FILE_ACTIONS_ADDOPEN(3)
define(["spawn.h"], [
  "int posix_spawn_file_actions_addopen(posix_spawn_file_actions_t* file_actions, int fildes, const char* restrict path, int oflag, mode_t mode)",
  "int posix_spawn_file_actions_adddup2(posix_spawn_file_actions_t* file_actions, int fildes, int newfildes)",
  "int posix_spawn_file_actions_addclose(posix_spawn_file_actions_t* file_actions, int fildes)"
])

# POSIX_SPAWN_FILE_ACTIONS_INIT(3)
define(["spawn.h"], [
  "int posix_spawn_file_actions_init(posix_spawn_file_actions_t* file_actions)",
  "int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t* file_actions)"
])

# POSIX_SPAWNATTR_INIT(3)
define(["spawn.h"], [
  "int posix_spawnattr_init(posix_spawnattr_t* attr)",
  "int posix_spawnattr_destroy(posix_spawnattr_t* attr)"
])

# POSIX_SPAWNATTR_GETFLAGS(3)
define(["spawn.h"], [
  "int posix_spawnattr_getflags(const posix_spawnattr_t* restrict attr, short* restrict flags)",
  "int posix_spawnattr_setflags(posix_spawnattr_t* attr, short flags)"
])

# POSIX_SPAWNATTR_GETPGROUP(3)
define(["spawn.h"], [
  "int posix_spawnattr_getpgroup(const posix_spawnattr_t* restrict attr, pid_t* restrict pgroup)",
  "int posix_spawnattr_setpgroup(posix_spawnattr_t* attr, pid_t pgroup)"
])

# POSIX_SPAWNATTR_GETSCHEDPARAM(3)
define(["spawn.h"], [
  "int posix_spawnattr_getschedparam(const posix_spawnattr_t* restrict attr, struct sched_param* restrict schedparam)",
  "int posix_spawnattr_setschedparam(posix_spawnattr_t* attr, const struct sched_param* restrict schedparam)"
])

# POSIX_SPAWNATTR_GETSCHEDPOLICY(3)
define(["spawn.h"], [
  "int posix_spawnattr_getschedpolicy(const posix_spawnattr_t* restrict attr, int* restrict schedpolicy)",
  "int posix_spawnattr_setschedpolicy(posix_spawnattr_t* attr, int schedpolicy)"
])

# POSIX_SPAWNATTR_GETSIGDEFAULT(3)
define(["spawn.h"], [
  "int posix_spawnattr_getsigdefault(const posix_spawnattr_t* restrict attr, sigset_t* restrict sigdefault)",
  "int posix_spawnattr_setsigdefault(posix_spawnattr_t* attr, const sigset_t* restrict sigdefault)"
])

# POSIX_SPAWNATTR_GETSIGMASK(3)
define(["spawn.h"], [
  "int posix_spawnattr_getsigmask(const posix_spawnattr_t* restrict attr, sigset_t* restrict sigmask)",
  "int posix_spawnattr_setsigmask(posix_spawnattr_t* attr, const sigset_t* restrict sigmask)"
])

# PSIGNAL(3)
define(["signal.h", "string.h"], [
  "void psignal(int sig, const char* s)",
  "char* strsignal(int sig)"
])

# PTHREAD_AFFINITY_NP(3)
define([LINUX ? "pthread.h" : "pthread_np.h"], [
  "int pthread_getaffinity_np(pthread_t td, size_t cpusetsize, cpuset_t* cpusetp)",
  "int pthread_setaffinity_np(pthread_t td, size_t cpusetsize, const cpuset_t* cpusetp)"
])

# PTHREAD_ATFORK(3)
define(["pthread.h"], [
  "int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))"
])

# PTHREAD_ATTR(3)
define(["pthread.h"], [
  "int pthread_attr_init(pthread_attr_t* attr)",
  "int pthread_attr_destroy(pthread_attr_t* attr)",
  "int pthread_attr_setstack(pthread_attr_t* attr, void* stackaddr, size_t stacksize)",
  "int pthread_attr_getstack(const pthread_attr_t* restrict attr, void** restrict stackaddr, size_t* restrict stacksize)",
  "int pthread_attr_setstacksize(pthread_attr_t* attr, size_t stacksize)",
  "int pthread_attr_getstacksize(const pthread_attr_t* attr, size_t* stacksize)",
  "int pthread_attr_setguardsize(pthread_attr_t* attr, size_t guardsize)",
  "int pthread_attr_getguardsize(const pthread_attr_t* attr, size_t* guardsize)",
  "int pthread_attr_setstackaddr(pthread_attr_t* attr, void* stackaddr)",
  "int pthread_attr_getstackaddr(const pthread_attr_t* attr, void** stackaddr)",
  "int pthread_attr_setdetachstate(pthread_attr_t* attr, int detachstate)",
  "int pthread_attr_getdetachstate(const pthread_attr_t* attr, int* detachstate)",
  "int pthread_attr_setinheritsched(pthread_attr_t* attr, int inheritsched)",
  "int pthread_attr_getinheritsched(const pthread_attr_t* attr, int* inheritsched)",
  "int pthread_attr_setschedparam(pthread_attr_t* attr, const struct sched_param* param)",
  "int pthread_attr_getschedparam(const pthread_attr_t* attr, struct sched_param* param)",
  "int pthread_attr_setschedpolicy(pthread_attr_t* attr, int policy)",
  "int pthread_attr_getschedpolicy(const pthread_attr_t* attr, int* policy)",
  "int pthread_attr_setscope(pthread_attr_t* attr, int contentionscope)",
  "int pthread_attr_getscope(const pthread_attr_t* attr, int* contentionscope)"
])

# PTHREAD_ATTR_AFFINITY_NP(3)
define([LINUX ? "pthread.h" : "pthread_np.h"], [
  "int pthread_attr_getaffinity_np(const pthread_attr_t* pattr, size_t cpusetsize, cpuset_t* cpusetp)",
  "int pthread_attr_setaffinity_np(pthread_attr_t* pattr, size_t cpusetsize, const cpuset_t* cpusetp)"
])

# PTHREAD_BARRIER(3)
define(["pthread.h"], [
  "int pthread_barrier_destroy(pthread_barrier_t* barrier)",
  "int pthread_barrier_init(pthread_barrier_t* barrier, const pthread_barrierattr_t* attr, unsigned count)",
  "int pthread_barrier_wait(pthread_barrier_t* barrier)"
])

# PTHREAD_BARRIERATTR(3)
define(["pthread.h"], [
  "int pthread_barrierattr_destroy(pthread_barrierattr_t* attr)",
  "int pthread_barrierattr_getpshared(const pthread_barrierattr_t* attr, int* pshared)",
  "int pthread_barrierattr_init(pthread_barrierattr_t* attr)",
  "int pthread_barrierattr_setpshared(pthread_barrierattr_t* attr, int pshared)"
])

# PTHREAD_CANCEL(3)
define(["pthread.h"], [
  "int pthread_cancel(pthread_t thread)"
])

# PTHREAD_COND_BROADCAST(3)
define(["pthread.h"], [
  "int pthread_cond_broadcast(pthread_cond_t* cond)"
])

# PTHREAD_COND_DESTROY(3)
define(["pthread.h"], [
  "int pthread_cond_destroy(pthread_cond_t* cond)"
])

# PTHREAD_COND_INIT(3)
define(["pthread.h"], [
  "int pthread_cond_init(pthread_cond_t* cond, const pthread_condattr_t* attr)"
])

# PTHREAD_COND_SIGNAL(3)
define(["pthread.h"], [
  "int pthread_cond_signal(pthread_cond_t* cond)"
])

# PTHREAD_COND_TIMEDWAIT(3)
define(["pthread.h"], [
  "int pthread_cond_timedwait(pthread_cond_t* cond, pthread_mutex_t* mutex, const struct timespec* abstime)"
])

# PTHREAD_COND_WAIT(3)
define(["pthread.h"], [
  "int pthread_cond_wait(pthread_cond_t* cond, pthread_mutex_t* mutex)"
])

# PTHREAD_CONDATTR(3)
define(["pthread.h"], [
  "int pthread_condattr_init(pthread_condattr_t* attr)",
  "int pthread_condattr_destroy(pthread_condattr_t* attr)",
  "int pthread_condattr_getclock(pthread_condattr_t* restrict attr, clockid_t* restrict clock_id)",
  "int pthread_condattr_setclock(pthread_condattr_t* attr, clockid_t clock_id)",
  "int pthread_condattr_getpshared(pthread_condattr_t* restrict attr, int* restrict pshared)",
  "int pthread_condattr_setpshared(pthread_condattr_t* attr, int pshared)"
])

# PTHREAD_CREATE(3)
define(["pthread.h"], [
  "int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg)"
])

# PTHREAD_DETACH(3)
define(["pthread.h"], [
  "int pthread_detach(pthread_t thread)"
])

# PTHREAD_EQUAL(3)
define(["pthread.h"], [
  "int pthread_equal(pthread_t t1, pthread_t t2)"
])

# PTHREAD_EXIT(3)
define(["pthread.h"], [
  "void pthread_exit(void* value_ptr)"
])

# PTHREAD_GETCONCURRENCY(3)
define(["pthread.h"], [
  "int pthread_getconcurrency(void)",
  "int pthread_setconcurrency(int new_level)"
])

# PTHREAD_GETCPUCLOCKID(3)
define(["pthread.h", "time.h"], [
  "int pthread_getcpuclockid(pthread_t thread_id, clockid_t* clock_id)"
])

# PTHREAD_SCHEDPARAM(3)
define(["pthread.h"], [
  "int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param* param)",
  "int pthread_getschedparam(pthread_t thread, int* policy, struct sched_param* param)"
])

# PTHREAD_GETSPECIFIC(3)
define(["pthread.h"], [
  "void* pthread_getspecific(pthread_key_t key)"
])

# PTHREAD_JOIN(3)
define(LINUX ? ["pthread.h"] : ["pthread.h", "pthread_np.h"], [
  "int pthread_join(pthread_t thread, void** value_ptr)",
  "int pthread_timedjoin_np(pthread_t thread, void** value_ptr, const struct timespec* abstime)"
])

# PTHREAD_KEY_CREATE(3)
define(["pthread.h"], [
  "int pthread_key_create(pthread_key_t* key, void (*destructor)(void*))"
])

# PTHREAD_KEY_DELETE(3)
define(["pthread.h"], [
  "int pthread_key_delete(pthread_key_t key)"
])

# PTHREAD_KILL(3)
define(["pthread.h", "signal.h"], [
  "int pthread_kill(pthread_t thread, int sig)"
])

# PTHREAD_MUTEX_CONSISTENT(3)
define(["pthread.h"], [
  "int pthread_mutex_consistent(pthread_mutex_t* mutex)"
])

# PTHREAD_MUTEX_DESTROY(3)
define(["pthread.h"], [
  "int pthread_mutex_destroy(pthread_mutex_t* mutex)"
])

# PTHREAD_MUTEX_INIT(3)
define(["pthread.h"], [
  "int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr)"
])

# PTHREAD_MUTEX_LOCK(3)
define(["pthread.h"], [
  "int pthread_mutex_lock(pthread_mutex_t* mutex)"
])

# PTHREAD_MUTEX_TIMEDLOCK(3)
define(["pthread.h", "time.h"], [
  "int pthread_mutex_timedlock(pthread_mutex_t* mutex, const struct timespec* abs_timeout)"
])

# PTHREAD_MUTEX_TRYLOCK(3)
define(["pthread.h"], [
  "int pthread_mutex_trylock(pthread_mutex_t* mutex)"
])

# PTHREAD_MUTEX_UNLOCK(3)
define(["pthread.h"], [
  "int pthread_mutex_unlock(pthread_mutex_t* mutex)"
])

# PTHREAD_MUTEXATTR(3)
define(["pthread.h"], [
  "int pthread_mutexattr_init(pthread_mutexattr_t* attr)",
  "int pthread_mutexattr_destroy(pthread_mutexattr_t* attr)",
  "int pthread_mutexattr_setprioceiling(pthread_mutexattr_t* attr, int prioceiling)",
  "int pthread_mutexattr_getprioceiling(pthread_mutexattr_t* attr, int* prioceiling)",
  "int pthread_mutexattr_setprotocol(pthread_mutexattr_t* attr, int protocol)",
  "int pthread_mutexattr_getprotocol(pthread_mutexattr_t* attr, int* protocol)",
  "int pthread_mutexattr_setrobust(pthread_mutexattr_t* attr, int robust)",
  "int pthread_mutexattr_getrobust(pthread_mutexattr_t* attr, int* robust)",
  "int pthread_mutexattr_settype(pthread_mutexattr_t* attr, int type)",
  "int pthread_mutexattr_gettype(pthread_mutexattr_t* attr, int* type)"
])

# PTHREAD_MUTEXATTR_GETKIND_NP(3)
define([LINUX ? "pthread.h" : "pthread_np.h"], [
  "int pthread_mutexattr_getkind_np(const pthread_mutexattr_t* attr, int* kind)",
  "int pthread_mutexattr_setkind_np(pthread_mutexattr_t* attr, int kind)"
])

# PTHREAD_ONCE(3)
define(["pthread.h"], [
  "int pthread_once(pthread_once_t* once_control, void (*init_routine)(void))"
])

# PTHREAD_RWLOCK_DESTROY(3)
define(["pthread.h"], [
  "int pthread_rwlock_destroy(pthread_rwlock_t* lock)"
])

# PTHREAD_RWLOCK_INIT(3)
define(["pthread.h"], [
  "int pthread_rwlock_init(pthread_rwlock_t* lock, const pthread_rwlockattr_t* attr)"
])

# PTHREAD_RWLOCK_RDLOCK(3)
define(["pthread.h"], [
  "int pthread_rwlock_rdlock(pthread_rwlock_t* lock)",
  "int pthread_rwlock_tryrdlock(pthread_rwlock_t* lock)"
])

# PTHREAD_RWLOCK_TIMEDRDLOCK(3)
define(["pthread.h"], [
  "int pthread_rwlock_timedrdlock(pthread_rwlock_t* rwlock, const struct timespec* abs_timeout)"
])

# PTHREAD_RWLOCK_TIMEDWRLOCK(3)
define(["pthread.h"], [
  "int pthread_rwlock_timedwrlock(pthread_rwlock_t* rwlock, const struct timespec* abs_timeout)"
])

# PTHREAD_RWLOCK_WRLOCK(3)
define(["pthread.h"], [
  "int pthread_rwlock_wrlock(pthread_rwlock_t* lock)",
  "int pthread_rwlock_trywrlock(pthread_rwlock_t* lock)"
])

# PTHREAD_RWLOCK_UNLOCK(3)
define(["pthread.h"], [
  "int pthread_rwlock_unlock(pthread_rwlock_t* lock)"
])

# PTHREAD_RWLOCKATTR_DESTROY(3)
define(["pthread.h"], [
  "int pthread_rwlockattr_destroy(pthread_rwlockattr_t* attr)"
])

# PTHREAD_RWLOCKATTR_GETPSHARED(3)
define(["pthread.h"], [
  "int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t* attr, int* pshared)"
])

# PTHREAD_RWLOCKATTR_INIT(3)
define(["pthread.h"], [
  "int pthread_rwlockattr_init(pthread_rwlockattr_t* attr)"
])

# PTHREAD_RWLOCKATTR_SETPSHARED(3)
define(["pthread.h"], [
  "int pthread_rwlockattr_setpshared(pthread_rwlockattr_t* attr, int pshared)"
])

# PTHREAD_SELF(3)
define(["pthread.h"], [
  "pthread_t pthread_self(void)"
])

# PTHREAD_TESTCANCEL(3)
define(["pthread.h"], [
  "int pthread_setcancelstate(int state, int* oldstate)",
  "int pthread_setcanceltype(int type, int* oldtype)",
  "void pthread_testcancel(void)"
])

# PTHREAD_SETSPECIFIC(3)
define(["pthread.h"], [
  "int pthread_setspecific(pthread_key_t key, const void* value)"
])

# PTHREAD_SIGMASK(3)
define(["pthread.h", "signal.h"], [
  "int pthread_sigmask(int how, const sigset_t* restrict set, sigset_t* restrict oset)"
])

# PTHREAD_SPIN_INIT(3)
define(["pthread.h"], [
  "int pthread_spin_init(pthread_spinlock_t* lock, int pshared)",
  "int pthread_spin_destroy(pthread_spinlock_t* lock)"
])

# PTHREAD_SPIN_LOCK(3)
define(["pthread.h"], [
  "int pthread_spin_lock(pthread_spinlock_t* lock)",
  "int pthread_spin_trylock(pthread_spinlock_t* lock)",
  "int pthread_spin_unlock(pthread_spinlock_t* lock)"
])

# PTHREAD_YIELD(3)
define(["pthread.h"], [
  #~ "int pthread_yield(void)"
])

# QUICK_EXIT(3)
define(["stdlib.h"], [
  #~ "_Noreturn void quick_exit(int status)"
])

# RAISE(3)
define(["signal.h"], [
  "int raise(int sig)"
])

# RAND(3)
define(["stdlib.h"], [
  "void srand(unsigned seed)",
  "int rand(void)",
  "int rand_r(unsigned* ctx)"
])

# REALPATH(3)
define(["stdlib.h"], [
  "char* realpath(const char* restrict pathname, char* restrict resolved_path)"
])

# REGEX(3)
define(["regex.h"], [
  "int regcomp(regex_t* restrict preg, const char* restrict pattern, int cflags)",
  "int regexec(const regex_t* restrict preg, const char* restrict string, size_t nmatch, regmatch_t pmatch[restrict], int eflags)",
  "size_t regerror(int errcode, const regex_t* restrict preg, char* restrict errbuf, size_t errbuf_size)",
  "void regfree(regex_t* preg)"
])

# REMAINDER(3)
define(["math.h"], [
  "double remainder(double x, double y)",
  "float remainderf(float x, float y)",
  "long double remainderl(long double x, long double y)",
  "double remquo(double x, double y, int* quo)",
  "float remquof(float x, float y, int* quo)",
  "long double remquol(long double x, long double y, int* quo)"
])

# REMOVE(3)
define(["stdio.h"], [
  "int remove(const char* path)"
])

# ROUND(3)
define(["math.h"], [
  "double round(double x)",
  "float roundf(float x)",
  "long double roundl(long double x)"
])

# RPC_SVC_ERR(3)
define(["rpc/rpc.h"], [
  "void svcerr_auth(SVCXPRT* xprt, enum auth_stat why)",
  "void svcerr_decode(SVCXPRT* xprt)",
  "void svcerr_noproc(SVCXPRT* xprt)",
  "void svcerr_noprog(SVCXPRT* xprt)",
  "void svcerr_progvers(SVCXPRT* xprt, rpcvers_t low_vers, rpcvers_t high_vers)",
  "void svcerr_systemerr(SVCXPRT* xprt)",
  "void svcerr_weakauth(SVCXPRT* xprt)"
])

# RPC_XDR(3)
define(["rpc/rpc.h"], [
  "bool_t xdr_accepted_reply(XDR* xdrs, struct accepted_reply* ar)",
  "bool_t xdr_callhdr(XDR* xdrs, struct rpc_msg* chdr)",
  "bool_t xdr_callmsg(XDR* xdrs, struct rpc_msg* cmsg)",
  "bool_t xdr_opaque_auth(XDR* xdrs, struct opaque_auth* ap)",
  "bool_t xdr_rejected_reply(XDR* xdrs, struct rejected_reply* rr)",
  "bool_t xdr_replymsg(XDR* xdrs, struct rpc_msg* rmsg)"
])

# RPMATCH(3)
define(["stdlib.h"], [
  "int rpmatch(const char* response)"
])

# RTIME(3)
define(["sys/types.h", "sys/time.h", "netinet/in.h"], [
  "int rtime(struct sockaddr_in* addrp, struct timeval* timep, struct timeval* timeout)"
])

# SCALBN(3)
define(["math.h"], [
  "double scalbln(double x, long n)",
  "float scalblnf(float x, long n)",
  "long double scalblnl(long double x, long n)",
  "double scalbn(double x, int n)",
  "float scalbnf(float x, int n)",
  "long double scalbnl(long double x, int n)"
])

# SEM_TIMEDWAIT(3)
define(["semaphore.h", "time.h"], [
  "int sem_timedwait(sem_t* sem, const struct timespec* abs_timeout)"
])

# SEM_OPEN(3)
define(["semaphore.h"], [
  #~ "sem_t* sem_open(const char* name, int oflag, ...)",
  "int sem_close(sem_t* sem)",
  "int sem_unlink(const char* name)"
])

# SEM_DESTROY(3)
define(["semaphore.h"], [
  "int sem_destroy(sem_t* sem)"
])

# SEM_GETVALUE(3)
define(["semaphore.h"], [
  "int sem_getvalue(sem_t* restrict sem, int* restrict sval)"
])

# SEM_INIT(3)
define(["semaphore.h"], [
  "int sem_init(sem_t* sem, int pshared, unsigned int value)"
])

# SEM_POST(3)
define(["semaphore.h"], [
  "int sem_post(sem_t* sem)"
])

# SEM_WAIT(3)
define(["semaphore.h"], [
  "int sem_wait(sem_t* sem)",
  "int sem_trywait(sem_t* sem)"
])

# SETBUF(3)
define(["stdio.h"], [
  "void setbuf(FILE* restrict stream, char* restrict buf)",
  "void setbuffer(FILE* stream, char* buf, size_t size)",
  "void setlinebuf(FILE* stream)",
  "int setvbuf(FILE* restrict stream, char* restrict buf, int mode, size_t size)"
])

# SETLOCALE(3)
define(["locale.h"], [
  "char* setlocale(int category, const char* locale)"
])

# SIGSETOPS(3)
define(["signal.h"], [
  "int sigemptyset(sigset_t* set)",
  "int sigfillset(sigset_t* set)",
  "int sigaddset(sigset_t* set, int signo)",
  "int sigdelset(sigset_t* set, int signo)",
  "int sigismember(const sigset_t* set, int signo)"
])

# SIGINTERRUPT(3)
define(["signal.h"], [
  "int siginterrupt(int sig, int flag)"
])

# SIN(3)
define(["math.h"], [
  "double sin(double x)",
  "float sinf(float x)",
  "long double sinl(long double x)"
])

# SINCOS(3)
define(["math.h"], [
  "void sincos(double x, double* s, double* c)",
  "void sincosf(float x, float* s, float* c)",
  "void sincosl(long double x, long double* s, long double* c)"
])

# SINH(3)
define(["math.h"], [
  "double sinh(double x)",
  "float sinhf(float x)",
  "long double sinhl(long double x)"
])

# SLEEP(3)
define(["unistd.h"], [
  "unsigned int sleep(unsigned int seconds)"
])

# SOCKATMARK(3)
define(["sys/socket.h"], [
  "int sockatmark(int s)"
])

# STRCPY(3)
define(["string.h"], [
  "char* stpcpy(char* restrict dst, const char* restrict src)",
  "char* stpncpy(char* restrict dst, const char* restrict src, size_t len)",
  "char* strcpy(char* restrict dst, const char* restrict src)",
  "char* strncpy(char* restrict dst, const char* restrict src, size_t len)"
])

# STRCASECMP(3)
define(["strings.h", "xlocale.h"], [
  "int strcasecmp(const char* s1, const char* s2)",
  "int strncasecmp(const char* s1, const char* s2, size_t len)",
  "int strcasecmp_l(const char* s1, const char* s2, locale_t loc)",
  "int strncasecmp_l(const char* s1, const char* s2, size_t len, locale_t loc)"
])

# STRSTR(3)
define(["string.h"], [
  "char* strstr(const char* big, const char* little)",
  "char* strcasestr(const char* big, const char* little)"
])

# STRCAT(3)
define(["string.h"], [
  "char* strcat(char* restrict s, const char* restrict append)",
  "char* strncat(char* restrict s, const char* restrict append, size_t count)"
])

# STRCHR(3)
define(["string.h"], [
  "char* strchr(const char* s, int c)",
  "char* strrchr(const char* s, int c)",
  "char* strchrnul(const char* s, int c)"
])

# STRCMP(3)
define(["string.h"], [
  "int strcmp(const char* s1, const char* s2)",
  "int strncmp(const char* s1, const char* s2, size_t len)"
])

# STRCOLL(3)
define(["string.h"], [
  "int strcoll(const char* s1, const char* s2)",
  "int strcoll_l(const char* s1, const char* s2, locale_t loc)"
])

# STRSPN(3)
define(["string.h"], [
  "size_t strspn(const char* s, const char* charset)",
  "size_t strcspn(const char* s, const char* charset)"
])

# STRDUP(3)
define(["string.h"], [
  "char* strdup(const char* str)",
  "char* strndup(const char* str, size_t len)"
])

# STRFMON(3)
define(["monetary.h"], [
  #~ "ssize_t strfmon(char* restrict s, size_t maxsize, const char* restrict format, ...)",
  #~ "ssize_t strfmon_l(char* restrict s, size_t maxsize, locale_t loc, const char* restrict format, ...)"
])

# STRFTIME(3)
define(["time.h"], [
  "size_t strftime(char* restrict buf, size_t maxsize, const char* restrict format, const struct tm* restrict timeptr)",
  "size_t strftime_l(char* restrict buf, size_t maxsize, const char* restrict format, const struct tm* restrict timeptr, locale_t loc)"
])

# STRLEN(3)
define(["string.h"], [
  "size_t strlen(const char* s)",
  "size_t strnlen(const char* s, size_t maxlen)"
])

# STRPBRK(3)
define(["string.h"], [
  "char* strpbrk(const char* s, const char* charset)"
])

# STRPTIME(3)
define(["time.h", "xlocale.h"], [
  "char* strptime(const char* restrict buf, const char* restrict format, struct tm* restrict timeptr)",
  "char* strptime_l(const char* restrict buf, const char* restrict format, struct tm* restrict timeptr, locale_t loc)"
])

# STRSEP(3)
define(["string.h"], [
  "char* strsep(char** stringp, const char* delim)"
])

# STRTOD(3)
define(["stdlib.h"], [
  "double strtod(const char* restrict nptr, char** restrict endptr)",
  "float strtof(const char* restrict nptr, char** restrict endptr)",
  "long double strtold(const char* restrict nptr, char** restrict endptr)"
])

# STRTOL(3)
define(["stdlib.h", "limits.h", "inttypes.h", "sys/types.h"], [
  "long strtol(const char* restrict nptr, char** restrict endptr, int base)",
  "long long strtoll(const char* restrict nptr, char** restrict endptr, int base)",
  "intmax_t strtoimax(const char* restrict nptr, char** restrict endptr, int base)",
  "long long int strtoq(const char* nptr, char** endptr, int base)"
])

# STRTOK(3)
define(["string.h"], [
  "char* strtok(char* str, const char* sep)",
  "char* strtok_r(char* str, const char* sep, char** last)"
])

# STRTOUL(3)
define(["stdlib.h", "limits.h", "inttypes.h", "sys/types.h"], [
  "unsigned long strtoul(const char* restrict nptr, char** restrict endptr, int base)",
  "unsigned long long strtoull(const char* restrict nptr, char** restrict endptr, int base)",
  "uintmax_t strtoumax(const char* restrict nptr, char** restrict endptr, int base)",
  "unsigned long long strtouq(const char* nptr, char** endptr, int base)"
])

# STRXFRM(3)
define(["string.h"], [
  "size_t strxfrm(char* restrict dst, const char* restrict src, size_t n)",
  "size_t strxfrm_l(char* restrict dst, const char* restrict src, size_t n, locale_t loc)"
])

# SWAB(3)
define(["unistd.h"], [
  "void swab(const void* restrict src, void* restrict dst, ssize_t len)"
])

# SYSCONF(3)
define(["unistd.h"], [
  "long sysconf(int name)"
])

# SYSCTL(3)
define(["sys/types.h", "sys/sysctl.h"], [
  "int sysctl(int* name, int namelen, void* oldp, size_t* oldlenp, void* newp, size_t newlen)"
])

# SYSTEM(3)
define(["stdlib.h"], [
  "int system(const char* string)"
])

# TAN(3)
define(["math.h"], [
  "double tan(double x)",
  "float tanf(float x)",
  "long double tanl(long double x)"
])

# TANH(3)
define(["math.h"], [
  "double tanh(double x)",
  "float tanhf(float x)",
  "long double tanhl(long double x)"
])

# TCSENDBREAK(3)
define(["termios.h"], [
  "int tcdrain(int fd)",
  "int tcflow(int fd, int action)",
  "int tcflush(int fd, int action)",
  "int tcsendbreak(int fd, int len)"
])

# TCGETPGRP(3)
define(["sys/types.h", "unistd.h"], [
  "pid_t tcgetpgrp(int fd)"
])

# TCGETSID(3)
define(["sys/types.h", "termios.h"], [
  "pid_t tcgetsid(int fd)"
])

# TCSETPGRP(3)
define(["sys/types.h", "unistd.h"], [
  "int tcsetpgrp(int fd, pid_t pgrp_id)"
])

# TMPFILE(3)
define(["stdio.h"], [
  "FILE* tmpfile(void)",
  "char* tmpnam(char* str)",
  "char* tempnam(const char* tmpdir, const char* prefix)"
])

# TIME(3)
define(["time.h"], [
  "time_t time(time_t* tloc)"
])

# TIMES(3)
define(["sys/times.h"], [
  "clock_t times(struct tms* tp)"
])

# TIMEZONE(3)
define([], [
  "char* timezone(int zone, int dst)"
])

# WCTRANS(3)
define(["wctype.h"], [
  "wint_t towctrans(wint_t wc, wctrans_t desc)",
  "wctrans_t wctrans(const char* charclass)"
])

# TOWLOWER(3)
define(["wctype.h"], [
  "wint_t towlower(wint_t wc)"
])

# TOWUPPER(3)
define(["wctype.h"], [
  "wint_t towupper(wint_t wc)"
])

# TRUNC(3)
define(["math.h"], [
  "double trunc(double x)",
  "float truncf(float x)",
  "long double truncl(long double x)"
])

# TZSET(3)
define(["time.h"], [
  "void tzset(void)"
])

# UALARM(3)
define(["unistd.h"], [
  "useconds_t ualarm(useconds_t microseconds, useconds_t interval)"
])

# ULIMIT(3)
define(["ulimit.h"], [
  #~ "long ulimit(int cmd, ...)"
])

# UNAME(3)
define(["sys/utsname.h"], [
  "int uname(struct utsname* name)"
])

# UNGETC(3)
define(["stdio.h"], [
  "int ungetc(int c, FILE* stream)"
])

# UNGETWC(3)
define(["stdio.h", "wchar.h"], [
  "wint_t ungetwc(wint_t wc, FILE* stream)"
])

# USELOCALE(3)
define(["locale.h"], [
  "locale_t uselocale(locale_t locale)"
])

# USLEEP(3)
define(["unistd.h"], [
  "int usleep(useconds_t microseconds)"
])

# UTIME(3)
define(["utime.h"], [
  "int utime(const char* file, const struct utimbuf* timep)"
])

# VALLOC(3)
define(["unistd.h"], [
  "void* valloc(size_t size)"
])

# WMEMCHR(3)
define(["wchar.h"], [
  "wchar_t* wmemchr(const wchar_t* s, wchar_t c, size_t n)",
  "int wmemcmp(const wchar_t* s1, const wchar_t* s2, size_t n)",
  "wchar_t* wmemcpy(wchar_t* restrict s1, const wchar_t* restrict s2, size_t n)",
  "wchar_t* wmemmove(wchar_t* s1, const wchar_t* s2, size_t n)",
  "wchar_t* wmemset(wchar_t* s, wchar_t c, size_t n)",
  "wchar_t* wcpcpy(wchar_t* s1, const wchar_t* s2)",
  "wchar_t* wcpncpy(wchar_t* s1, const wchar_t* s2, size_t n)",
  "int wcscasecmp(const wchar_t* s1, const wchar_t* s2)",
  "wchar_t* wcscat(wchar_t* restrict s1, const wchar_t* restrict s2)",
  "wchar_t* wcschr(const wchar_t* s, wchar_t c)",
  "int wcscmp(const wchar_t* s1, const wchar_t* s2)",
  "wchar_t* wcscpy(wchar_t* restrict s1, const wchar_t* restrict s2)",
  "size_t wcscspn(const wchar_t* s1, const wchar_t* s2)",
  "wchar_t* wcsdup(const wchar_t* s)",
  "size_t wcslen(const wchar_t* s)",
  "int wcsncasecmp(const wchar_t* s1, const wchar_t* s2, size_t n)",
  "wchar_t* wcsncat(wchar_t* restrict s1, const wchar_t* restrict s2, size_t n)",
  "int wcsncmp(const wchar_t* s1, const wchar_t* s2, size_t n)",
  "wchar_t* wcsncpy(wchar_t* restrict s1, const wchar_t* restrict s2, size_t n)",
  "size_t wcsnlen(const wchar_t* s, size_t maxlen)",
  "wchar_t* wcspbrk(const wchar_t* s1, const wchar_t* s2)",
  "wchar_t* wcsrchr(const wchar_t* s, wchar_t c)",
  "size_t wcsspn(const wchar_t* s1, const wchar_t* s2)",
  "wchar_t* wcsstr(const wchar_t* restrict s1, const wchar_t* restrict s2)"
])

# WCSCOLL(3)
define(["wchar.h"], [
  "int wcscoll(const wchar_t* s1, const wchar_t* s2)"
])

# WCSFTIME(3)
define(["wchar.h"], [
  "size_t wcsftime(wchar_t* restrict wcs, size_t maxsize, const wchar_t* restrict format, const struct tm* restrict timeptr)"
])

# WCSRTOMBS(3)
define(["wchar.h"], [
  "size_t wcsrtombs(char* restrict dst, const wchar_t** restrict src, size_t len, mbstate_t* restrict ps)",
  "size_t wcsnrtombs(char* restrict dst, const wchar_t** restrict src, size_t nwc, size_t len, mbstate_t* restrict ps)"
])

# WCSTOD(3)
define(["wchar.h"], [
  "float wcstof(const wchar_t* restrict nptr, wchar_t** restrict endptr)",
  "long double wcstold(const wchar_t* restrict nptr, wchar_t** restrict endptr)",
  "double wcstod(const wchar_t* restrict nptr, wchar_t** restrict endptr)"
])

# WCSTOL(3)
define(["wchar.h", "inttypes.h"], [
  "long wcstol(const wchar_t* restrict nptr, wchar_t** restrict endptr, int base)",
  "unsigned long wcstoul(const wchar_t* restrict nptr, wchar_t** restrict endptr, int base)",
  "long long wcstoll(const wchar_t* restrict nptr, wchar_t** restrict endptr, int base)",
  "unsigned long long wcstoull(const wchar_t* restrict nptr, wchar_t** restrict endptr, int base)",
  "intmax_t wcstoimax(const wchar_t* restrict nptr, wchar_t** restrict endptr, int base)",
  "uintmax_t wcstoumax(const wchar_t* restrict nptr, wchar_t** restrict endptr, int base)"
])

# WCSTOK(3)
define(["wchar.h"], [
  "wchar_t* wcstok(wchar_t* restrict str, const wchar_t* restrict sep, wchar_t** restrict last)"
])

# WCSTOMBS(3)
define(["stdlib.h"], [
  "size_t wcstombs(char* restrict mbstring, const wchar_t* restrict wcstring, size_t nbytes)"
])

# WCSWIDTH(3)
define(["wchar.h"], [
  "int wcswidth(const wchar_t* pwcs, size_t n)"
])

# WCSXFRM(3)
define(["wchar.h"], [
  "size_t wcsxfrm(wchar_t* restrict dst, const wchar_t* restrict src, size_t n)"
])

# WCTOMB(3)
define(["stdlib.h"], [
  "int wctomb(char* mbchar, wchar_t wchar)"
])

# WCWIDTH(3)
define(["wchar.h"], [
  "int wcwidth(wchar_t wc)"
])

# WORDEXP(3)
define(["wordexp.h"], [
  "int wordexp(const char* restrict words, wordexp_t* restrict we, int flags)",
  "void wordfree(wordexp_t* we)"
])

# DL_ITERATE_PHDR(3)
define(["link.h"], [
  "int dl_iterate_phdr(int (*callback)(struct dl_phdr_info*, size_t, void*), void* data)"
])

define([
  "sys/types.h",
  "sys/socket.h",
  "netdb.h"
], [
  "int getaddrinfo(const char* restrict node, const char* restrict service, const struct addrinfo* restrict hints, struct addrinfo** restrict res)",
  "void freeaddrinfo(struct addrinfo* res)",
  "int getnameinfo(const struct sockaddr* restrict addr, socklen_t addrlen, char* restrict host, socklen_t hostlen, char* restrict serv, socklen_t servlen, int flags)",
  "const char* gai_strerror(int ecode)"
])

# 14.5. Interface Definitions for libc
lsb_define([
  "unistd.h",
  "ctype.h",
  "stdio.h",
  "wchar.h",
  "fcntl.h",
  "sys/stat.h",
  "stdlib.h",
  "string.h",
  "sys/socket.h",
  "syslog.h",
  "libgen.h",
  "signal.h",
  "dirent.h",
  "time.h",
  #~ "argz.h",
  "libintl.h",
  "sys/types.h",
  "rpc/rpc.h",
  "termios.h",
  "locale.h",
  "link.h",
  #~ "utmp.h",
  #~ "envz.h",
  ("sys/epoll.h" if LINUX),
  "err.h",
  #~ "error.h",
  ("sys/statfs.h" if LINUX),
  "sys/time.h",
  "grp.h",
  "netdb.h",
  "ifaddrs.h",
  "getopt.h",
  "pwd.h",
  "netinet/ip.h",
  "glob.h",
  #~ "gnu/libc-version.h",
  "search.h",
  "sys/inotify.h",
  "sys/ioctl.h",
  "net/if.h",
  "netinet/in.h",
  "sys/mman.h",
  "rpc/pmap_clnt.h",
  "sys/ptrace.h",
  "sched.h",
  #~ "sys/sendfile.h",
  "sys/param.h",
  "sys/utsname.h",
  "limits.h",
  "rpc/svc.h",
  #~ "sys/sysinfo.h",
  "stdarg.h",
  "sys/resource.h",
  "sys/wait.h",
  "rpc/xdr.h"
].compact, [
  "int _IO_feof(_IO_FILE* __fp)",
  "int _IO_getc(_IO_FILE* __fp)",
  "int _IO_putc(int __c, _IO_FILE* __fp)",
  "int _IO_puts(const char* __c)",
  "void __assert_fail(const char* assertion, const char* file, unsigned int line, const char* function)",
  "void __chk_fail(void)",
  "size_t __confstr_chk(int name, char* buf, size_t len, size_t buflen)",
  "const unsigned short** __ctype_b_loc(void)",
  "size_t __ctype_get_mb_cur_max(void)",
  "const int32_t** __ctype_tolower_loc(void)",
  "const int32_t** __ctype_toupper_loc(void)",
  "int __cxa_atexit(void (*func)(void*), void* arg, void* dso_handle)",
  "void __cxa_finalize(void* d)",
  "int* __errno_location(void)",
  "char* __fgets_chk(char* s, size_t size, int strsize, FILE* stream)",
  "char* __fgets_unlocked_chk(char* s, size_t size, int strsize, FILE* stream)",
  "wchar_t* __fgetws_chk(wchar_t* ws, size_t size, int strsize, FILE* stream)",
  "wchar_t* __fgetws_unlocked_chk(wchar_t* ws, size_t strsize, int n, FILE* stream)",
  "size_t __fpending(FILE* stream)",
  "int __fprintf_chk(FILE* stream, int flag, const char* format, ...)",
  "int __fwprintf_chk(FILE* stream, int flag, const wchar_t* format, ...)",
  "int __fxstatat(int ver, int dirfd, const char* path, struct stat* stat_buf, int flags)",
  "int __fxstatat64(int ver, int dirfd, const char* path, struct stat64* stat_buf, int flags)",
  "char* __getcwd_chk(char* buf, size_t len, size_t buflen)",
  "int __getgroups_chk(int size, gid_t* list, size_t listlen)",
  "int __gethostname_chk(char* buf, size_t buflen, size_t maxlen)",
  "int __getlogin_r_chk(char* buf, size_t buflen, size_t maxlen)",
  "int __getpagesize(void)",
  "pid_t __getpgid(pid_t pid)",
  "int* __h_errno_location(void)",
  "int __isinf(double arg)",
  "int __isinff(float arg)",
  "int __isinfl(long double arg)",
  "int __isnan(double arg)",
  "int __isnanf(float arg)",
  "int __isnanl(long double arg)",
  "int __libc_current_sigrtmax(void)",
  "int __libc_current_sigrtmin(void)",
  "size_t __mbsnrtowcs_chk(wchar_t* dest, const char** src, size_t nmc, size_t len, mbstate_t* ps, size_t destlen)",
  "size_t __mbsrtowcs_chk(wchar_t* dest, const char** src, size_t len, mbstate_t* ps, size_t destlen)",
  "size_t __mbstowcs_chk(wchar_t* dest, const char* src, size_t len, size_t destlen)",
  "void* __memcpy_chk(void* dest, const void* src, size_t len, size_t destlen)",
  "void* __memmove_chk(void* dest, const void* src, size_t len, size_t destlen)",
  "void* __mempcpy(void* restrict dest, const void* restrict src, size_t n)",
  "void* __mempcpy_chk(void* dest, const void* src, size_t len, size_t destlen)",
  "void* __memset_chk(void* dest, int c, size_t len, size_t destlen)",
  "ssize_t __pread64_chk(int fd, void* buf, size_t nbytes, off64_t offset, size_t buflen)",
  "ssize_t __pread_chk(int fd, void* buf, size_t nbytes, off_t offset, size_t buflen)",
  "int __printf_chk(int flag, const char* format, ...)",
  "void* __rawmemchr(const void* s, int c)",
  "ssize_t __read_chk(int fd, void* buf, size_t nbytes, size_t buflen)",
  "ssize_t __readlink_chk(const char* path, char* buf, size_t len, size_t buflen)",
  "char* __realpath_chk(const char* path, char* resolved_path, size_t resolved_len)",
  "ssize_t __recv_chk(int fd, void* buf, size_t len, size_t buflen, int flag)",
  "ssize_t __recvfrom_chk(int fd, void* buf, size_t len, size_t buflen, int flag, struct sockaddr* from, socklen_t* fromlen)",
  "int __register_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void), void* __dso_handle)",
  "int __sigsetjmp(jmp_buf env, int savemask)",
  "int __snprintf_chk(char* str, size_t maxlen, int flag, size_t strlen, const char* format, ...)",
  "int __sprintf_chk(char* str, int flag, size_t strlen, const char* format, ...)",
  "void __stack_chk_fail(void)",
  "char* __stpcpy(char* dest, const char* src)",
  "char* __stpcpy_chk(char* dest, const char* src, size_t destlen)",
  "char* __stpncpy_chk(char* dest, const char* src, size_t n, size_t destlen)",
  "char* __strcat_chk(char* dest, const char* src, size_t destlen)",
  "char* __strcpy_chk(char* dest, const char* src, size_t destlen)",
  "char* __strdup(const char* string)",
  "char* __strncat_chk(char* s1, const char* s2, size_t n, size_t s1len)",
  "char* __strncpy_chk(char* s1, const char* s2, size_t n, size_t s1len)",
  "double __strtod_internal(const char* __nptr, char** __endptr, int __group)",
  "float __strtof_internal(const char* __nptr, char** __endptr, int __group)",
  "char* __strtok_r(char* restrict s, const char* restrict delim, char** restrict save_ptr)",
  "long int __strtol_internal(const char* __nptr, char** __endptr, int __base, int __group)",
  "long double __strtold_internal(const char* __nptr, char** __endptr, int __group)",
  "long long __strtoll_internal(const char* __nptr, char** __endptr, int __base, int __group)",
  "unsigned long int __strtoul_internal(const char* __nptr, char** __endptr, int __base, int __group)",
  "unsigned long long __strtoull_internal(const char* __nptr, char** __endptr, int __base, int __group)",
  "int __swprintf_chk(wchar_t* s, size_t n, int flag, size_t slen, const wchar_t* format)",
  "long __sysconf(int name)",
  "void __syslog_chk(int priority, int flag, const char* format)",
  "__sighandler_t __sysv_signal(int sig, __sighandler_t handler)",
  "int __ttyname_r_chk(int fd, char* buf, size_t buflen, size_t nreal)",
  "int __vfprintf_chk(FILE* fp, int flag, const char* format, va_list ap)",
  "int __vfwprintf_chk(FILE* fp, int flag, const wchar_t* format, va_list ap)",
  "int __vprintf_chk(int flag, const char* format, va_list ap)",
  "int __vsnprintf_chk(char* s, size_t maxlen, int flag, size_t slen, const char* format, va_list args)",
  "int __vsprintf_chk(char* s, int flag, size_t slen, const char* format, va_list args)",
  "int __vswprintf_chk(wchar_t* s, size_t maxlen, int flag, size_t slen, const wchar_t* format, va_list args)",
  "void __vsyslog_chk(int priority, int flag, const char* format, va_list ap)",
  "int __vwprintf_chk(int flag, const wchar_t* format, va_list ap)",
  "wchar_t* __wcpcpy_chk(wchar_t* dest, const wchar_t* src, size_t destlen)",
  "wchar_t* __wcpncpy_chk(wchar_t* dest, const wchar_t* src, size_t n, size_t destlen)",
  "size_t __wcrtomb_chk(char* s, wchar_t wchar, mbstate_t* ps, size_t buflen)",
  "wchar_t* __wcscat_chk(wchar_t* dest, const wchar_t* src, size_t destlen)",
  "wchar_t* __wcscpy_chk(wchar_t* dest, const wchar_t* src, size_t n)",
  "wchar_t* __wcsncat_chk(wchar_t* dest, const wchar_t* src, size_t n, size_t destlen)",
  "wchar_t* __wcsncpy_chk(wchar_t* dest, const wchar_t* src, size_t n, size_t destlen)",
  "size_t __wcsnrtombs_chk(char* dest, const wchar_t** src, size_t nwc, size_t len, mbstate_t* ps, size_t destlen)",
  "size_t __wcsrtombs_chk(char* dest, const wchar_t** src, size_t len, mbstate_t* ps, size_t destlen)",
  "double __wcstod_internal(const wchar_t* nptr, wchar_t** endptr, int group)",
  "float __wcstof_internal(const wchar_t* nptr, wchar_t** endptr, int group)",
  "long __wcstol_internal(const wchar_t* nptr, wchar_t** endptr, int base, int group)",
  "long double __wcstold_internal(const wchar_t* nptr, wchar_t** endptr, int group)",
  "size_t __wcstombs_chk(char* dest, const wchar_t* src, size_t len, size_t destlen)",
  "unsigned long __wcstoul_internal(const wchar_t* restrict nptr, wchar_t** restrict endptr, int base, int group)",
  "int __wctomb_chk(char* s, wchar_t wchar, size_t buflen)",
  "wchar_t* __wmemcpy_chk(wchar_t* s1, const wchar_t* s2, size_t n, size_t ns1)",
  "wchar_t* __wmemmove_chk(wchar_t* s1, const wchar_t* s2, size_t n, size_t ns1)",
  "wchar_t* __wmempcpy_chk(wchar_t* s1, const wchar_t* s2, size_t n, size_t ns1)",
  "wchar_t* __wmemset_chk(wchar_t* s, wchar_t c, size_t n, size_t destlen)",
  "int __wprintf_chk(int flag, const wchar_t* format)",
  "int __xmknod(int ver, const char* path, mode_t mode, dev_t* dev)",
  "int __xmknodat(int ver, int dirfd, const char* path, mode_t path, dev_t* dev)",
  "char* __xpg_basename(const char* path)",
  "int __xpg_sigpause(int sig)",
  "int __xpg_strerror_r(int errnum, char* buf, size_t buflen)",
  "int __xstat(int ver, const char* path, struct stat* stat_buf)",
  "int __lxstat(int ver, const char* path, struct stat* stat_buf)",
  "int __fxstat(int ver, int fildes, struct stat* stat_buf)",
  "int __xstat64(int ver, const char* path, struct stat64* stat_buf)",
  "int __lxstat64(int ver, const char* path, struct stat64* stat_buf)",
  "int __fxstat64(int ver, int fildes, struct stat64* stat_buf)",
  "int alphasort64(const struct dirent64** d1, const struct dirent64** d2)",
  "error_t argz_add(char** argz, size_t* argz_len, const char* str)",
  "error_t argz_add_sep(char** argz, size_t* argz_len, const char* str, int sep)",
  "error_t argz_append(char** argz, size_t* argz_len, const char* buf, size_t buf_len)",
  "size_t argz_count(const char* argz, size_t* argz_len)",
  "error_t argz_create(char* const argv, char** argz, size_t* argz_len)",
  "error_t argz_create_sep(const char* str, int sep, char** argz, size_t* argz_len)",
  "void argz_delete(char** argz, size_t* argz_len, char* entry)",
  "void argz_extract(const char* argz, size_t argz_len, char** argv)",
  "error_t argz_insert(char** argz_insert, size_t* argz_len, char* before, const char* entry)",
  "char argz_next(const char* argz, size_t argz_len, const char* entry)",
  "error_t argz_replace(char** argz, size_t* argz_len, const char* str, const char* with, unsigned int* replace_count)",
  "void argz_stringify(char* argz, size_t argz_len, int sep)",
  "char* bind_textdomain_codeset(const char* domainname, const char* codeset)",
  "char* bindtextdomain(const char* domainname, const char* dirname)",
  "char* dcgettext(const char* domainname, const char* msgid, int category)",
  "char* dcngettext(const char* domainname, const char* msgid1, const char* msgid2, unsigned long int n, int category)",
  "char* dgettext(const char* domainname, const char* msgid)",
  "char* dngettext(const char* domainname, const char* msgid1, const char* msgid2, unsigned long int n)",
  "int drand48_r(struct drand48_data* buffer, double* result)",
  "void endutent(void)",
  "error_t envz_add(char** envz, size_t* envz_len, const char* name, const char* value)",
  "char envz_entry(const char* envz, size_t envz_len, const char* name)",
  "char envz_get(const char* envz, size_t envz_len, const char* name)",
  "error_t envz_merge(char** envz, size_t* envz_len, const char* envz2, size_t envz2_len, int override)",
  "void envz_remove(char** envz, size_t* envz_len, const char* name)",
  "void envz_strip(char** envz, size_t* envz_len)",
  "int epoll_create(int size)",
  "int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event)",
  "int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)",
  "int erand48_r(unsigned short xsubi[3], struct drand48_data* buffer, double* result)",
  "void error(int status, int errnum, const char* format, ...)",
  "int fstatfs64(int fd, struct statfs64* buf)",
  "int gethostbyaddr_r(const void* restrict addr, socklen_t len, int type, struct hostent* restrict result_buf, char* restrict buf, size_t buflen, struct hostent** restrict result, int* h_errnop)",
  "int gethostbyname2_r(const char* restrict name, int af, struct hostent* restrict result_buf, char* restrict buf, size_t buflen, struct hostent** restrict result, int* restrict h_errnop)",
  "int gethostbyname_r(const char* restrict name, struct hostent* restrict result_buf, char* restrict buf, size_t buflen, struct hostent** restrict result, int* restrict h_errnop)",
  "int getifaddrs(struct ifaddrs** ifap)",
  "void freeifaddrs(struct ifaddrs* ifa)",
  "int getprotobyname_r(const char* name, struct protoent* result_buf, char* buf, size_t buflen, struct protoent** result)",
  "int getprotobynumber_r(int proto, struct protoent* result_buf, char* buf, size_t buflen, struct protoent** result)",
  "int getprotoent_r(struct protoent* result_buf, char* buf, size_t buflen, struct protoent** result)",
  "int getservbyname_r(const char* name, const char* proto, struct servent* result_buf, char* buf, size_t buflen, struct servent** result)",
  "int getservbyport_r(int port, const char* proto, struct servent* result_buf, char* buf, size_t buflen, struct servent** result)",
  "int getservent_r(struct servent* result_buf, char* buf, size_t buflen, struct servent** result)",
  "char* gettext(const char* msgid)",
  "struct utmp* getutent(void)",
  "int getutent_r(struct utmp* buffer, struct utmp** result)",
  "int glob64(const char* pattern, int flags, int (*errfunc)(const char*, int), glob64_t* pglob)",
  "void globfree64(glob64_t* pglob)",
  "const char* gnu_get_libc_version(void)",
  "const char* gnu_get_libc_release(void)",
  "int initstate_r(unsigned int seed, char* statebuf, size_t statelen, struct random_data* buffer)",
  "int inotify_add_watch(int fd, const char* path, uint32_t mask)",
  "int inotify_init(void)",
  "int inotify_rm_watch(int fd, int wd)",
  "int jrand48_r(unsigned short xsubi[3], struct drand48_data* buffer, long int* result)",
  "int lcong48_r(unsigned short param[7], struct drand48_data* buffer)",
  "int lrand48_r(struct drand48_data* buffer, long int* result)",
  "int mkstemp64(char* template)",
  "int mrand48_r(struct drand48_data* buffer, long int* result)",
  "void* mremap(void* old_address, size_t old_size, size_t new_size, int flags, ...)",
  "char* ngettext(const char* msgid1, const char* msgid2, unsigned long int n)",
  "int nrand48_r(unsigned short xsubi[3], struct drand48_data* buffer, long int* result)",
  "int openat64(int fd, const char* path, int oflag, ...)",
  #~ "u_short* pmap_getport(struct sockaddr_in* address, const u_long program, const u_long* version, u_int protocol)",
  "bool_t pmap_set(const u_long program, const u_long version, int protocol, u_short port)",
  "bool_t pmap_unset(u_long prognum, u_long versnum)",
  "int posix_fadvise64(int fd, off64_t offset, off64_t len, int advice)",
  "int posix_fallocate64(int fd, off64_t offset, off64_t len)",
  "ssize_t pread64(int fd, void* buf, size_t count, off64_t offset)",
  #~ "long ptrace(enum __ptrace_request request, pid_t pid, void* addr, void* data)",
  "ssize_t pwrite64(int fd, const void* buf, size_t count, off64_t offset)",
  "int random_r(struct random_data* buffer, int32_t* result)",
  "int readdir64_r(DIR* dirp, struct dirent64* entry, struct dirent64** result)",
  "int scandir64(const char* dir, struct dirent64*** namelist, int (*selector)(const struct dirent64*), int (*cmp)(const struct dirent64**, const struct dirent64**))",
  "int sched_getaffinity(pid_t pid, unsigned int cpusetsize, cpu_set_t* mask)",
  "int sched_setaffinity(pid_t pid, unsigned int cpusetsize, cpu_set_t* mask)",
  "int seed48_r(unsigned short seed16v[3], struct drand48_data* buffer)",
  #~ "ssize_t sendfile(int out_fd, int in_fd, off_t* offset, size_t count)",
  #~ "ssize_t sendfile64(int out_fd, int in_fd, off64_t* offset, size_t count)",
  "int setstate_r(char* statebuf, struct random_data* buf)",
  "void setutent(void)",
  "int sigandset(sigset_t* set, const sigset_t* left, const sigset_t* right)",
  "int sigisemptyset(const sigset_t* set)",
  "int sigorset(sigset_t* set, const sigset_t* left, const sigset_t* right)",
  "int sigpause(int sig)",
  "int sigreturn(struct sigcontext* scp)",
  "int srand48_r(long int seedval, struct drand48_data* buffer)",
  "int srandom_r(unsigned int seed, struct random_data* buffer)",
  "int statfs64(const char* path, struct statfs64* buf)",
  "int stime(const time_t* t)",
  "bool_t svc_register(SVCXPRT* xprt, rpcprog_t prognum, rpcvers_t versnum, __dispatch_fn_t dispatch, rpcprot_t protocol)",
  "SVCXPRT* svctcp_create(int sock, u_int send_buf_size, u_int recv_buf_size)",
  "int sysinfo(struct sysinfo* info)",
  "char* textdomain(const char* domainname)",
  "int utmpname(const char* dbname)",
  "long long int wcstoq(const wchar_t* restrict nptr, wchar_t** restrict endptr, int base)",
  "unsigned long long wcstouq(const wchar_t* restrict nptr, wchar_t** restrict endptr, int base)",
  "int xdr_u_int(XDR* xdrs, unsigned int* up)",
  "void xdrstdio_create(XDR* xdrs, FILE* file, enum xdr_op op)"
])

# 14.8. Interface Definitions for libm
lsb_define(["math.h", "complex.h", "fenv.h"], [
  "int __finite(double arg)",
  "int __finitef(float arg)",
  "int __finitel(long double arg)",
  "int __fpclassify(double arg)",
  "int __fpclassifyf(float arg)",
  "int __signbit(double arg)",
  "int __signbitf(float arg)",
  "double complex clog10(double complex z)",
  "float complex clog10f(float complex z)",
  "long double complex clog10l(long double complex z)",
  "double drem(double x, double y)",
  "float dremf(float x, float y)",
  "long double dreml(long double x, long double y)",
  "double exp10(double x)",
  "float exp10f(float x)",
  "long double exp10l(long double x)",
  "int finite(double arg)",
  "int finitef(float arg)",
  "int finitel(long double arg)",
  "long double gammal(long double x)",
  "long double j0l(long double x)",
  "long double j1l(long double x)",
  "long double jnl(int n, long double x)",
  "int matherr(struct exception* __exc)",
  "double pow10(double x)",
  "float pow10f(float x)",
  "long double pow10l(long double x)",
  "long double scalbl(long double x, long double exp)",
  "long double significandl(long double x)",
  "long double y0l(long double x)",
  "long double y1l(long double x)",
  "long double ynl(int n, long double x)"
])

# 14.11. Interface Definitions for libpthread
lsb_define(["pthread.h"], [
  "int pthread_getattr_np(pthread_t thread, pthread_attr_t* attr)",
  "int pthread_mutex_consistent_np(pthread_mutex_t* __mutex)",
  "int pthread_mutexattr_getrobust_np(const pthread_mutexattr_t* __attr, int* __robustness)",
  "int pthread_mutexattr_setrobust_np(const pthread_mutexattr_t* __attr, int __robustness)",
  "int pthread_rwlockattr_getkind_np(const pthread_rwlockattr_t* attr, int* pref)",
  "int pthread_rwlockattr_setkind_np(pthread_rwlockattr_t* attr, int* pref)"
])

# 14.17. Interface Definitions for libdl
lsb_define(["dlfcn.h"], [
  "void* dlvsym(void* handle, const char* name, const char* version)"
])

lsb_define([], [
  "int64_t __divdi3(int64_t a, int64_t b)",
  "int64_t __moddi3(int64_t a, int64_t b)",
  "int __isoc99_fscanf(FILE* stream, const char* format, ...)",
  "int __libc_start_main(int (*main)(int, char**, char**), int argc, char** ubp_av, void (*init)(int, char**, char**), void (*fini)(void), void (*rtld_fini)(void), void* stack_end)",
  "locale_t __newlocale(int category_mask, const char* locale, locale_t base)",
  "locale_t __duplocale(locale_t locale)",
  "locale_t __uselocale(locale_t locale)",
  "void __freelocale(locale_t locale)",
  "uint64_t __udivdi3(uint64_t a, uint64_t b)",
  "uint64_t __umoddi3(uint64_t a, uint64_t b)",
  "int __vasprintf_chk(char** result_ptr, int flags, const char* format, va_list args)",
  "wctype_t __wctype_l(const char* property, locale_t locale)",
  "int dladdr1(void* addr, Dl_info* info, void** extra_info, int flags)",
  "void* dlmopen(Lmid_t lmid, const char* path, int mode)",
  "int eventfd(unsigned int initval, int flags)",
  "int fcntl64(int fd, int cmd, ...)",
  "int fgetpos64(FILE* stream, fpos64_t* pos)",
  "FILE* fopen64(const char* filename, const char* type)",
  "int ftruncate64(int fd, off64_t length)",
  "int ftw(const char* path, int (*fn)(const char*, const struct stat*, int), int maxfds)",
  "unsigned long getauxval(unsigned long type)",
  "int getrlimit64(int resource, struct rlimit64* rlp)",
  "int get_nprocs(void)",
  "off64_t lseek64(int fd, off64_t offset, int whence)",
  "struct mallinfo mallinfo(void)",
  "void* memalign(size_t alignment, size_t size)",
  "void* mmap64(void *addr, size_t len, int prot, int linux_flags, int fd, off64_t offset)",
  "int nftw(const char* path, int (*fn)(const char*, const struct stat*, int, struct FTW*), int maxfds, int flags)",
  "int open64(const char* path, int oflag, ...)",
  "int prctl(int option, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5)",
  "int pthread_getname_np(pthread_t thread, char *name, size_t len)",
  "int pthread_mutexattr_setpshared(pthread_mutexattr_t* attr, int pshared)",
  "int pthread_setname_np(pthread_t thread, const char *name)",
  "struct dirent64* readdir64(DIR* dirp)",
  "char* secure_getenv(const char* name)",
  "int sigaction(int signum, const struct sigaction* act, struct sigaction* oldact)",
  "sig_t signal(int sig, sig_t func)"
])

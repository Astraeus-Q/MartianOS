#if !defined(ECE391SYSNUM_H)
#define ECE391SYSNUM_H

#define SYS_HALT    1
#define SYS_EXECUTE 2
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETARGS 7
#define SYS_VIDMAP  8
#define SYS_SET_HANDLER  9
#define SYS_SIGRETURN  10
// New syscalls:
#define SYS_GOIN 11
#define SYS_CREATEFOLDER 12
#define SYS_DELETEFOLDER 13
#define SYS_CREATEFILE 14
#define SYS_DELETEFILE 15

#endif /* ECE391SYSNUM_H */

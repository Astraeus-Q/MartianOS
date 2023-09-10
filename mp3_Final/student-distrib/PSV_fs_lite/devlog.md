# Modifications
1. Modify harddisk.h and harddisk.c
2. Create PSV_fs_lite folder
3. Create PSV.h and PSV.c
4. Remove MAX and MIN.
5. Create math.h and math.c, and change dependence of _4K, etc
6. Create helper.c using to enable some helper functions.
7. Add print_bin() in lib.c, lib.h.
8. Modify syscalls/ece391ls.c and generate new ELF "ls" by ```make``` and copying it in to fsdir/
9. In mp3/ use elf "createfs" to generate new file_img.
10. To operate with in-memory filesys, we include fs.h.
11. Add ```#include "PSV_fs_lite/PSV.h"``` in fs.c, syscall_handler.h.
12. Modify ```read_dentry_by_name```.
13. Modify ```syscall_open```.
14. Delete the ```printf``` in ```syscall_execute```, when read_by_name fails.
15. Add new system call: ```ece391_goin```.
16. Add new command cd.
17. Modify parse_args.
18. Add new command echo.
19. Add new system call ```ece391_createfile```
20. Add new system call ```ece391_deletefile```
21. Add new system call ```ece391_createfolder```
22. Add new system call ```ece391_deletefolder```
23. Add new command touch.
24. Add new command rm.
25. Add new command mkdir.
26. Add new command rmdir.

    

# Steps to add new syscall
0. Under syscalls/
1. Add the head in ece391syscall.h.
2. Add the signums in ece391sysnum.h.
3. Add corresponding Do_call in ece391syscall.S.
5. Under student-distrib/
6. Add the head, put it into ```syscall_jump_table``` and increase ```MAX_SYSCALL_IDX```, in syscall_handler_entry.S.
6. Add the head in syscall_handler.h.
7. Define syscall in syscall_handler.c.

# Modify Command
Modify syscalls/ece391($command).c and generate new ELF "($command)" by ```make``` and copying it in to fsdir/
# Generate Command
Just create and finish a similar new syscalls/ece391($command).c file and add the command in Makefile. And do the procedure of Modify Command above.
# Generate New file_sys_img
```
./createfs --input fsdir/ --output student-distrib/filesys_img
```
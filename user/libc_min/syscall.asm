[bits 32]
global sys_write, sys_exit, sys_yield, sys_sleep, sys_getpid
global sys_open, sys_close, sys_read, sys_lseek, sys_stat, sys_getdents, sys_execve, sys_waitpid, sys_spawn
global sys_fork, sys_pipe, sys_dup2, sys_kill, sys_mmap, sys_munmap
sys_write: mov eax,1
           mov ebx,[esp+4]
           mov ecx,[esp+8]
           mov edx,[esp+12]
           int 0x80
           ret
sys_exit: mov eax,2
          mov ebx,[esp+4]
          int 0x80
          ret
sys_yield: mov eax,3
           int 0x80
           ret
sys_sleep: mov eax,4
           mov ebx,[esp+4]
           int 0x80
           ret
sys_getpid: mov eax,5
            int 0x80
            ret
sys_open: mov eax,6
          mov ebx,[esp+4]
          mov ecx,[esp+8]
          int 0x80
          ret
sys_close: mov eax,7
           mov ebx,[esp+4]
           int 0x80
           ret
sys_read: mov eax,8
          mov ebx,[esp+4]
          mov ecx,[esp+8]
          mov edx,[esp+12]
          int 0x80
          ret
sys_lseek: mov eax,9
           mov ebx,[esp+4]
           mov ecx,[esp+8]
           mov edx,[esp+12]
           int 0x80
           ret
sys_stat: mov eax,10
          mov ebx,[esp+4]
          mov ecx,[esp+8]
          int 0x80
          ret
sys_getdents: mov eax,11
              mov ebx,[esp+4]
              mov ecx,[esp+8]
              mov edx,[esp+12]
              int 0x80
              ret
sys_execve: mov eax,12
            mov ebx,[esp+4]
            int 0x80
            ret
sys_waitpid: mov eax,13
             mov ebx,[esp+4]
             mov ecx,[esp+8]
             int 0x80
             ret
sys_spawn: mov eax,14
           mov ebx,[esp+4]
           int 0x80
           ret
sys_fork: mov eax,15
          int 0x80
          ret
sys_pipe: mov eax,16
          mov ebx,[esp+4]
          int 0x80
          ret
sys_dup2: mov eax,17
          mov ebx,[esp+4]
          mov ecx,[esp+8]
          int 0x80
          ret
sys_kill: mov eax,18
          mov ebx,[esp+4]
          mov ecx,[esp+8]
          int 0x80
          ret

sys_mmap: mov eax,19
          mov ebx,[esp+4]
          mov ecx,[esp+8]
          mov edx,[esp+12]
          mov esi,[esp+16]
          mov edi,[esp+20]
          mov ebp,[esp+24]
          int 0x80
          ret
sys_munmap: mov eax,20
            mov ebx,[esp+4]
            mov ecx,[esp+8]
            int 0x80
            ret
global sys_mkdir, sys_unlink, sys_rename, sys_sync
sys_mkdir: mov eax,21
           mov ebx,[esp+4]
           int 0x80
           ret
sys_unlink: mov eax,22
            mov ebx,[esp+4]
            int 0x80
            ret
sys_rename: mov eax,23
            mov ebx,[esp+4]
            mov ecx,[esp+8]
            int 0x80
            ret
sys_sync: mov eax,24
          int 0x80
          ret

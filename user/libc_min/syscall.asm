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
global sys_socket, sys_bind, sys_connect, sys_send, sys_recv, sys_sendto, sys_recvfrom, sys_sockclose, sys_netctl
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

sys_socket: mov eax,25
            mov ebx,[esp+4]
            mov ecx,[esp+8]
            mov edx,[esp+12]
            int 0x80
            ret
sys_bind: mov eax,26
          mov ebx,[esp+4]
          mov ecx,[esp+8]
          mov edx,[esp+12]
          int 0x80
          ret
sys_connect: mov eax,27
             mov ebx,[esp+4]
             mov ecx,[esp+8]
             mov edx,[esp+12]
             int 0x80
             ret
sys_send: mov eax,28
          mov ebx,[esp+4]
          mov ecx,[esp+8]
          mov edx,[esp+12]
          mov esi,[esp+16]
          int 0x80
          ret
sys_recv: mov eax,29
          mov ebx,[esp+4]
          mov ecx,[esp+8]
          mov edx,[esp+12]
          mov esi,[esp+16]
          int 0x80
          ret
sys_sendto: mov eax,30
            mov ebx,[esp+4]
            mov ecx,[esp+8]
            mov edx,[esp+12]
            mov esi,[esp+16]
            mov edi,[esp+20]
            mov ebp,[esp+24]
            int 0x80
            ret
sys_recvfrom: mov eax,31
              mov ebx,[esp+4]
              mov ecx,[esp+8]
              mov edx,[esp+12]
              mov esi,[esp+16]
              mov edi,[esp+20]
              mov ebp,[esp+24]
              int 0x80
              ret
sys_sockclose: mov eax,32
               mov ebx,[esp+4]
               int 0x80
               ret
sys_netctl: mov eax,33
            mov ebx,[esp+4]
            mov ecx,[esp+8]
            mov edx,[esp+12]
            int 0x80
            ret
global sys_getuid, sys_geteuid, sys_setuid, sys_chmod, sys_chown, sys_umask, sys_chroot, sys_capget, sys_capset, sys_auth
sys_getuid: mov eax,34
            int 0x80
            ret
sys_setuid: mov eax,35
            mov ebx,[esp+4]
            int 0x80
            ret
sys_geteuid: mov eax,36
             int 0x80
             ret
sys_chmod: mov eax,37
           mov ebx,[esp+4]
           mov ecx,[esp+8]
           int 0x80
           ret
sys_chown: mov eax,38
           mov ebx,[esp+4]
           mov ecx,[esp+8]
           mov edx,[esp+12]
           int 0x80
           ret
sys_umask: mov eax,39
           mov ebx,[esp+4]
           int 0x80
           ret
sys_chroot: mov eax,40
            mov ebx,[esp+4]
            int 0x80
            ret
sys_capget: mov eax,41
            int 0x80
            ret
sys_capset: mov eax,42
            mov ebx,[esp+4]
            mov ecx,[esp+8]
            int 0x80
            ret
sys_auth: mov eax,43
          mov ebx,[esp+4]
          mov ecx,[esp+8]
          mov edx,[esp+12]
          mov esi,[esp+16]
          int 0x80
          ret
global sys_getrandom, sys_time, sys_settime
sys_getrandom: mov eax,44
               mov ebx,[esp+4]
               mov ecx,[esp+8]
               mov edx,[esp+12]
               int 0x80
               ret
sys_time: mov eax,45
          int 0x80
          ret
sys_settime: mov eax,46
             mov ebx,[esp+4]
             int 0x80
             ret
global sys_clone, sys_unshare, sys_seccomp, sys_cgset
sys_clone: mov eax,47
           mov ebx,[esp+4]
           int 0x80
           ret
sys_unshare: mov eax,48
             mov ebx,[esp+4]
             int 0x80
             ret
sys_seccomp: mov eax,49
             mov ebx,[esp+4]
             int 0x80
             ret
sys_cgset: mov eax,50
           mov ebx,[esp+4]
           mov ecx,[esp+8]
           mov edx,[esp+12]
           int 0x80
           ret

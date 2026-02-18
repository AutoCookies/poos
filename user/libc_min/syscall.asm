[bits 32]
global sys_write, sys_exit, sys_yield, sys_sleep, sys_getpid
sys_write: mov eax,1 ; ebx=ptr ecx=len
           mov ebx,[esp+4]
           mov ecx,[esp+8]
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

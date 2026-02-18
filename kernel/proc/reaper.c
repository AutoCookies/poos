#include "proc.h"
#include "../vfs/fdtable.h"
#include "../mem/heap.h"

extern struct proc* proc_find(u32 pid);

void proc_reap_zombies(void) {
    struct proc* it = proc_find(0);
    while (it) {
        if (it->state == PROC_DEAD) {
            for (u32 i = 0; i < FD_MAX; ++i) {
                if (it->fdt.files[i]) {
                    file_put(it->fdt.files[i]);
                    it->fdt.files[i] = 0;
                }
            }
            it->state = PROC_ZOMBIE; /* prevent double free path in tiny kernel */
        }
        it = it->next;
    }
}

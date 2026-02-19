#include "mm.h"
#include "addrspace.h"
#include "pagecache.h"
#include "../proc/proc.h"

void vga_write(const char*);
void vga_write_u32(u32);
void vga_write_hex(u32);

static struct vm_counters g_vm;

void mm_init(void) { g_vm = (struct vm_counters){0}; pagecache_init(); }
struct vm_counters* mm_counters(void) { return &g_vm; }

void mm_log_vmstat(void) {
    u32 hits=0, misses=0, entries=0;
    pagecache_stats(&hits,&misses,&entries);
    vga_write("vmstat faults="); vga_write_u32(g_vm.faults_total);
    vga_write(" handled="); vga_write_u32(g_vm.faults_handled);
    vga_write(" segv="); vga_write_u32(g_vm.faults_sigsegv);
    vga_write(" cow="); vga_write_u32(g_vm.cow_faults);
    vga_write(" copies="); vga_write_u32(g_vm.cow_copies);
    vga_write(" anon="); vga_write_u32(g_vm.anon_pages_alloc);
    vga_write(" file="); vga_write_u32(g_vm.file_pages_loaded);
    vga_write(" pchit="); vga_write_u32(hits);
    vga_write(" pcmiss="); vga_write_u32(misses);
    vga_write(" pcent="); vga_write_u32(entries);
    vga_write("\n");
}

void mm_debug_dump_vmas(struct proc* p) {
    if (!p || !p->as) return;
    vga_write("vmas pid="); vga_write_u32(p->pid); vga_write("\n");
    for (struct vma* it=p->as->vmas; it; it=it->next) {
        vga_write("  "); vga_write_hex(it->start); vga_write("-"); vga_write_hex(it->end);
        vga_write(" prot="); vga_write_hex(it->prot); vga_write(" flags="); vga_write_hex(it->flags);
        vga_write(" off="); vga_write_hex(it->file_off); vga_write("\n");
    }
}

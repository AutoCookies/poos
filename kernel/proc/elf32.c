#include "elf32.h"
#include "../mem/pmm.h"
#include "../mem/vmm.h"
#include "../mem/mem.h"

typedef struct { u8 e_ident[16]; u16 e_type; u16 e_machine; u32 e_version; u32 e_entry; u32 e_phoff; u32 e_shoff; u32 e_flags; u16 e_ehsize; u16 e_phentsize; u16 e_phnum; u16 e_shentsize; u16 e_shnum; u16 e_shstrndx; } Elf32_Ehdr;
typedef struct { u32 p_type,p_offset,p_vaddr,p_paddr,p_filesz,p_memsz,p_flags,p_align; } Elf32_Phdr;

int elf32_load_image(struct proc* p, const u8* image, u32 size, u32* out_entry) {
    u32 old_cr3 = read_cr3();
    write_cr3(p->cr3);

    if (size < sizeof(Elf32_Ehdr)) { write_cr3(old_cr3); return -1; }
    const Elf32_Ehdr* eh = (const Elf32_Ehdr*)image;
    if (eh->e_ident[0] != 0x7F || eh->e_ident[1] != 'E' || eh->e_ident[2] != 'L' || eh->e_ident[3] != 'F') { write_cr3(old_cr3); return -1; }
    if (eh->e_machine != 3U) { write_cr3(old_cr3); return -1; }

    for (u32 i = 0; i < eh->e_phnum; ++i) {
        const Elf32_Phdr* ph = (const Elf32_Phdr*)(image + eh->e_phoff + i * eh->e_phentsize);
        if (ph->p_type != 1U) continue;
        u32 start = ph->p_vaddr & ~0xFFFU;
        u32 end = (ph->p_vaddr + ph->p_memsz + 0xFFFU) & ~0xFFFU;
        for (u32 va = start; va < end; va += 4096U) {
            u32 fr = pmm_alloc_frame();
            if (!fr) { write_cr3(old_cr3); return -1; }
            vmm_map_page_in(p->cr3, va, fr, PAGE_USER | PAGE_RW);
            mem_set((void*)va, 0, 4096U);
        }
        mem_copy((void*)ph->p_vaddr, image + ph->p_offset, ph->p_filesz);
    }

    *out_entry = eh->e_entry;
    write_cr3(old_cr3);
    return 0;
}

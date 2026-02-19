#include "pci.h"

static inline void outl(u16 p,u32 v){__asm__ volatile("outl %0,%1"::"a"(v),"Nd"(p));}
static inline u32 inl(u16 p){u32 v;__asm__ volatile("inl %1,%0":"=a"(v):"Nd"(p));return v;}

static u32 pci_cfg_read32(u8 bus, u8 slot, u8 func, u8 off) {
    u32 addr=(1U<<31)|((u32)bus<<16)|((u32)slot<<11)|((u32)func<<8)|(off&0xFCU);
    outl(0xCF8,addr);
    return inl(0xCFC);
}

static void pci_cfg_write16(u8 bus, u8 slot, u8 func, u8 off, u16 val) {
    u32 addr=(1U<<31)|((u32)bus<<16)|((u32)slot<<11)|((u32)func<<8)|(off&0xFCU);
    outl(0xCF8,addr);
    u32 old=inl(0xCFC);
    u32 shift=(off&2U)*8U;
    u32 nv=(old & ~(0xFFFFU<<shift))|((u32)val<<shift);
    outl(0xCFC,nv);
}

void pci_init(void) {}

int pci_find(u16 vendor, u16 device, pci_dev_t* out) {
    for (u16 b = 0; b < 256; ++b) {
        for (u8 s = 0; s < 32; ++s) {
            for (u8 f = 0; f < 8; ++f) {
                u32 vd = pci_cfg_read32((u8)b, s, f, 0x00);
                if ((vd & 0xFFFFU) == 0xFFFFU) { if (f == 0) break; continue; }
                if ((vd & 0xFFFFU) == vendor && ((vd >> 16) & 0xFFFFU) == device) {
                    if (!out) return 0;
                    out->bus = (u8)b; out->slot = s; out->func = f;
                    out->vendor_id = vendor; out->device_id = device;
                    u32 cls = pci_cfg_read32((u8)b, s, f, 0x08);
                    out->prog_if = (u8)(cls >> 8); out->subclass = (u8)(cls >> 16); out->class_code = (u8)(cls >> 24);
                    for (u32 i = 0; i < 6; ++i) {
                        u32 bar = pci_cfg_read32((u8)b, s, f, (u8)(0x10 + i * 4));
                        out->bar[i] = bar;
                        out->bar_io[i] = (bar & 1U) ? 1U : 0U;
                    }
                    u32 il = pci_cfg_read32((u8)b, s, f, 0x3C);
                    out->irq_line = (u8)(il & 0xFFU);
                    return 0;
                }
                if (f == 0) {
                    u32 hdr = pci_cfg_read32((u8)b, s, f, 0x0C);
                    if (((hdr >> 16) & 0x80U) == 0) break;
                }
            }
        }
    }
    return -1;
}

void pci_enable_bus_master(const pci_dev_t* dev) {
    u32 c = pci_cfg_read32(dev->bus, dev->slot, dev->func, 0x04);
    u16 cmd = (u16)(c & 0xFFFFU);
    cmd |= (1U << 2) | (1U << 0);
    pci_cfg_write16(dev->bus, dev->slot, dev->func, 0x04, cmd);
}

u32 pci_map_bar(const pci_dev_t* dev, u32 idx, bool* is_io) {
    if (idx >= 6) return 0;
    if (is_io) *is_io = dev->bar_io[idx] != 0;
    return dev->bar_io[idx] ? (dev->bar[idx] & ~3U) : (dev->bar[idx] & ~0xFU);
}

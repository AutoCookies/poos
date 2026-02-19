#ifndef POOS_PCI_H
#define POOS_PCI_H

#include "../types.h"

typedef struct {
    u8 bus, slot, func;
    u16 vendor_id, device_id;
    u8 class_code, subclass, prog_if;
    u8 irq_line;
    u32 bar[6];
    u8 bar_io[6];
} pci_dev_t;

void pci_init(void);
int pci_find(u16 vendor, u16 device, pci_dev_t* out);
void pci_enable_bus_master(const pci_dev_t* dev);
u32 pci_map_bar(const pci_dev_t* dev, u32 idx, bool* is_io);

#endif

#include "audit.h"
void vga_write(const char*);
void audit_log(const char* msg){ vga_write("[AUDIT] "); vga_write(msg); vga_write("\n"); }

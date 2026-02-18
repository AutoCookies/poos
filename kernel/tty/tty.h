#ifndef POOS_TTY_H
#define POOS_TTY_H
#include "../types.h"
void tty_init(void);
void tty_push_char(char c);
int tty_read_line(void* buf, u32 len);
void kbd_init(void);
#endif

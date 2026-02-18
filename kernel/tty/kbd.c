#include "tty.h"
#include "../types.h"
#include "../arch/x86/idt.h"

static const char map[128] = {
0,27,'1','2','3','4','5','6','7','8','9','0','-','=',8,9,
'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s',
'd','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v',
'b','n','m',',','.','/',0,'*',0,' '
};
static void kbd_irq(struct trapframe* tf){ (void)tf; u8 sc=inb(0x60); if(sc&0x80) return; char c=(sc<128)?map[sc]:0; if(c) tty_push_char(c); }
void kbd_init(void){ idt_register_handler(33, kbd_irq); }

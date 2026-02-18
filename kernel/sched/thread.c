#include "thread.h"

void thread_fill_name(struct thread* t, const char* name) {
    u32 i = 0;
    for (; i < THREAD_NAME_MAX - 1U && name[i] != '\0'; ++i) {
        t->name[i] = name[i];
    }
    t->name[i] = '\0';
    for (++i; i < THREAD_NAME_MAX; ++i) {
        t->name[i] = '\0';
    }
}

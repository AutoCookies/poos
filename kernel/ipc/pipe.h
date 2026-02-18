#ifndef POOS_IPC_PIPE_H
#define POOS_IPC_PIPE_H
#include "../types.h"
struct file;
int pipe_create_files(struct file** rd, struct file** wr);
#endif

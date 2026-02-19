#ifndef POOS_TLS_H
#define POOS_TLS_H
#include "../../types.h"
#define TLS_ERR_UNSUPPORTED -100
int tls_connect_socket(int fd, const char* host);
#endif

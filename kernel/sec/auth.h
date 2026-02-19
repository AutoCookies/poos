#ifndef POOS_SEC_AUTH_H
#define POOS_SEC_AUTH_H
#include "../types.h"
int auth_lookup_user(const char* user, u32* uid, u32* gid);
int auth_verify_password(const char* user, const char* pass);
#endif

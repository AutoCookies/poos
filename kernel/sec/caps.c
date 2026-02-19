#include "caps.h"
int caps_has(u32 eff, u32 cap){ if(cap>=CAP_MAX) return 0; return (eff & (1U<<cap))!=0; }

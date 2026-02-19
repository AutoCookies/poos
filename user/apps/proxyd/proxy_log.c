extern int printf_min(const char*, ...);
void proxy_log(const char* m){ printf_min("proxyd: %s\n",m); }

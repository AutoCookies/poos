int strcmp_min(const char* a, const char* b){int i=0;while(a[i]&&b[i]){if(a[i]!=b[i]) return (int)((unsigned char)a[i]-(unsigned char)b[i]);i++;}return (int)((unsigned char)a[i]-(unsigned char)b[i]);}
int strlen_min(const char* s){int i=0;while(s[i])i++;return i;}
void strcpy_min(char* d,const char* s){while(*s){*d++=*s++;}*d=0;}

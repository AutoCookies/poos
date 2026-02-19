extern int printf_min(const char*, ...);
void proxy_log(const char* m){
    char safe[96]; int p=0;
    for(int i=0;m&&m[i]&&p<95;i++){ char c=m[i]; if(c<' '||c>'~') c='?'; safe[p++]=c; }
    safe[p]=0;
    printf_min("proxyd: %s\n",safe);
}

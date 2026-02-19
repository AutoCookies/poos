int proxy_timeout_should_drop(unsigned int start_ms, unsigned int now_ms, unsigned int total_timeout_ms){
    return now_ms - start_ms > total_timeout_ms;
}

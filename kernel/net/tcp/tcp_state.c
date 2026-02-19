#include "tcp_state.h"

const char* tcp_state_name(enum tcp_state s){
    switch(s){
        case TCP_CLOSED: return "CLOSED";
        case TCP_LISTEN: return "LISTEN";
        case TCP_SYN_SENT: return "SYN_SENT";
        case TCP_SYN_RECEIVED: return "SYN_RECEIVED";
        case TCP_ESTABLISHED: return "ESTABLISHED";
        case TCP_FIN_WAIT_1: return "FIN_WAIT_1";
        case TCP_FIN_WAIT_2: return "FIN_WAIT_2";
        case TCP_CLOSE_WAIT: return "CLOSE_WAIT";
        case TCP_LAST_ACK: return "LAST_ACK";
        case TCP_TIME_WAIT: return "TIME_WAIT";
        default: return "?";
    }
}

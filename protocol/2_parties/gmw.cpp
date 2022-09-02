#include "gmw.h"

void GMW::and_c(T *a, T *b, T *c, uint32_t lens, T *a_p, T *b_p, T*c_p){
    T alpha[2*lens],Alpha[2*lens],res[lens];
    for(int i = 0; i < lens; i++){
        alpha[i] = a[i] ^ a_p[i];
        alpha[i+lens] = b[i] ^ b_p[i];
    }
    if(Config::myconfig->check("player0")){
        P2Pchannel::mychnl->send_data_to("player1", alpha, lens*2*sizeof(T));
        P2Pchannel::mychnl->recv_data_from("player1", Alpha, lens*2*sizeof(T));
    }else if(Config::myconfig->check("player1")){
        P2Pchannel::mychnl->send_data_to("player0", alpha, lens*2*sizeof(T));
        P2Pchannel::mychnl->recv_data_from("player0", Alpha, lens*2*sizeof(T));
    }
    for(int i = 0; i < 2*lens; i++){
        alpha[i] ^= Alpha[i];
    }
    for(int i = 0; i < lens; i++){
        res[i] = b[i]&Alpha[i + lens] ^ a[i]&Alpha[i] ^ c_p[i];
        if(Config::myconfig->check("player1"))
        res[i] ^= Alpha[i]&Alpha[i + lens];
    }
}
#ifndef _PRE_OP_BASED_H__
#define _PRE_OP_BASED_H__
#include "net.hpp"
#include "gc_op.h"
#include <openssl/rand.h>
template<typename T>
void shared_select(const T* data0, const T* data1, bool select_bit, T* res, uint32_t lens, std::pair<std::string, std::string> tarro){
    /*alpha & (z) ^ z0*/
    size_t times = sizeof(T) / sizeof(uint8_t);
    size_t times_t = lens * times;
    
    uint8_t z[times_t], r[times_t], z_r[times_t];
    RAND_bytes(r, times_t);
    Pratical_OT *ot = new Pratical_OT(P2Pchannel::mychnl, Config::myconfig->get_player());
    for(int i = 0; i < times_t; i++){
        z[i] = data0[i] ^ data1[i];
        z_r[i] = z[i] ^ r[i];
    }
    std::cout<<"\n";
    /*2,3*/
    if(Config::myconfig->check(tarro.first)){
        ot->send(reinterpret_cast<emp::block*>(r), reinterpret_cast<emp::block*>(z_r), times_t/16, tarro.first, tarro.second);
        ot->recv(reinterpret_cast<emp::block*>(res), &select_bit, times_t/16, tarro.first, tarro.second);
    }else if(Config::myconfig->check(tarro.second)){
        ot->recv(reinterpret_cast<emp::block*>(res), &select_bit, times_t/16, tarro.second, tarro.first);
        ot->send(reinterpret_cast<emp::block*>(r), reinterpret_cast<emp::block*>(z_r), times_t/16, tarro.second, tarro.first);
    }
    /*a_p & z_p ^ z_p & a_'p*/
    
    for(int i = 0; i < times_t; i++){
        res[i] ^= r[i];
        
        if(select_bit){
            res[i] ^= z[i];
        }
        res[i] ^= data0[i];
    }
    delete ot;
}
#endif
#ifndef _PRE_OP_BASED_H__
#define _PRE_OP_BASED_H__
#include "net.hpp"
#include "gc_op.h"
#include <openssl/rand.h>
template<typename T, bool Is_xor = true>
void shared_select(const T* data0, const T* data1, bool select_bit, T* res, uint32_t lens, std::pair<std::string, std::string> tarro){
    /*
    data0, data1 and select_bit are xor_shared
    */
    /*alpha & (z) ^ z0*/
    size_t times = sizeof(T) / sizeof(uint8_t);
    size_t times_t = lens * times;
    bool select_bits[times_t/16];
    for(int i = 0; i < times_t/16; i++) select_bits[i] = select_bit;
    uint8_t z[times_t], r[times_t], z_r[times_t];
    RAND_bytes(r, times_t);
    Pratical_OT *ot = new Pratical_OT(P2Pchannel::mychnl, Config::myconfig->get_player());
    for(int i = 0; i < times_t; i++){
        if(Is_xor){
            z[i] = data0[i] ^ data1[i];
            z_r[i] = z[i] ^ r[i];
        }else{
            z[i] = data1[i] - data0[i];
            z_r[i] = z[i] + r[i];
        }
    }
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
        if(Is_xor) res[i] ^= r[i];
        else res[i] -= r[i];
        if(select_bit){
            if(Is_xor) res[i] ^= z[i];
            else res[i] += z[i];
        }
        if(Is_xor) res[i] ^= data0[i];
        else res[i] += data0[i];
    }
    delete ot;
}

template<typename T>
void add_shared_select(const T* data0, const T* data1, bool select_bit, T* res, uint32_t lens, std::pair<std::string, std::string> tarro){
    /*
    data is arithmatic_shared and select_bit is binary_shared
    */
    size_t times = sizeof(T) / sizeof(uint8_t);
    size_t times_t = lens * times;
    bool select_bits[times_t/16];
    T z[lens], r[lens], z_r[lens];
    RAND_bytes(reinterpret_cast<uint8_t*>(r), times_t);
    Pratical_OT *ot = new Pratical_OT(P2Pchannel::mychnl, Config::myconfig->get_player());
    
    for(int i = 0; i < lens; i++){
        z[i] = data0[i] - r[i];
        z_r[i] = data1[i] - r[i];
    }
    for(int i = 0; i < times_t/16; i++) select_bits[i] = select_bit;
    if(Config::myconfig->check(tarro.first)){
        if(select_bit){
            ot->send(reinterpret_cast<emp::block*>(z), reinterpret_cast<emp::block*>(z_r), times_t/16, tarro.first, tarro.second);
        }else{
            ot->send(reinterpret_cast<emp::block*>(z_r), reinterpret_cast<emp::block*>(z), times_t/16, tarro.first, tarro.second);
        }
        ot->recv(reinterpret_cast<emp::block*>(res), select_bits, times_t/16, tarro.first, tarro.second);
    }else if(Config::myconfig->check(tarro.second)){
        ot->recv(reinterpret_cast<emp::block*>(res), select_bits, times_t/16, tarro.second, tarro.first);
        if(select_bit){
            ot->send(reinterpret_cast<emp::block*>(z), reinterpret_cast<emp::block*>(z_r), times_t/16, tarro.second, tarro.first);
        }else{
            ot->send(reinterpret_cast<emp::block*>(z_r), reinterpret_cast<emp::block*>(z), times_t/16, tarro.second, tarro.first);
        }
    }
    for(int i = 0; i < lens; i++) res[i] += r[i];
    delete ot;
}
template<typename T>
void b_mul_arith(const T* data, bool select_bit, T* res, uint32_t lens, std::pair<std::string, std::string> tarro){
    /*
    data is arithmatic_shared and select_bit is binary_shared
    */
    T data0[lens];
    add_shared_select<T>(data0, data, select_bit, res, lens, tarro);
}
template<typename T, bool is_second = true>
void half_select(const T* data0, const T* data1, bool select_bit, T* res, uint32_t lens, std::pair<std::string, std::string> tarro){
    size_t times = sizeof(T) / sizeof(uint8_t);
    size_t times_t = lens * times;
    bool select_bits[times_t/16];
    for(int i = 0; i < times_t/16; i++) select_bits[i] = select_bit;
    T z[lens], z_r[lens];
    RAND_bytes(reinterpret_cast<uint8_t*>(res), times_t);
    Pratical_OT *ot = new Pratical_OT(P2Pchannel::mychnl, Config::myconfig->get_player());
    if(is_second){
        if(Config::myconfig->check(tarro.first)){
            for(int i = 0; i < lens; i++){
                z[i] = data0[i] - res[i];
                z_r[i] = data1[i] - res[i];
            }
            ot->send(reinterpret_cast<emp::block*>(z), reinterpret_cast<emp::block*>(z_r), times_t/16, tarro.first, tarro.second);
        }else if(Config::myconfig->check(tarro.second)){
            ot->recv(reinterpret_cast<emp::block*>(res), select_bits, times_t/16, tarro.second, tarro.first);
        }
    }else{
       if(Config::myconfig->check(tarro.second)){
            for(int i = 0; i < lens; i++){
                z[i] = data0[i] - res[i];
                z_r[i] = data1[i] - res[i];
            }
            ot->send(reinterpret_cast<emp::block*>(z), reinterpret_cast<emp::block*>(z_r), times_t/16, tarro.second, tarro.first);
        }else if(Config::myconfig->check(tarro.first)){
            ot->recv(reinterpret_cast<emp::block*>(res), select_bits, times_t/16, tarro.first, tarro.second);
        } 
    }
    delete ot;
}
#endif
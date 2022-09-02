#ifndef _FACTORY_H__
#define _FACTORY_H__
#include <set>
#include <stdint.h>
#include <iostream>
#include <string.h>
#include <gmp.h>
#include <gmpxx.h>
template <typename T>
void get_rand(std::set<std::string> roles, T* data, uint32_t lens){
    /*TODO: 对roles中的角色使用共同持有的seed生成随机数*/
    memset(data, 2, lens*sizeof(T));
}
template<typename T>
void get_seed(std::set<std::string> roles, T* data, uint32_t lens){  
    if(Config::myconfig->check(*roles.begin())){
        RAND_bytes(reinterpret_cast<uint8_t*>(data), lens*sizeof(T));
        for(auto & rl:roles){
            if(!Config::myconfig->check(rl)){
                P2Pchannel::mychnl->send_data_to(rl, data, sizeof(T)*lens);
            }
        }
    }else{
        for(auto & rl:roles){
            if(Config::myconfig->check(rl)){
                P2Pchannel::mychnl->recv_data_from(*roles.begin(), data, sizeof(T)*lens);
            }
        }
    }
    P2Pchannel::mychnl->flush_all();
}
template <uint8_t trans = 8>
void get_seed(std::set<std::string> roles, mpz_class* data,  mpz_class pri, uint32_t lens){  
    uint8_t temp[lens * trans];
    if(Config::myconfig->check(*roles.begin())){
        RAND_bytes(temp, lens*trans);
        for(auto & rl:roles){
            if(!Config::myconfig->check(rl)){
                P2Pchannel::mychnl->send_data_to(rl, temp, trans*lens);
            }
        }
    }else{
        for(auto & rl:roles){
            if(Config::myconfig->check(rl)){
                P2Pchannel::mychnl->recv_data_from(*roles.begin(), temp, trans*lens);
            }
        }
    }
    P2Pchannel::mychnl->flush_all();
    for(int i = 0; i < lens; i++){
        mpz_import(data[i].get_mpz_t(), trans, 1, sizeof(uint8_t), 0, 0, temp + trans * i);
        data[i] = data[i] % pri;
    }
}
#endif
#ifndef _FACTORY_H__
#define _FACTORY_H__
#include <set>
#include <stdint.h>
#include <iostream>
#include <string.h>
#include "fss-common.h"
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
}
#endif
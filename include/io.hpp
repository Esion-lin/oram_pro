#ifndef _IO_MPC_H__
#define _IO_MPC_H__
#include "net.hpp"
#include "config.hpp"
#include <set>
#include <functional>
#include <openssl/rand.h>
#include "factory.hpp"
template<typename T>
void array_op(std::function<T (T, T)>op_cb, T* a, T* b, T* c, size_t len){
    for(int i = 0; i < len; i++){
        c[i] = op_cb(a[i], b[i]);
    }
}

template<typename T>
void add_share(std::string owner, std::set<std::string> holders, T* data, uint32_t lens,
    std::function<T (T, T)>op_cb = [](T a, T b) -> T{return a - b;},
    std::function<T (T, T)>nop_cb = [](T a, T b) -> T{return a + b;}){
    /*该方法可能会改写data*/
    bool incl = false;
    if(Config::myconfig->check(owner)){
        T mydata[lens];
        for(auto player:holders){
            if(!Config::myconfig->check(player)){
                T tmp[lens];
                get_seed<T>({owner, player}, tmp, lens);
                array_op<T>(op_cb, data, tmp, data, lens);
                
            }else{
                incl = true;
                RAND_bytes(reinterpret_cast<uint8_t*>(mydata), lens*sizeof(T));
                array_op<T>(op_cb, data, mydata, data, lens);
            }
        }
        if(incl){
            array_op<T>(nop_cb, data, mydata, data, lens);
        }else{
            P2Pchannel::mychnl->send_data_to(*holders.begin(), data, lens*sizeof(T));
        }
    }else{
        for(auto player:holders){
            if(Config::myconfig->check(player)){
                get_seed<T>({owner, player}, data, lens);
            }
            if(player == owner){
                incl = true;
            }
        }
        if((!incl) && Config::myconfig->check(*holders.begin())){
            T tmp[lens];
            P2Pchannel::mychnl->recv_data_from(owner, tmp, lens*sizeof(T));
            array_op<T>(nop_cb, data, tmp, data, lens);
        }
    }
}
template<typename T>
void add_reveal(std::set<std::string> owner, std::set<std::string> holders, T* data, uint32_t lens,std::function<T (T, T)>op_cb = [](T a, T b) -> T{return a + b;}){
    bool is_holder = false;
    for(auto& player: holders){
        if(Config::myconfig->check(player)){
            is_holder = true;
            for(auto &tar: owner)
                if(player != tar) 
                    P2Pchannel::mychnl->send_data_to(tar, data, lens*sizeof(T));
        }
    }
    for(auto &player: owner){
        if(Config::myconfig->check(player)){
            if(!is_holder)
                memset(data, 0, lens*sizeof(T));
            for(auto &tar: holders){
                if(player != tar) {
                    T tmp[lens];
                    P2Pchannel::mychnl->recv_data_from(tar, tmp, lens*sizeof(T));
                    array_op<T>(op_cb, data, tmp, data, lens);
                    
                }
            }
        }
    }
    
}
template<typename T>
void deliver(T* data, T* newdata, uint32_t lens){
    P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), data, lens*sizeof(T));
    P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), newdata, lens*sizeof(T));
}
template<typename T>
void replicated_share(std::string owner, std::set<std::string> holders, T* data1, T* data2, uint32_t lens,
    std::function<T (T, T)>op_cb = [](T a, T b) -> T{return a - b;},
    std::function<T (T, T)>nop_cb = [](T a, T b) -> T{return a + b;}){
    add_share<T>(owner, holders, data1, lens, op_cb, nop_cb);
    deliver<T>(data1, data2, lens);

}
#endif
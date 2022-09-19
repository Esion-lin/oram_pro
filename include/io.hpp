#ifndef _IO_MPC_H__
#define _IO_MPC_H__
#include "net.hpp"
#include "config.hpp"
#include <set>
#include <functional>
#include <openssl/rand.h>
#include "factory.hpp"
template<uint32_t trans = 8>
void mpz2byte(uint8_t* target, mpz_class* source, uint32_t lens_of_source){
    size_t b=0;
    for(int i = 0; i < lens_of_source; i++){
        mpz_export(target + i * trans, &b, -1, sizeof(uint8_t), 0, 0, source[i].get_mpz_t());
        
    }
}

template<uint32_t trans = 8>
void byte2mpz(mpz_class* target, uint8_t* source, uint32_t lens_of_target){
    for(int i = 0; i < lens_of_target; i++){
        mpz_import(target[i].get_mpz_t(), trans, -1, sizeof(source[0]), 0, 0, source + trans * i);
    }
}
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
inline void add_share(std::string owner, std::set<std::string> holders, mpz_class* data, mpz_class pri, uint32_t lens){  
    if(Config::myconfig->check(owner)){
        mpz_class mydata[lens];
        for(auto player:holders){
            if(!Config::myconfig->check(player)){
                mpz_class tmp[lens];
                get_seed<>({owner, player}, tmp, pri, lens);
                array_op<mpz_class>([pri](mpz_class a, mpz_class b) -> mpz_class{return (a + pri - b) % pri;}, data, tmp, data, lens);
                
            }
        }
    }else{
        for(auto player:holders){
            if(player != owner && Config::myconfig->check(player)){
                get_seed<>({owner, player}, data, pri, lens);
            }
        }
    }
}
template<uint32_t trans = 8>
void add_reveal(std::string owner, std::set<std::string> holders, mpz_class* data, mpz_class pri, uint32_t lens){
    uint8_t temp[lens * trans] = {0};
    
    if(Config::myconfig->check(owner)){
        mpz_class temp2[lens];
        for(auto& player: holders)
            if(player != owner){
                P2Pchannel::mychnl->recv_data_from(player, temp, lens*trans);
                
                byte2mpz<>(temp2, temp, lens);
                std::cout<<player<< temp2[0] <<std::endl;
                array_op<mpz_class>([pri](mpz_class a, mpz_class b) -> mpz_class{return (a + b) % pri;}, temp2, data, data, lens);
                
            }
            
    }else{
        memset(temp, 0, sizeof(uint8_t)*lens * trans);
        mpz2byte<>(temp, data, lens);
        std::cout<< data[0] <<std::endl;
        for(auto& player: holders){
            if( Config::myconfig->check(player)){
                P2Pchannel::mychnl->send_data_to(owner, temp, lens*trans);
                
            }
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
void add_reveal(std::set<std::string> owner, std::set<std::string> holders, T* data, uint32_t lens,T* data2, uint32_t lens2,T* data3, uint32_t lens3,std::function<T (T, T)>op_cb = [](T a, T b) -> T{return a + b;}){
    P2Pchannel::mychnl->set_flush(false);
    bool is_holder = false;
    for(auto& player: holders){
        if(Config::myconfig->check(player)){
            is_holder = true;
            for(auto &tar: owner)
                if(player != tar){
                    P2Pchannel::mychnl->send_data_to(tar, data, lens*sizeof(T));
                    P2Pchannel::mychnl->send_data_to(tar, data2, lens2*sizeof(T));
                    P2Pchannel::mychnl->send_data_to(tar, data3, lens3*sizeof(T));
                }
        }
    }
    P2Pchannel::mychnl->flush_all();
    for(auto &player: owner){
        if(Config::myconfig->check(player)){
            if(!is_holder){
                memset(data, 0, lens*sizeof(T));
                memset(data2, 0, lens2*sizeof(T));
                memset(data3, 0, lens3*sizeof(T));
            }
            for(auto &tar: holders){
                if(player != tar) {
                    T tmp[lens],tmp2[lens2],tmp3[lens3];
                    P2Pchannel::mychnl->recv_data_from(tar, tmp, lens*sizeof(T));
                    array_op<T>(op_cb, data, tmp, data, lens);
                    P2Pchannel::mychnl->recv_data_from(tar, tmp2, lens2*sizeof(T));
                    array_op<T>(op_cb, data2, tmp2, data2, lens2);
                    P2Pchannel::mychnl->recv_data_from(tar, tmp3, lens3*sizeof(T));
                    array_op<T>(op_cb, data3, tmp3, data3, lens3);
                    
                }
            }
        }
    }
    P2Pchannel::mychnl->flush_all();
    P2Pchannel::mychnl->set_flush(true);
    
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
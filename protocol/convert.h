#ifndef _CONVERT_H__
#define _CONVERT_H__
#include <stdint.h>
#include "net.hpp"
#include "preliminaries.hpp"
using namespace std;
class Convert{
private:

    void* zero_share;
    uint16_t zero_share_len;
    bool zero_flag = false;
    P2Pchannel * p2pchnl;
    string st;
public:
    Convert(string st, P2Pchannel *p2pchnl):st(st),p2pchnl(p2pchnl){}
    template <typename T>
    void fourpc_zeroshare(uint16_t len){
        /*生成四方零share*/
        zero_share_len = len;
        zero_share = malloc(len*sizeof(T));
        if(st == "aid"){
            T r1[len],r2[len],r3[len],r0[len];
            rand_t<T>(r0, len);rand_t<T>(r1, len);rand_t<T>(r2, len);
            for(int i = 0; i < len; i++){
                r3[i] = (uint32_t)0 - r0[i] - r1[i] - r2[i];
            }
            p2pchnl->send_data_to("player0", r0, sizeof(T)*len);
            p2pchnl->send_data_to("player1", r1, sizeof(T)*len);
            p2pchnl->send_data_to("player2", r2, sizeof(T)*len);
            p2pchnl->send_data_to("player3", r3, sizeof(T)*len);
            free(zero_share);
        }else{
            p2pchnl->recv_data_from("aid", reinterpret_cast<T*>(zero_share), sizeof(T)*len);
        }
        
        zero_flag = true;
    }
    template <typename T>
    void fourpc_share_2_replicated_share(T * data, uint16_t len){
        if(!zero_flag) return;
        if(len > zero_share_len) return;
        if(st == "aid"){
            return;
        }
        T data2[len];
        
        for(int i = 0; i < len; i ++){
            data2[i] = data[i] + reinterpret_cast<T*>(zero_share)[i];
        }
        
        twopc_reveal<T>(data2, data, len, st, p2pchnl);
        free(zero_share);
        zero_flag = false;
    }

};
#endif
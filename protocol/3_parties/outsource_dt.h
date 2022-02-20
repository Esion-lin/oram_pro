#ifndef _OUT_SOURCING_H__
#define _OUT_SOURCING_H__
#include "csot.hpp"
#include "sot.hpp"
template <typename T> 
struct Node{
    T k, t;
};
template <typename T> 
class Dt{
    private:
    uint32_t data_lens, node_lens, v_lens;
    std::string owner;
    std::string S[2];
    Node<T> * Ps;
    T * vs;
    public:

    Dt(Node<T> * Ps, T * vs, uint32_t data_lens, uint32_t node_lens, uint32_t v_lens, std::string owner, std::string S0, std::string S1):Ps(Ps),vs(vs), data_lens(data_lens), node_lens(node_lens), v_lens(v_lens), owner(owner){
        S[0] = S0; S[1] = S1;
        Node<T> * Ps0, *Ps1;
        T * vs0, vs1;
        Ps0 = (Node<T> *)malloc(sizeof(Node<T>) * node_lens);
        Ps1 = (Node<T> *)malloc(sizeof(Node<T>) * node_lens);
        vs0 = (T*) malloc(sizeof(T) * v_lens);
        vs1 = (T*) malloc(sizeof(T) * v_lens);
        for(int i = 0; i < node_lens; i++){
            RAND_bytes(reinterpret_cast<uint8_t*>(&Ps0[i].k), sizeof(T));
            RAND_bytes(reinterpret_cast<uint8_t*>(&Ps0[i].t), sizeof(T));
            Ps0[i].k %= data_lens;
            Ps1[i].k = (Ps[i].k - Ps0[i].k + data_lens) % data_lens;
            Ps1[i].t = (Ps[i].t - Ps0[i].t);
            
        }
        for(int i = 0; i < v_lens; i++){
            RAND_bytes(reinterpret_cast<uint8_t*>(&vs0[i]), sizeof(T));
            vs1[i] = vs[i] - vs0[i];
            
        }
        if(Config::myconfig->check(owner)){
            P2Pchannel::mychnl->send_data_to(S[0], Ps0, sizeof(Node<T>) * node_lens);
            P2Pchannel::mychnl->send_data_to(S[1], Ps1, sizeof(Node<T>) * node_lens);
            P2Pchannel::mychnl->send_data_to(S[0], vs0, sizeof(T) * v_lens);
            P2Pchannel::mychnl->send_data_to(S[1], vs1, sizeof(T) * v_lens);
        }else{
            for(int i = 0; i < 2; i++ ){
                if(Config::myconfig->check(S[i])){
                    P2Pchannel::mychnl->recv_data_from(owner, Ps, sizeof(Node<T>) * node_lens);
                    P2Pchannel::mychnl->recv_data_from(owner, vs, sizeof(T) * v_lens);
                }
            }
        }
        free(Ps0);free(Ps1);
    }
    ~Dt(){
        
    }

};
    
#endif
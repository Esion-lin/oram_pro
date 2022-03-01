#ifndef _CSOT_H__
#define _CSOT_H__
#include "preliminaries.hpp"
#include "scmp.hpp"
#include "factory.hpp"
template<typename T, uint32_t L = 32, uint16_t Lambda = 16>
class CSOT{
    private:
    uint8_t n0[Lambda], n1[Lambda], n2[Lambda];
    AES_KEY n_0, n_1, n_2;
    Thr_pc_ic_id<T> *ic[3];
    T rou, tao;

    public:
    CSOT(){
        init();
    }
    ~CSOT(){
        
        for(int i = 0; i < 3; i++)
        delete ic[i];
    }
    void init(){
        get_seed<uint8_t>({"player0", "player1"}, n0, Lambda);
        AES_set_encrypt_key(n0, 128, &n_0);
        get_seed<uint8_t>({"player1", "player2"}, n1, Lambda);
        AES_set_encrypt_key(n1, 128, &n_1);
        get_seed<uint8_t>({"player2", "player0"}, n2, Lambda);
        AES_set_encrypt_key(n2, 128, &n_2);
        ic[0] = new Thr_pc_ic_id<T>(L, "player0");
        ic[1] = new Thr_pc_ic_id<T>(L, "player1");
        ic[2] = new Thr_pc_ic_id<T>(L, "player2");
    }
    void offline(uint32_t len = 1){
        RAND_bytes(reinterpret_cast<uint8_t*>(&rou), sizeof(T));
        tao = (T)1 << (L - 1) - 1;
        for(int i = 0; i < 3; i++){
            ic[i]->twopc_ic_off(len, 0, tao, ((uint32_t)1<<L), &rou);
        }

    }
    template<typename R = uint32_t>
    R online(R *block, T *m, uint32_t sid = 0, uint32_t mod = 0){
        /*
        R: overwrite mod add
        */
        
        T deltas[3][3], delta[3];
        for(int i = 0; i < 3; i ++){
            uint8_t tmmm[16],tmp[16] = {0};
            tmp[4] = i;
            memcpy(tmp, &sid, 4);
            T w0 =0, w1 =0;
            if(Config::myconfig->check("player0")){
                prf(tmmm, tmp, 16, &n_0, 1);
                memcpy(&w0, tmmm, sizeof(T));
                prf(tmmm, tmp, 16, &n_2, 1);
                memcpy(&w1, tmmm, sizeof(T));
                //prf(reinterpret_cast<uint8_t*>(&w1), tmp, sizeof(T), &n_2, 1);
            }
            else if(Config::myconfig->check("player1")){
                prf(tmmm, tmp, 16, &n_1, 1);
                memcpy(&w0, tmmm, sizeof(T));
                prf(tmmm, tmp, 16, &n_0, 1);
                memcpy(&w1, tmmm, sizeof(T));
                //prf(reinterpret_cast<uint8_t*>(&w1), tmp, sizeof(T), &n_0, 1);
            }
            else if(Config::myconfig->check("player2")){
                //prf(reinterpret_cast<uint8_t*>(&w0), tmp, sizeof(T), &n_2, 1);
                prf(tmmm, tmp, 16, &n_2, 1);
                memcpy(&w0, tmmm, sizeof(T));
                prf(tmmm, tmp, 16, &n_1, 1);
                memcpy(&w1, tmmm, sizeof(T));
            }

            deltas[i][Config::myconfig->get_idex()] = (m[1] - m[0] - w0 + w1);
        }
        deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()] = deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()] + rou;
        R blocks[3][2];
        uint32_t len_R = sizeof(R)>=16?sizeof(R):16;
        if(len_R % 16 != 0) len_R = (len_R / 16 + 1) * 16; 
        
        for(int i = 0; i < 2; i ++){
            uint8_t tmmm[len_R],tmp[len_R] = {0};
            R zeta0_0, zeta0_1;
            tmp[4] = i;
            memcpy(tmp, &sid, 4);
            if(Config::myconfig->check("player0")){
                prf(tmmm, tmp, len_R, &n_0, 1);
                memcpy(&zeta0_0, tmmm, sizeof(R));
                prf(tmmm, tmp, len_R, &n_2, 1);
                memcpy(&zeta0_1, tmmm, sizeof(R));
            }
            if(Config::myconfig->check("player1")){
                prf(tmmm, tmp, len_R, &n_1, 1);
                memcpy(&zeta0_0, tmmm, sizeof(R));
                prf(tmmm, tmp, len_R, &n_0, 1);
                memcpy(&zeta0_1, tmmm, sizeof(R));
            }
            if(Config::myconfig->check("player2")){
                prf(tmmm, tmp, len_R, &n_2, 1);
                memcpy(&zeta0_0, tmmm, sizeof(R));
                prf(tmmm, tmp, len_R, &n_1, 1);
                memcpy(&zeta0_1, tmmm, sizeof(R));
            }
            if(mod != 0){
                zeta0_1 %= mod;
                zeta0_0 %= mod;
                blocks[Config::myconfig->get_idex()][i] = (block[i] + zeta0_0 + mod - zeta0_1) % mod;
            }
            else{
                blocks[Config::myconfig->get_idex()][i] = block[i] + zeta0_0 - zeta0_1;
            }
        }   
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()], sizeof(T));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_idex()], sizeof(T));
        /*X*/
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), blocks[Config::myconfig->get_idex()], 2*sizeof(R));
        
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()], sizeof(T));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_idex()], sizeof(T));

        /*recv*/
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_suc_idex()], sizeof(T));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_suc_idex()], sizeof(T));
        /*X*/
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), blocks[Config::myconfig->get_suc_idex()], 2*sizeof(R));

        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_pre_idex()], sizeof(T));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_pre_idex()], sizeof(T));
        for(int i = 0; i < 3; i ++){
            if(Config::myconfig->get_idex() != i){
                /*set delta_k = */
                delta[i] = (deltas[i][0] + deltas[i][1] + deltas[i][2]);
            }
        }
        uint32_t beta[2];
        ic[Config::myconfig->get_suc_idex()]->twopc_ic(&delta[Config::myconfig->get_suc_idex()], 1, &beta[1]);
        ic[Config::myconfig->get_pre_idex()]->twopc_ic(&delta[Config::myconfig->get_pre_idex()], 1, &beta[0]);
        R res = blocks[Config::myconfig->get_idex()][1];
        for(int q = 0; q < 2; q++){
            for(int k = 0; k < 2; k++){ 
                
                if(mod != 0){
                    if(k == 0)
                        res = (res + blocks[(q + Config::myconfig->get_idex()) % 3][k]*(beta[1 - q]%mod)) % mod;
                    else
                        res = (res + mod - (blocks[(q + Config::myconfig->get_idex()) % 3][k]*(beta[1 - q]%mod) % mod)) % mod;
                }else{
                    res = res + (-1 + 2*(1-k)) * blocks[(q + Config::myconfig->get_idex()) % 3][k]*beta[1 - q];
                }
            }
        }
        
        return res;
    }


};
#endif
#ifndef _CSOT_H__
#define _CSOT_H__
#include "preliminaries.hpp"
#include "scmp.hpp"
template<typename T, uint32_t L = 32, uint16_t Lambda = 16>
class CSOT{
    private:
    uint8_t n0[Lambda], n1[Lambda], n2[Lambda];
    AES_KEY n_0, n_1, n_2;
    Thr_pc_ic_id<T> *ic[3];
    T rou, tao;

    public:
    void init(){
        get_seed<uint8_t>({"player0", "player1"}, n0, Lambda);
        for(int i = 0; i < 16; i++) std::cout<< (uint32_t) n0[i] << " ";
        AES_set_encrypt_key(n0, 128, &n_0);
        get_seed<uint8_t>({"player1", "player2"}, n1, Lambda);
        for(int i = 0; i < 16; i++) std::cout<< (uint32_t) n1[i] << " ";
        AES_set_encrypt_key(n1, 128, &n_1);
        get_seed<uint8_t>({"player2", "player0"}, n2, Lambda);
        for(int i = 0; i < 16; i++) std::cout<< (uint32_t) n2[i] << " ";
        AES_set_encrypt_key(n2, 128, &n_2);
        ic[0] = new Thr_pc_ic_id<T>(L, "player0");
        ic[1] = new Thr_pc_ic_id<T>(L, "player1");
        ic[2] = new Thr_pc_ic_id<T>(L, "player2");
    }
    void offline(uint32_t len = 1){
        RAND_bytes(reinterpret_cast<uint8_t*>(&rou), sizeof(T));
        tao = (T)1 << (L - 1) - 1;
        for(int i = 0; i < 3; i++){
            ic[i]->twopc_ic_off(len, 0, tao, (1<<L) - 1, tou);
        }

    }
    template<typename R>
    R online(R *block, T *m, uint32_t sid = 0){
        /*
        R: overwrite mod add
        */
        T deltas[3][3], delta[3];
        uint8_t tmp[sizeof(T)];
        for(int i = 0; i < 3; i ++){
            if(sizeof(T) > 4) tmp[4] = i;
            else tmp[sizeof(T) - 2] = i
            if(sizeof(T) > 4) memcpy(tmp, &sid, 4);
            else memcpy(tmp, &sid, sizeof(T) - 1);
            T w0, w1;
            if(Config::myconfig->check("player0")){
                prf(reinterpret_cast<uint8_t*>(&w0), tmp, sizeof(T), &n_0, 1);
                prf(reinterpret_cast<uint8_t*>(&w1), tmp, sizeof(T), &n_2, 1);
            }
            if(Config::myconfig->check("player1")){
                prf(reinterpret_cast<uint8_t*>(&w0), tmp, sizeof(T), &n_1, 1);
                prf(reinterpret_cast<uint8_t*>(&w1), tmp, sizeof(T), &n_0, 1);
            }
            if(Config::myconfig->check("player2")){
                prf(reinterpret_cast<uint8_t*>(&w0), tmp, sizeof(T), &n_2, 1);
                prf(reinterpret_cast<uint8_t*>(&w1), tmp, sizeof(T), &n_1, 1);
            }

            deltas[i][Config::myconfig->get_idex()] = (m[1] - m[0] - w0 + w1);
        }
        deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()] = deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()] + rou;
        R blocks[3][2];
        for(int i = 0; i < 2; i ++){
            R zeta0_0, zeta0_1;
            uint8_t tmp2[sizeof(R)];
            if(sizeof(R) > 4) tmp2[4] = i;
            else tmp2[sizeof(R) - 2] = i;
            if(sizeof(R) > 4) memcpy(tmp2, &sid, 4);
            else memcpy(tmp2, &sid, sizeof(R) - 1);
            if(Config::myconfig->check("player0")){
                prf(reinterpret_cast<uint8_t*>(&zeta0_0), tmp2, sizeof(R), &n_0, 1);
                prf(reinterpret_cast<uint8_t*>(&zeta0_1), tmp2, sizeof(R), &n_2, 1);
            }
            if(Config::myconfig->check("player1")){
                prf(reinterpret_cast<uint8_t*>(&zeta0_0), tmp2, sizeof(R), &n_1, 1);
                prf(reinterpret_cast<uint8_t*>(&zeta0_1), tmp2, sizeof(R), &n_0, 1);
            }
            if(Config::myconfig->check("player2")){
                prf(reinterpret_cast<uint8_t*>(&zeta0_0), tmp2, sizeof(R), &n_2, 1);
                prf(reinterpret_cast<uint8_t*>(&zeta0_1), tmp2, sizeof(R), &n_1, 1);
            }
            
            blocks[Config::myconfig->get_idex()][i] = block[i] + zeta0_0 - zeta0_1;
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
        ic[Config::myconfig->get_suc_idex()]->twopc_ic(delta[Config::myconfig->get_suc_idex()], 1, &beta[1]);
        ic[Config::myconfig->get_pre_idex()]->twopc_ic(delta[Config::myconfig->get_pre_idex()], 1, &beta[0]);
        R res = blocks[Config::myconfig->get_idex()][1];
        for(int q = 0; q < 2; q++){
            for(int k = 0; k < 2; k++){
                res = res + (-1 + 2*k) * blocks[q + Config::myconfig->get_idex()][k]*beta[1 - q];
            }
        }
        return res;
    }


};
#endif
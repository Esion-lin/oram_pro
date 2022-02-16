
#ifndef _SOT_H__
#define _SOT_H__
#include "fss-common.h"
#include "fss-client.h"
#include "preliminaries.hpp"
#include "fss-server.h"
#include "config.hpp"
#include "net.hpp"
#include "fss_help.hpp"
#include <set>
template<typename T>
void get_seed(std::set<std::string> roles, T* data, uint32_t lens){  
    if(Config::myconfig->check(*roles.begin())){
        RAND_bytes(data, lens);
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
template<uint16_t Lambda = 16, uint16_t N = 2048>
class SOT{
    private:
    Fss myfss;
    /*i+1, i-1*/
    ServerKeyEq gk, sk;
    uint32_t *data0;
    uint32_t *data1; 
    uint16_t data_lens;
    public:
    SOT(uint32_t *data0, uint32_t * data1, uint16_t data_lens):data0(data0),data1(data1),data_lens(data_lens){
        init();
        offline();
    }
    ~SOT(){

        free_fss_key(myfss);
    }
    uint8_t n0[Lambda], n1[Lambda], n2[Lambda];
    AES_KEY n_0, n_1, n_2;
    uint32_t fili;
    void init(){
        get_seed<uint8_t>({"player0", "player1"}, n0, Lambda);
        AES_set_encrypt_key(n0, 128, &n_0);
        get_seed<uint8_t>({"player1", "player2"}, n1, Lambda);
        AES_set_encrypt_key(n1, 128, &n_1);
        get_seed<uint8_t>({"player2", "player0"}, n2, Lambda);
        AES_set_encrypt_key(n2, 128, &n_2);
        if(Config::myconfig->check("player0")){
            initializeClient(&myfss, logn(N), 2);
            send_role_mpz(myfss.prime, P2Pchannel::mychnl, {"player1", "player2"});
            send_all({"player1", "player2"}, &myfss,sizeof(Fss)-16);
            send_all({"player1", "player2"}, myfss.aes_keys,sizeof(AES_KEY)*myfss.numKeys);
        }
        else{
            recv_fss_key(myfss, P2Pchannel::mychnl, "player0");
        }

        
    }
    void offline(){
        RAND_bytes(reinterpret_cast<uint8_t*>(&fili), 4);
        ServerKeyEq k0, k1;
        generateTreeEq(&myfss, &k0, &k1, fili, 1);
        send_eq_key(k0, myfss, Config::myconfig->get_suc());
        send_eq_key(k1, myfss, Config::myconfig->get_pre());
        recv_eq_key(sk, myfss, Config::myconfig->get_pre());
        recv_eq_key(gk, myfss, Config::myconfig->get_suc());
        free_key<ServerKeyEq>(k0);
        free_key<ServerKeyEq>(k1);
    }
    uint32_t online(uint32_t idex, uint32_t sid = 0){
        uint16_t lens = data_lens;
        uint64_t tmp;
        uint32_t deltas[3][3], delta[3];
        for(int i = 0; i < 3; i ++){
            tmp = sid + (uint64_t)i << 32;
            uint64_t w0, w1;
            if(Config::myconfig->check("player0")){
                prf(reinterpret_cast<uint8_t*>(&w0), reinterpret_cast<uint8_t*>(&tmp), 8, &n_0, 1);
                prf(reinterpret_cast<uint8_t*>(&w1), reinterpret_cast<uint8_t*>(&tmp), 8, &n_2, 1);
            }
            if(Config::myconfig->check("player1")){
                prf(reinterpret_cast<uint8_t*>(&w0), reinterpret_cast<uint8_t*>(&tmp), 8, &n_1, 1);
                prf(reinterpret_cast<uint8_t*>(&w1), reinterpret_cast<uint8_t*>(&tmp), 8, &n_0, 1);
            }
            if(Config::myconfig->check("player2")){
                prf(reinterpret_cast<uint8_t*>(&w0), reinterpret_cast<uint8_t*>(&tmp), 8, &n_2, 1);
                prf(reinterpret_cast<uint8_t*>(&w1), reinterpret_cast<uint8_t*>(&tmp), 8, &n_1, 1);
            }
            
            deltas[i][Config::myconfig->get_idex()] = idex - w0 + w1;
            if(Config::myconfig->get_idex() == i) deltas[i][Config::myconfig->get_idex()] = deltas[i][Config::myconfig->get_idex()] - fili;
        }
        /*
        send delta_j, delta_{j+1} to S_{j+2} 
        */
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));
        /*recv*/
        uint32_t deltap[2], deltapp[2];
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_suc_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_suc_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_pre_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_pre_idex()], sizeof(uint32_t));
        for(int i = 0; i < 3; i ++){
            if(Config::myconfig->get_idex() != i){
                /*set delta_k = */
                delta[i] = deltas[i][0] + deltas[i][1] + deltas[i][2];
            }
        }
        mpz_class ans_list[N];
        uint32_t ans = 0;
        evaluateEq(&myfss, &gk, ans_list, N);
        free_key<ServerKeyEq>(gk);
        for(uint32_t i = 0; i < N; i++){
            ans += - data0[( i - delta[Config::myconfig->get_suc_idex()] + N )% N] * mpz_get_ui(ans_list[i].get_mpz_t());
        }
        evaluateEq(&myfss, &sk, ans_list, N);
        free_key<ServerKeyEq>(sk);
        for(uint32_t i = 0; i < N; i++){
            ans += data1[( i - delta[Config::myconfig->get_pre_idex()] + N )% N] * mpz_get_ui(ans_list[i].get_mpz_t());
        }
        return ans;
    }
};


#endif
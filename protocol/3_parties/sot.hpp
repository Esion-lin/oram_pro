
#ifndef _SOT_H__
#define _SOT_H__
#include "fss-common.h"
#include "fss-client.h"
#include "preliminaries.hpp"
#include "fss-server.h"
#include "config.hpp"
#include "net.hpp"
#include "fss_help.hpp"
#include "factory.hpp"
#include <set>
#include "timer.hpp"
#define round_size 4000
template<typename T = uint32_t, uint16_t Lambda = 16>
class SOT{
    private:
    Fss myfss;
    /*i+1, i-1*/
    std::vector<ServerKeyEq> gks, sks;
    T *data0;
    T *data1; 
    uint16_t data_lens;
    uint32_t times;
    uint8_t n0[Lambda], n1[Lambda], n2[Lambda];
    AES_KEY n_0, n_1, n_2;
    public:
    SOT(T *data0, T * data1, uint16_t data_lens):data0(data0),data1(data1),data_lens(data_lens){
        init();
    }
    ~SOT(){

        free_fss_key(myfss);
    }
    std::vector<uint32_t> filis;

    void init(){
        get_seed<uint8_t>({"player0", "player1"}, n0, Lambda);
        AES_set_encrypt_key(n0, 128, &n_0);
        get_seed<uint8_t>({"player1", "player2"}, n1, Lambda);
        AES_set_encrypt_key(n1, 128, &n_1);
        get_seed<uint8_t>({"player2", "player0"}, n2, Lambda);
        AES_set_encrypt_key(n2, 128, &n_2);
        if(Config::myconfig->check("player0")){
            initializeClient(&myfss, logn(data_lens), 2);
            send_role_mpz(myfss.prime, P2Pchannel::mychnl, {"player1", "player2"});
            send_all({"player1", "player2"}, &myfss,sizeof(Fss)-16);
            send_all({"player1", "player2"}, myfss.aes_keys,sizeof(AES_KEY)*myfss.numKeys);
        }
        else{
            recv_fss_key(myfss, P2Pchannel::mychnl, "player0");
        }

        
    }
    void offline_t(uint32_t times){
        this->times = times;
        sks.clear();gks.clear();
        for(int i = 0; i < times; i ++){
            uint32_t fili;
            RAND_bytes(reinterpret_cast<uint8_t*>(&fili), 4);
            fili %= data_lens;
            filis.push_back(fili);
            ServerKeyEq k0, k1;
            generateTreeEq(&myfss, &k0, &k1, fili, 1);
            send_eq_key(k0, myfss, Config::myconfig->get_suc());
            send_eq_key(k1, myfss, Config::myconfig->get_pre());
            free_key<ServerKeyEq>(k0);
            free_key<ServerKeyEq>(k1);
            ServerKeyEq k_0, k_1;
            recv_eq_key(k_0, myfss, Config::myconfig->get_pre());
            sks.push_back(k_0);
            recv_eq_key(k_1, myfss, Config::myconfig->get_suc());
            gks.push_back(k_1);
        }
    }
    void offline(uint32_t times){
        this->times = times;
        P2Pchannel::mychnl->set_flush(false);
        for(int i = 0; i < times; i ++){
            uint32_t fili;
            RAND_bytes(reinterpret_cast<uint8_t*>(&fili), 4);
            fili %= data_lens;
            filis.push_back(fili);
            ServerKeyEq k0, k1;
            generateTreeEq(&myfss, &k0, &k1, fili, 1);
            send_eq_key(k0, myfss, Config::myconfig->get_suc());
            send_eq_key(k1, myfss, Config::myconfig->get_pre());
            free_key<ServerKeyEq>(k0);
            free_key<ServerKeyEq>(k1);
        }
        sks.clear();gks.clear();
        P2Pchannel::mychnl->flush_all();
        for(int i = 0; i < times; i ++){
            ServerKeyEq k0, k1;
            recv_eq_key(k0, myfss, Config::myconfig->get_pre());
            sks.push_back(k0);
            recv_eq_key(k1, myfss, Config::myconfig->get_suc());
            gks.push_back(k1);
        
        }
        P2Pchannel::mychnl->flush_all();
        P2Pchannel::mychnl->set_flush(true);
        
    }
    T online(uint32_t* idex, T * res){
        uint32_t deltas[times][9], delta[times][3];
        uint32_t recv_ptr = 0;

        P2Pchannel::mychnl->set_flush(false);
        for(int t = 0; t < times; t++){
            for(int i = 0; i < 3; i ++){
                uint8_t tmmm[16],tmp[16] = {0};
                tmp[4] = i;
                memcpy(tmp, &t, 4);
                uint32_t w0, w1;
                if(Config::myconfig->check("player0")){
                    prf(tmmm, tmp, 16, &n_0, 1);
                    memcpy(&w0, tmmm, sizeof(uint32_t));
                    prf(tmmm, tmp, 16, &n_2, 1);
                    memcpy(&w1, tmmm, sizeof(uint32_t));
                }
                if(Config::myconfig->check("player1")){
                    prf(tmmm, tmp, 16, &n_1, 1);
                    memcpy(&w0, tmmm, sizeof(uint32_t));
                    prf(tmmm, tmp, 16, &n_0, 1);
                    memcpy(&w1, tmmm, sizeof(uint32_t));
                }
                if(Config::myconfig->check("player2")){
                    prf(tmmm, tmp, 16, &n_2, 1);
                    memcpy(&w0, tmmm, sizeof(uint32_t));
                    prf(tmmm, tmp, 16, &n_1, 1);
                    memcpy(&w1, tmmm, sizeof(uint32_t));
                }
                w0 %= data_lens;w1 %= data_lens;
                deltas[t][i*3 + Config::myconfig->get_idex()] = (idex[t]  + data_lens - w0 + w1) % data_lens;
                if(Config::myconfig->get_idex() == i) deltas[t][i*3 + Config::myconfig->get_idex()] = (deltas[t][i*3 + Config::myconfig->get_idex()] + data_lens - filis[t]) % data_lens;
            }
            /*
            send delta_j, delta_{j+1} to S_{j+2} 
            */
            P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &deltas[t][Config::myconfig->get_idex() * 3 + Config::myconfig->get_idex()], sizeof(uint32_t));
            P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &deltas[t][Config::myconfig->get_suc_idex() * 3 + Config::myconfig->get_idex()], sizeof(uint32_t));
            P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &deltas[t][Config::myconfig->get_idex() * 3 + Config::myconfig->get_idex()], sizeof(uint32_t));
            P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &deltas[t][Config::myconfig->get_pre_idex() * 3 + Config::myconfig->get_idex()], sizeof(uint32_t));
            
            // if(t%round_size == 0){
            //     P2Pchannel::mychnl->flush_all();
            //     for(int s = recv_ptr; s < t; s ++){
            //         P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[s][Config::myconfig->get_suc_idex() * 3 + Config::myconfig->get_suc_idex()], sizeof(uint32_t));
            //         P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[s][Config::myconfig->get_pre_idex() * 3 + Config::myconfig->get_suc_idex()], sizeof(uint32_t));
            //         P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[s][Config::myconfig->get_pre_idex() * 3 + Config::myconfig->get_pre_idex()], sizeof(uint32_t));
            //         P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[s][Config::myconfig->get_suc_idex() * 3 + Config::myconfig->get_pre_idex()], sizeof(uint32_t));
            //     }
            //     recv_ptr = t;
            // }
        }
        // for(int t = recv_ptr; t < times; t++){
            
        // }
        P2Pchannel::mychnl->flush_all();
        
        for(int t = 0; t < times; t++){
            /*recv*/
            
            P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[t][Config::myconfig->get_suc_idex() * 3 + Config::myconfig->get_suc_idex()], sizeof(uint32_t));
            P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[t][Config::myconfig->get_pre_idex() * 3 + Config::myconfig->get_suc_idex()], sizeof(uint32_t));
            P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[t][Config::myconfig->get_pre_idex() * 3 + Config::myconfig->get_pre_idex()], sizeof(uint32_t));
            P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[t][Config::myconfig->get_suc_idex() * 3 + Config::myconfig->get_pre_idex()], sizeof(uint32_t));
            
            for(int i = 0; i < 3; i ++){
                if(Config::myconfig->get_idex() != i){
                    /*set delta_k = */
                    delta[t][i] = (deltas[t][i * 3 + 0] + deltas[t][i * 3 + 1] + deltas[t][i * 3 + 2] + data_lens) % data_lens;
                }
            }
            mpz_class ans_list[data_lens];
            T ans;
            memset(&ans, 0, sizeof(T));
            evaluateEq(&myfss, &gks[t], ans_list, data_lens);
            free_key<ServerKeyEq>(gks[t]);
            for(uint32_t i = 0; i < data_lens; i++){
                ans += - data0[( i + delta[t][Config::myconfig->get_suc_idex()] + data_lens )% data_lens] * mpz_get_ui(ans_list[i].get_mpz_t());
            }
            evaluateEq(&myfss, &sks[t], ans_list, data_lens);

            free_key<ServerKeyEq>(sks[t]);
            for(uint32_t i = 0; i < data_lens; i++){
                ans += data1[( i + delta[t][Config::myconfig->get_pre_idex()] + data_lens )% data_lens] * mpz_get_ui(ans_list[i].get_mpz_t());
            }
            res[t] = ans;
            
        }
        P2Pchannel::mychnl->set_flush(true);
        return res[0];
    }
    T online(uint32_t idex, uint32_t t = 0){
        
        
        uint32_t deltas[3][3], delta[3];
        for(int i = 0; i < 3; i ++){
            uint8_t tmmm[16],tmp[16] = {0};
            tmp[4] = i;
            memcpy(tmp, &t, 4);
            uint32_t w0, w1;
            if(Config::myconfig->check("player0")){
                prf(tmmm, tmp, 16, &n_0, 1);
                memcpy(&w0, tmmm, sizeof(uint32_t));
                prf(tmmm, tmp, 16, &n_2, 1);
                memcpy(&w1, tmmm, sizeof(uint32_t));
            }
            if(Config::myconfig->check("player1")){
                prf(tmmm, tmp, 16, &n_1, 1);
                memcpy(&w0, tmmm, sizeof(uint32_t));
                prf(tmmm, tmp, 16, &n_0, 1);
                memcpy(&w1, tmmm, sizeof(uint32_t));
            }
            if(Config::myconfig->check("player2")){
                prf(tmmm, tmp, 16, &n_2, 1);
                memcpy(&w0, tmmm, sizeof(uint32_t));
                prf(tmmm, tmp, 16, &n_1, 1);
                memcpy(&w1, tmmm, sizeof(uint32_t));
            }
            w0 %= data_lens;w1 %= data_lens;
            deltas[i][Config::myconfig->get_idex()] = (idex  + data_lens - w0 + w1) % data_lens;
            if(Config::myconfig->get_idex() == i) deltas[i][Config::myconfig->get_idex()] = (deltas[i][Config::myconfig->get_idex()] + data_lens - filis[t]) % data_lens;
        }
        /*
        send delta_j, delta_{j+1} to S_{j+2} 
        */
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));

        /*recv*/
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_suc_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_suc_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_pre_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_pre_idex()], sizeof(uint32_t));
        
        for(int i = 0; i < 3; i ++){
            if(Config::myconfig->get_idex() != i){
                /*set delta_k = */
                delta[i] = (deltas[i][0] + deltas[i][1] + deltas[i][2] + data_lens) % data_lens;
            }
        }
        mpz_class ans_list[data_lens];
        T ans;
        memset(&ans, 0, sizeof(T));
        evaluateEq(&myfss, &gks[t], ans_list, data_lens);
        
        free_key<ServerKeyEq>(gks[t]);
        //std::cout<<"freee \n";
        for(uint32_t i = 0; i < data_lens; i++){
            ans -= data0[( i + delta[Config::myconfig->get_suc_idex()])% data_lens] * mpz_get_ui(ans_list[i].get_mpz_t());
            //ans.t -= data0[( i + delta[Config::myconfig->get_suc_idex()])% data_lens].t * mpz_get_ui(ans_list[i].get_mpz_t());
        }
        evaluateEq(&myfss, &sks[t], ans_list, data_lens);
        
        free_key<ServerKeyEq>(sks[t]);
        //std::cout<<"freee \n";
        for(uint32_t i = 0; i < data_lens; i++){
            ans += data1[( i + delta[Config::myconfig->get_pre_idex()])% data_lens] * mpz_get_ui(ans_list[i].get_mpz_t());
            //ans.t += data1[( i + delta[Config::myconfig->get_pre_idex()])% data_lens].t * mpz_get_ui(ans_list[i].get_mpz_t());
        }
        return ans;
    }
    uint32_t deltas[3][3], delta[3];
    void online_1(uint32_t idex, uint32_t t = 0){
        
        
        
        for(int i = 0; i < 3; i ++){
            uint8_t tmmm[16],tmp[16] = {0};
            tmp[4] = i;
            memcpy(tmp, &t, 4);
            uint32_t w0, w1;
            if(Config::myconfig->check("player0")){
                prf(tmmm, tmp, 16, &n_0, 1);
                memcpy(&w0, tmmm, sizeof(uint32_t));
                prf(tmmm, tmp, 16, &n_2, 1);
                memcpy(&w1, tmmm, sizeof(uint32_t));
            }
            if(Config::myconfig->check("player1")){
                prf(tmmm, tmp, 16, &n_1, 1);
                memcpy(&w0, tmmm, sizeof(uint32_t));
                prf(tmmm, tmp, 16, &n_0, 1);
                memcpy(&w1, tmmm, sizeof(uint32_t));
            }
            if(Config::myconfig->check("player2")){
                prf(tmmm, tmp, 16, &n_2, 1);
                memcpy(&w0, tmmm, sizeof(uint32_t));
                prf(tmmm, tmp, 16, &n_1, 1);
                memcpy(&w1, tmmm, sizeof(uint32_t));
            }
            w0 %= data_lens;w1 %= data_lens;
            deltas[i][Config::myconfig->get_idex()] = (idex  + data_lens - w0 + w1) % data_lens;
            if(Config::myconfig->get_idex() == i) deltas[i][Config::myconfig->get_idex()] = (deltas[i][Config::myconfig->get_idex()] + data_lens - filis[t]) % data_lens;
        }
        /*
        send delta_j, delta_{j+1} to S_{j+2} 
        */
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->send_data_to(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_idex()], sizeof(uint32_t));
    }
    T online_2(uint32_t idex, uint32_t t = 0){
        /*recv*/
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_suc_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_suc(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_suc_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_pre_idex()][Config::myconfig->get_pre_idex()], sizeof(uint32_t));
        P2Pchannel::mychnl->recv_data_from(Config::myconfig->get_pre(), &deltas[Config::myconfig->get_suc_idex()][Config::myconfig->get_pre_idex()], sizeof(uint32_t));
        
        for(int i = 0; i < 3; i ++){
            if(Config::myconfig->get_idex() != i){
                /*set delta_k = */
                delta[i] = (deltas[i][0] + deltas[i][1] + deltas[i][2] + data_lens) % data_lens;
            }
        }
        mpz_class ans_list[data_lens];
        T ans;
        memset(&ans, 0, sizeof(T));
        evaluateEq(&myfss, &gks[t], ans_list, data_lens);
        
        free_key<ServerKeyEq>(gks[t]);
        //std::cout<<"freee \n";
        for(uint32_t i = 0; i < data_lens; i++){
            ans -= data0[( i + delta[Config::myconfig->get_suc_idex()])% data_lens] * mpz_get_ui(ans_list[i].get_mpz_t());
            //ans.t -= data0[( i + delta[Config::myconfig->get_suc_idex()])% data_lens].t * mpz_get_ui(ans_list[i].get_mpz_t());
        }
        evaluateEq(&myfss, &sks[t], ans_list, data_lens);
        
        free_key<ServerKeyEq>(sks[t]);
        //std::cout<<"freee \n";
        for(uint32_t i = 0; i < data_lens; i++){
            ans += data1[( i + delta[Config::myconfig->get_pre_idex()])% data_lens] * mpz_get_ui(ans_list[i].get_mpz_t());
            //ans.t += data1[( i + delta[Config::myconfig->get_pre_idex()])% data_lens].t * mpz_get_ui(ans_list[i].get_mpz_t());
        }
        return ans;
    }
};


#endif
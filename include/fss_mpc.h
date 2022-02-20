#ifndef _FSS_MPC_H__
#define _FSS_MPC_H__
/*
gen/eval fss key under MPC
*/
#include "net.hpp"
#include "fss-common.h"
#include "fss_help.hpp"

#include <vector>
#include <queue>
#include "io.hpp"
#include "pre_op.h"
void local_block1(const uint8_t * s, uint8_t t, const uint8_t * sigma, uint8_t tao, uint8_t * s0_out, uint8_t * s1_out, uint8_t & t_out, AES_KEY_FSS* aes_key);
void local_block2(const uint8_t * s, uint8_t t, const uint8_t * sigma, const uint8_t * gamma, uint8_t tao, uint8_t * s_out, uint8_t & t_out);
void mpc_block1(const uint8_t* z0, const uint8_t* z1, uint8_t alpha, uint8_t* out_zetta, uint8_t & tao0, uint8_t & tao1, std::pair<std::string,std::string> tarro);
void mpc_block2(const uint8_t* z0, const uint8_t* z1, uint8_t alpha, uint8_t* beta, uint8_t* out_zetta, uint8_t* out_gamma, uint8_t & tao0, uint8_t & tao1, std::pair<std::string,std::string> tarro);
template< typename T, uint16_t deep = 32>
class FSS_MPC{
    /*
    output_bytes with Little Endian
    */
private:
    std::pair<std::string,std::string> tarro;

    AES_KEY_FSS *keys;
    uint16_t keys_num = 2;
    vector<uint8_t*> sigmas;
    uint8_t* gammas;
    vector<uint8_t> tau0,tau1;
    uint8_t s0[32];
    uint8_t t;
    std::queue<uint8_t*> level;
    std::queue<uint8_t> tlevel;

public:
    FSS_MPC(std::pair<std::string,std::string> tarro):tarro(tarro){
        uint8_t arr[16];
        keys = (AES_KEY_FSS*) malloc(sizeof(AES_KEY_FSS) * keys_num);
        gammas = (uint8_t*) malloc(16 * sizeof(uint8_t));
        for(int i = 0; i < keys_num; i++){
            // get_rand<uint8_t>({tarro.first, tarro.second}, arr, 16);
            if(Config::myconfig->check(tarro.first)){
                RAND_bytes(arr, 16);
                P2Pchannel::mychnl->send_data_to(tarro.second, arr, 16);
            }else if(Config::myconfig->check(tarro.second)){
                P2Pchannel::mychnl->recv_data_from(tarro.first, arr, 16);
            }
            AES_set_encrypt_key(arr, 128, &keys[i]);
        }
        if(!RAND_bytes(s0, 16)) {
            printf("Random bytes failed\n");
            exit(1);
        }
        
    }
    ~FSS_MPC(){
        free(gammas);
        free(keys);
        for(auto ele : sigmas) free(ele);
    }

    void gen(T alpha, T beta, uint8_t t_int){
        /*transfer beta to bytes*/
        uint8_t betas[16] = {0};
        memcpy(betas, &beta, sizeof(T));
        uint8_t* s;
        uint8_t tt = t_int;
        uint8_t z0[16], z1[16], tao0, tao1;
        
        uint8_t* next_s = (uint8_t*)malloc(32*sizeof(uint8_t));
        prf(next_s, s0, 32, keys, 2);
        for(uint8_t i = 0; i < 16; i ++ ){
            z0[i] = next_s[i];
            z1[i] = next_s[i+16];
        }
        s = (uint8_t*)malloc(16*sizeof(uint8_t));
        memcpy(s, next_s, 16);
        level.push(s);
        s = (uint8_t*)malloc(16*sizeof(uint8_t));
        memcpy(s, next_s + 16, 16);
        level.push(s);
        free(next_s);
        tlevel.push(tt);
        tlevel.push(tt);
        for(int i = 0; i < deep; i++){
            uint8_t alphabit = 1&(alpha>>(deep - 1 - i));
            uint8_t * zetta = (uint8_t*)malloc(16*sizeof(uint8_t));
            if(i == deep - 1) mpc_block2(z0, z1, alphabit, betas, zetta, gammas, tao0, tao1, tarro);
            else mpc_block1(z0, z1, alphabit, zetta, tao0, tao1, tarro);
            
            sigmas.push_back(zetta);
            tau0.push_back(tao0);tau1.push_back(tao1);
            memset(z0, 0, 16);memset(z1, 0, 16);
            for(int j = 0; j < (1 << (i+1)); j ++){
                s = level.front();level.pop();
                tt = tlevel.front();tlevel.pop();
                if(i == deep - 1) {free(s); continue;}
                uint8_t *s0_out = (uint8_t*)malloc(16*sizeof(uint8_t));
                uint8_t *s1_out = (uint8_t*)malloc(16*sizeof(uint8_t));
                uint8_t tt_out;
                
                if(i == deep - 1) local_block2(s, tt, zetta, gammas, j%2==0?tao0:tao1, s0_out, tt_out);
                else local_block1(s, tt, zetta, j%2==0?tao0:tao1, s0_out, s1_out, tt_out, keys);
                
                if(i != deep - 1) {
                    level.push(s0_out);level.push(s1_out);
                    /*计算z0, z1*/
                    for(int i = 0; i < 16; i++){
                        z0[i] ^= s0_out[i];
                        z1[i] ^= s1_out[i];
                    }

                }else{
                    uint64_t ret;
                    memcpy(&ret, s0_out, 8);
                    std::cout<<ret<<"\n";
                    free(s0_out);free(s1_out);
                }
                tlevel.push(tt_out);tlevel.push(tt_out);
                free(s);
            }

        }
        
    }
    void get_list(uint8_t t_int){

        uint8_t* s;
        uint8_t tt = t_int;   
        uint8_t* next_s = (uint8_t*)malloc(32*sizeof(uint8_t));
        prf(next_s, s0, 32, keys, 2);
        s = (uint8_t*)malloc(16*sizeof(uint8_t));
        memcpy(s, next_s, 16);
        level.push(s);
        s = (uint8_t*)malloc(16*sizeof(uint8_t));
        memcpy(s, next_s + 16, 16);
        level.push(s);
        free(next_s);
        tlevel.push(tt);
        tlevel.push(tt);
        for(int i = 0; i < deep; i++){        
            for(int j = 0; j < (1 << (i+1)); j ++){
                uint8_t *s0_out = (uint8_t*)malloc(16*sizeof(uint8_t));
                uint8_t *s1_out = (uint8_t*)malloc(16*sizeof(uint8_t));
                uint8_t tt_out;
                s = level.front();level.pop();
                tt = tlevel.front();tlevel.pop();

                if(i == deep - 1) local_block2(s, tt, sigmas[i], gammas, j%2==0?tau0[i]:tau1[i], s0_out, tt_out);
                else local_block1(s, tt, sigmas[i], j%2==0?tau0[i]:tau1[i], s0_out, s1_out, tt_out, keys);
                if(i != deep - 1) {
                    level.push(s0_out);level.push(s1_out);

                }else{
                    uint64_t ret;
                    memcpy(&ret, s0_out, 8);
                    std::cout<<ret<<"\n";
                    free(s0_out);free(s1_out);
                }
                tlevel.push(tt_out);tlevel.push(tt_out);
                free(s);
            }

        }
    }
    void evl(T x, uint8_t t_int){
        uint8_t alphabit = 1&(x>>(deep - 1));
        uint8_t* s;
        uint8_t tt = t_int;   
        uint8_t* next_s = (uint8_t*)malloc(32*sizeof(uint8_t));
        uint8_t *s0_out = (uint8_t*)malloc(16*sizeof(uint8_t));
        uint8_t *s1_out = (uint8_t*)malloc(16*sizeof(uint8_t));
        uint8_t tt_out;
        prf(next_s, s0, 32, keys, 2);
        s = (uint8_t*)malloc(16*sizeof(uint8_t));
        if(alphabit == 0)
            memcpy(s, next_s, 16);
        else
            memcpy(s, next_s + 16, 16);
        free(next_s);
        for(int i = 0; i < deep; i++){     
            if(i == deep - 1) local_block2(s, tt, sigmas[i], gammas, alphabit==0?tau0[i]:tau1[i], s0_out, tt_out);
            else local_block1(s, tt, sigmas[i], alphabit==0?tau0[i]:tau1[i], s0_out, s1_out, tt_out, keys);
            if(i == deep - 1){
                uint64_t ret;
                memcpy(&ret, s0_out, 8);
                std::cout<<ret<<"\n";
            }
            alphabit = 1&(x>>(deep - 2 - i));
            if(alphabit == 0) memcpy(s, s0_out, 16);
            else memcpy(s, s1_out, 16);
            tt = tt_out;

        }
        free(s0_out);free(s1_out);
        free(s);
    }

};
template< typename T, typename R, uint16_t deep = 32, uint16_t lambda = 32>
class DCF_MPC{
    /*
    gen ServerKeyLt with mpc
    */
    private:
    Fss fss_key;
    ServerKeyLt res_key;
    std::pair<std::string,std::string> players;
    bool bit;
    uint8_t s0[lambda];
    uint8_t t0;
    R V_alpha = 0;
    uint8_t V_L, V_R, s_L, s_R;
    public:
    DCF(std::pair<std::string,std::string> players):players(players){
        bit = Config::myconfig->check(players.second);
        if(Config::myconfig->check(players.first)){
            initializeClient(&fss_key, deep, 2);
            send_role_mpz(myfss.prime, P2Pchannel::mychnl, {players.second});
            send_all({players.second}, &myfss,sizeof(Fss)-16);
            send_all({players.second}, myfss.aes_keys,sizeof(AES_KEY)*myfss.numKeys);
        }
        else if(Config::myconfig->check(players.second)){
            recv_fss_key(myfss, P2Pchannel::mychnl, players.first);
        }
        
    }
    void gen(T alpha, R beta){
        /*sample s_b*/
        if(!RAND_bytes(s0, lambda)) {
            printf("Random bytes failed\n");
            exit(1);
        }
        t0 = bit?1:0;
        for(int j = 1; i < deep + 1; j++){
            V_L = V_R = 0; 
            s_L = s_R = 0;
            for(int w = 0; w < (1 << (j - 1)); w ++){
                //prf();
            }
        }
        


    }
    ~DCF(){
        free_key<ServerKeyLt> res_key;
    }
    

};

#endif
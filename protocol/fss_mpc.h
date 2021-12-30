#ifndef _FSS_MPC_H__
#define _FSS_MPC_H__
/*
gen fss key under MPC
*/
#include "net.hpp"
#include "fss-common.h"
#include <vector>
#include <queue>
void local_block1(uint8_t * s, uint8_t t, uint8_t * sigma, uint8_t tao, uint8_t * s0_out, uint8_t * s1_out, uint8_t & t_out, AES_KEY aes_key);
void local_block2(uint8_t * s, uint8_t t, uint8_t * sigma, uint8_t * gamma, uint8_t tao, uint8_t * s_out, uint8_t & t_out);
void mpc_block1(uint8_t* z0, uint8_t* z1, uint8_t alpha, uint8_t* out_zetta, uint8_t & tao0, uint8_t & tao1, std::pair<std::string,std::string> tarro);
void mpc_block2(uint8_t* z0, uint8_t* z1, uint8_t alpha, uint32_t beta, uint8_t* out_zetta, uint8_t* out_gamma, uint8_t & tao0, uint8_t & tao1, std::pair<std::string,std::string> tarro);
template< typename T, uint16_t deep = 32>
class FSS_MPC{
private:
    std::string tarst;

    AES_KEY *keys;
    uint16_t keys_num = 2;
    vector<uint8_t*> sigmas;
    uint8_t* gammas;
    vector<uint8_t> tau0,tau1;
    uint8_t s0[32];
    uint8_t t;
    std::queue<uint8_t*> level;
    std::queue<uint8_t> tlevel;

public:
    FSS_MPC(std::string tarst):tarst(tarst){
        uint8_t arr[16];
        keys = (AES_KEY*) malloc(sizeof(AES_KEY) * keys_num);
        gammas = (uint8_t*) malloc(16 * sizeof(uint8_t));
        for(int i = 0; i < keys_num; i++){
            RAND_bytes(arr, 16);
            AES_set_encrypt_key(arr, 128, &keys[i]);
        }
        if(!RAND_bytes(s0, 16)) {
            printf("Random bytes failed\n");
            exit(1);
        }
        
    }

    void gen(T alpha, T beta, uint8_t t_int){
        
        uint8_t* s;
        uint8_t tt = t_int;
        uint8_t z0[16], z1[16], tao0, tao1;
        
        uint8_t* next_s = (uint8_t*)malloc(32*sizeof(uint8_t));
        prf(next_s, s0, 32, key, 2);
        for(uint8_t i = 0; i < 16; i ++ ){
            z0[i] = next_s[i];
            z1[i] = next_s[i+16];
        }
        level.push(&next_s[0]);
        level.push(&next_s[16]);
        tlevel.push(tt);
        tlevel.push(tt);
        for(int i = 0; i < deep; i++){
            uint8_t alphabit = 1&(alpha>>i);
            uint8_t * zetta = (uint8_t*)malloc(16*sizeof(uint8_t));
            if(i == deep - 1) mpc_block2(z0, z1, alphabit, beta, zetta, gammas, tao0, tao1);
            else mpc_block1(z0, z1, alphabit, zetta, tao0, tao1);
            sigmas.push_back(zetta);
            tau0.push_back(tao0);tau1.push_back(tao1);
            memset(z0, 0, 16);memset(z1, 0, 16);
            for(int j = 0; j < (1 << (i+1)); j ++){
                uint8_t *s0_out = (uint8_t*)malloc(16*sizeof(uint8_t));
                uint8_t *s1_out = (uint8_t*)malloc(16*sizeof(uint8_t));
                uint8_t tt_out;
                s = level.pop();
                tt = tlevel.pop();
                if(i == deep - 1) local_block2(s, tt, zetta, gammas, j%2==0?tao0:tao1, s0_out, tt_out);
                else local_block1(s, tt, zetta, j%2==0?tao0:tao1, s0_out, s1_out, tt_out, keys);
                if(i != deep - 1) {
                    level.push(s0_out);level.push(s1_out);
                    /*计算z0, z1*/
                    for(int i = 0; i < 16; i++){
                        z0[i] ^= s0_out[i];
                        z1[i] ^= s1_out[i];
                    }

                }
                tlevel.push(tt_out);tlevel.push(tt_out);
                free(s);
            }

        }
        
    }
    void evl(T x){

    }


};


#endif
#include "fss_mpc.h"
#include "io.hpp"
#include "gc_op.h"
#include "pre_op.h"
void local_block1(uint8_t * s, uint8_t t, uint8_t * sigma, uint8_t tao, uint8_t * s0_out, uint8_t * s1_out, uint8_t & t_out, AES_KEY aes_key){
    /*s [16] sigma [16]*/
    uint8_t s_plus[32], s_out[32];
    memset(s_plus, 0, 32);
    for(int i = 0; i < 16; i ++) {
        s_plus[i] = s[i] ^ (t * sigma[i]);
    }
    prf(s_out, s_plus, 256, &aes_key, 2);
    memcpy(s0_out, s_out, 16);
    memcpy(s1_out, s_out+16, 16);
    t_out = (1&(s[0])) ^ (t* tao);
}
void local_block2(uint8_t * s, uint8_t t, uint8_t * sigma, uint8_t * gamma, uint8_t tao, uint8_t * s_out, uint8_t & t_out){
    /**/
    uint8_t s_plus[16];
    for(int i = 0; i < 16; i ++) {
        s_plus[i] = s[i] ^ (t * sigma[i]);
    }
    t_out = (1&(s[0])) ^ (t* tao);
    for(int i = 0; i < 16; i ++) {
        s_out[i] = s_plus[i] ^ (t_out * gamma[i]);
    }
}
void mpc_block1(const uint8_t* z0, const uint8_t* z1, uint8_t alpha, uint8_t* out_zetta, uint8_t & tao0, uint8_t & tao1, std::pair<std::string,std::string> tarro){
    /*
    alpha & (z1 ^ z0) ^ z0
    a0 & z_1 ^ a1 & z_0
    */
    uint8_t tmp[16];
    bool select = (alpha == 1);
    tao0 = 1&(z0[0]) ^ 1&(alpha);
    tao1 = 1&(z1[0]) ^ 1&(alpha);
    if(Config::myconfig->check(tarro.first)) {
        select = !select;
        tao0 ^= 1;
    }
    shared_select<uint8_t>(z0, z1, select, tmp, 16, tarro);
    add_reveal<uint8_t>({tarro.first,tarro.second}, {tarro.first,tarro.second}, out_zetta, 16, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    add_reveal<uint8_t>({tarro.first,tarro.second}, {tarro.first,tarro.second}, &tao0, 1, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    add_reveal<uint8_t>({tarro.first,tarro.second}, {tarro.first,tarro.second}, &tao1, 1, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    

}
void mpc_block2(const uint8_t* z0, const uint8_t* z1, uint8_t alpha, uint8_t* beta, uint8_t* out_zetta, uint8_t* out_gamma, uint8_t & tao0, uint8_t & tao1, std::pair<std::string,std::string> tarro){
    mpc_block1(z0, z1, alpha, out_zetta, tao0, tao1, tarro);
    uint8_t tmp[16];
    bool select = (alpha == 1);
    shared_select<uint8_t>(z0, z1, select, tmp, 16, tarro);
    for(int i = 0; i < 16; i++){
        if(Config::myconfig->check(tarro.first)) {
            tmp[i] ^= (out_zetta[i] ^ beta[i]); 
        }else{
            tmp[i] ^= beta[i]; 
        }
    }
    add_reveal<uint8_t>({tarro.first,tarro.second}, {tarro.first,tarro.second}, out_gamma, 16, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    
}
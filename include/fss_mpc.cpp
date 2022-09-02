#include "fss_mpc.h"
#include "io.hpp"
void local_block1(const uint8_t * s, uint8_t t, const uint8_t * sigma, uint8_t tao, uint8_t * s0_out, uint8_t * s1_out, uint8_t & t_out, AES_KEY_FSS* aes_key){
    /*s [16] sigma [16]*/
    /*!!!!!!!!!!!!!存在一点bug*/
    uint8_t s_plus[32], s_out[32];
    //memset(s_plus, 0, 32);
    for(int i = 0; i < 16; i ++) {
        s_plus[i] = s[i] ^ (t * sigma[i]);
    
    }
    prf(s0_out, s_plus, 32, aes_key, 2);
    t_out = (1&(s[0])) ^ (t* tao);
}
void local_block2(const uint8_t * s, uint8_t t, const uint8_t * sigma, const uint8_t * gamma, uint8_t tao, uint8_t * s_out, uint8_t & t_out){
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
    bool select = (alpha == 1);
    tao0 = 1&(z0[0]) ^ 1&(alpha);
    tao1 = 1&(z1[0]) ^ 1&(alpha);
    if(Config::myconfig->check(tarro.first)) {
        select = !select;
        tao0 ^= 1;
    }
    shared_select<uint8_t>(z0, z1, select, out_zetta, 16, tarro);;
    add_reveal<uint8_t>({tarro.first,tarro.second}, {tarro.first,tarro.second}, out_zetta, 16, &tao0, 1, &tao1, 1, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    // add_reveal<uint8_t>({tarro.first,tarro.second}, {tarro.first,tarro.second}, &tao0, 1, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    // add_reveal<uint8_t>({tarro.first,tarro.second}, {tarro.first,tarro.second}, &tao1, 1, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    

}
void mpc_block2(const uint8_t* z0, const uint8_t* z1, uint8_t alpha, uint8_t* beta, uint8_t* out_zetta, uint8_t* out_gamma, uint8_t & tao0, uint8_t & tao1, std::pair<std::string,std::string> tarro){
    mpc_block1(z0, z1, alpha, out_zetta, tao0, tao1, tarro);
    bool select = (alpha == 1);
    shared_select<uint8_t>(z0, z1, select, out_gamma, 16, tarro);
    for(int i = 0; i < 16; i++){
        if(Config::myconfig->check(tarro.first)) {
            out_gamma[i] ^= (out_zetta[i] ^ beta[i]); 
        }else{
            out_gamma[i] ^= beta[i]; 
        }
    }
    add_reveal<uint8_t>({tarro.first,tarro.second}, {tarro.first,tarro.second}, out_gamma, 16, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    
}
void Oram::prepare_write(uint16_t ntimes){
    r1 = rand()%data_lens; r2 = rand()%data_lens; v =rand()%(1<<8);
    if(Config::myconfig->check("player0")||Config::myconfig->check("player1")){
        
        
        if(Config::myconfig->check("player0")) {
            write0->gen(r1, 0, 0);
            write0->send_key_to("player1");
            write1->recv_key_from("player1");
            r2 = 0;
        }
        else {
            write1->gen(r2, v, 0);
            write1->send_key_to("player0");
            write0->recv_key_from("player0");
            r1 = 0;
        }
    }else{
        
        
        if(Config::myconfig->check("player2")) {
            write0->gen(r1, 0, 1);
            write0->send_key_to("player3");
            write1->recv_key_from("player3");
            r2 = 0;
        }
        else {
            write1->gen(r2, v, 1);
            write1->send_key_to("player2");
            write0->recv_key_from("player2");
            r1 = 0;
        }
    }
    
}

void Oram::write(uint32_t idex, uint32_t target, uint32_t org){
    uint32_t deltaV = target ^ org ^ v;

    add_reveal<uint32_t>({"player0","player1","player2","player3"}, {"player0","player1","player2","player3"}, &deltaV, 1, [](uint32_t a, uint32_t b) -> uint32_t{return a ^ b;});
    uint32_t delta_r[2] = {idex ^ r1, idex ^ r2};
    add_reveal<uint32_t>({"player0","player1","player2","player3"}, {"player0","player1","player2","player3"}, delta_r, 2, [](uint32_t a, uint32_t b) -> uint32_t{return a ^ b;});
    
    if(Config::myconfig->check("player0")||Config::myconfig->check("player1")){
        write0->get_list(0);
        write1->get_list(0);
    }else{
        write0->get_list(1);
        write1->get_list(1);
    }
    for(int i = 0; i < data_lens; i++){
        uint32_t tmp;
        if((i^delta_r[0])%data_lens < write0->res_list.size())
            tmp = write0->res_list[(i^delta_r[0])%data_lens];
        data_ptr[i]^=tmp;
        if((i^delta_r[1])%data_lens < write1->res_list.size())
            tmp = write1->res_list[(i^delta_r[1])%data_lens];
        data_ptr[i]^= deltaV&tmp;
    }
}
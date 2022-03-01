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


template<typename T>
T byteArr2Block(uint8_t * bytes){
    //TODO: transfer to group
    T res;
    memcpy(&res, bytes, sizeof(T));
    return res;

}
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
template< typename T, typename R, uint16_t deep = 32, uint16_t lambda = 16>
class DCF_MPC{
    /*
    gen ServerKeyLt with mpc
    */
    private:
    Fss fss_key;
    ServerKeyLt res_key;
    std::pair<std::string,std::string> players;
    bool bit;
    uint8_t s0[lambda],s_L_R[lambda*2];
    uint8_t t0, t_L_R[2];
    R V_alpha = 0;
    R V_L_R[2];
    NCWLt CW[deep + 2];
    std::queue<uint8_t *>out_level, s_level;
    std::queue<uint8_t> t_level;
    public:
    DCF_MPC(std::pair<std::string,std::string> players):players(players){
        bit = Config::myconfig->check(players.second);
        if(Config::myconfig->check(players.first)){
            initializeClient(&fss_key, deep, 2);
            send_role_mpz(fss_key.prime, P2Pchannel::mychnl, {players.second});
            send_all({players.second}, &fss_key,sizeof(Fss)-16);
            send_all({players.second}, fss_key.aes_keys,sizeof(AES_KEY_FSS)*fss_key.numKeys);
        }
        else if(Config::myconfig->check(players.second)){
            recv_fss_key(fss_key, P2Pchannel::mychnl, players.first);
        }
        
    }
    void mpc_dcf(const uint8_t alpha_bit, const R beta, uint8_t * s_b, uint8_t * t_b, R * V_b, uint8_t t_p, R& V_alpha, uint32_t j){
        /*
        share S, t, V
        */
        uint8_t cw_0[lambda + 2], cw_1[lambda + 2], cw[lambda + 2];
        memcpy(cw_0, s_b + lambda, lambda);cw_0[lambda] = t_b[0];cw_0[lambda+1] = t_b[1];
        memcpy(cw_1, s_b, lambda);cw_1[lambda] = t_b[0];cw_1[lambda+1] = t_b[1];
        if(Config::myconfig->check(players.first)){
            cw_0[lambda] ^= 1;
            cw_1[lambda+1] ^= 1;
        } 
        shared_select<uint8_t>(cw_0, cw_1, alpha_bit%2 == 1, cw, lambda + 2, players);
        /*
        update V_cw

        */
        R tmp, tmps[2];
        if(Config::myconfig->check(players.first)){
            tmps[0] = - V_b[1] - V_alpha;
            tmps[1] = - V_b[0] - V_alpha + beta;
            
        }else if(Config::myconfig->check(players.second)){
            tmps[0] = V_b[1] - V_alpha;
            tmps[1] = V_b[0] - V_alpha + beta;
        }
        add_shared_select<R>(&tmps[0], &tmps[1], alpha_bit%2 == 1, &tmp, 1, players);
        /*t_p*/
        tmps[0] = tmp; tmps[1] = - tmp;
        half_select<R>(&tmps[0], &tmps[1], t_p%2 == 1, &tmp, 1, players);
        /*tmp : V_cw*/
        /*
        update V_alpha
        */
        if(Config::myconfig->check(players.first)){
            tmps[0] = V_b[0] - V_b[1];
            tmps[1] = V_b[1] - V_b[0] + beta;
        }else if(Config::myconfig->check(players.second)){
            tmps[0] = V_b[1] - V_b[0];
            tmps[1] = V_b[0] - V_b[1] + beta;
        }
        add_shared_select<R>(&tmps[0], &tmps[1], alpha_bit%2 == 1, &V_alpha, 1, players);
        memcpy(CW[j].cs, cw, lambda);
        CW[j].cv = tmp;
        CW[j].ct[0] = cw[lambda];
        CW[j].ct[1] = cw[lambda + 1];
        
    }
    void gen(T alpha, R beta){
        /*
        alpha binary shared
        */
        V_alpha = 0;
        uint8_t s_w[lambda * 2], t_w[2];
        R V_w[2];
        /*sample s_b*/
        if(!RAND_bytes(s0, lambda)) {
            printf("Random bytes failed\n");
            exit(1);
        }
        t0 = bit?1:0;
        uint8_t* tmp_s = (uint8_t*)malloc(lambda);
        memcpy(tmp_s, s0, 16);
        s_level.push(tmp_s);
        t_level.push(t0);
        
        for(int j = 1; j < deep + 1; j++){
            uint8_t alphabit = 1&(alpha>>(deep - j));
            V_L_R[0] = V_L_R[1] = 0; 
            t_L_R[0] = t_L_R[1] = 0;
            memset(s_L_R, 0, 2*lambda);
            for(int w = 0; w < (1 << (j - 1)); w ++){
                //0:L 1:R
                uint8_t * out = (uint8_t *)malloc(4*lambda);
                uint8_t *key = s_level.front();
                s_level.pop();
                prf(out, key, 4*lambda, fss_key.aes_keys, fss_key.numKeys);
                out_level.push(out);
                memcpy(s_w, out, lambda * 2);
                t_w[0] = out[lambda * 2] % 2;
                t_w[1] = out[lambda * 2 + 1] % 2;
                V_w[0] = byteArr2Block<R>((uint8_t*) (out + lambda * 2 + 2));
                V_w[1] = byteArr2Block<R>((uint8_t*) (out + lambda * 2 + 2 + sizeof(R)));
                V_L_R[0] += V_w[0]; V_L_R[1] += V_w[1];
                for(int i = 0; i < 2*lambda; i++) s_L_R[i] ^= s_w[i];
                t_L_R[0] ^= t_w[0];t_L_R[1] ^= t_w[1];
                free(key);
            }
            uint32_t tmp_t = alpha>>(deep - j);
            std::cout<<"----------------------testsetset\n";
            mpc_dcf(alphabit, beta, s_L_R, t_L_R, V_L_R,t0, V_alpha, j);
            for(int w = 0; w < (1 << (j - 1)); w ++){
                uint8_t *out = out_level.front();out_level.pop();
                uint8_t t_b = t_level.front();t_level.pop();
                uint8_t * next_s0, *next_s1;
                next_s0 = (uint8_t*) malloc(lambda);
                next_s1 = (uint8_t*) malloc(lambda);
                memcpy(next_s0, out, lambda);
                memcpy(next_s1, out, lambda);
                t_w[0] = out[lambda * 2] % 2;
                t_w[1] = out[lambda * 2 + 1] % 2;
                for(int i = 0; i < lambda; i++){
                    next_s0[i] ^= t_b*CW[j].cs[i];
                    next_s1[i] ^= t_b*CW[j].cs[i];
                }
                t_w[0] ^= t_b*CW[j].ct[0];
                t_w[1] ^= t_b*CW[j].ct[1];
                s_level.push(next_s0);s_level.push(next_s1);
                t_level.push(t_w[0]);t_level.push(t_w[1]);
                free(out);
                if(w * 2 == tmp_t) t0 = t_w[0];
                else if(w * 2 + 1 == tmp_t) t0 = t_w[1];
            }
        }
    }
    void evl(T x){
        V_alpha = 0;
        t0 = bit?1:0;
        for(int j = 1; j < deep + 1; j++){
            uint8_t xbit = 1&(x>>(deep - j));
            uint8_t * out = (uint8_t *)malloc(4*lambda);
            uint8_t t_hat[2];
            prf(out, s0, 4*lambda, fss_key.aes_keys, fss_key.numKeys);
            uint8_t * next_s0, *next_s1;
            next_s0 = (uint8_t*) malloc(lambda);
            next_s1 = (uint8_t*) malloc(lambda);
            memcpy(next_s0, out, lambda);
            memcpy(next_s1, out, lambda);
            for(int i = 0; i < lambda; i++){
                next_s0[i] ^= t0*CW[j].cs[i];
                next_s1[i] ^= t0*CW[j].cs[i];
            }
            t_hat[0] = out[lambda * 2] % 2;
            t_hat[1] = out[lambda * 2 + 1] % 2;
            t_hat[0] ^= t0*CW[j].ct[0];
            t_hat[1] ^= t0*CW[j].ct[1];
            if(xbit == 0){
                if(Config::myconfig->check(players.first))
                    V_alpha = V_alpha + byteArr2Block<R>((uint8_t*) (out + lambda * 2 + 2)) + t0 * CW[j].cv;
                else
                    V_alpha = V_alpha - (byteArr2Block<R>((uint8_t*) (out + lambda * 2 + 2)) + t0 * CW[j].cv);
                memcpy(s0, next_s0, lambda);
                t0 = t_hat[0];
            }else{
                if(Config::myconfig->check(players.first))
                    V_alpha = V_alpha + byteArr2Block<R>((uint8_t*) (out + lambda * 2 + 2)) + t0 * CW[j].cv;
                else
                    V_alpha = V_alpha - (byteArr2Block<R>((uint8_t*) (out + lambda * 2 + 2 + sizeof(R))) + t0 * CW[j].cv);
                memcpy(s0, next_s1, lambda);
                t0 = t_hat[1];
            }
        }
        /*output*/
        uint64_t res;
        memcpy(&res, s0, sizeof(uint64_t));
        std::cout<< " s: "<<res<<" ; t: "<<t0<<" ; V: "<<V_alpha<<std::endl;

    }
    ~DCF_MPC(){
        free_key<ServerKeyLt> (res_key);
    }
    

};

#endif
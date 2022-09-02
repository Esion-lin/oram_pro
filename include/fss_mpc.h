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

inline uint32_t get_bits(uint32_t x){
    for(uint32_t i = 0; i < 32; i++){
        if(pow(2, i) >= x)
            return i;
    }
    return 32;
}
template<typename T>
T byteArr2Block(uint8_t * bytes){
    //TODO: transfer to group
    T res;
    memcpy(&res, bytes, sizeof(T));
    return res;

}
template< typename T>
class FSS_MPC{
    /*
    output_bytes with Little Endian
    */
private:
    std::pair<std::string,std::string> tarro;
    Fss faccess; 
    bool is_store = false;
    AES_KEY_FSS *keys;
    uint16_t keys_num = 4;
    //vector<uint8_t*> sigmas;
    uint8_t* sigmas;
    uint8_t* gammas;
    //vector<uint8_t> tau0,tau1;
    uint8_t * tau0, *tau1;
    uint8_t s0[32];
    uint8_t t;
    std::queue<uint8_t*> level;
    std::queue<uint8_t> tlevel;
    uint16_t deep;

public:
    std::vector<uint64_t> res_list;
    FSS_MPC(std::pair<std::string,std::string> tarro, uint16_t deep):tarro(tarro), deep(deep){
        uint8_t arr[16];
        initializeClient(&faccess, deep, 2);
        
        gammas = (uint8_t*) malloc(16 * sizeof(uint8_t));
        sigmas = (uint8_t*) malloc(16 * sizeof(uint8_t) * deep);
        tau0 = (uint8_t*) malloc(sizeof(uint8_t) * deep);
        tau1 = (uint8_t*) malloc(sizeof(uint8_t) * deep);
        if(Config::myconfig->check(tarro.first))
        P2Pchannel::mychnl->send_data_to(tarro.second, faccess.aes_keys, sizeof(AES_KEY_FSS)*faccess.numKeys);
        else if(Config::myconfig->check(tarro.second))
        P2Pchannel::mychnl->recv_data_from(tarro.first, faccess.aes_keys, sizeof(AES_KEY_FSS)*faccess.numKeys);
        keys = faccess.aes_keys;
        if(!RAND_bytes(s0, 16)) {
            printf("Random bytes failed\n");
            exit(1);
        }
        
    }
    ~FSS_MPC(){
        free(gammas);
        free(keys);
        free(sigmas);
        free(tau0);free(tau1);
        //for(auto ele : sigmas) free(ele);
    }
    void send_key_to(std::string targe){
        /*sigmas gammas, tau0, tau1, s0, t*/
    }
    void recv_key_from(std::string targe){

    }
    void gen(T alpha, T beta, uint8_t t_int){
        is_store = true;
        uint32_t iter = 0;
        /*transfer beta to bytes*/
        uint8_t betas[16] = {0};
        memcpy(betas, &beta, sizeof(T));
        uint8_t* total_s = (uint8_t*)malloc((1<<(deep+2)) *sizeof(uint8_t)*16);
        uint8_t tt = t_int;
        uint8_t z0[16], z1[16], tao0, tao1;
        
    
        prf(total_s, s0, 32, keys, 2);
        for(uint8_t i = 0; i < 16; i ++ ){
            z0[i] = total_s[i];
            z1[i] = total_s[i+16];
        }
        
        tlevel.push(tt);
        tlevel.push(tt);
        for(int i = 0; i < deep; i++){
            uint8_t alphabit = 1&(alpha>>(deep - 1 - i));
            if(i == deep - 1) mpc_block2(z0, z1, alphabit, betas, &sigmas[i*16], gammas, tau0[i], tau1[i], tarro);
            else mpc_block1(z0, z1, alphabit, &sigmas[i*16], tau0[i], tau1[i], tarro);
            
            memset(z0, 0, 16);memset(z1, 0, 16);
            for(int j = 0; j < (1 << (i+1)); j ++){
                tt = tlevel.front();tlevel.pop();
                uint8_t tt_out;
                
                if(i == deep - 1) local_block2(&total_s[iter * 16], tt, &sigmas[i*16], gammas, j%2==0?tau0[i]:tau1[i], &total_s[(iter * 2 + 2) * 16], tt_out);
                else local_block1(&total_s[iter * 16], tt, &sigmas[i*16], j%2==0?tau0[i]:tau1[i], &total_s[(iter * 2 + 2) * 16], &total_s[(iter * 2 + 3) * 16], tt_out, keys);
                
                if(i != deep - 1) {
   
                    /*计算z0, z1*/
                    for(int i = 0; i < 16; i++){
                        z0[i] ^= total_s[(iter * 2 + 2) * 16 + i];
                        z1[i] ^= total_s[(iter * 2 + 3) * 16 + i];
                    }

                }else{
                    uint64_t ret;
                    memcpy(&ret, &total_s[(iter * 2 + 2) * 16], 8);
                    res_list.push_back(ret);
                }
                tlevel.push(tt_out);tlevel.push(tt_out);
                iter++;
            }

        }
        free(total_s);
        printf("lens:%u\n",res_list.size());
        
    }
    void get_list(uint8_t t_int){
        if(is_store) return;
        uint8_t tt = t_int;   

        uint8_t* total_s = (uint8_t*)malloc((1<<(deep+2)) *sizeof(uint8_t)*16);
        prf(total_s, s0, 32, keys, 2);
        uint32_t iter = 0;
        tlevel.push(tt);
        tlevel.push(tt);
        for(int i = 0; i < deep; i++){        
            for(int j = 0; j < (1 << (i+1)); j ++){
                uint8_t tt_out;
                tt = tlevel.front();tlevel.pop();
                if(i == deep - 1) local_block2(&total_s[iter * 16], tt, &sigmas[16*i], gammas, j%2==0?tau0[i]:tau1[i], &total_s[(iter * 2 + 2) * 16], tt_out);
                else local_block1(&total_s[iter * 16], tt, &sigmas[16*i], j%2==0?tau0[i]:tau1[i], &total_s[(iter * 2 + 2) * 16], &total_s[(iter * 2 + 3) * 16], tt_out, keys);
                if(i == deep - 1){
                    uint64_t ret;
                    memcpy(&ret, &total_s[(iter * 2 + 2) * 16], 8);
                    res_list.push_back(ret);
                    
                }
                tlevel.push(tt_out);tlevel.push(tt_out);
                iter++;
            }

        }
        free(total_s);
    }
    void evl(T x, uint8_t t_int){
        uint8_t alphabit = 1&(x>>(deep - 1));
        uint8_t* s, *s1_out;
        uint8_t tt = t_int;   
        uint8_t* next_s = (uint8_t*)malloc(32*sizeof(uint8_t));
        uint8_t tt_out;
        prf(next_s, s0, 32, keys, 2);
        s = (uint8_t*)malloc(16*sizeof(uint8_t));
        if(alphabit == 0)
            memcpy(s, next_s, 16);
        else
            memcpy(s, next_s + 16, 16);
        
        for(int i = 0; i < deep; i++){     
            if(i == deep - 1) local_block2(s, tt, &sigmas[16*i], gammas, alphabit==0?tau0[i]:tau1[i], next_s, tt_out);
            else local_block1(s, tt, &sigmas[16*i], alphabit==0?tau0[i]:tau1[i], next_s, s1_out, tt_out, keys);
            if(i == deep - 1){
                uint64_t ret;
                memcpy(&ret, next_s, 8);
                std::cout<<ret<<"\n";
            }
            alphabit = 1&(x>>(deep - 2 - i));
            if(alphabit == 0) memcpy(s, next_s, 16);
            else memcpy(s, &next_s[16], 16);
            tt = tt_out;

        }
        free(next_s);
        free(s);
    }

};
class Oram{
    private:
    uint32_t data_lens;
    uint32_t* data_ptr;
    FSS_MPC<uint32_t> *write0, *write1;
    uint32_t r1 , r2, v;
    public:
    Oram(uint32_t data_lens, uint32_t *data_ptr):data_lens(data_lens),data_ptr(data_ptr){
        //printf("test--------------\n");
        write0 = new FSS_MPC<uint32_t>({"player0", "player2"}, get_bits(data_lens));
        write1 = new FSS_MPC<uint32_t>({"player1", "player3"}, get_bits(data_lens));
    }
    ~Oram(){
        delete write0;delete write1;
    }
    void prepare_write(uint16_t ntimes);
    void write(uint32_t idex, uint32_t target, uint32_t org);
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
#include "fss-common.h"
#include "config.hpp"
#include "net.hpp"
#include <set>
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
<uint16_t Lambda = 16>
class SOT{
    private:
    FSS myfss;
    /*i+1, i-1*/
    ServerKeyEq gk, sk;
    public:
    uint8_t n0[Lambda], n1[Lambda], n2[Lambda];
    AES_KEY_FSS n_0, n_1, n_2;
    uint32_t fili;
    void init(){
        get_seed({"player0", "player1"}, n0, Lambda);
        AES_set_encrypt_key(n0, 128, &n_0);
        get_seed({"player1", "player2"}, n1, Lambda);
        AES_set_encrypt_key(n1, 128, &n_1);
        get_seed({"player2", "player0"}, n2, Lambda);
        AES_set_encrypt_key(n2, 128, &n_2);
        if(Config::myconfig->check("player0")){
            initializeClient(&myfss, 32, 2);
            send_mpz(myfss.prime);
            send_all({"player1", "player2"}, &myfss,sizeof(Fss)-16);
            send_all({"player1", "player2"}, myfss.aes_keys,sizeof(AES_KEY)*myfss.numKeys);
        }
        else{
            recv_fss_key(myfss, P2Pchannel::mychnl, "player0");
        }

        
    }
    void offline(){
        RAND_bytes(&fili, 4);
        ServerKeyEq k0, k1;
        generateTreeEq(&myfss, &k0, &k1, fili, 1);
        std::map suc,pre;
        suc["player0"] = "player1";suc["player1"] = "player2";suc["player2"] = "player0"; 
        pre["player1"] = "player0";pre["player2"] = "player1";pre["player0"] = "player2"; 
        send_eq_key(k0, myfss, suc[Config::myconfig->get_player()]);
        send_eq_key(k1, myfss, pre[Config::myconfig->get_player()]);
        recv_eq_key(sk, myfss, pre[Config::myconfig->get_player()]);
        recv_eq_key(gk, myfss, suc[Config::myconfig->get_player()]);
    }
    void online(uint32_t *data0, uint32_t * data1, uint16_t lens, uint32_t idex, uint32_t sid){
        uint64_t tmp;

        for(int i = 0; i < 3; i ++){
            tmp = sid + (uint64_t)i << 32;
            uint64_t w0, w1;
            if(Config::myconfig->check("player0")){
                prf(&w0, &tmp, 8, n_0, 1);
                prf(&w1, &tmp, 8, n_2, 1);
            }
            if(Config::myconfig->check("player1")){
                prf(&w0, &tmp, 8, n_1, 1);
                prf(&w1, &tmp, 8, n_0, 1);
            }
            if(Config::myconfig->check("player2")){
                prf(&w0, &tmp, 8, n_2, 1);
                prf(&w1, &tmp, 8, n_1, 1);
            }
            
            uint32_t delta = i - w0 + w1;
        }

    }
};
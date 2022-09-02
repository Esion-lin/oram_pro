#include "gc_op.h"
#include <openssl/rand.h>
struct shareholder{
    uint8_t types = 0;
    uint8_t data = 0;
};
template<typename T>
class GMW{
    public:
    GMW(){}
    void and_c(T *a, T *b, T *c, uint32_t lens){
        T delta[lens*2],delta_p[lens*2];
        bool select_bit[16*lens*sizeof(T)];
        if(Config::myconfig->check("player0")){
            /*send*/
            RAND_bytes(delta, 2*lens*sizeof(T));
            for(int i = 0; i < lens; i++){
                delta_p[i] = delta[i] ^ a[i];
                delta_p[i + lens] = delta[i + lens] ^ b[i];
            }
        }else{
            /*recv*/
            for(int i = 0; i < lens; i++){
                for(int j = 0; j < sizeof(T) * 8; j++){
                    
                }
            }
            memcpy(delta, b, lens*sizeof(T));
            memcpy(delta + lens, a, lens*sizeof(T));
        }
        
        Pratical_OT *ot = new Pratical_OT(P2Pchannel::mychnl, Config::myconfig->get_player());
        if(Config::myconfig->check("player0")){
            ot->send(reinterpret_cast<emp::block*>(delta), reinterpret_cast<emp::block*>(delta_p), 2*lens*sizeof(T), "player0", "player1");
        }else if(Config::myconfig->check("player1")){
            ot->recv(reinterpret_cast<emp::block*>(delta_p), &select_bit, 2*lens*sizeof(T), "player1", "player0");
        }
    }
    void and_c(T *a, T *b, T *c, uint32_t lens, T *a_p, T *b_p, T*c_p){
        T alpha[2*lens],Alpha[2*lens],res[lens];
        for(int i = 0; i < lens; i++){
            alpha[i] = a[i] ^ a_p[i];
            alpha[i+lens] = b[i] ^ b_p[i];
        }
        if(Config::myconfig->check("player0")){
            P2Pchannel::mychnl->send_data_to("player1", alpha, lens*2*sizeof(T));
            P2Pchannel::mychnl->recv_data_from("player1", Alpha, lens*2*sizeof(T));
        }else if(Config::myconfig->check("player1")){
            P2Pchannel::mychnl->send_data_to("player0", alpha, lens*2*sizeof(T));
            P2Pchannel::mychnl->recv_data_from("player0", Alpha, lens*2*sizeof(T));
        }
        for(int i = 0; i < 2*lens; i++){
            alpha[i] ^= Alpha[i];
        }
        for(int i = 0; i < lens; i++){
            res[i] = b[i]&Alpha[i + lens] ^ a[i]&Alpha[i] ^ c_p[i];
            if(Config::myconfig->check("player1"))
            res[i] ^= Alpha[i]&Alpha[i + lens];
        }
    }
    void scale_c(T *a, T *b, T *c, uint32_t lens){
        /*a scaler b cipher*/
        for(int i = 0; i < lens; i++) c[i] = a[i] & b[i];

    }
    void xor_c(T * a, T *b, T*c, uint32_t lens){
        for(int i = 0; i < lens; i++) c[i] = a[i] ^ b[i];
    }
    T scale_c(T a, T b){
        return a & b;
    }
    T xor_c(T a, T b){
        return a ^ b;
    }
    T and_c(T a, T b, T a_p, T b_p, T c_p){
        T alpha[2],Alpha[2],res;
        
        alpha[0] = a ^ a_p;
        alpha[1] = b ^ b_p;
        if(Config::myconfig->check("player0")){
            P2Pchannel::mychnl->send_data_to("player1", alpha, 2*sizeof(T));
            P2Pchannel::mychnl->recv_data_from("player1", Alpha, 2*sizeof(T));
        }else if(Config::myconfig->check("player1")){
            P2Pchannel::mychnl->send_data_to("player0", alpha, 2*sizeof(T));
            P2Pchannel::mychnl->recv_data_from("player0", Alpha, 2*sizeof(T));
        }
        for(int i = 0; i < 2; i++){
            alpha[i] ^= Alpha[i];
        }
        res = b&Alpha[1] ^ a&Alpha[0] ^ c_p;
        if(Config::myconfig->check("player1"))
            res ^= Alpha[0]&Alpha[1];
        return res;
    }
    
    
};
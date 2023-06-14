#pragma once 
#include <vector>
#include "encode.hpp"
#include "config.hpp"
#include "net.hpp"
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <inttypes.h>
#define THREADS 2
template<class T>
void papply(T* z, const T* x, const T* y, uint32_t lens, std::function<T(T, T)> op){
//     omp_set_num_threads(THREADS);
// #pragma omp parallel
//     {
// #pragma omp for
        for( auto i = 0; i < lens; i++){
            z[i] = op(x[i], y[i]);
        }
    // }
}
template <typename T, uint32_t precision>
class FloatEncode{
    public:
    T threshold;
    FloatEncode(){
        threshold = ((T)1 << (sizeof(T)*8 - 1));
    }
    T encode(float value){
        return (T)(value  *  (1 << precision));
    }
    float decode(T value){
        if(value > threshold){
            return -((float)(-value) / (1 << precision));
        }
        else{
            return (float)value / (1 << precision);
        }
    }

};
template<class T>
void random_T(T* target, uint32_t lens){
    AES_KEY aes_key;
    uint8_t seeds[16] = {0};
    memset(seeds, 0, 16);
    AES_set_encrypt_key(seeds, 128, &(aes_key));
    for(int i = 0; i < lens * sizeof(T) / 16; i ++)
        AES_encrypt(seeds, ((uint8_t*)target) + 16*i, &aes_key);

}
template<class T>
void sub_T(T* z, const T* x, const T* y, uint32_t lens){
    papply<T>(z, x, y, lens, [](T a, T b) -> T{return a - b;});
}
template<class T>
void sub_T_mod(T* z, const T* x, const T* y, uint32_t lens, T mod){
    papply<T>(z, x, y, lens, [mod](T a, T b) -> T{return (a + mod - b) % mod;});
}
template<class T>
void add_T(T* z, const T* x, const T* y, uint32_t lens){
    papply<T>(z, x, y, lens, [](T a, T b) -> T{return a + b;});
}
template<class T>
void add_T_mod(T* z, const T* x, const T* y, uint32_t lens, T mod){
    papply<T>(z, x, y, lens, [mod](T a, T b) -> T{return (a + b) % mod;});
}
template<class T>
void mul_T(T* z, const T* x, const T* y, uint32_t lens){
    papply<T>(z, x, y, lens, [](T a, T b) -> T{return a * b;});
}
template<class T>
void mul_T_mod(T* z, const T* x, const T* y, uint32_t lens, T mod){
    papply<T>(z, x, y, lens, [mod](T a, T b) -> T{return (a * b) % mod;});
}

template<class T>
struct AShare{
    uint32_t lens;
    T* r_1; // m_x
    T* r_2;
    T* r;
    
};
template<class T>
void init_ashare(AShare<T> &ashare, uint32_t lens){
    ashare.lens = lens;
    ashare.r_1 = (T *)malloc(sizeof(T)*lens);
    ashare.r_2 = (T *)malloc(sizeof(T)*lens); 
    ashare.r = (T *)malloc(sizeof(T)*lens); 
}
template<class T>
void release_ashare(AShare<T> &ashare){
    free(ashare.r_1);
    free(ashare.r_2);
    free(ashare.r);
}
class Protocol{
    public:
    Protocol(){}
    //setp phase
    // virtual void set_up();
    // //set input/output address
    // virtual void fill();
    // //online phase
    // virtual void online();
    // //postprocessing phase
    // virtual void verify();
    // virtual void dump();
};
//<x>
template<class T>
bool ShareA(const T* org, AShare<T> share_value){
    random_T<T>(share_value.r_2, share_value.lens);
    T temp[share_value.lens];
    if(Config::myconfig->check("player0")){
        memcpy(share_value.r_1, share_value.r_2, sizeof(T)*share_value.lens);
        add_T<T>(share_value.r, share_value.r_2, share_value.r_1, share_value.lens);
        add_T<T>(temp, org, share_value.r, share_value.lens);
        P2Pchannel::mychnl->send_data_to("player1", temp, share_value.lens*sizeof(T));
        P2Pchannel::mychnl->send_data_to("player2", temp, share_value.lens*sizeof(T));
    }else{
        P2Pchannel::mychnl->recv_data_from("player0", share_value.r_1, share_value.lens*sizeof(T));

    }
    return true;
}
template<class T>
bool RevealA(T* target, const AShare<T> share_value){
    if(Config::myconfig->check("player0")){
        P2Pchannel::mychnl->recv_data_from("player1", target, share_value.lens*sizeof(T));
        sub_T<T>(target, target, share_value.r, share_value.lens);
        // add_T<T>(target, temp, share_value.r_2, share_value.lens);

    }else if(Config::myconfig->check("player1")){
        P2Pchannel::mychnl->send_data_to("player0", share_value.r_1, share_value.lens*sizeof(T));
    }
    return true;
}
template<class T>
bool RobustRevealA(T* org, T* target, uint32_t lens){
    return true;
}
//[x]
template<class T>
bool ShareB(const T* org, T* target, uint32_t lens){
    random_T<T>(target, lens);
    if(Config::myconfig->check("player0")){
        T temp[lens];
        sub_T<T>(temp, org, target, lens);
        P2Pchannel::mychnl->send_data_to("player1", temp, lens*sizeof(T));;
    }else if(Config::myconfig->check("player1")){
        P2Pchannel::mychnl->recv_data_from("player0", target, lens*sizeof(T));
    }
    return true;
}
template<class T>
bool RevealB(T* target, const T* org, uint32_t lens, std::string recvp, std::string sendp){
    if(Config::myconfig->check(recvp)){
        T temp[lens];
        P2Pchannel::mychnl->recv_data_from(sendp, temp, lens*sizeof(T));
        add_T<T>(target, org, temp, lens);
    }else if(Config::myconfig->check(sendp)){
        P2Pchannel::mychnl->send_data_to(recvp, org, lens*sizeof(T));
    }
    return true;
}
template<class T>
bool RevealB(T* org, uint32_t lens){
    T temp[lens];
    if(Config::myconfig->check("player1")){
        P2Pchannel::mychnl->send_data_to("player2", org, lens*sizeof(T));
        P2Pchannel::mychnl->recv_data_from("player2", temp, lens*sizeof(T));
    }else if(Config::myconfig->check("player2")){
        P2Pchannel::mychnl->send_data_to("player1", org, lens*sizeof(T));
        P2Pchannel::mychnl->recv_data_from("player1", temp, lens*sizeof(T));
    }
    add_T<T>(org, org, temp, lens);
    return true;
}
//[[x]]
template<class T>
bool ShareC(const T* org, T* target, uint32_t lens, T p){
    random_T<T>(target, lens);
    for(int i = 0; i < target.lens; i++){
        target[i] %= p;
    }
    if(Config::myconfig->check("player0")){
        T temp[lens];
        sub_T_mod<T>(temp, org, target, lens, p);
        P2Pchannel::mychnl->send_data_to("player1", temp, lens*sizeof(T));;
    }else if(Config::myconfig->check("player1")){
        P2Pchannel::mychnl->recv_data_from("player0", target, lens*sizeof(T));
    }
    return true;
}
template<class T>
bool RevealC(T* target, AShare<T>& org, T p, std::string recvp, std::string sendp){
    if(Config::myconfig->check(recvp)){
        P2Pchannel::mychnl->recv_data_from(sendp, target, org.lens*sizeof(T));
        add_T_mod(target, target, org.r_2, org.lens, p);
    }else if(Config::myconfig->check(sendp)){
        P2Pchannel::mychnl->send_data_to(recvp, org.r_2, org.lens*sizeof(T));
    }
    return true;
}

template<class T>
class Mult:public Protocol{
    public:
    Mult(){}
    //need_gen: need generate r_z
    void set_up(const AShare<T> x, const AShare<T> y, AShare<T>& output, bool need_gen){
        //P0
        T gammas[output.lens];
        if(Config::myconfig->check("player0")){
            if(need_gen){
                random_T<T>(output.r_1, output.lens);
                random_T<T>(output.r_2, output.lens);
                add_T<T>(output.r, output.r_1, output.r_2, output.lens);
                
            }
            for(int i = 0; i < output.lens; i ++){
                gammas[i] = x.r[i]*y.r[i] + output.r[i];
            }
            T temp[output.lens];
            ShareB<uint64_t>(gammas, temp, output.lens );
            
            
        }else{
            if(need_gen){
                
                random_T<T>(output.r_2, output.lens);
            }
            ShareB<uint64_t>(nullptr, output.r_1, output.lens);
            
        }
    }
    void online(const AShare<T> x,const AShare<T> y, AShare<T>& output){
        if(!Config::myconfig->check("player0")){
            T temp[output.lens];
            if(Config::myconfig->check("player1")){
                mul_T<T>(temp, x.r_1, y.r_1, output.lens);
                add_T<T>(output.r_1, output.r_1, temp, output.lens);
            }
            mul_T<T>(temp, x.r_1, y.r_2, output.lens);
            sub_T<T>(output.r_1, output.r_1, temp, output.lens);
            mul_T<T>(temp, x.r_2, y.r_1, output.lens);
            sub_T<T>(output.r_1, output.r_1, temp, output.lens);
            RevealB<T>(output.r_1, output.lens);
        }
    }
    void verify(){}
    private:
};
template<class T>
class Dot:public Protocol{
    public:
    Dot(){}
    //need_gen: need generate r_z
    void set_up(const AShare<T> x, const AShare<T> y, AShare<T>& output, bool need_gen){
        //P0
        T gammas;
        if(Config::myconfig->check("player0")){
            if(need_gen){
                random_T<T>(output.r_1, output.lens);
                random_T<T>(output.r_2, output.lens);
                add_T<T>(output.r, output.r_1, output.r_2, output.lens);
                
            }
            for(int i = 0; i < output.lens; i ++){
                gammas[i] = x.r[i]*y.r[i] + output.r[i];
            }
            T temp[output.lens];
            ShareB<uint64_t>(gammas, temp, output.lens );
            
            
        }else{
            if(need_gen){
                
                random_T<T>(output.r_2, output.lens);
            }
            ShareB<uint64_t>(nullptr, output.r_1, output.lens);
            
        }
    }
    void online(const AShare<T> x,const AShare<T> y, AShare<T>& output){
        if(!Config::myconfig->check("player0")){
            T temp[output.lens];
            if(Config::myconfig->check("player1")){
                mul_T<T>(temp, x.r_1, y.r_1, output.lens);
                add_T<T>(output.r_1, output.r_1, temp, output.lens);
            }
            mul_T<T>(temp, x.r_1, y.r_2, output.lens);
            sub_T<T>(output.r_1, output.r_1, temp, output.lens);
            mul_T<T>(temp, x.r_2, y.r_1, output.lens);
            sub_T<T>(output.r_1, output.r_1, temp, output.lens);
            RevealB<T>(output.r_1, output.lens);
        }
    }
    void verify(){}
    private:
};
template<class T, uint32_t prec>
class Trunc:public Protocol{
    public:
    Trunc(){}
    void set_up(const AShare<T> x, AShare<T>& output){
        if(Config::myconfig->check("player0")){
            for(int i = 0; i < output.lens; i++)
                output.r[i] = (int64_t)x.r[i] >> prec;
            T temp[output.lens];
            ShareB<uint64_t>(output.r, temp, output.lens );
        }else{
            ShareB<uint64_t>(nullptr, output.r_2, output.lens);
        }
    }
    void online(const AShare<T> x, AShare<T>& output){
        if(!Config::myconfig->check("player0")){
            for(int i = 0; i < output.lens; i++){
                output.r_1[i] = (int64_t)x.r_1[i] >> prec;
            }
        }
    }
    void verify();
};
template<uint32_t lens>
struct Plist{
    uint8_t rb[lens];
};
template<class T>
void chop(T org, uint32_t* tar){
    for(int i = 0; i < sizeof(T)*8; i++){
        tar[i] = (org >> i) & 1;
    }
}
template<class T>
class Sign:public Protocol{
    public:
    Sign(){}
    void set_up(const AShare<T> x, AShare<T>& output){
        rls = (Plist<8*sizeof(T)>*)malloc(sizeof(Plist<8*sizeof(T)>)*output.lens);
        r_z_p = (T*) malloc(output.lens * sizeof(T));
        //chops r_x to r_1,...,r_l
        if(Config::myconfig->check("player0")){
            for(int i = 0; i < output.lens; i++){
                uint8_t temp[8*sizeof(T)];
                chop<T>(x.r[i], rls[i]);
                ShareC<uint8_t>(rls[i], temp, 8*sizeof(T), 67);
            }
        }else{
            for(int i = 0; i < output.lens; i++){
                ShareC<uint8_t>(nullptr, rls[i], 8*sizeof(T), 67);
            }
        }
        
    }
    void online();
    void verify();
    private:
    Plist<8*sizeof(T)>* rls;
    T* r_z_p; 
};
class Relu:public Protocol{
    public:
    Relu(){}
    void set_up();
    void online();
    void verify();
};
class Conv:public Protocol{
    public:
    Conv(){}
    void set_up();
    void online();
    void verify();
};
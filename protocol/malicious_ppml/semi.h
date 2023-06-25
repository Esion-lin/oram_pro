#pragma once 
#include <vector>
#include "encode.hpp"
#include "config.hpp"
#include "net.hpp"
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <inttypes.h>
#include <NTL/ZZX.h>
using namespace NTL;
#define THREADS 2

template<class T>
void random_T(T* target, uint32_t lens){
    AES_KEY aes_key;
    uint8_t seeds[16] = {0};
    memset(seeds, 0, 16);
    AES_set_encrypt_key(seeds, 128, &(aes_key));
    for(int i = 0; i < lens * sizeof(T) / 16; i ++)
        AES_encrypt(seeds, ((uint8_t*)target) + 16*i, &aes_key);
    for(int i = (lens * sizeof(T) / 16 )*16; i < lens; i++){
        target[i] = 1;
    }

}
template<uint32_t D>
ZZX randomzzx(){
    ZZX res;
    long a = 17;
    for(int i = 0; i <D; i++){
        a *= a;
        SetCoeff(res, i, a*2);
    }
    return res;
}
template<uint32_t D>
void randomzzx(ZZX * target, uint32_t lens){
    long a = 17;
    for(int j = 0; j <lens; j++)
    for(int i = 0; i <D; i++){
        a *= a;
        SetCoeff(target[j], i, a*2);
    }
}
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
void mul_TRing(ZZX* z, const ZZX* x, const ZZX* y, const ZZX P, uint32_t lens){
    papply<ZZX>(z, x, y, lens, [P](ZZX a, ZZX b) -> ZZX{return MulMod(a, b, P);});
}
template<class T>
void mul_T_mod(T* z, const T* x, const T* y, uint32_t lens, T mod){
    papply<T>(z, x, y, lens, [mod](T a, T b) -> T{return (a * b) % mod;});
}
template<uint32_t LENS>
struct AShareRing{
    ZZX r_1[LENS];
    ZZX r_2[LENS];
    ZZX r[LENS];
};
template<class T, uint32_t LENS>
struct AShare{
    uint32_t lens =1;
    T r_1[LENS]; // m_x
    T r_2[LENS];
    T r[LENS];
    // ~AShare(){
    //     if(r_1!=nullptr){
    //         free(r_1);
    //         r_1 = nullptr;
    //     }
    //     if(r_2!=nullptr){
    //         free(r_2);
    //         r_2 = nullptr;
    //     }
    //     if(r!=nullptr){
    //         free(r);
    //         r = nullptr;
    //     }
    // }
    inline AShare<T, LENS> operator+(AShare<T, LENS> a) const {
        AShare<T, LENS> res;
        res.lens = lens;
        for(int i = 0; i < lens; i ++){
            res.r[i] = a.r[i] + r[i];
            res.r_1[i] = a.r_1[i] + r_1[i];
            res.r_2[i] = a.r_2[i] + r_2[i];
        }
        return res;
    }
    inline void operator+=(AShare<T, LENS> a) {
        AShare<T, LENS> res; 
        for(int i = 0; i < lens; i ++){
            res.r[i] += a.r[i];
            res.r_1[i] += a.r_1[i];
            res.r_2[i] += a.r_2[i];
        }
    }
    inline AShare<T, LENS> operator-(AShare<T, LENS> a) const {
        AShare<T, LENS> res;
        res.lens = lens;
        for(int i = 0; i < lens; i ++){
            res.r[i] = a.r[i] - r[i];
            res.r_1[i] = a.r_1[i] - r_1[i];
            res.r_2[i] = a.r_2[i] - r_2[i];
        }
        return res;
    }
    inline void operator-=(AShare<T, LENS> a) {
        AShare<T, LENS> res; 
        for(int i = 0; i < lens; i ++){
            res.r[i] -= a.r[i];
            res.r_1[i] -= a.r_1[i];
            res.r_2[i] -= a.r_2[i];
        }
    }
    inline AShare<T, LENS> operator*(T a) const {
        AShare<T, LENS> res;
        res.lens = lens;
        for(int i = 0; i < lens; i ++){
            res.r[i] = r[i]*a;
            res.r_1[i] = r_1[i]*a;
            res.r_2[i] = r_2[i]*a;
        }
        return res;
    }
    inline void operator*=(AShare<T, LENS> a) {
        AShare<T, LENS> res; 
        for(int i = 0; i < lens; i ++){
            res.r[i] *= a.r[i];
            res.r_1[i] *= a.r_1[i];
            res.r_2[i] *= a.r_2[i];
        }
    }
};
// template<class T>
// void init_ashare(AShare<T> &ashare, uint32_t lens){
//     ashare.lens = lens;
//     ashare.r_1 = (T *)malloc(sizeof(T)*lens);
//     ashare.r_2 = (T *)malloc(sizeof(T)*lens); 
//     ashare.r = (T *)malloc(sizeof(T)*lens); 
// }
// template<class T>
// void release_ashare(AShare<T> &ashare){
//     free(ashare.r_1);
//     free(ashare.r_2);
//     free(ashare.r);
// }
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
template<class T, uint32_t LENS>
bool ShareA(const T* org, AShare<T, LENS> share_value){
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
template<class T, uint32_t LENS>
bool RevealA(T* target, const AShare<T, LENS> share_value){
    if(Config::myconfig->check("player0")){
        P2Pchannel::mychnl->recv_data_from("player1", target, share_value.lens*sizeof(T));
        sub_T<T>(target, target, share_value.r, share_value.lens);
        // add_T<T>(target, temp, share_value.r_2, share_value.lens);

    }else if(Config::myconfig->check("player1")){
        P2Pchannel::mychnl->send_data_to("player0", share_value.r_1, share_value.lens*sizeof(T));
    }
    return true;
}
template<uint32_t LENS>
bool RevealARing(ZZX* target,  AShareRing<LENS> share_value){
    uint64_t list[LENS * 32];
    if(Config::myconfig->check("player0")){
        uint32_t itr = 0;
        
        P2Pchannel::mychnl->recv_data_from("player1", list, LENS*32*sizeof(uint64_t));
        for(int i = 0; i < LENS; i++){
            for(int j = 0; j < 32; j++)
                SetCoeff(target[i], j, list[itr++]);
        }
        // std::cout<<"-------"<<target[0]<<" "<<share_value.r[0]<<std::endl;
        sub_T<ZZX>(target, target, share_value.r, LENS);
        // add_T<T>(target, temp, share_value.r_2, share_value.lens);

    }else if(Config::myconfig->check("player1")){
        uint32_t itr = 0;
        
        for(int i = 0; i < LENS; i++){
            share_value.r_1[i].SetMaxLength(32);
            for(int j = 0; j < 32; j++){
                conv(list[itr++], share_value.r_1[i][j]);
            }
        }
        P2Pchannel::mychnl->send_data_to("player0", list, LENS*32*sizeof(uint64_t));
    }
    return true;
}
template<class T>
bool RobustRevealA(T* org, T* target, uint32_t lens){
    return true;
}
//[x]
template<class T, uint32_t dd=1>
bool ShareB(const T* org, T* target, uint32_t lens){
    if(dd == 1){
    random_T<T>(target, lens);
    if(Config::myconfig->check("player0")){
        T temp[lens];
        sub_T<T>(temp, org, target, lens);
        P2Pchannel::mychnl->send_data_to("player1", temp, lens*sizeof(T));;
    }else if(Config::myconfig->check("player1")){
        P2Pchannel::mychnl->recv_data_from("player0", target, lens*sizeof(T));
    }
    }else{
        T temps[lens * dd];
        random_T<T>(temps, lens * dd);
        if(Config::myconfig->check("player0")){
            T temp[lens * dd];
            sub_T<T>(temp, temps, temps, lens * dd);
            P2Pchannel::mychnl->send_data_to("player1", temp, dd*lens*sizeof(T));;
        }else if(Config::myconfig->check("player1")){
            P2Pchannel::mychnl->recv_data_from("player0", temps, dd*lens*sizeof(T));
        }
    }
    return true;
}
bool ShareBRing(const ZZX* org, ZZX* target, uint32_t lens){
    randomzzx<32>(target, lens);
    uint64_t list[lens * 32];
    if(Config::myconfig->check("player0")){
        ZZX temp[lens];
        
        sub_T<ZZX>(temp, org, target, lens);
        
        uint32_t itr = 0;
        for(int i = 0; i < lens; i++){
            for(int j = 0; j < 32; j++)
                conv(list[itr++], temp[i][j]);
        }
        P2Pchannel::mychnl->send_data_to("player1", list, lens*sizeof(uint64_t)*32);;
    }else if(Config::myconfig->check("player1")){
        P2Pchannel::mychnl->recv_data_from("player0", list, lens*sizeof(uint64_t)*32);
        uint32_t itr = 0;
        for(int i = 0; i < lens; i++){
            for(int j = 0; j < 32; j++)
                SetCoeff(target[i], j, list[itr++]);
        }
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
template<class T, uint32_t dd = 1>
bool RevealB(T* org, uint32_t lens){
    T temp[lens*dd];
    if(dd != 1){
        if(Config::myconfig->check("player1")){
            P2Pchannel::mychnl->send_data_to("player2", temp, dd*lens*sizeof(T));
            P2Pchannel::mychnl->recv_data_from("player2", temp, dd*lens*sizeof(T));
        }else if(Config::myconfig->check("player2")){
            P2Pchannel::mychnl->send_data_to("player1", temp, dd*lens*sizeof(T));
            P2Pchannel::mychnl->recv_data_from("player1", temp, dd*lens*sizeof(T));
        }
    }else{
        if(Config::myconfig->check("player1")){
            P2Pchannel::mychnl->send_data_to("player2", org, lens*sizeof(T));
            P2Pchannel::mychnl->recv_data_from("player2", temp, lens*sizeof(T));
        }else if(Config::myconfig->check("player2")){
            P2Pchannel::mychnl->send_data_to("player1", org, lens*sizeof(T));
            P2Pchannel::mychnl->recv_data_from("player1", temp, lens*sizeof(T));
        }
        add_T<T>(org, org, temp, lens);
    }
    return true;
}
bool RevealBRing(ZZX* org, uint32_t lens){
    ZZX* temp = new ZZX[lens];
    uint64_t list[lens * 32];
    if(Config::myconfig->check("player1")){
        uint32_t itr = 0;
        for(int i = 0; i < lens; i++){
            for(int j = 0; j < 32; j++)
                conv(list[itr++], org[i][j]);
        }
        P2Pchannel::mychnl->send_data_to("player2", list, lens * 32*sizeof(uint64_t));
        P2Pchannel::mychnl->recv_data_from("player2", list, lens * 32*sizeof(uint64_t));
        itr = 0;
        for(int i = 0; i < lens; i++){
            temp[i].SetMaxLength(32);
            for(int j = 0; j < 32; j++)
                SetCoeff(temp[i], j, list[itr++]);
        }
    }else if(Config::myconfig->check("player2")){
        uint32_t itr = 0;
        for(int i = 0; i < lens; i++){
            for(int j = 0; j < 32; j++)
                conv(list[itr++], org[i][j]);
        }
        P2Pchannel::mychnl->send_data_to("player1", list, lens * 32*sizeof(uint64_t));
        P2Pchannel::mychnl->recv_data_from("player1", list, lens * 32*sizeof(uint64_t));
        itr = 0;
        for(int i = 0; i < lens; i++){
            temp[i].SetMaxLength(32);
            for(int j = 0; j < 32; j++)
                SetCoeff(temp[i], j, list[itr++]);
        }
    }
    add_T<ZZX>(org, org, temp, lens);
    delete[] temp;
    return true;
}
//[[x]]
template<class T>
bool ShareC(const T* org, T* target, uint32_t lens, T p){
    random_T<T>(target, lens);
    for(int i = 0; i < lens; i++){
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
bool RevealC(T* org, uint32_t lens, T p){
    if(Config::myconfig->check("player2") || Config::myconfig->check("player1")){
        P2Pchannel::mychnl->send_data_to("player0", org, lens*sizeof(T));
        
    }else if(Config::myconfig->check("player0")){
        T temp[lens];
        P2Pchannel::mychnl->recv_data_from("player1", temp, lens*sizeof(T));
        P2Pchannel::mychnl->recv_data_from("player2", org, lens*sizeof(T));
        add_T_mod(org, org, temp, lens, p);
    }
    return true;
}

template<class T, uint32_t LENS>
class Mult:public Protocol{
    public:
    Mult(){}
    //need_gen: need generate r_z
    void set_up(const AShare<T, LENS> x, const AShare<T, LENS> y, AShare<T, LENS>& output, bool need_gen){
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
    void online(const AShare<T, LENS> x,const AShare<T, LENS> y, AShare<T, LENS>& output){
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
template<class T, uint32_t LENS, uint32_t dd = 1>
class Dot:public Protocol{
    public:
    Dot(){}
    //need_gen: need generate r_z
    void set_up(const AShare<T, LENS> *x, const AShare<T, LENS> *y, AShare<T, LENS>& output, uint32_t lens, bool need_gen){
        //P0
        T gammas[output.lens];
        if(Config::myconfig->check("player0")){
            if(need_gen){
                random_T<T>(output.r_1, output.lens);
                random_T<T>(output.r_2, output.lens);
                add_T<T>(output.r, output.r_1, output.r_2, output.lens);
                
            }
            for(int i = 0; i < output.lens; i ++){
                gammas[i] = 0;
                for(int j = 0; j < lens; j++)
                    gammas[i] += x[j].r[i]*y[j].r[i];
                gammas[i] += output.r[i];
            }
            T temp[output.lens];
            ShareB<T, dd>(gammas, temp, output.lens);
            printf("---------lens:%d\n", output.lens);
            
        }else{
            if(need_gen){
                
                random_T<T>(output.r_2, output.lens);
            }
            ShareB<T, dd>(nullptr, output.r_1, output.lens);
            
        }
    }
    void online(const AShare<T, LENS> *x, const AShare<T, LENS> *y, AShare<T, LENS>& output, uint32_t lens){
        if(!Config::myconfig->check("player0")){
            T temp[output.lens];
            for(int j = 0; j < lens; j++){
                if(Config::myconfig->check("player1")){
                    mul_T<T>(temp, x[j].r_1, y[j].r_1, output.lens);
                    add_T<T>(output.r_1, output.r_1, temp, output.lens);
                }
                mul_T<T>(temp, x[j].r_1, y[j].r_2, output.lens);
                sub_T<T>(output.r_1, output.r_1, temp, output.lens);
                mul_T<T>(temp, x[j].r_2, y[j].r_1, output.lens);
                sub_T<T>(output.r_1, output.r_1, temp, output.lens);
            }
            RevealB<T, dd>(output.r_1, output.lens);
        }
    }
    void verify(){}
    private:
};
template<uint32_t LENS>
class DotForRing:public Protocol{
    public:
    bool ismalicious = false;
    DotForRing(){}
    //need_gen: need generate r_z
    void set_up(const AShareRing<LENS>*x, const AShareRing<LENS> *y, AShareRing<LENS>& output, uint32_t lens, bool need_gen){
        //P0
        
        SetCoeff(P, 32, 1);
        ZZX gammas[LENS];
        if(Config::myconfig->check("player0")){
            if(need_gen){
                randomzzx<32>(output.r_1, LENS);
                randomzzx<32>(output.r_2, LENS);

                add_T<ZZX>(output.r, output.r_1, output.r_2, LENS);
                
            }
            for(int i = 0; i < LENS; i ++){
                gammas[i] = 0;
                for(int j = 0; j < lens; j++)
                    gammas[i] += MulMod(x[j].r[i], y[j].r[i], P);
                
                gammas[i] += output.r[i];
                if(ismalicious){
                    gammas[i] += 1;
                }
            }
            
            ZZX temp[LENS];
            ShareBRing(gammas, temp, LENS );
            
            
        }else{
            if(need_gen){
                randomzzx<32>(output.r_2, LENS);
            }
            ShareBRing(nullptr, output.r_1, LENS);
            
        }
    }
    void online(const AShareRing<LENS> *x, const AShareRing<LENS> *y, AShareRing<LENS>& output, uint32_t lens){
        if(!Config::myconfig->check("player0")){
            ZZX temp[LENS];
            for(int j = 0; j < lens; j++){
            if(Config::myconfig->check("player1")){
                mul_TRing(temp, x[j].r_1, y[j].r_1,P, LENS);
                
                add_T<ZZX>(output.r_1, output.r_1, temp, LENS);
            }
            mul_TRing(temp, x[j].r_1, y[j].r_2,P, LENS);
            sub_T<ZZX>(output.r_1, output.r_1, temp, LENS);
            mul_TRing(temp, x[j].r_2, y[j].r_1,P, LENS);
            sub_T<ZZX>(output.r_1, output.r_1, temp, LENS);
            }
            RevealBRing(output.r_1, LENS);
        }
    }
    void verify(){}
    private:
    ZZX P;
    
};
template<class T, uint32_t prec, uint32_t LENS>
class Trunc:public Protocol{
    public:
    Trunc(){}
    void set_up(const AShare<T, LENS> x, AShare<T, LENS>& output){
        if(Config::myconfig->check("player0")){
            for(int i = 0; i < output.lens; i++)
                output.r[i] = (int64_t)x.r[i] >> prec;
            T temp[output.lens];
            ShareB<uint64_t>(output.r, temp, output.lens );
        }else{
            ShareB<uint64_t>(nullptr, output.r_2, output.lens);
        }
    }
    void online(const AShare<T, LENS> x, AShare<T, LENS>& output){
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
void chop(T org, uint8_t* tar){
    for(int i = 0; i < sizeof(T)*8; i++){
        tar[sizeof(T)*8 - 1 -i] = (org >> i) & 1;
    }
}
bool check_list(uint8_t *u, uint8_t *v, uint32_t lens){
    for(int i = 0; i < lens; i++){
        if(u[i] == 0 && v[i] != 0)
            return true;
    }
    return false;
}
template<class T, uint32_t LENS>
class Sign:public Protocol{
    public:
    Sign(){}
    ~Sign(){
        if(rls != nullptr){
            free(rls);
            rls = nullptr;
        }
        if(r_z_p != nullptr){
            free(r_z_p);
            r_z_p = nullptr;
        }
        if(delta != nullptr){
            free(delta);
            delta = nullptr;
        }
        if(gammas != nullptr){
            free(gammas);
            gammas = nullptr;
        }
    }
    void set_up(const AShare<T, LENS> x, AShare<T, LENS>& output, bool need_gen){
        rls = (Plist<8*sizeof(T)+1>*)malloc(sizeof(Plist<8*sizeof(T)+1>)*output.lens);
        r_z_p = (T*) malloc(output.lens * sizeof(T));
        
        //chops r_x to r_1,...,r_l
        if(Config::myconfig->check("player0")){
            for(int i = 0; i < output.lens; i++){
                uint8_t temp[8*sizeof(T)+1];
                chop<T>(x.r[i], rls[i].rb);
                rls[i].rb[8*sizeof(T)] = 1;
                // for(int j = 0; j < 8*sizeof(T); j ++){
                //     printf("[%u]", (uint32_t)rls[i].rb[j]);
                // }
                // printf("\n");
                
                ShareC<uint8_t>(rls[i].rb, temp, 8*sizeof(T)+1, 67);
                
            }
            if(need_gen){
                random_T<T>(output.r_1, output.lens);
                random_T<T>(output.r_2, output.lens);
                add_T<T>(output.r, output.r_1, output.r_2, output.lens);
            }
            T temp_r[output.lens];
            random_T<T>(temp_r, output.lens);
            random_T<T>(r_z_p, output.lens);
            add_T<T>(r_z_p, r_z_p, temp_r, output.lens);
        }else{
            delta = (uint8_t*)malloc(output.lens*sizeof(uint8_t));
            gammas = (T*) malloc(output.lens * sizeof(T));
            
            
            for(int i = 0; i < output.lens; i++){
                ShareC<uint8_t>(nullptr, rls[i].rb, 8*sizeof(T)+1, 67);
            }
            if(need_gen){
                random_T<T>(output.r_2, output.lens);
            }
            random_T<T>(r_z_p, output.lens);
            //pick delta
            random_T<uint8_t>(delta, output.lens);
            // calculate delta + r_z_p - 2 delta r_z_p + r_z
            for(int i = 0; i < output.lens; i++){
                delta[i] %= 2;
                gammas[i] = r_z_p[i] - 2*r_z_p[i]*delta[i] + output.r_2[i];
                if(Config::myconfig->check("player1"))
                    gammas[i]+=delta[i];
            }
            RevealB<T>(gammas, output.lens);
        }
        
    }
    void online(const AShare<T, LENS> x, AShare<T, LENS>& output){
        if(!Config::myconfig->check("player0")){
            Plist<8*sizeof(T)+1> m_sigmas[output.lens],m_j[output.lens],w_j[output.lens],w_j_p[output.lens],u_j[output.lens],v_j[output.lens],temp[output.lens];
            uint32_t shiftsize = sizeof(T)*8 - 1;
            for(int i = 0; i < output.lens; i++){
                //pick w_j, w'_j
                random_T<uint8_t>(w_j[i].rb, 8*sizeof(T)+1);
                random_T<uint8_t>(w_j_p[i].rb, 8*sizeof(T)+1);
                chop<T>(x.r_1[i] - (1<<shiftsize) + 1, m_sigmas[i].rb);
                
                m_sigmas[i].rb[8*sizeof(T)] = 0;
                //m_j = m_sigma + rls - 2*m_sigma*rls
                uint8_t sum_of_m = 0;
                for(int j = 0; j < sizeof(T)*8+1; j++){
                    uint8_t temp;//mj'
                    m_j[i].rb[j] = (rls[i].rb[j] + 67 - 2*m_sigmas[i].rb[j] *rls[i].rb[j])%67;
                    
                    if(Config::myconfig->check("player1"))
                        m_j[i].rb[j] = (m_j[i].rb[j] + m_sigmas[i].rb[j]) %67;
                    
                    // for(int j = 0; j < 8*sizeof(T); j ++){
                    //     printf("{%u}", (uint32_t)m_j[i].rb[j]);
                    // }
                    // printf("\n");
                    //sum_of m_j - 2m_j + 1 
                    sum_of_m = (sum_of_m+m_j[i].rb[j])%67;
                    temp = ((uint32_t)sum_of_m + 134 - 2*m_j[i].rb[j]) % 67;
                    if(Config::myconfig->check("player1"))
                        temp += 1;
                    w_j[i].rb[j] = (w_j[i].rb[j] % 66)+1;
                    w_j_p[i].rb[j] = (w_j[i].rb[j] % 66)+1;
                    
                    if(Config::myconfig->check("player1")){
                        u_j[i].rb[j] = (((uint32_t)temp)*w_j[i].rb[j]) % 67;
                        v_j[i].rb[j] = (((((uint32_t)temp)*w_j[i].rb[j]) + 1)*w_j_p[i].rb[j]) % 67;
                    }else{
                        u_j[i].rb[j] = (((uint32_t)temp)*w_j[i].rb[j] + (1^delta[i]^m_sigmas[i].rb[j])) % 67;
                        v_j[i].rb[j] = (((uint32_t)temp)*w_j[i].rb[j]*w_j_p[i].rb[j]) % 67;
                    }
                }
                // for(int j = 0; j < 8*sizeof(T)+1; j ++){
                //         printf("{%03u}", (uint32_t) temp[i].rb[j]);
                //     }
                //reveal to P_0
                // RevealC<uint8_t>(m_j[i].rb, 8*sizeof(T)+1, 67);
                RevealC<uint8_t>(u_j[i].rb, 8*sizeof(T)+1, 67);
                RevealC<uint8_t>(v_j[i].rb, 8*sizeof(T)+1, 67);

            }
            T backmessage[output.lens];
            P2Pchannel::mychnl->recv_data_from("player0", backmessage, sizeof(T)*output.lens);
            for(int i = 0; i < output.lens; i++){
                output.r_1[i] = backmessage[i] - 2 * delta[i] * backmessage[i] + gammas[i];
            }
            
        }else{
            Plist<8*sizeof(T)+1>u_j[output.lens],v_j[output.lens];
            T backmessage[output.lens];
            for(int i = 0; i < output.lens; i++){
                RevealC<uint8_t>(u_j[i].rb, 8*sizeof(T)+1, 67);
                RevealC<uint8_t>(v_j[i].rb, 8*sizeof(T)+1, 67);
                if(check_list(u_j[i].rb, v_j[i].rb, 8*sizeof(T)+1)){
                    //send -r_z_p
                    backmessage[i] = -r_z_p[i];
                }else{
                    backmessage[i] = -r_z_p[i] + 1;
                }
                // printf("u:");
                // for(int j = 0; j < 8*sizeof(T)+1; j++){
                //     printf("%03u ",(uint32_t)u_j[i].rb[j]);
                // }
                // printf("\n");
                // printf("v:");
                // for(int j = 0; j < 8*sizeof(T)+1; j++){
                //     printf("%03u ",(uint32_t)v_j[i].rb[j]);
                // }
                // printf("\n");
            }
            P2Pchannel::mychnl->send_data_to("player1", backmessage, sizeof(T)*output.lens);
            P2Pchannel::mychnl->send_data_to("player2", backmessage, sizeof(T)*output.lens);
            
        }
    }
    void verify();
    private:
    Plist<8*sizeof(T)+1>* rls = nullptr;
    T* r_z_p = nullptr; 
    uint8_t* delta = nullptr;
    T* gammas;
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


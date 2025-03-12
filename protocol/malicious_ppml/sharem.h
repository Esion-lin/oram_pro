
#pragma once 
#include <vector>
#include "encode.hpp"
#include "config.hpp"
#include "unit.h"
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <inttypes.h>
template<uint32_t lens>
struct Plist{
    uint8_t rb[lens];
};
template<uint32_t lens, uint32_t ll>
struct Pvlist{
    uint8_t rb[lens];
    uint8_t mac[lens][ll];
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
template<class T>
void papply(T* z, const T* x, const T* y, uint32_t lens, std::function<T(T, T)> op){
//     omp_set_num_threads(THREADS);
    #pragma omp parallel for
    for( auto i = 0; i < lens; i++){
        z[i] = op(x[i], y[i]);
    }
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
void mul_TRing(ZZX* z, const ZZX* x, const ZZX* y, const ZZX P, uint32_t lens){
    papply<ZZX>(z, x, y, lens, [P](ZZX a, ZZX b) -> ZZX{return MulMod(a, b, P);});
}
template<class T>
void mul_T_mod(T* z, const T* x, const T* y, uint32_t lens, T mod){
    papply<T>(z, x, y, lens, [mod](T a, T b) -> T{return (a * b) % mod;});
}

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
template<class T>
struct AShareT{
    T r_1, r_2, r; // m_x
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
    inline AShareT operator+(AShareT a) const {

        AShareT res;
        res.r = a.r + r;
        res.r_1 = a.r_1 + r_1;
        res.r_2 = a.r_2 + r_2;
        return res;
    }
    inline void operator+=(AShareT a) {
        r += a.r;
        r_1 += a.r_1;
        r_2 += a.r_2;
    }
    inline AShareT operator-(AShareT a) const {
        AShareT res;
        res.r = a.r - r;
        res.r_1 = a.r_1 - r_1;
        res.r_2 = a.r_2 - r_2;
        return res;
    }
    inline void operator-=(AShareT a) {
        r -= a.r;
        r_1 -= a.r_1;
        r_2 -= a.r_2;
    }
    inline AShareT operator*(T a) const {
        AShareT res;
            res.r = r*a;
            res.r_1 = r_1*a;
            res.r_2 = r_2*a;
        return res;
    }
    inline void operator*=(AShareT a) {
            r *= a.r;
            r_1 *= a.r_1;
            r_2 *= a.r_2;
    }
};
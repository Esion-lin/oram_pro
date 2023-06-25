#pragma once
#include <stdint.h>
#include "semi.h"
#include <NTL/ZZX.h>
#include <time.h> 
using namespace NTL;

template <class T, uint32_t LENS>
bool inter(T d0, T d1, T d2, AShare<T, LENS> x_0, AShare<T, LENS> x_1, AShare<T, LENS> x_2, AShare<T, LENS> &result){
    for(int i = 0; i < result.lens; i++){
        result.r[i] = x_0.r[i] * d0 + x_1.r[i] * d1 + x_2.r[i] * d2;
        result.r_1[i] = x_0.r_1[i] * d0 + x_1.r_1[i] * d1 + x_2.r_1[i] * d2;
        result.r_2[i] = x_0.r_2[i] * d0 + x_1.r_2[i] * d1 + x_2.r_2[i] * d2;
    }
    return true;
}
template <class T, uint32_t LENS>
bool inter(T d0, T d1, AShare<T, LENS> x_0, AShare<T, LENS> x_1, AShare<T, LENS> &result){
    for(int i = 0; i < result.lens; i++){
        result.r[i] = x_0.r[i] * d0 + x_1.r[i] * d1 ;
        result.r_1[i] = x_0.r_1[i] * d0 + x_1.r_1[i]*d1;
        result.r_2[i] = x_0.r_2[i] * d0 + x_1.r_2[i]*d1;
    }
    return true;
}
template <uint32_t LENS>
bool inter(ZZX d0, ZZX d1, ZZX d2, AShareRing<LENS> x_0, AShareRing<LENS> x_1, AShareRing<LENS> x_2, ZZX P, AShareRing<LENS> &result){
    for(int i = 0; i < LENS; i++){
        result.r[i] = MulMod(x_0.r[i], d0, P) + MulMod(x_1.r[i], d1, P) + MulMod(x_2.r[i], d2, P);
        result.r_1[i] = MulMod(x_0.r_1[i], d0, P) + MulMod(x_1.r_1[i], d1, P) + MulMod(x_2.r_1[i], d2, P);
        result.r_2[i] = MulMod(x_0.r_2[i], d0, P) + MulMod(x_1.r_2[i], d1, P) + MulMod(x_2.r_2[i], d2, P);
    }
    return true;
}

template <uint32_t LENS>
bool inter(ZZX d0, ZZX d1, AShareRing<LENS> x_0, AShareRing<LENS> x_1, ZZX P, AShareRing<LENS> &result){
    for(int i = 0; i < LENS; i++){
        result.r[i] = MulMod(x_0.r[i], d0, P) + MulMod(x_1.r[i], d1, P);
        result.r_1[i] = MulMod(x_0.r_1[i], d0, P) + MulMod(x_1.r_1[i], d1, P);
        result.r_2[i] = MulMod(x_0.r_2[i], d0, P) + MulMod(x_1.r_2[i], d1, P);
    }
    return true;
}

template <class T, uint32_t LENS>
class PolyVerify{
    //N: pairs
    //n fan-ins
    //D degree
    public:
    uint32_t N;
    std::vector<T> in_list;
    std::vector<T> out_list;
    // void set_up(const AShare<T> x, const AShare<T> y, AShare<T>& output, bool need_gen){
    //     random_T<T>(r_list, N/2);
    // }
    PolyVerify(uint32_t N_){
        N = N_;
    }
    bool verify(AShare<T, LENS>* x, AShare<T, LENS>* y, AShare<T, LENS> z, AShare<T, LENS>* x_p, AShare<T, LENS>* y_p, AShare<T, LENS>& z_p){
        //set N/2 linear function f^{N/2}_m
        
        AShare<T, LENS> *ff[3], *gg[3], h[3];
        ff[0] = new AShare<T, LENS>[N/2];
        ff[1] = new AShare<T, LENS>[N/2];
        ff[2] = new AShare<T, LENS>[N/2];
        gg[0] = new AShare<T, LENS>[N/2];
        gg[1] = new AShare<T, LENS>[N/2];
        gg[2] = new AShare<T, LENS>[N/2];
        T delta = 1024;
        T d0 = (delta - 1)*(delta - 2) /2;
        T d1 = delta*(2 - delta);
        T d2 = (delta - 1)*delta /2;
        // for(int kk = 0; kk < 32; kk ++)
        for(int i = 0; i < N /2; i++){

            ff[0][i] = x[2*i];
            ff[1][i] = x[2*i + 1];
            papply<T>(ff[2][i].r, ff[1][i].r, ff[0][i].r, LENS, [](T a, T b) -> T{return 2*a - b;});
            papply<T>(ff[2][i].r_1, ff[1][i].r_1, ff[0][i].r_1, LENS, [](T a, T b) -> T{return 2*a - b;});
            papply<T>(ff[2][i].r_2, ff[1][i].r_2, ff[0][i].r_2, LENS, [](T a, T b) -> T{return 2*a - b;});
            
            gg[0][i] = y[2*i];
            gg[1][i] = y[2*i + 1];
            papply<T>(gg[2][i].r, gg[1][i].r, gg[0][i].r, LENS, [](T a, T b) -> T{return 2*a - b;});
            papply<T>(gg[2][i].r_1, gg[1][i].r_1, gg[0][i].r_1, LENS, [](T a, T b) -> T{return 2*a - b;});
            papply<T>(gg[2][i].r_2, gg[1][i].r_2, gg[0][i].r_2, LENS, [](T a, T b) -> T{return 2*a - b;});
            inter(1-delta, delta, ff[0][i], ff[1][i], x_p[i]);
            inter(1-delta, delta, gg[0][i], gg[1][i], y_p[i]);
            
        }
        dot1.set_up(ff[0], gg[0], h[0], N/2,true);
        dot1.online(ff[0], gg[0], h[0], N/2);
        // h[1] = z - h[0];
        papply<T>(h[1].r, z.r, h[0].r, LENS, [](T a, T b) -> T{return a - b;});
        papply<T>(h[1].r_1, z.r_1, h[0].r_1, LENS, [](T a, T b) -> T{return a - b;});
        papply<T>(h[1].r_2, z.r_2, h[0].r_2, LENS, [](T a, T b) -> T{return a - b;});
        
        dot2.set_up(ff[2], gg[2], h[2], N/2,true);
        dot2.online(ff[2], gg[2], h[2], N/2);

        inter(d0, d1, d2, h[0], h[1], h[2], z_p);
        delete[] ff[0];
        delete[] ff[1];
        delete[] ff[2];
        delete[] gg[0];
        delete[] gg[1];
        delete[] gg[2];
        return true;
    }
    private:
    Dot<T, LENS, 32> dot1,dot2;

};
template<uint32_t LENS>
void dummy_transfer(AShareRing<LENS>& tar, uint32_t d){
    for(int i = 0; i < LENS; i++){
        SetCoeff(tar.r[i], 0, 1);
        SetCoeff(tar.r_1[i], 0, 1);
        SetCoeff(tar.r_2[i], 0, 1);
        for(int j = 1; j <d; j++){
            SetCoeff(tar.r[i], j, 0);
            SetCoeff(tar.r_1[i], j, 0);
            SetCoeff(tar.r_2[i], j, 0);
        }
    }
}
template<uint32_t LENS>
void local_transfer(AShareRing<LENS>& tar, AShare<uint64_t, LENS> org, uint32_t d){
    for(int i = 0; i < LENS; i++){
        SetCoeff(tar.r[i], 0, org.r[i]);
        SetCoeff(tar.r_1[i], 0, org.r_1[i]);
        SetCoeff(tar.r_2[i], 0, org.r_2[i]);
        for(int j = 1; j <d; j++){
            SetCoeff(tar.r[i], j, 0);
            SetCoeff(tar.r_1[i], j, 0);
            SetCoeff(tar.r_2[i], j, 0);
        }
    }
}
template <uint32_t LENS, uint32_t D>
class PolyRingVerify{
    //N: pairs
    //n fan-ins
    //D degree
    public:
    // void set_up(const AShare<T> x, const AShare<T> y, AShare<T>& output, bool need_gen){
    //     random_T<T>(r_list, N/2);
    // }
    PolyRingVerify(uint32_t N_){
        N = N_;
    }
    uint32_t N;
    bool verify(AShareRing<LENS>* x, AShareRing<LENS>* y, AShareRing<LENS> z, AShareRing<LENS>* x_p, AShareRing<LENS>* y_p, AShareRing<LENS>& z_p){
        SetCoeff(P, 32, 1);
        //set N/2 linear function f^{N/2}_m
        AShareRing<LENS> ff[3][N/2], gg[3][N/2], h[3];
        ZZX delta = randomzzx<D>();
        ZZX d0 = MulMod(delta - 1, delta - 2, P) /2;
        ZZX d1 = MulMod(delta, 2 - delta, P);
        ZZX d2 = MulMod(delta - 1, delta, P) /2;
        for(int i = 0; i < N /2; i++){

            ff[0][i] = x[2*i];
            ff[1][i] = x[2*i + 1];
            papply<ZZX>(ff[2][i].r, ff[1][i].r, ff[0][i].r, LENS, [](ZZX a, ZZX b) -> ZZX{return 2*a - b;});
            papply<ZZX>(ff[2][i].r_1, ff[1][i].r_1, ff[0][i].r_1, LENS, [](ZZX a, ZZX b) -> ZZX{return 2*a - b;});
            papply<ZZX>(ff[2][i].r_2, ff[1][i].r_2, ff[0][i].r_2, LENS, [](ZZX a, ZZX b) -> ZZX{return 2*a - b;});
            
            gg[0][i] = y[2*i];
            gg[1][i] = y[2*i + 1];
            papply<ZZX>(gg[2][i].r, gg[1][i].r, gg[0][i].r, LENS, [](ZZX a, ZZX b) -> ZZX{return 2*a - b;});
            papply<ZZX>(gg[2][i].r_1, gg[1][i].r_1, gg[0][i].r_1, LENS, [](ZZX a, ZZX b) -> ZZX{return 2*a - b;});
            papply<ZZX>(gg[2][i].r_2, gg[1][i].r_2, gg[0][i].r_2, LENS, [](ZZX a, ZZX b) -> ZZX{return 2*a - b;});
            inter(1-delta, delta, ff[0][i], ff[1][i], P, x_p[i]);
            inter(1-delta, delta, gg[0][i], gg[1][i], P, y_p[i]);
            
        }
        dot1.ismalicious = true;
        dot1.set_up(ff[0], gg[0], h[0], N/2,true);
        dot1.online(ff[0], gg[0], h[0], N/2);
        // h[1] = z - h[0];
        papply<ZZX>(h[1].r, z.r, h[0].r, LENS, [](ZZX a, ZZX b) -> ZZX{return a - b;});
        papply<ZZX>(h[1].r_1, z.r_1, h[0].r_1, LENS, [](ZZX a, ZZX b) -> ZZX{return a - b;});
        papply<ZZX>(h[1].r_2, z.r_2, h[0].r_2, LENS, [](ZZX a, ZZX b) -> ZZX{return a - b;});

        dot2.set_up(ff[2], gg[2], h[2], N/2,true);
        dot2.online(ff[2], gg[2], h[2], N/2);

        inter(d0, d1, d2, h[0], h[1], h[2], P, z_p);
        // for(int i = 0; i < N /2; i++){
        //     // release_ashare<T>(ff[0][i]);
        //     // release_ashare<T>(ff[1][i]);
        //     release_ashare<T>(ff[2][i]);
        //     // release_ashare<T>(gg[0][i]);
        //     // release_ashare<T>(gg[1][i]);
        //     release_ashare<T>(gg[2][i]);
        // }
        // release_ashare<T>(h[0]);
        // release_ashare<T>(h[1]);
        // release_ashare<T>(h[2]);
        return true;
    }
    private:
        ZZX P;
    DotForRing<LENS>  dot1,dot2;
};
template<uint32_t LENS>
class Dotverify{
    public:
    ZZX P;
    uint32_t N;
    AShareRing<LENS> alpha;
    ZZX* temp_list = nullptr;
    Dotverify(uint32_t N_){
        N = N_;
        temp_list = new ZZX[3*N];
    }
    ~Dotverify(){
        delete[] temp_list;
    }
    void set_up(const AShareRing<LENS>*x, const AShareRing<LENS> *y, AShareRing<LENS> z){
        SetCoeff(P, 32, 1);
        if(Config::myconfig->check("player0")){
            randomzzx<32>(alpha.r, LENS);
            randomzzx<32>(alpha.r_1, LENS);
            randomzzx<32>(alpha.r_2, LENS);
        }else{
            randomzzx<32>(alpha.r, LENS);
            randomzzx<32>(alpha.r_1, LENS);
            randomzzx<32>(alpha.r_2, LENS);
        }
        ZZX gammas[LENS];
        if(Config::myconfig->check("player0")){
            for(int i = 0; i < LENS; i ++){
                gammas[i] = 0;
                for(int j = 0; j < N; j++){
                    ZZX tmp = MulMod(x[j].r[i], y[j].r[i], P);
                    gammas[i] += MulMod(tmp, alpha.r[i], P);
                    temp_list[j * 3] = tmp;
                    temp_list[j * 3 + 1] = MulMod(x[j].r[i], alpha.r[i], P);
                    temp_list[j * 3 + 2] = MulMod(y[j].r[i], alpha.r[i], P);
                }
                gammas[i] += z.r[i];
            }
            
            ZZX temp[LENS];
            ZZX temp2[N*3*LENS];
            ShareBRing(gammas, temp, LENS);   
            ShareBRing(temp_list, temp2, N*3*LENS);         
        }else{
            ShareBRing(nullptr, z.r_1, LENS);
            ShareBRing(nullptr, temp_list, N*3*LENS);
        }
    }
    bool verify(AShareRing<LENS>* x, AShareRing<LENS>* y, AShareRing<LENS> z){
        if(!Config::myconfig->check("player0")){
            ZZX temp[LENS];
            for(int j = 0; j < N; j++){
            if(Config::myconfig->check("player1")){
                mul_TRing(temp, x[j].r_1, y[j].r_1,P, LENS);
                mul_TRing(temp, temp, alpha.r_1,P, LENS);
                add_T<ZZX>(z.r_1, z.r_1, temp, LENS);
            }
            mul_TRing(temp, x[j].r_1, y[j].r_2,P, LENS);
            mul_TRing(temp, x[j].r_1, y[j].r_2,P, LENS);
            mul_TRing(temp, x[j].r_1, y[j].r_2,P, LENS);
            sub_T<ZZX>(z.r_1, z.r_1, temp, LENS);
            mul_TRing(temp, x[j].r_2, y[j].r_1,P, LENS);
            mul_TRing(temp, x[j].r_2, y[j].r_1,P, LENS);
            mul_TRing(temp, x[j].r_2, y[j].r_1,P, LENS);
            sub_T<ZZX>(z.r_1, z.r_1, temp, LENS);
            }
            RevealBRing(z.r_1, LENS);
        }
        return true;
    }
};


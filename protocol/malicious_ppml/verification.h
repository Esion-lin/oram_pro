#pragma once
#include <stdint.h>
#include "semi.h"
template <class T, uint32_t D>
struct Poly{
    T value[D];
};
template <class T, uint32_t D>
Poly<T, D> div_poly(Poly<T, D> x, Poly<T, D> y){
    Poly<T, D>res;
    //
}
template <class T>
bool inter(T d0, T d1, T d2, AShare<T> x_0, AShare<T> x_1, AShare<T> x_2, AShare<T> &result){
    for(int i = 0; i < result.lens; i++){
        result.r[i] = x_0.r[i] * d0 + x_1.r[i] * d1 + x_2.r[i] * d2;
        result.r_1[i] = x_0.r_1[i] * d0 + x_1.r_1[i] * d1 + x_2.r_1[i] * d2;
        result.r_2[i] = x_0.r_2[i] * d0 + x_1.r_2[i] * d1 + x_2.r_2[i] * d2;
    }
    return true;
}
template <class T>
bool inter(T d0, T d1, AShare<T> x_0, AShare<T> x_1, AShare<T> &result){
    for(int i = 0; i < result.lens; i++){
        result.r[i] = x_0.r[i] * d0 + x_1.r[i] * d1 ;
        result.r_1[i] = x_0.r_1[i] * d0 + x_1.r_1[i]*d1;
        result.r_2[i] = x_0.r_2[i] * d0 + x_1.r_2[i]*d1;
    }
    return true;
}

template <class T, uint32_t N>
class PolyVerify{
    //N: pairs
    //n fan-ins
    //D degree
    public:
    std::vector<T> in_list;
    std::vector<T> out_list;
    // void set_up(const AShare<T> x, const AShare<T> y, AShare<T>& output, bool need_gen){
    //     random_T<T>(r_list, N/2);
    // }
    bool verify(AShare<T>* x, AShare<T>* y, AShare<T> z, AShare<T>* x_p, AShare<T>* y_p, AShare<T>& z_p){
        //set N/2 linear function f^{N/2}_m
        AShare<T> ff[3][N/2], gg[3][N/2], h[3];
        T delta = 1024;
        T d0 = (delta - 1)*(delta - 2) /2;
        T d1 = delta*(2 - delta);
        T d2 = (delta - 1)*delta /2;
        for(int i = 0; i < N /2; i++){

            init_ashare<T>(ff[2][i], 1024);
            init_ashare<T>(gg[2][i], 1024);
            ff[0][i] = x[2*i];
            ff[1][i] = x[2*i + 1];
            papply<T>(ff[2][i].r, ff[1][i].r, ff[0][i].r, 1024, [](T a, T b) -> T{return 2*a - b;});
            papply<T>(ff[2][i].r_1, ff[1][i].r_1, ff[0][i].r_1, 1024, [](T a, T b) -> T{return 2*a - b;});
            papply<T>(ff[2][i].r_2, ff[1][i].r_2, ff[0][i].r_2, 1024, [](T a, T b) -> T{return 2*a - b;});
            
            gg[0][i] = y[2*i];
            gg[1][i] = y[2*i + 1];
            papply<T>(gg[2][i].r, gg[1][i].r, gg[0][i].r, 1024, [](T a, T b) -> T{return 2*a - b;});
            papply<T>(gg[2][i].r_1, gg[1][i].r_1, gg[0][i].r_1, 1024, [](T a, T b) -> T{return 2*a - b;});
            papply<T>(gg[2][i].r_2, gg[1][i].r_2, gg[0][i].r_2, 1024, [](T a, T b) -> T{return 2*a - b;});
            inter(1-delta, delta, ff[0][i], ff[1][i], x_p[i]);
            inter(1-delta, delta, gg[0][i], gg[1][i], y_p[i]);
            
        }
        init_ashare<T>(h[0], 1024);
        dot1.set_up(ff[0], gg[0], h[0], N/2,true);
        dot1.online(ff[0], gg[0], h[0], N/2);
        // h[1] = z - h[0];
        init_ashare<T>(h[1], 1024);
        papply<T>(h[1].r, z.r, h[0].r, 1024, [](T a, T b) -> T{return a - b;});
        papply<T>(h[1].r_1, z.r_1, h[0].r_1, 1024, [](T a, T b) -> T{return a - b;});
        papply<T>(h[1].r_2, z.r_2, h[0].r_2, 1024, [](T a, T b) -> T{return a - b;});
        
        init_ashare<T>(h[2], 1024);
        dot2.set_up(ff[2], gg[2], h[2], N/2,true);
        dot2.online(ff[2], gg[2], h[2], N/2);

        inter(d0, d1, d2, h[0], h[1], h[2], z_p);
        for(int i = 0; i < N /2; i++){
            // release_ashare<T>(ff[0][i]);
            // release_ashare<T>(ff[1][i]);
            release_ashare<T>(ff[2][i]);
            // release_ashare<T>(gg[0][i]);
            // release_ashare<T>(gg[1][i]);
            release_ashare<T>(gg[2][i]);
        }
        release_ashare<T>(h[0]);
        release_ashare<T>(h[1]);
        release_ashare<T>(h[2]);
        return true;
    }
    private:
    Dot<T> dot1,dot2;
};

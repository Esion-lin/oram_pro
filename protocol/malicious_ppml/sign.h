#pragma once 
#include <vector>
#include "encode.hpp"
#include "config.hpp"
#include "net.hpp"
#include "sharem.h"
#include "newnetshare.h"
#include "unit.h"
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <inttypes.h>
#include <NTL/ZZX.h>
#include "timer.hpp"
#include <omp.h>
using namespace NTL;
#define THREADS 2



template<class T>
class Sign{
    private:
    static const uint32_t LIST_LEN=8*sizeof(T)+1;
    int batch_len;
    Plist<LIST_LEN>* r_x_i = nullptr;
    T* r_z_p = nullptr; 
    uint8_t* delta = nullptr;
    T* gammas = nullptr;
    public:
    
    Sign(int batch):batch_len(batch){
    }
    ~Sign(){
        if(r_x_i != nullptr){
            free(r_x_i);
            r_x_i = nullptr;
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
    void set_up(const std::vector<AShareT<T>>& x, std::vector<AShareT<T>>& output, bool need_gen){


        const size_t WIDTH = output.size();
        r_x_i = (Plist<LIST_LEN>*)malloc(sizeof(Plist<LIST_LEN>)*WIDTH);
        r_z_p = (T*) malloc(WIDTH * sizeof(T));
        size_t max_threads = omp_get_max_threads();
        omp_set_num_threads(std::min(WIDTH, max_threads));
        Timer::record("Round 1");
        //seed 0 1 -> r'1 seed 0 2 -> r'2   r'1 + r'2 = r'
        //but here we generate all of them locally using same seed
        random_T<T>(r_z_p, WIDTH);
        uint32_t shiftsize = sizeof(T)*8 - 1;
        //chops r_x to r_1,...,r_l -> r_x_i
        if(Config::myconfig->check("player0")){
            #pragma omp parallel sections
            {
                #pragma omp section
                {
                    add_T<T>(r_z_p, r_z_p, r_z_p, WIDTH);
                    //chop r_x to r_1,...,r_l,without sign bit
                    #pragma omp parallel for 
                    for(int i = 0; i < WIDTH; i++){
                        chop<T>(x[i].r_1 + x[i].r_2, r_x_i[i].rb);//check
                        r_x_i[i].rb[8*sizeof(T)-1] = 0;
                    }
                    //make share of r_x_i in p = 67
                    uint8_t* temp = (uint8_t*) malloc(WIDTH*(LIST_LEN));
                    Timer::record("communication");
                    ShareCt<uint8_t>((uint8_t*)r_x_i, temp, WIDTH*(LIST_LEN), 67);
                    Timer::stop("communication");
                    free(temp);
                }
                #pragma omp section
                if(need_gen){
                    T* r_1= (T*)malloc(WIDTH*sizeof(T));
                    random_T<T>(r_1, WIDTH);
                    T* r_2= (T*)malloc(WIDTH*sizeof(T));
                    random_T<T>(r_2, WIDTH);
                    #pragma omp parallel for 
                    for(int i = 0; i < WIDTH; i++){
                        output[i].r = r_1[i] + r_2[i];
                        output[i].r_1 = r_1[i];
                        output[i].r_2 = r_2[i];
                    }
                    free(r_1);
                    free(r_2);
                }
            }


        }else{
            #pragma omp parallel sections
            {
            #pragma omp section
            if(need_gen){
                T* rr= (T*)malloc(WIDTH*sizeof(T));
                random_T<T>(rr, WIDTH);
                //generate r_z
                #pragma omp parallel for 
                for(int i = 0; i < WIDTH; i++){
                    output[i].r_1 = rr[i];
                }
                free(rr);
            }
            #pragma omp section
            {            
                delta = (uint8_t*)malloc(WIDTH*sizeof(uint8_t));
                random_T<uint8_t>(delta, WIDTH);
                memset(delta, 0, WIDTH*sizeof(uint8_t));
                random_T<T>(r_z_p, WIDTH);
                gammas = (T*) malloc(WIDTH * sizeof(T));
                // calculate delta + r_z_p - 2 delta r_z_p + r_z
                #pragma omp parallel for 
                for(int i = 0; i < WIDTH; i++){
                    delta[i]%=2;
                    gammas[i] = r_z_p[i] - 2*r_z_p[i]*delta[i] + output[i].r_2;
                }
                if(Config::myconfig->check("player1"))
                {
                    #pragma omp parallel for 
                    for(int i = 0; i < WIDTH; i++){
                            gammas[i]+=delta[i];
                    }
                }
                Timer::record("communication");
                ShareCt<uint8_t>(nullptr, (unsigned char*)r_x_i, WIDTH*(LIST_LEN), 67);
                RevealBt<T>(gammas, WIDTH);
                Timer::stop("communication");
            }
            }



        }
    }
    void online(const std::vector<AShareT<T>>& x, std::vector<AShareT<T>>& output){
        const size_t WIDTH = output.size();
        size_t max_threads = std::min(WIDTH+3, (size_t)omp_get_max_threads());
        omp_set_num_threads(max_threads);
        if(!Config::myconfig->check("player0")){
            Timer::record("online-compute");
            uint32_t shiftsize = sizeof(T)*8 - 1;

            Plist<LIST_LEN>* u_j = (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));

            Plist<LIST_LEN>* m_sigmas = (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));
            Plist<LIST_LEN>* m_j= (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));
            Plist<LIST_LEN>* w = (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));
            //pick all w and w'
            random_T<uint8_t>((uint8_t*)w, WIDTH*(LIST_LEN));
            #pragma omp parallel for schedule(static)
            for(int k = 0; k < WIDTH; k++){
                 //maybe need to set m_sigmas_l and rl to 0
                chop<T>(x[k].r_1, m_sigmas[k].rb);
                auto sign_m=m_sigmas[k].rb[8*sizeof(T)-1];
                m_sigmas[k].rb[8*sizeof(T)-1] = 1;
                uint8_t sum_of_m[LIST_LEN];
                // cal sum of m
                sum_of_m[0] = m_sigmas[k].rb[0];
                for(int i=1;i<LIST_LEN;i++){
                    sum_of_m[i] = (m_j[k].rb[i]+sum_of_m[i-1]) % 67;
                }
                for(int i=0;i<LIST_LEN;i++){
                        uint8_t temp_m=0;
                        // step 3 m_j = (m_sigma + r_x_i - 2*m_sigma*r_x_i) mod p
                        m_j[k].rb[i]=(r_x_i[k].rb[i] + 134 - 2 * m_sigmas[k].rb[i] * r_x_i[k].rb[i] ) % 67;
                        if(Config::myconfig->check("player1"))
                            m_j[k].rb[i] = (m_j[k].rb[i] + 1) % 67;
                        //step 5 m' =(sum_of_m - 2*m_j + 1) mod p;
                        temp_m = (sum_of_m[i] - 2 * m_j[k].rb[i]) % 67;
                        if(Config::myconfig->check("player1"))
                            temp_m = (temp_m + 1) % 67;
                        //step 5 u_j 
                        if(delta[k]!=0)delta[k]=0xff;
                        u_j[k].rb[i] = w[k].rb[i] * temp_m * (1 ^ sign_m ^ m_sigmas[k].rb[i] ^ delta[k]) +
                            w[k].rb[i] * ( sign_m ^ m_sigmas[k].rb[i] ^ delta[k] );
                        u_j[k].rb[i] %= 67;
                    }
                }
                free(m_sigmas);
                free(m_j);
                free(w);
            //round 1
            Timer::stop("Round 1");

            T* backmessage=(T*)malloc(WIDTH*sizeof(T));
            Timer::record("communication");
            RevealCt<uint8_t>((uint8_t*)u_j, WIDTH*(LIST_LEN), 67);
            receiveVector<T>(backmessage, 0, WIDTH);
            Timer::stop("communication");

            Timer::stop("online-compute");

            #pragma omp parallel for
            for(int i = 0; i < WIDTH; i++){
                //r_2 is m_z
                output[i].r_2 = backmessage[i] - 2 * delta[i] * backmessage[i] + gammas[i];
            }
            
            free(u_j);
            free(backmessage);

        }else{
            Plist<LIST_LEN>* u_j = (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));
            T* backmessage=(T*)malloc(WIDTH*sizeof(T));
            
            Timer::record("communication");
            RevealCt<uint8_t>((uint8_t*)u_j, WIDTH*(LIST_LEN), 67);
            Timer::stop("communication");

            #pragma omp parallel for
            for(int i = 0; i < WIDTH; i++){
                //r_z_p is r' here
                auto sign_r_x=((x[i].r_1 + x[i].r_2) >> (sizeof(T) * 8 - 1)) & 1;
                if(check_list(u_j[i].rb, LIST_LEN)){
                    backmessage[i] = -r_z_p[i] - sign_r_x;
                }else{
                    backmessage[i] = -r_z_p[i] -( 1^(-sign_r_x));
                }

            }
            Timer::stop("Round 1");

            Timer::record("communication");
            sendVector<T>(backmessage, 1, WIDTH);
            sendVector<T>(backmessage, 2, WIDTH);
            Timer::stop("communication");

            free(u_j);
            free(backmessage);
        }
    }
    void verify();
    
};
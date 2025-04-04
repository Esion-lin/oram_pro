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
        r_x_i = (Plist<LIST_LEN>*)malloc(sizeof(Plist<LIST_LEN>)*output.size());
        r_z_p = (T*) malloc(output.size() * sizeof(T));

        //seed 0 1 -> r'1 seed 0 2 -> r'2   r'1 + r'2 = r'
        //but here we generate all of them locally using same seed
        random_T<T>(r_z_p, output.size());
        uint32_t shiftsize = sizeof(T)*8 - 1;
        //chops r_x to r_1,...,r_l -> r_x_i
        if(Config::myconfig->check("player0")){
            add_T<T>(r_z_p, r_z_p, r_z_p, output.size());
            //chop r_x to r_1,...,r_l,without sign bit
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                chop<T>(x[i].r_1 + x[i].r_2, r_x_i[i].rb);//check
                r_x_i[i].rb[8*sizeof(T)-1] = 0;
            }
            //make share of r_x_i in p = 67
            uint8_t* temp = (uint8_t*) malloc(output.size()*(LIST_LEN));
            ShareCt<uint8_t>((uint8_t*)r_x_i, temp, output.size()*(LIST_LEN), 67);
            free(temp);

            if(need_gen){
                T* r_1= (T*)malloc(output.size()*sizeof(T));
                random_T<T>(r_1, output.size());
                T* r_2= (T*)malloc(output.size()*sizeof(T));
                random_T<T>(r_2, output.size());
                #pragma omp parallel for
                for(int i = 0; i < output.size(); i++){
                    output[i].r = r_1[i] + r_2[i];
                    output[i].r_1 = r_1[i];
                    output[i].r_2 = r_2[i];
                }
                free(r_1);
                free(r_2);
            }

        }else{
            //for P1/P2 r_z_p represents [r']
            //generate r'1 or r'2
            random_T<T>(r_z_p, output.size());
            
            //pick delta
            delta = (uint8_t*)malloc(output.size()*sizeof(uint8_t));
            random_T<uint8_t>(delta, output.size());
            memset(delta, 0, output.size()*sizeof(uint8_t));


            gammas = (T*) malloc(output.size() * sizeof(T));

            ShareCt<uint8_t>(nullptr, (unsigned char*)r_x_i, output.size()*(LIST_LEN), 67);

            if(need_gen){
                T* rr= (T*)malloc(output.size()*sizeof(T));
                random_T<T>(rr, output.size());
                //generate r_z
                #pragma omp parallel for
                for(int i = 0; i < output.size(); i++){
                    output[i].r_1 = rr[i];
                }
                free(rr);
            }

            // calculate delta + r_z_p - 2 delta r_z_p + r_z
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                delta[i]%=2;
                gammas[i] = r_z_p[i] - 2*r_z_p[i]*delta[i] + output[i].r_2;
                if(Config::myconfig->check("player1"))
                    gammas[i]+=delta[i];
            }
            RevealBt<T>(gammas, output.size());
        }
        printf("setup done\n");
    }
    void online(const std::vector<AShareT<T>>& x, std::vector<AShareT<T>>& output){
        if(!Config::myconfig->check("player0")){
            Timer::record("online-compute");
            const size_t WIDTH = output.size();
            uint32_t shiftsize = sizeof(T)*8 - 1;

            Plist<LIST_LEN>* u_j = (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));
            if(Config::myconfig->check("player1")){
            Plist<LIST_LEN>* m_sigmas = (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));
            Plist<LIST_LEN>* m_j= (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));
            Plist<LIST_LEN>* w = (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));
            //pick all w and w'
            random_T<uint8_t>((uint8_t*)w, WIDTH*(LIST_LEN));

            #pragma omp parallel for
            for(int k = 0; k < output.size(); k++){

                //maybe need to set m_sigmas_l and rl to 0
                chop<T>(x[k].r_1, m_sigmas[k].rb);
                auto sign_m=m_sigmas[k].rb[8*sizeof(T)-1];
                m_sigmas[k].rb[8*sizeof(T)-1] = 1;
                uint8_t sum_of_m = 0;
                #pragma omp parallel for
                for(int i=0;i<LIST_LEN;i++){
                    uint8_t temp_m=0;
                    // step 3 m_j = (m_sigma + r_x_i - 2*m_sigma*r_x_i) mod p
                    m_j[k].rb[i]=(r_x_i[k].rb[i] + 134 - 2 * m_sigmas[k].rb[i] * r_x_i[k].rb[i] ) % 67;
                    if(Config::myconfig->check("player1"))
                        m_j[k].rb[i] =( m_j[k].rb[i] + m_sigmas[k].rb[i] ) % 67;
                    // cal sum of m
                    sum_of_m += (m_j[k].rb[i]) % 67;
                    //step 5 m' =(sum_of_m - 2*m_j + 1) mod p;
                    temp_m = (sum_of_m - 2* m_j[k].rb[i]) % 67;
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

            }
            printf("online connect\n");
            //round 1
            Timer::record("communication");
            RevealCt<uint8_t>((uint8_t*)u_j, output.size()*(LIST_LEN), 67);
            T* backmessage=(T*)malloc(output.size()*sizeof(T));
            receiveVector<T>(backmessage, 0, output.size());

            Timer::stop("communication");
            Timer::stop("online-compute");

            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                //r_2 is m_z
                output[i].r_2 = backmessage[i] - 2 * delta[i] * backmessage[i] + gammas[i];
            }
            
            free(u_j);
            free(backmessage);

        }else{
            Plist<LIST_LEN>* u_j = (Plist<LIST_LEN>*)malloc(output.size()*(LIST_LEN));
            T* backmessage=(T*)malloc(output.size()*sizeof(T));
            RevealCt<uint8_t>((uint8_t*)u_j, output.size()*(LIST_LEN), 67);

            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                //r_z_p is r' here
                auto sign_r_x=((x[i].r_1 + x[i].r_2) >> (sizeof(T) * 8 - 1)) & 1;
                if(check_list(u_j[i].rb, LIST_LEN)){
                    backmessage[i] = -r_z_p[i] - sign_r_x;
                }else{
                    backmessage[i] = -r_z_p[i] -( 1^(-sign_r_x));
                }

            }
            //round 1
            thread *threads = new thread[2];

            threads[0] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)backmessage, 1, output.size());
            threads[1] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)backmessage, 2, output.size());
            
            Timer::record("communication");
            for (int i = 0; i < 2; i++)
                threads[i].join();
            Timer::stop("communication");

            delete[] threads;
            free(u_j);
            free(backmessage);
        }
    }
    void verify();
    
};
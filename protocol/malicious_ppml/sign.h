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
    Plist<LIST_LEN>* rls = nullptr;
    T* r_z_p = nullptr; 
    uint8_t* delta = nullptr;
    T* gammas = nullptr;
    public:
    
    Sign(int batch):batch_len(batch){
    }
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
        void set_up(const std::vector<AShareT<T>>& x, std::vector<AShareT<T>>& output, bool need_gen){
        rls = (Plist<LIST_LEN>*)malloc(sizeof(Plist<LIST_LEN>)*output.size());
        r_z_p = (T*) malloc(output.size() * sizeof(T));
        uint32_t shiftsize = sizeof(T)*8 - 1;
        //chops r_x to r_1,...,r_l -> rls
        if(Config::myconfig->check("player0")){
            
            //chop r_x to r_1,...,r_l,without sign bit
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                //should use x.r?
                chop<T>(x[i].r, rls[i].rb);
                rls[i].rb[8*sizeof(T)] = 1;
            }
            //make share of rls in p = 67
            uint8_t* temp = (uint8_t*) malloc(output.size()*(LIST_LEN));
            ShareCt<uint8_t>((uint8_t*)rls, temp, output.size()*(LIST_LEN), 67);
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
            // is temp_r nessary?
            T* temp_r= (T*)malloc(output.size()*sizeof(T));
            random_T<T>(temp_r, output.size());
            random_T<T>(r_z_p, output.size());
            add_T<T>(r_z_p, r_z_p, temp_r, output.size());
            free(temp_r);
        }else{
            //maybe we can use a single delta value for a batch
            //pick delta
            delta = (uint8_t*)malloc(output.size()*sizeof(uint8_t));
            random_T<T>(r_z_p, output.size());

            gammas = (T*) malloc(output.size() * sizeof(T));
            random_T<uint8_t>(delta, output.size());

            ShareCt<uint8_t>(nullptr, (unsigned char*)rls, output.size()*(LIST_LEN), 67);

            //idk what this is
            if(need_gen){
                T* rr= (T*)malloc(output.size()*sizeof(T));
                random_T<T>(rr, output.size());
                #pragma omp parallel for
                for(int i = 0; i < output.size(); i++){
                    output[i].r_2 = rr[i];
                }
                free(rr);
            }

            // calculate delta + r_z_p - 2 delta r_z_p + r_z
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                if(delta[i]%2==0)delta[i]=0;
                else delta[i]=0xffff;
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

            Plist<LIST_LEN>* u_j = (Plist<LIST_LEN>*)malloc(2*WIDTH*(LIST_LEN));
            if(Config::myconfig->check("player1")){
            Plist<LIST_LEN>* m_sigmas = (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));
            Plist<LIST_LEN>* m_j= (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));
            Plist<LIST_LEN>* w = (Plist<LIST_LEN>*)malloc(WIDTH*(LIST_LEN));
            //why uint_8?
            //pick all w and w'
            random_T<uint8_t>((uint8_t*)w, WIDTH*(LIST_LEN));

            #pragma omp parallel for
            for(int k = 0; k < output.size(); k++){

                //maybe need to set m_sigmas_l and rl to 0
                chop<T>(x[k].r_1, m_sigmas[k].rb);
                auto sign_m=m_sigmas[k].rb[8*sizeof(T)];
                m_sigmas[k].rb[8*sizeof(T)] = 0;
                uint8_t sum_of_m = 0;
                #pragma omp parallel for
                for(int i=0;i<LIST_LEN;i++){
                    // step 3 m_j = (m_sigma + rls - 2*m_sigma*rls) mod p
                    m_j[k].rb[i]=(rls[k].rb[i] + 67 - 2 * m_sigmas[k].rb[i] * rls[k].rb[i] ) % 67;
                    //idk what this is
                    if(Config::myconfig->check("player1"))
                        m_j[k].rb[i] =( m_j[k].rb[i] + m_sigmas[k].rb[i] ) % 67;
                    // cal sum of m
                    sum_of_m += (m_j[k].rb[i]) % 67;
                    //step 5 m' =(sum_of_m - 2*m_j + 1) mod p;
                    m_j[k].rb[i] = (sum_of_m - 2* m_j[k].rb[i] + 1) % 67;
                    //step 5 u_j 
                    u_j[k].rb[i] = w[k].rb[i] * m_j[k].rb[i] * (1 ^ sign_m ^ m_sigmas[k].rb[i] ^ delta[k]) +
                        w[k].rb[i] * ( sign_m ^ m_sigmas[k].rb[i] ^ delta[k] );
                    u_j[k].rb[i] %= 67;
                }
                

            }
            free(m_sigmas);
            free(m_j);
            free(w);

            }
            printf("online connect\n");
            Timer::stop("online-compute");

            Timer::record("communication");
            RevealCt<uint8_t>((uint8_t*)u_j, 2*output.size()*(6*sizeof(T)+1), 67);
            T* backmessage=(T*)malloc(output.size()*sizeof(T));
            receiveVector<T>(backmessage, 0, output.size());
            Timer::stop("communication");

            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                output[i].r_1 = backmessage[i] - 2 * delta[i] * backmessage[i] + gammas[i];
            }
            
            free(u_j);
            free(backmessage);

        }else{
            Plist<LIST_LEN>* u_j = (Plist<LIST_LEN>*)malloc(2*output.size()*(LIST_LEN));
            T* backmessage=(T*)malloc(output.size()*sizeof(T));
            RevealCt<uint8_t>((uint8_t*)u_j, 2*output.size()*(6*sizeof(T)+1), 67);
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                //what is r_z_p?
                auto sign_r_x=(x[i].r >> (sizeof(T) * 8 - 1)) & 1;
                if(check_list(u_j[i].rb, u_j[i + output.size()].rb, LIST_LEN)){
                    backmessage[i] = -r_z_p[i] - sign_r_x;
                }else{
                    backmessage[i] = -r_z_p[i] -( 1^(-sign_r_x));
                }

            }

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
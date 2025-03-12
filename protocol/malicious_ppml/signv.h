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
#define mackey 4
template<class T>
class Signv{
    private:
    int batch_len;
    Pvlist<8*sizeof(T)+1, mackey>* rls = nullptr;
    T* r_z_p = nullptr; 
    uint8_t* delta = nullptr;
    T* gammas = nullptr;
    int8_t macs_key[mackey];
    public:
    
    Signv(int batch):batch_len(batch){
    }
    ~Signv(){
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
        rls = (Pvlist<8*sizeof(T)+1, mackey>*)malloc(sizeof(Pvlist<8*sizeof(T)+1, mackey>)*output.size());
        r_z_p = (T*) malloc(output.size() * sizeof(T));
        
        //chops r_x to r_1,...,r_l
        if(Config::myconfig->check("player0")){
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                chop<T>(x[i].r, rls[i].rb);
                rls[i].rb[8*sizeof(T)] = 1;
                memcpy(rls[i].mac[8*sizeof(T)], macs_key, mackey);
                
                
            }
            uint8_t* temp = (uint8_t*) malloc(output.size()*(8*sizeof(T)+1)*(mackey + 1));
            //uint8_t* temp = new uint8_t(output.size()*(8*sizeof(T)+1));
            // printf("123123 %d\n", output.size()*(8*sizeof(T)+1));
            ShareCt<uint8_t>((uint8_t*)rls, temp, output.size()*(8*sizeof(T)+1)*(mackey + 1), 67);
            //printf("123123\n");
            if(need_gen){
                std::vector<T> r_1(output.size());
                std::vector<T> r_2(output.size());
                #pragma omp parallel for
                for(int i = 0; i < output.size(); i++){
                    output[i].r = r_1[i] + r_2[i];
                    output[i].r_1 = r_1[i];
                    output[i].r_2 = r_2[i];
                }
            }
            free(temp);
            T temp_r[output.size()];
            random_T<T>(temp_r, output.size());
            random_T<T>(r_z_p, output.size());
            add_T<T>(r_z_p, r_z_p, temp_r, output.size());
        }else{
            delta = (uint8_t*)malloc(output.size()*sizeof(uint8_t));
            gammas = (T*) malloc(output.size() * sizeof(T));
            
            
            ShareCt<uint8_t>(nullptr, (unsigned char*)rls, output.size()*(8*sizeof(T)+1)*(mackey + 1), 67);
            T rr[output.size()];
            if(need_gen){
                random_T<T>(rr, output.size());
                #pragma omp parallel for
                for(int i = 0; i < output.size(); i++){
                    output[i].r_2 = rr[i];
                }
            }
            random_T<T>(r_z_p, output.size());
            //pick delta
            random_T<uint8_t>(delta, output.size());
            // calculate delta + r_z_p - 2 delta r_z_p + r_z
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                delta[i] %= 2;
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
            // Plist<8*sizeof(T)+1> m_sigmas[output.size()],m_j[output.size()],w_j[output.size()],w_j_p[output.size()],u_j[output.size()],v_j[output.size()],temp[output.size()];
            Timer::record("online-compute");
            Plist<8*sizeof(T)+1>* m_sigmas = (Plist<8*sizeof(T)+1>*)malloc(output.size()*(8*sizeof(T)+1));
            Pvlist<8*sizeof(T)+1, mackey>* m_j= (Pvlist<8*sizeof(T)+1, mackey>*)malloc(output.size()*(8*sizeof(T)+1)*(mackey + 1));
            Plist<8*sizeof(T)+1>* w_j = (Plist<8*sizeof(T)+1>*)malloc(output.size()*(8*sizeof(T)+1));
            Plist<8*sizeof(T)+1>* w_j_p = (Plist<8*sizeof(T)+1>*)malloc(output.size()*(8*sizeof(T)+1));
            Pvlist<8*sizeof(T)+1, mackey>* u_j = (Pvlist<8*sizeof(T)+1, mackey>*)malloc(2*output.size()*(8*sizeof(T)+1)*(mackey + 1));
            // Plist<8*sizeof(T)+1>* v_j = (Plist<8*sizeof(T)+1>*)malloc(output.size()*(8*sizeof(T)+1));
            Pvlist<8*sizeof(T)+1, mackey>* temp = (Pvlist<8*sizeof(T)+1, mackey>*)malloc(output.size()*(8*sizeof(T)+1)*(mackey + 1));
            int s_len = output.size();
            uint32_t shiftsize = sizeof(T)*8 - 1;
            random_T<uint8_t>((uint8_t*)w_j, output.size()*(8*sizeof(T)+1));
            random_T<uint8_t>((uint8_t*)w_j_p, output.size()*(8*sizeof(T)+1));
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                //pick w_j, w'_j
                
                chop<T>(x[i].r_1 - (1<<shiftsize) + 1, m_sigmas[i].rb);
                
                m_sigmas[i].rb[8*sizeof(T)] = 0;
                //m_j = m_sigma + rls - 2*m_sigma*rls
                uint8_t sum_of_m = 0, sum_mac[mackey], temp_mac[mackey];
                #pragma omp parallel for
                for(int j = 0; j < sizeof(T)*8+1; j++){
                    uint8_t temp;//mj'
                    m_j[i].rb[j] = (rls[i].rb[j] + 67 - 2*m_sigmas[i].rb[j] *rls[i].rb[j])%67;
                    for(int k = 0; k < mackey; k++)
                        m_j[i].mac[j][k] = (rls[i].mac[j][k] + 67 - 2*m_sigmas[i].rb[j] *rls[i].mac[j][k])%67;
                    if(Config::myconfig->check("player1"))
                        m_j[i].rb[j] = (m_j[i].rb[j] + m_sigmas[i].rb[j]) %67;
                    
                    // for(int j = 0; j < 8*sizeof(T); j ++){
                    //     printf("{%u}", (uint32_t)m_j[i].rb[j]);
                    // }
                    // printf("\n");
                    //sum_of m_j - 2m_j + 1 
                    sum_of_m = (sum_of_m+m_j[i].rb[j])%67;
                    for(int k = 0; k < mackey; k++){
                        sum_mac[k] = (sum_mac[k] + m_j[i].mac[j][k]) % 67;
                        temp_mac[k] = (sum_mac[k] + 134 - 2*m_j[i].mac[j][k]) % 67;
                    }
                        
                    temp = ((uint32_t)sum_of_m + 134 - 2*m_j[i].rb[j]) % 67;
                    if(Config::myconfig->check("player1"))
                        temp += 1;
                    w_j[i].rb[j] = (w_j[i].rb[j] % 66)+1;
                    w_j_p[i].rb[j] = (w_j[i].rb[j] % 66)+1;
                    
                    if(Config::myconfig->check("player1")){
                        u_j[i].rb[j] = (((uint32_t)temp)*w_j[i].rb[j]) % 67;
                        for(int k = 0; k < mackey; k++){
                            u_j[i].mac[j][k] = (((uint32_t)temp_mac[k])*w_j[i].rb[j] + (1^delta[i]^m_sigmas[i].rb[j])*macs_key[k]) % 67;
                        }
                        u_j[i + s_len].rb[j] = (((((uint32_t)temp)*w_j[i].rb[j]) + 1)*w_j_p[i].rb[j]) % 67;
                        for(int k = 0; k < mackey; k++){
                            u_j[i + s_len].mac[j][k] = (((((uint32_t)temp_mac[k])*w_j[i].rb[j]) + macs_key[k])*w_j_p[i].rb[j]) % 67;
                        }
                    }else{
                        u_j[i].rb[j] = (((uint32_t)temp)*w_j[i].rb[j] + (1^delta[i]^m_sigmas[i].rb[j])) % 67;
                        for(int k = 0; k < mackey; k++){
                            u_j[i].mac[j][k] = (((uint32_t)temp_mac[k])*w_j[i].rb[j] + (1^delta[i]^m_sigmas[i].rb[j])*macs_key[k]) % 67;
                        }
                        u_j[i + s_len].rb[j] = (((uint32_t)temp)*w_j[i].rb[j]*w_j_p[i].rb[j]) % 67;
                        for(int k = 0; k < mackey; k++){
                            u_j[i + s_len].mac[j][k] = (((((uint32_t)temp_mac[k])*w_j[i].rb[j]) + macs_key[k])*w_j_p[i].rb[j]) % 67;
                        }
                    }
                }
                // for(int j = 0; j < 8*sizeof(T)+1; j ++){
                //         printf("{%03u}", (uint32_t) temp[i].rb[j]);
                //     }
                //reveal to P_0
                // RevealC<uint8_t>(m_j[i].rb, 8*sizeof(T)+1, 67);
                

            }
            printf("online connect\n");
            Timer::stop("online-compute");
            RevealCt<uint8_t>((uint8_t*)u_j, 2*output.size()*(6*sizeof(T)+1)*(mackey + 1), 67);
            // RevealC<uint8_t>((uint8_t*)v_j, output.size()*(8*sizeof(T)+1), 67);
            T backmessage[output.size()];
            receiveVector<T>(backmessage, 0, output.size());
            // P2Pchannel::mychnl->recv_data_from("player0", backmessage, sizeof(T)*output.size());
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                output[i].r_1 = backmessage[i] - 2 * delta[i] * backmessage[i] + gammas[i];
            }
            free(m_sigmas);
            free(m_j);
            free(w_j);
            free(w_j_p);
            free(u_j);
            // free(v_j);
            free(temp);
            

        }else{
            Pvlist<8*sizeof(T)+1, mackey>* u_j = (Pvlist<8*sizeof(T)+1, mackey>*)malloc(2*output.size()*(8*sizeof(T)+1)*(mackey+1));
            // Plist<8*sizeof(T)+1>* v_j = (Plist<8*sizeof(T)+1>*)malloc(output.size()*(8*sizeof(T)+1));
            T backmessage[output.size()];
            RevealCt<uint8_t>((uint8_t*)u_j, 2*output.size()*(6*sizeof(T)+1)*(mackey + 1), 67);
            //TODO mac verify
            // RevealC<uint8_t>((uint8_t*)v_j, output.size()*(8*sizeof(T)+1), 67);
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                
                if(check_list(u_j[i].rb, u_j[i + output.size()].rb, 8*sizeof(T)+1)){
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
            thread *threads = new thread[2];

            threads[0] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)backmessage, 1, output.size());
            threads[1] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)backmessage, 2, output.size());

            for (int i = 0; i < 2; i++)
                threads[i].join();

            delete[] threads;
            
            // P2Pchannel::mychnl->send_data_to("player1", backmessage, sizeof(T)*output.size());
            // P2Pchannel::mychnl->send_data_to("player2", backmessage, sizeof(T)*output.size());
            free(u_j);
            // free(v_j);
        }
    }
    void verify();
    
};
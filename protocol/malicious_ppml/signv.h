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
const int P=67;
#define LENGTH 8*sizeof(T)+1
#define LAMBDA 6

template<class T>
class Signv{
    private:
    int batch_len;
    Pvlist<LENGTH, LENGTH>* r_x_i = nullptr;
    Pvlist<LENGTH, LENGTH>* r_x_i_l = nullptr;
    T* rl = nullptr;
    T* rll = nullptr;
    T* r_z = nullptr;
    T* r_z_l = nullptr;
    AShareT<T>* d_1_share = nullptr;
    AShareT<T>* d_2_share = nullptr;
    AShareT<T>* c_0_share = nullptr;
    AShareT<T>* c_2_share = nullptr;
    T* r_x_share = nullptr;
    T* r_x_l_share = nullptr;
    AShareT<T>* r_x_share_sigmas = nullptr;
    AShareT<T>* r_x_l_share_sigmas = nullptr;

    AShareMAC<T,LAMBDA>* r_x_l_share_mac = nullptr;
    AShareMAC<T,LAMBDA>* r_x_share_mac = nullptr;

    uint8_t* delta = nullptr;
    uint8_t* delta_l = nullptr;
    T* gammas = nullptr;
    T* gammas_l = nullptr;
    uint8_t alpha[LAMBDA];
    uint8_t alpha_l[LAMBDA];

    public:
    
    Signv(int batch):batch_len(batch){
    }
    ~Signv(){
        if(r_x_i != nullptr){
            free(r_x_i);
            r_x_i = nullptr;
        }
        if(r_x_i_l != nullptr){
            free(r_x_i_l);
            r_x_i_l = nullptr;
        }
        if(rl != nullptr){
            free(rl);
            rl = nullptr;
        }
        if(rll != nullptr){
            free(rll);
            rll = nullptr;
        }
        if(r_z != nullptr){
            free(r_z);
            r_z = nullptr;
        }
        if(r_z_l != nullptr){
            free(r_z_l);
            r_z_l = nullptr;
        }
        if(d_1_share != nullptr){
            free(d_1_share);
            d_1_share = nullptr;
        }
        if(d_2_share != nullptr){
            free(d_2_share);
            d_2_share = nullptr;
        }
        if(c_0_share != nullptr){
            free(c_0_share);
            c_0_share = nullptr;
        }
        if(c_2_share != nullptr){
            free(c_2_share);
            c_2_share = nullptr;
        }
        if(r_x_share != nullptr){
            free(r_x_share);
            r_x_share = nullptr;
        }
        if(r_x_l_share != nullptr){
            free(r_x_l_share);
            r_x_l_share = nullptr;
        }
        if(r_x_share_sigmas != nullptr){
            free(r_x_share_sigmas);
            r_x_share_sigmas = nullptr;
        }
        if(r_x_l_share_sigmas != nullptr){
            free(r_x_l_share_sigmas);
            r_x_l_share_sigmas = nullptr;
        }
        if(r_x_share_mac != nullptr){
            free(r_x_share_mac);
            r_x_share_mac = nullptr;
        }
        if(r_x_l_share_mac != nullptr){
            free(r_x_l_share_mac);
            r_x_l_share_mac = nullptr;
        }
        if(delta != nullptr){
            free(delta);
            delta = nullptr;
        }
        if(delta_l != nullptr){
            free(delta_l);
            delta_l = nullptr;
        }
        if(gammas != nullptr){
            free(gammas);
            gammas = nullptr;
        }
        if(gammas_l != nullptr){
            free(gammas_l);
            gammas_l = nullptr;
        }


        
    }
    void set_up(const std::vector<AShareT<T>>& x, std::vector<AShareT<T>>& output, bool need_gen){
        const size_t WIDTH = output.size();
        omp_set_num_threads(std::min(WIDTH+3, (size_t)omp_get_max_threads()/3));
        r_x_i = (Pvlist<LENGTH, LENGTH>*)malloc(sizeof(Pvlist<LENGTH, LENGTH>)*output.size());

        rl = (T*)malloc(sizeof(T)*WIDTH);
        rll = (T*)malloc(sizeof(T)*WIDTH);
        r_z = (T*)malloc(sizeof(T)*WIDTH);
        r_z_l = (T*)malloc(sizeof(T)*WIDTH);
        r_x_share = (T*)malloc(sizeof(T)*WIDTH);
        r_x_share_sigmas = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        r_x_l_share_sigmas = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        r_x_l_share = (T*)malloc(sizeof(T)*WIDTH);
        d_1_share = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        d_2_share = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        c_0_share = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        c_2_share = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);

        r_x_share_mac = (AShareMAC<T,LAMBDA>*)malloc(sizeof(AShareMAC<T,LAMBDA>)*WIDTH);
        r_x_l_share_mac = (AShareMAC<T,LAMBDA>*)malloc(sizeof(AShareMAC<T,LAMBDA>)*WIDTH);

        r_x_i=(Pvlist<LENGTH, LENGTH>*)malloc(sizeof(Pvlist<LENGTH, LENGTH>)*WIDTH);
        r_x_i_l=(Pvlist<LENGTH, LENGTH>*)malloc(sizeof(Pvlist<LENGTH, LENGTH>)*WIDTH);

        random_T<T>(rl, WIDTH);
        random_T<T>(rll, WIDTH);
        random_T<T>(r_z, WIDTH);
        random_T<T>(r_z_l, WIDTH);
        int8_t* d_1=(int8_t*)malloc(sizeof(int8_t)*WIDTH);
        int8_t* d_2=(int8_t*)malloc(sizeof(int8_t)*WIDTH);
        int8_t* c_0=(int8_t*)malloc(sizeof(int8_t)*WIDTH);
        int8_t* c_2=(int8_t*)malloc(sizeof(int8_t)*WIDTH);
        random_T<int8_t>(d_1, WIDTH);
        random_T<int8_t>(d_2, WIDTH);
        random_T<int8_t>(c_0, WIDTH);
        random_T<int8_t>(c_2, WIDTH);
        if(Config::myconfig->check("player0")){
                uint8_t alpha_share[LAMBDA];
                random_T<uint8_t>(alpha, LAMBDA);
                random_T<uint8_t>(alpha_share, LAMBDA);

                sendVector<uint8_t>(alpha, 1, LAMBDA);
                sendVector<uint8_t>(alpha_share, 2, LAMBDA);

                add_T<uint8_t>(alpha, alpha_share, alpha, LAMBDA);

                uint8_t alpha_l_share[LAMBDA];
                random_T<uint8_t>(alpha_l, LAMBDA);
                random_T<uint8_t>(alpha_l_share, LAMBDA);

                sendVector<uint8_t>(alpha_l, 1, LAMBDA);
                sendVector<uint8_t>(alpha_l_share, 2, LAMBDA);

                add_T<uint8_t>(alpha_l, alpha_l_share, alpha_l, LAMBDA);
        }else{
                receiveVector<uint8_t>(alpha, 0, LAMBDA);
                receiveVector<uint8_t>(alpha_l, 0, LAMBDA);
        }
       if(!Config::myconfig->check("player0")){//P1 P2 generates delta and gamma
        delta = (uint8_t*)malloc(WIDTH*sizeof(uint8_t));
        random_T<uint8_t>(delta, output.size());
        gammas = (T*) malloc(output.size() * sizeof(T));
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                delta[i]%=2;
                gammas[i] = rl[i]- 2*rl[i]*delta[i] + r_z[i];
                if(Config::myconfig->check("player1"))
                    gammas[i]+=delta[i];
            }
       }
       if(!Config::myconfig->check("player1")){//P0 P2 generates delta' and gamma'
        delta_l = (uint8_t*)malloc(output.size()*sizeof(uint8_t));
        random_T<uint8_t>(delta_l, output.size());
        gammas_l = (T*) malloc(output.size() * sizeof(T));
            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                delta_l[i]%=2;
                gammas_l[i] = rll[i] - 2*rll[i]*delta_l[i] + r_z_l[i];
                if(Config::myconfig->check("player0"))
                    gammas_l[i]+=delta_l[i];
            }
        }
        if(!Config::myconfig->check("player0")){
            #pragma omp parallel for
            for(int j = 0; j < output.size(); j++){
                d_1_share[j].r_1 =d_1[j]%2;
                d_1_share[j].r_2 = 0;
                d_2_share[j].r_1 =0;
                d_2_share[j].r_2 =d_2[j]%2;
                c_0_share[j].r_1 =0;
                c_0_share[j].r_2 =0;
                c_2_share[j].r_1 =0;
                c_2_share[j].r_2 =c_2[j]%2;
            }
       }
        else if(!Config::myconfig->check("player1")){
        #pragma omp parallel for
        for(int j = 0; j < output.size(); j++){
            d_1_share[j].r_1 = 0;
            d_1_share[j].r_2 = 0;
            d_2_share[j].r_1 = 0;
            d_2_share[j].r_2 = d_2[j]%2;
            c_0_share[j].r_1 = c_0[j]%2;
            c_0_share[j].r_2 = 0;
            c_2_share[j].r_1 = 0;
            c_2_share[j].r_2 = c_2[j]%2;
        }
        }
        else if(!Config::myconfig->check("player2")){
            #pragma omp parallel for
            for(int j = 0; j < output.size(); j++){
                d_1_share[j].r_1 = 0;
                d_1_share[j].r_2 = d_1[j]%2;
                d_2_share[j].r_1 =0;
                d_2_share[j].r_2 =0;
                c_0_share[j].r_1 = c_0[j]%2;
                c_0_share[j].r_2 = 0;
                c_2_share[j].r_1 = 0;
                c_2_share[j].r_2 = 0;
            }
       }
        //calculate r_x_i_sigmas r_x r_x_l
        //call F_mutiverify and F_innerVerify
        if(Config::myconfig->check("player0")){
            #pragma omp parallel for
            for(int i=0;i<WIDTH;i++){
                    for(int j=0;j<LENGTH-1;j++){
                    auto r_rx_l=(1<<LENGTH-1);
                    auto m_rx_l=(1<<LENGTH-1);
                    
                    r_rx_l -= (c_0_share[j].r_1 ^ c_2_share[j].r_1) * (1<<j);
                    r_rx_l += (c_0_share[j].r_1 ^ c_2_share[j].r_1) * (1<<LENGTH-1);

                    m_rx_l -= (c_0_share[j].r_2 ^ c_2_share[j].r_2) * (1<<j);
                    m_rx_l += (c_0_share[j].r_2 ^ c_2_share[j].r_2) * (1<<LENGTH-1);

                    r_x_l_share[i]=r_rx_l+m_rx_l;


                    r_x_share_sigmas[i].r_1 = d_1_share[j].r_1 ^ d_2_share[j].r_1;
                    r_x_share_sigmas[i].r_2 = d_1_share[j].r_2 ^ d_2_share[j].r_2;

                    r_x_l_share_sigmas[i].r_1 = c_0_share[j].r_1 ^ c_2_share[j].r_1;
                    r_x_l_share_sigmas[i].r_2 = c_0_share[j].r_2 ^ c_2_share[j].r_2;
                }
            }
        }
        else if(Config::myconfig->check("player1")){
            #pragma omp parallel for
            for(int i=0;i<WIDTH;i++){
                for(int j=0;j<LENGTH-1;j++){
                    auto r_rx=(1<<LENGTH-1);
                    auto m_rx=(1<<LENGTH-1);
                    r_rx -= (d_1_share[j].r_1 ^ d_2_share[j].r_1) * (1<<j);
                    r_rx += (d_1_share[j].r_1 ^ d_2_share[j].r_1) * (1<<LENGTH-1);

                    m_rx -= (d_1_share[j].r_2 ^ d_2_share[j].r_2) * (1<<j);
                    m_rx += (d_1_share[j].r_2 ^ d_2_share[j].r_2) * (1<<LENGTH-1);

                    r_x_share[i]=r_rx+m_rx;


                    r_x_share_sigmas[i].r_1 = d_1_share[j].r_1 ^ d_2_share[j].r_1;
                    r_x_share_sigmas[i].r_2 = d_1_share[j].r_2 ^ d_2_share[j].r_2;

                    r_x_l_share_sigmas[i].r_1 = c_0_share[j].r_1 ^ c_2_share[j].r_1;
                    r_x_l_share_sigmas[i].r_2 = c_0_share[j].r_2 ^ c_2_share[j].r_2;

                }
            } 
        }else if(Config::myconfig->check("player2")){
            #pragma omp parallel for
            for(int i=0;i<WIDTH;i++){
                for(int j=0;j<LENGTH-1;j++){
                    auto r_rx=(1<<LENGTH-1);
                    auto r_rx_l=(1<<LENGTH-1);
                    r_rx -= (d_1_share[j].r_1 ^ d_2_share[j].r_1) * (1<<j);
                    r_rx += (d_1_share[j].r_1 ^ d_2_share[j].r_1) * (1<<LENGTH-1);

                    r_rx_l -= (c_0_share[j].r_1 ^ c_2_share[j].r_1) * (1<<j);
                    r_rx_l += (c_0_share[j].r_1 ^ c_2_share[j].r_1) * (1<<LENGTH-1);

                    r_x_share[i]=r_rx;
                    r_x_l_share[i]=r_rx_l;


                    r_x_share_sigmas[i].r_1 = d_1_share[j].r_1 ^ d_2_share[j].r_1;
                    r_x_share_sigmas[i].r_2 = d_1_share[j].r_2 ^ d_2_share[j].r_2;

                    r_x_l_share_sigmas[i].r_1 = c_0_share[j].r_1 ^ c_2_share[j].r_1;
                    r_x_l_share_sigmas[i].r_2 = c_0_share[j].r_2 ^ c_2_share[j].r_2;
                }
            }
        }
        //caculate r_x_i and r_x_i_l
        if(!Config::myconfig->check("player0")){
            #pragma omp parallel for
            for(int i=0;i<WIDTH;i++){
                r_x_l_share[i]=r_x_l_share[i] % P;
                r_x_share[i]=r_x_share[i] % P;
                Plist<LENGTH> temp,temp_l;
                chop<T>(r_x_share[i], temp.rb);
                chop<T>(r_x_l_share[i], temp_l.rb);
                for(int l=0;l<LAMBDA;l++){
                    for(int j=0;j<LENGTH;j++){
                        r_x_share_mac[i].r_mac[l][j]=(temp.rb[j] * alpha[l]) % P;
                        r_x_l_share_mac[i].r_mac[l][j]=(temp_l.rb[j] * alpha_l[l]) % P;
                    }

                }
            }
        }
        free(d_1);
        free(d_2);
        free(c_0);
        free(c_2);
        printf("setup complete\n");

   }
    void online(const std::vector<AShareT<T>>& x, std::vector<AShareT<T>>& output) {
        const size_t WIDTH = output.size();

        // Allocate memory for arrays with proper size                                                                                                                  
        T* m_x_l = (T*)malloc(sizeof(T) * WIDTH);
        T* m_z = (T*)malloc(sizeof(T) * WIDTH);
        T* m_z_l = (T*)malloc(sizeof(T) * WIDTH);
        T* m_l = (T*)malloc(sizeof(T) * WIDTH); // Ensure correct size allocation
        T* m_ll=(T*)malloc(output.size()*sizeof(T));
        T* small_delta = (T*)malloc(sizeof(T) * WIDTH);
        T* send_buffer = (T*)malloc(sizeof(T) * WIDTH); // 用于批量发送数据
        uint32_t shiftsize = sizeof(T) * 8 - 1;
        Plist<LENGTH>* u_j = (Plist<LENGTH>*)malloc(WIDTH*(sizeof(Plist<LENGTH>)));
        AShareMAC<T,LAMBDA>* u_j_mac=(AShareMAC<T,LAMBDA>*)malloc(WIDTH*(sizeof(AShareMAC<T,LAMBDA>)));
        Plist<LENGTH>* m_j = (Plist<LENGTH>*)malloc(WIDTH*(sizeof(Plist<LENGTH>)));
        AShareMAC<T,LAMBDA>* m_j_mac=(AShareMAC<T,LAMBDA>*)malloc(WIDTH*(sizeof(AShareMAC<T,LAMBDA>)));
        Plist<LENGTH>* m_sigmas = (Plist<LENGTH>*)malloc(WIDTH*(sizeof(Plist<LENGTH>)));
        Plist<LENGTH>* r_x_share_i = (Plist<LENGTH>*)malloc(WIDTH*(sizeof(Plist<LENGTH>)));
        Plist<LENGTH>* r_x_share_l_i = (Plist<LENGTH>*)malloc(WIDTH*(sizeof(Plist<LENGTH>)));
        Plist<LENGTH>* w = (Plist<LENGTH>*)malloc(WIDTH*(sizeof(Plist<LENGTH>)));

        if (!m_x_l || !m_z || !m_z_l || !m_l || !small_delta || !send_buffer) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        // Calculate m_x_l
        if (!Config::myconfig->check("player0")) { // P1 P2
            #pragma omp parallel for
            for (int i = 0; i < WIDTH; i++) {
                small_delta[i] = x[i].r_1 - r_x_l_share[i];
            }
            sendVector<T>(small_delta, 0, WIDTH);
        } else {
            receiveVector<T>(small_delta, 1, WIDTH);
            receiveVector<T>(small_delta, 2, WIDTH);

            #pragma omp parallel for
            for (int i = 0; i < WIDTH; i++) {
                m_x_l[i] = r_x_share[i] + r_x_share[i] - r_x_l_share[i] + small_delta[i];
            }
        }

        if (!Config::myconfig->check("player2")) {
            #pragma omp parallel for
            for (int i = 0; i < WIDTH; i++) {
                small_delta[i] = r_x_share[i] - r_x_l_share[i];
            }
            sendVector<T>(small_delta, 2, WIDTH);
        } else {
            receiveVector<T>(small_delta, 0, WIDTH);
            receiveVector<T>(small_delta, 1, WIDTH);

            #pragma omp parallel for
            for (int i = 0; i < WIDTH; i++) {
                m_x_l[i] = r_x_share[i] + x[i].r_1 - r_x_l_share[i] + small_delta[i];
            }
        }


        if(!Config::myconfig->check("player0")){




            
            //pick all w and w'
            random_T<uint8_t>((uint8_t*)w, WIDTH*(LENGTH));


            #pragma omp parallel for
            for(int k = 0; k < output.size(); k++){
                //maybe need to set m_sigmas_l and rl to 0
                chop<T>(x[k].r_1, m_sigmas[k].rb);
                chop<T>(r_x_share[k],r_x_share_i[k].rb);
                auto sign_m=m_sigmas[k].rb[8*sizeof(T)-1];
                m_sigmas[k].rb[8*sizeof(T)-1] = 1;
                uint8_t sum_of_m = 0;
                for(int i=0;i<LENGTH;i++){
                    Plist<LENGTH> temp_m_i;
                    // step 3 m_j = (m_sigma + r_x_i - 2*m_sigma*r_x_i) mod p
                    m_j[k].rb[i]=(r_x_share_i[k].rb[i] + 134 - 2 * m_sigmas[k].rb[i] * r_x_share_i[k].rb[i] ) % 67;
                    if(Config::myconfig->check("player1")){
                        m_j[k].rb[i] =( m_j[k].rb[i] + m_sigmas[k].rb[i] ) % 67;
                    }
                    // cal sum of m
                    sum_of_m += (m_j[k].rb[i]) % 67;
                    //step 5 m' =(sum_of_m - 2*m_j i+ 1) mod p;
                    temp_m_i.rb[i] = (sum_of_m - 2* m_j[k].rb[i]) % 67;
                    if(Config::myconfig->check("player1")){
                        temp_m_i.rb[i] = (temp_m_i.rb[i] + 1) % 67;
                    }
                    //step 5 u_j 
                    if(delta[k] % 2 == 1)delta[k]=0xff;
                    u_j[k].rb[i] = w[k].rb[i] * temp_m_i.rb[i] * (1 ^ sign_m ^ m_sigmas[k].rb[i] ^ delta[k]) +
                        w[k].rb[i] * ( sign_m ^ m_sigmas[k].rb[i] ^ delta[k] );
                    u_j[k].rb[i] %= 67;

                }
                #pragma omp parallel for
                for(int l=0;l<LAMBDA;l++){
                    uint8_t sum_of_m = 0;
                    for(int i=0;i<LENGTH;i++){
                        Plist<LENGTH> temp_m_i;
                        m_j_mac[k].r_mac[l][i]=(r_x_share_mac[k].r_mac[l][i] + 134 - 2 * m_sigmas[k].rb[i] * r_x_share_mac[k].r_mac[l][i] ) % 67;
                        if(Config::myconfig->check("player1")){
                            m_j_mac[k].r_mac[l][i] =(  m_j_mac[k].r_mac[l][i] + alpha[l] * m_sigmas[k].rb[i] ) % 67;
                        }
                        sum_of_m += (m_j_mac[k].r_mac[l][i]) % 67;
                        temp_m_i.rb[i] = (sum_of_m - 2* m_j_mac[k].r_mac[l][i]) % 67;
                        if(Config::myconfig->check("player1")){
                            temp_m_i.rb[i] = (temp_m_i.rb[i] + alpha[l] * 1) % 67;
                        }
                        if(delta[k]!=0)delta[k]=0xff;
                        u_j_mac[k].r_mac[l][i] = w[k].rb[i] * temp_m_i.rb[i] * (1 ^ sign_m ^ m_sigmas[k].rb[i] ^ delta[k]) +
                            w[k].rb[i] * ( sign_m ^ m_sigmas[k].rb[i] ^ delta[k] );
                        u_j_mac[k].r_mac[l][i] %= 67;
                    }
                }

            }
            printf("online connect\n");

            RevealCt<uint8_t>((uint8_t*)u_j, output.size()*(LENGTH), P);
            uint16_t t_k[LAMBDA];
            uint16_t w_k[LAMBDA];
            receiveVector<uint16_t>(w_k, 0, LAMBDA);
            memset(t_k, 0, sizeof(t_k));
            #pragma omp parallel
            {
                uint16_t t_k_local[LAMBDA] = {0}; // 每个线程的局部变量
                #pragma omp for
                for (int i = 0; i < WIDTH; i++) {
                    for (int k = 0; k < LENGTH; k++) {
                        for (int l = 0; l < LAMBDA; l++) {
                            t_k_local[l] = (alpha[l] * w_k[l] * u_j_mac[i].r_mac[l][k] + t_k_local[l]) % P;
                        }
                    }
                }
            
                // 合并每个线程的局部结果到全局 t_k
                #pragma omp critical
                {
                    for (int l = 0; l < LAMBDA; l++) {
                        t_k[l] = (t_k[l] + t_k_local[l]) % P;
                    }
                }
            }
            sendVector<uint16_t>(t_k, 0, LAMBDA);

            receiveVector<T>(m_l, 0, output.size());

            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                m_z[i] = m_l[i] - 2 * delta[i] * m_l[i] + gammas[i];
            }
        }else{
            RevealCt<uint8_t>((uint8_t*)u_j, output.size()*(LENGTH), P);
            //batchrec
            uint16_t w_k[LAMBDA];
            uint16_t t_k_2[LAMBDA];
            uint16_t t_k_1[LAMBDA];
            uint16_t t_k_sigma[LAMBDA];
            memset(t_k_sigma, 0, sizeof(t_k_sigma));
            random_T<uint16_t>(w_k, LAMBDA);
            #pragma omp parallel for
            for(int i=0;i<LAMBDA;i++){
                w_k[i] = (w_k[i] + 1) % 67;
            }
            sendVector<uint16_t>(w_k, 1, LAMBDA);
            sendVector<uint16_t>(w_k, 2, LAMBDA);
            receiveVector<uint16_t>(t_k_2, 1, LAMBDA);
            receiveVector<uint16_t>(t_k_1, 2, LAMBDA);
            add_T_mod<uint16_t>(t_k_1, t_k_2, t_k_1, LAMBDA,P);
            #pragma omp parallel
            {
                uint16_t t_k_local[LAMBDA] = {0}; // 每个线程的局部变量
                #pragma omp for
                for (int i = 0; i < WIDTH; i++) {
                    for (int k = 0; k < LENGTH; k++) {
                        for (int l = 0; l < LAMBDA; l++) {
                            t_k_local[l] = (alpha[l] * w_k[l] * u_j_mac[i].r_mac[l][k] + t_k_local[l]) % P;
                        }
                    }
                }
            
                // 合并每个线程的局部结果到全局 t_k
                #pragma omp critical
                {
                    for (int l = 0; l < LAMBDA; l++) {
                        t_k_sigma[l] = (t_k_sigma[l] + t_k_local[l]) % P;
                    }
                }
            }

            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                //rl is r' here
                auto sign_r_x=((x[i].r_1 + x[i].r_2) >> (sizeof(T) * 8 - 1)) & 1;
                if(check_list(u_j[i].rb, LENGTH)){
                    m_l[i] = -rl[i] - sign_r_x;
                }else{
                    m_l[i] = -rl[i] -( 1^(-sign_r_x));
                }

            }

            sendVector<T>(m_l, 1, WIDTH);
            sendVector<T>(m_l, 2, WIDTH);
        }

        if(!Config::myconfig->check("player1")){


            
            //pick all w and w'
            random_T<uint8_t>((uint8_t*)w, WIDTH*(LENGTH));



            #pragma omp parallel for
            for(int k = 0; k < output.size(); k++){
                //maybe need to set m_sigmas_l and rl to 0
                chop<T>(x[k].r_1, m_sigmas[k].rb);
                chop<T>(r_x_share[k],r_x_share_l_i[k].rb);
                auto sign_m=m_sigmas[k].rb[8*sizeof(T)-1];
                m_sigmas[k].rb[8*sizeof(T)-1] = 1;
                uint8_t sum_of_m = 0;
                for(int i=0;i<LENGTH;i++){
                    Plist<LENGTH> temp_m_i;
                    // step 3 m_j = (m_sigma + r_x_i - 2*m_sigma*r_x_i) mod p
                    m_j[k].rb[i]=(r_x_share_l_i[k].rb[i] + 134 - 2 * m_sigmas[k].rb[i] * r_x_share_l_i[k].rb[i] ) % 67;
                    if(Config::myconfig->check("player1")){
                        m_j[k].rb[i] =( m_j[k].rb[i] + m_sigmas[k].rb[i] ) % 67;
                    }
                    // cal sum of m
                    sum_of_m += (m_j[k].rb[i]) % 67;
                    //step 5 m' =(sum_of_m - 2*m_j i+ 1) mod p;
                    temp_m_i.rb[i] = (sum_of_m - 2* m_j[k].rb[i]) % 67;
                    if(Config::myconfig->check("player1")){
                        temp_m_i.rb[i] = (temp_m_i.rb[i] + 1) % 67;
                    }
                    //step 5 u_j 
                    if(delta_l[k]!=0)delta_l[k]=0xff;
                    u_j[k].rb[i] = w[k].rb[i] * temp_m_i.rb[i] * (1 ^ sign_m ^ m_sigmas[k].rb[i] ^ delta_l[k]) +
                        w[k].rb[i] * ( sign_m ^ m_sigmas[k].rb[i] ^ delta_l[k] );
                    u_j[k].rb[i] %= 67;

                }
                #pragma omp parallel for
                for(int l=0;l<LAMBDA;l++){
                    uint8_t sum_of_m = 0;
                    for(int i=0;i<LENGTH;i++){
                        Plist<LENGTH> temp_m_i;
                        m_j_mac[k].r_mac[l][i]=(r_x_l_share_mac[k].r_mac[l][i] + 134 - 2 * m_sigmas[k].rb[i] * r_x_l_share_mac[k].r_mac[l][i] ) % 67;
                        if(Config::myconfig->check("player0")){
                            m_j_mac[k].r_mac[l][i] =(  m_j_mac[k].r_mac[l][i] + alpha[l] * m_sigmas[k].rb[i] ) % 67;
                        }
                        sum_of_m += (m_j_mac[k].r_mac[l][i]) % 67;
                        temp_m_i.rb[i] = (sum_of_m - 2* m_j_mac[k].r_mac[l][i]) % 67;
                        if(Config::myconfig->check("player0")){
                            temp_m_i.rb[i] = (temp_m_i.rb[i] + alpha[l] * 1) % 67;
                        }
                        if(delta_l[k]!=0)delta_l[k]=0xff;
                        u_j_mac[k].r_mac[l][i] = w[k].rb[i] * temp_m_i.rb[i] * (1 ^ sign_m ^ m_sigmas[k].rb[i] ^ delta_l[k]) 
                            + w[k].rb[i] * ( sign_m ^ m_sigmas[k].rb[i] ^ delta_l[k] );
                        u_j_mac[k].r_mac[l][i] %= 67;
                    }
                }

            }

            printf("online connect\n");

            RevealCt1<uint8_t>((uint8_t*)u_j, output.size()*(LENGTH), 67);
            uint16_t t_k[LAMBDA];
            uint16_t w_k[LAMBDA];
            receiveVector<uint16_t>(w_k, 1, LAMBDA);
            memset(t_k, 0, sizeof(t_k));
            #pragma omp parallel
            {
                uint16_t t_k_local[LAMBDA] = {0}; // 每个线程的局部变量
                #pragma omp for
                for (int i = 0; i < WIDTH; i++) {
                    for (int k = 0; k < LENGTH; k++) {
                        for (int l = 0; l < LAMBDA; l++) {
                            t_k_local[l] = (alpha_l[l] * w_k[l] * u_j_mac[i].r_mac[l][k] + t_k_local[l]) % P;
                        }
                    }
                }
            
                // 合并每个线程的局部结果到全局 t_k
                #pragma omp critical
                {
                    for (int l = 0; l < LAMBDA; l++) {
                        t_k[l] = (t_k[l] + t_k_local[l]) % P;
                    }
                }
            }
            sendVector<uint16_t>(t_k, 1, LAMBDA);


            receiveVector<T>(m_l, 1, output.size());

            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                m_z[i] = m_l[i] - 2 * delta_l[i] * m_l[i] + gammas_l[i];
            }

        }else{
            RevealCt1<uint8_t>((uint8_t*)u_j, output.size()*(LENGTH), 67);
            //batchrec
            uint16_t w_k[LAMBDA];
            uint16_t t_k_2[LAMBDA];
            uint16_t t_k_1[LAMBDA];
            uint16_t t_k_sigma[LAMBDA];
            memset(t_k_sigma, 0, sizeof(t_k_sigma));
            random_T<uint16_t>(w_k, LAMBDA);
            #pragma omp parallel for
            for(int i=0;i<LAMBDA;i++){
                w_k[i] = (w_k[i] + 1) % 67;
            }
            sendVector<uint16_t>(w_k, 0, LAMBDA);
            sendVector<uint16_t>(w_k, 2, LAMBDA);
            receiveVector<uint16_t>(t_k_2, 0, LAMBDA);
            receiveVector<uint16_t>(t_k_1, 2, LAMBDA);
            add_T_mod<uint16_t>(t_k_1, t_k_2, t_k_1, LAMBDA,P);
            #pragma omp parallel
            {
                uint16_t t_k_local[LAMBDA] = {0}; // 每个线程的局部变量
                #pragma omp for
                for (int i = 0; i < WIDTH; i++) {
                    for (int k = 0; k < LENGTH; k++) {
                        for (int l = 0; l < LAMBDA; l++) {
                            t_k_local[l] = (alpha_l[l] * w_k[l] * u_j_mac[i].r_mac[l][k] + t_k_local[l]) % P;
                        }
                    }
                }
            
                // 合并每个线程的局部结果到全局 t_k
                #pragma omp critical
                {
                    for (int l = 0; l < LAMBDA; l++) {
                        t_k_sigma[l] = (t_k_sigma[l] + t_k_local[l]) % P;
                    }
                }
            }

            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                //rl is r' here
                auto sign_r_x=((x[i].r_1 + x[i].r_2) >> (sizeof(T) * 8 - 1)) & 1;
                if(check_list(u_j[i].rb, LENGTH)){
                    m_ll[i] = -rl[i] - sign_r_x;
                }else{
                    m_ll[i] = -rl[i] -( 1^(-sign_r_x));
                }

            }

            thread *threads = new thread[2];

            threads[0] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)m_ll, 0, output.size());
            threads[1] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)m_ll, 2, output.size());
            
            for (int i = 0; i < 2; i++)
                threads[i].join();

            delete[] threads;
        }

        free(u_j);
        free(u_j_mac);
        free(m_j);
        free(m_j_mac);
        free(m_l);
        free(m_ll);
        free(m_x_l);
        free(m_z);
        free(m_z_l);
        free(small_delta);
        free(send_buffer);
        free(m_sigmas);
        free(r_x_share_i);
        free(r_x_share_l_i);
        free(w);
    }

    void verify();
};
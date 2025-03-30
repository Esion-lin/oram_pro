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
const int P=67;
#define LENGTH 8*sizeof(T)+1
#define MACKEY_SIZE 8

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
    AShareT<T>* r_x_share = nullptr;
    AShareT<T>* r_x_l_share = nullptr;
    AShareT<T>* r_x_share_sigmas = nullptr;
    AShareT<T>* r_x_l_share_sigmas = nullptr;

    AShare<uint8_t,LENGTH>* r_x_l_share_mac = nullptr;
    AShare<uint8_t,LENGTH>* r_x_share_mac = nullptr;

    uint8_t* delta = nullptr;
    uint8_t* delta_l = nullptr;
    T* gammas = nullptr;
    T* gammas_l = nullptr;
    uint8_t* alpha = nullptr;
    uint8_t* alpha_l = nullptr;

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
        r_x_i = (Pvlist<LENGTH, LENGTH>*)malloc(sizeof(Pvlist<LENGTH, LENGTH>)*output.size());
        const size_t WIDTH = output.size();
        rl = (T*)malloc(sizeof(T)*WIDTH);
        rll = (T*)malloc(sizeof(T)*WIDTH);
        r_z = (T*)malloc(sizeof(T)*WIDTH);
        r_z_l = (T*)malloc(sizeof(T)*WIDTH);
        r_x_share = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        r_x_share_sigmas = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        r_x_l_share_sigmas = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        r_x_l_share = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        d_1_share = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        d_2_share = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        c_0_share = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);
        c_2_share = (AShareT<T>*)malloc(sizeof(AShareT<T>)*WIDTH);

        r_x_share_mac = (AShare<uint8_t,LENGTH>*)malloc(sizeof(AShare<uint8_t,LENGTH>)*WIDTH);
        r_x_l_share_mac = (AShare<uint8_t,LENGTH>*)malloc(sizeof(AShare<uint8_t,LENGTH>)*WIDTH);

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
        if(need_gen){//gen rz rz'
            if(Config::myconfig->check("player0")){

            }else if(Config::myconfig->check("player1")){
                

            }else if(Config::myconfig->check("player2")){

            }
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
        #pragma omp parallel for
        for(int i=0;i<WIDTH;i++){
            r_x_share[i].r_1 = (1<<LENGTH-1);
            #pragma omp parallel for
            for(int j=0;j<LENGTH-1;j++){
                r_x_share[i].r_1 -= (d_1_share[j].r_1 ^ d_2_share[j].r_1) * (1<<j);
                r_x_share[i].r_1 += (d_1_share[j].r_1 ^ d_2_share[j].r_1) * (1<<LENGTH-1);
                r_x_share[i].r_2 -= (d_1_share[j].r_2 ^ d_2_share[j].r_2) * (1<<j);
                r_x_share[i].r_2 += (d_1_share[j].r_2 ^ d_2_share[j].r_2) * (1<<LENGTH-1);

                r_x_l_share[i].r_1 -= (c_0_share[j].r_1 ^ c_2_share[j].r_1) * (1<<j);
                r_x_l_share[i].r_1 += (c_0_share[j].r_1 ^ c_2_share[j].r_1) * (1<<LENGTH-1);
                r_x_l_share[i].r_2 -= (c_0_share[j].r_2 ^ c_2_share[j].r_2) * (1<<j);
                r_x_l_share[i].r_2 += (c_0_share[j].r_2 ^ c_2_share[j].r_2) * (1<<LENGTH-1);

                r_x_share_sigmas[i].r_1 = d_1_share[j].r_1 ^ d_2_share[j].r_1;
                r_x_share_sigmas[i].r_2 = d_1_share[j].r_2 ^ d_2_share[j].r_2;

                r_x_l_share_sigmas[i].r_1 = c_0_share[j].r_1 ^ c_2_share[j].r_1;
                r_x_l_share_sigmas[i].r_2 = c_0_share[j].r_2 ^ c_2_share[j].r_2;
            }
        }
        //call F_mutiverify and F_innerVerify
        if(Config::myconfig->check("player0")){
                #pragma omp parallel for
                for(int i=0;i<WIDTH;i++){
                    r_x_share[i].r_1=d_2[i]%2;
                    r_x_share[i].r_1=c_2[i]%2;
                }
            }else{
                //set r_x_i and rx'i
                T r_x_another=0;
                T r_x_l_another=0;
            if(Config::myconfig->check("player1")){
                #pragma omp parallel for
                for(int i=0;i<WIDTH;i++){

                    sendVector<T>(&r_x_share[i].r_2, 2,1);
                    receiveVector<T>(&r_x_l_another,2, 1);
                    r_x_share[i].r_1+=r_x_share[i].r_2;
                    r_x_share[i].r_2=r_x_another;

                    sendVector<T>(&r_x_l_share[i].r_2, 2,1);
                    receiveVector<T>(&r_x_l_another,2, 1);
                    r_x_share[i].r_1+=r_x_share[i].r_2;
                    r_x_share[i].r_2=r_x_another;
                }
            }else if(Config::myconfig->check("player2")){
                #pragma omp parallel for
                for(int i=0;i<WIDTH;i++){
                    sendVector<T>(&r_x_share[i].r_2, 1,1);
                    receiveVector<T>(&r_x_l_another,1, 1);
                    r_x_share[i].r_1+=r_x_another;

                    sendVector<T>(&r_x_l_share[i].r_2, 1,1);
                    receiveVector<T>(&r_x_l_another,1, 1);
                    r_x_l_share[i].r_1+=r_x_l_another;
                } 
            }

        }

        free(d_1);
        free(d_2);
        free(c_0);
        free(c_2);

   }
    void online(const std::vector<AShareT<T>>& x, std::vector<AShareT<T>>& output) {
        const size_t WIDTH = output.size();

        // Allocate memory for arrays with proper size
        T* m_x_l = (T*)malloc(sizeof(T) * WIDTH);
        T* m_z = (T*)malloc(sizeof(T) * WIDTH);
        T* m_z_l = (T*)malloc(sizeof(T) * WIDTH);
        T* m_l = (T*)malloc(sizeof(T) * WIDTH); // Ensure correct size allocation
        T* small_delta = (T*)malloc(sizeof(T) * WIDTH);
        T* send_buffer = (T*)malloc(sizeof(T) * WIDTH); // 用于批量发送数据
        uint32_t shiftsize = sizeof(T) * 8 - 1;

        if (!m_x_l || !m_z || !m_z_l || !m_l || !small_delta || !send_buffer) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        // Calculate m_x_l
        if (!Config::myconfig->check("player0")) { // P1 P2
            #pragma omp parallel for
            for (int i = 0; i < WIDTH; i++) {
                small_delta[i] = x[i].r_1 - r_x_l_share[i].r_2;
            }
            Timer::record("communication");
            sendVector<T>(small_delta, 0, WIDTH);
            Timer::stop("communication");
        } else {
            Timer::record("communication");
            receiveVector<T>(small_delta, 1, WIDTH);
            receiveVector<T>(small_delta, 2, WIDTH);
            Timer::stop("communication");

            #pragma omp parallel for
            for (int i = 0; i < WIDTH; i++) {
                m_x_l[i] = r_x_share[i].r_1 + r_x_share[i].r_2 - r_x_l_share[i].r_1 + small_delta[i];
            }
        }

        if (!Config::myconfig->check("player2")) {
            #pragma omp parallel for
            for (int i = 0; i < WIDTH; i++) {
                small_delta[i] = r_x_share[i].r_1 - r_x_l_share[i].r_1;
            }
            Timer::record("communication");
            sendVector<T>(small_delta, 2, WIDTH);
            Timer::stop("communication");
        } else {
            Timer::record("communication");
            receiveVector<T>(small_delta, 0, WIDTH);
            receiveVector<T>(small_delta, 1, WIDTH);
            Timer::stop("communication");

            #pragma omp parallel for
            for (int i = 0; i < WIDTH; i++) {
                m_x_l[i] = r_x_share[i].r_2 + x[i].r_1 - r_x_l_share[i].r_2 + small_delta[i];
            }
        }

        AShare<uint8_t,LENGTH>* u_j=(AShare<uint8_t,LENGTH>*)malloc(WIDTH*(sizeof(AShare<uint8_t,LENGTH>)));
        AShare<uint8_t,LENGTH>* m_j=(AShare<uint8_t,LENGTH>*)malloc(WIDTH*(sizeof(AShare<uint8_t,LENGTH>)));

        if(!Config::myconfig->check("player0")){
            Plist<LENGTH>* m_sigmas = (Plist<LENGTH>*)malloc(WIDTH*(LENGTH));
            Plist<LENGTH>* w = (Plist<LENGTH>*)malloc(WIDTH*(LENGTH));



            
            //pick all w and w'
            random_T<uint8_t>((uint8_t*)w, WIDTH*(LENGTH));


            #pragma omp parallel for
            for(int k = 0; k < output.size(); k++){
                //maybe need to set m_sigmas_l and rl to 0
                chop<T>(x[k].r_1, m_sigmas[k].rb);
                auto sign_m=m_sigmas[k].rb[8*sizeof(T)-1];
                m_sigmas[k].rb[8*sizeof(T)-1] = 1;
                uint8_t sum_of_m_r_1 = 0;
                uint8_t sum_of_m_r_2 = 0;
                #pragma omp parallel for
                for(int i=0;i<LENGTH;i++){
                    AShare<uint8_t,LENGTH> temp_m_i;
                    // step 3 m_j = (m_sigma + r_x_i - 2*m_sigma*r_x_i) mod p
                    m_j[k].r_1[i]=(r_x_share_mac[k].r_1[i] + 134 - 2 * m_sigmas[k].rb[i] * r_x_share_mac[k].r_1[i] ) % 67;
                    m_j[k].r_2[i]=(r_x_share_mac[k].r_2[i] + 134 - 2 * m_sigmas[k].rb[i] * r_x_share_mac[k].r_2[i] ) % 67;
                    if(Config::myconfig->check("player1")){
                        m_j[k].r_1[i] =( m_j[k].r_1[i] + m_sigmas[k].rb[i] ) % 67;
                        m_j[k].r_2[i] =( m_j[k].r_2[i] + m_sigmas[k].rb[i] ) % 67;
                    }
                    // cal sum of m
                    sum_of_m_r_1 += (m_j[k].r_1[i]) % 67;
                    sum_of_m_r_2 += (m_j[k].r_2[i]) % 67;
                    //step 5 m' =(sum_of_m - 2*m_j i+ 1) mod p;
                    temp_m_i.r_1[i] = (sum_of_m_r_1 - 2* m_j[k].r_1[i]) % 67;
                    temp_m_i.r_2[i] = (sum_of_m_r_2 - 2* m_j[k].r_2[i]) % 67;
                    if(Config::myconfig->check("player1")){
                        temp_m_i.r_1[i] = (temp_m_i.r_1[i] + 1) % 67;
                        temp_m_i.r_2[i] = (temp_m_i.r_2[i] + 1) % 67;
                    }
                    //step 5 u_j 
                    if(delta[k]!=0)delta[k]=0xff;
                    u_j[k].r_1[i] = w[k].rb[i] * temp_m_i.r_1[i] * (1 ^ sign_m ^ m_sigmas[k].rb[i] ^ delta[k]) +
                        w[k].rb[i] * ( sign_m ^ m_sigmas[k].rb[i] ^ delta[k] );
                    u_j[k].r_1[i] %= 67;

                    u_j[k].r_2[i] = w[k].rb[i] * temp_m_i.r_1[i] * (1 ^ sign_m ^ m_sigmas[k].rb[i] ^ delta[k]) +
                    w[k].rb[i] * ( sign_m ^ m_sigmas[k].rb[i] ^ delta[k] );
                    u_j[k].r_2[i] %= 67;
                }
                

            }
            free(m_sigmas);
            free(w);

            printf("online connect\n");

            Timer::record("communication");
            /*
            #pragma omp parallel for
            for(int k=0;k<WIDTH;k++){
                RevealCt<uint8_t>(u_j[k].r_1, (LENGTH), 67);
                RevealCt<uint8_t>(u_j[k].r_2, (LENGTH), 67);
            }
            */

            receiveVector<T>(m_l, 0, output.size());
            Timer::stop("communication");

            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                m_z[i] = m_l[i] - 2 * delta[i] * m_l[i] + gammas[i];
            }
        }else{
            Timer::record("communication");
            /*

            #pragma omp parallel for
            for(int k=0;k<WIDTH;k++){
                RevealCt<uint8_t>(u_j[k].r_1, (LENGTH), 67);
                RevealCt<uint8_t>(u_j[k].r_2, (LENGTH), 67);
            }
            */

            Timer::stop("communication");

            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                //rl is r' here
                auto sign_r_x=((x[i].r_1 + x[i].r_2) >> (sizeof(T) * 8 - 1)) & 1;
                if(check_list(u_j[i].r_1, LENGTH)){
                    m_l[i] = -rl[i] - sign_r_x;
                }else{
                    m_l[i] = -rl[i] -( 1^(-sign_r_x));
                }

            }

            memcpy(send_buffer, m_l, sizeof(T) * WIDTH);
            Timer::record("communication");
            sendVector<T>(send_buffer, 1, WIDTH);
            sendVector<T>(send_buffer, 2, WIDTH);
            Timer::stop("communication");
        }

        if(!Config::myconfig->check("player1")){
            Plist<LENGTH>* m_sigmas = (Plist<LENGTH>*)malloc(WIDTH*(LENGTH));
            Plist<LENGTH>* w = (Plist<LENGTH>*)malloc(WIDTH*(LENGTH));



            
            //pick all w and w'
            random_T<uint8_t>((uint8_t*)w, WIDTH*(LENGTH));



            #pragma omp parallel for
            for(int k = 0; k < output.size(); k++){
                //maybe need to set m_sigmas_l and rl to 0
                chop<T>(m_x_l[k], m_sigmas[k].rb);
                auto sign_m=m_sigmas[k].rb[8*sizeof(T)-1];
                m_sigmas[k].rb[8*sizeof(T)-1] = 1;
                uint8_t sum_of_m_r_1 = 0;
                uint8_t sum_of_m_r_2 = 0;
                #pragma omp parallel for
                for(int i=0;i<LENGTH;i++){
                    AShare<uint8_t,LENGTH> temp_m_i;
                    // step 3 m_j = (m_sigma + r_x_i - 2*m_sigma*r_x_i) mod p
                    m_j[k].r_1[i]=(r_x_l_share_mac[k].r_1[i] + 134 - 2 * m_sigmas[k].rb[i] * r_x_l_share_mac[k].r_1[i] ) % 67;
                    m_j[k].r_2[i]=(r_x_l_share_mac[k].r_2[i] + 134 - 2 * m_sigmas[k].rb[i] * r_x_l_share_mac[k].r_2[i] ) % 67;
                    if(Config::myconfig->check("player1")){
                        m_j[k].r_1[i] =( m_j[k].r_1[i] + m_sigmas[k].rb[i] ) % 67;
                        m_j[k].r_2[i] =( m_j[k].r_2[i] + m_sigmas[k].rb[i] ) % 67;
                    }
                    // cal sum of m
                    sum_of_m_r_1 += (m_j[k].r_1[i]) % 67;
                    sum_of_m_r_2 += (m_j[k].r_2[i]) % 67;
                    //step 5 m' =(sum_of_m - 2*m_j i+ 1) mod p;
                    temp_m_i.r_1[i] = (sum_of_m_r_1 - 2* m_j[k].r_1[i]) % 67;
                    temp_m_i.r_2[i] = (sum_of_m_r_2 - 2* m_j[k].r_2[i]) % 67;
                    if(Config::myconfig->check("player1")){
                        temp_m_i.r_1[i] = (temp_m_i.r_1[i] + 1) % 67;
                        temp_m_i.r_2[i] = (temp_m_i.r_2[i] + 1) % 67;
                    }
                    //step 5 u_j 
                    if(delta_l[k]!=0)delta_l[k]=0xff;
                    u_j[k].r_1[i] = w[k].rb[i] * temp_m_i.r_1[i] * (1 ^ sign_m ^ m_sigmas[k].rb[i] ^ delta_l[k]) +
                        w[k].rb[i] * ( sign_m ^ m_sigmas[k].rb[i] ^ delta_l[k] );
                    u_j[k].r_1[i] %= 67;

                    u_j[k].r_2[i] = w[k].rb[i] * temp_m_i.r_1[i] * (1 ^ sign_m ^ m_sigmas[k].rb[i] ^ delta_l[k]) +
                    w[k].rb[i] * ( sign_m ^ m_sigmas[k].rb[i] ^ delta_l[k] );
                    u_j[k].r_2[i] %= 67;
                }
                

            }
            free(m_sigmas);
            free(w);

            printf("online connect\n");

            Timer::record("communication");
            /*
            #pragma omp parallel for
            for(int k=0;k<WIDTH;k++){
                RevealCt<uint8_t>(u_j[k].r_1, (LENGTH), 67);
                RevealCt<uint8_t>(u_j[k].r_2, (LENGTH), 67);
            }
            */


            receiveVector<T>(m_l, 1, output.size());
            Timer::stop("communication");

            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                m_z[i] = m_l[i] - 2 * delta_l[i] * m_l[i] + gammas_l[i];
            }

        }else{
            T* m_ll=(T*)malloc(output.size()*sizeof(T));
            Timer::record("communication");
            /*
            #pragma omp parallel for
            for(int k=0;k<WIDTH;k++){
                RevealCt<uint8_t>(u_j[k].r_1, (LENGTH), 67);
                RevealCt<uint8_t>(u_j[k].r_2, (LENGTH), 67);
            }
            Timer::stop("communication");
            */

            #pragma omp parallel for
            for(int i = 0; i < output.size(); i++){
                //rl is r' here
                auto sign_r_x=((x[i].r_1 + x[i].r_2) >> (sizeof(T) * 8 - 1)) & 1;
                if(check_list(u_j[i].r_1, LENGTH)){
                    m_ll[i] = -rl[i] - sign_r_x;
                }else{
                    m_ll[i] = -rl[i] -( 1^(-sign_r_x));
                }

            }

            thread *threads = new thread[2];

            threads[0] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)m_ll, 0, output.size());
            threads[1] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)m_ll, 2, output.size());
            
            Timer::record("communication");
            for (int i = 0; i < 2; i++)
                threads[i].join();
            Timer::stop("communication");

            delete[] threads;
        }
        free(u_j);
        free(m_l);
        free(m_x_l);
        free(m_z);
        free(m_z_l);
        free(small_delta);
        free(send_buffer);
        free(m_j);

    }

    void verify();
    
};
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
const int P = 67;
#define LENGTH 8 * sizeof(T) + 1
#define LAMBDA 6

template<class T>
class Signv {
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

    AShareMAC<T, LAMBDA>* r_x_l_share_mac = nullptr;
    AShareMAC<T, LAMBDA>* r_x_share_mac = nullptr;

    uint8_t* delta = nullptr;
    uint8_t* delta_l = nullptr;
    T* gammas = nullptr;
    T* gammas_l = nullptr;
    uint8_t alpha[LAMBDA];
    uint8_t alpha_l[LAMBDA];

public:
    Signv(int batch) : batch_len(batch) {}

    ~Signv() {
        if (r_x_i != nullptr) {
            free(r_x_i);
            r_x_i = nullptr;
        }
        if (r_x_i_l != nullptr) {
            free(r_x_i_l);
            r_x_i_l = nullptr;
        }
        if (rl != nullptr) {
            free(rl);
            rl = nullptr;
        }
        if (rll != nullptr) {
            free(rll);
            rll = nullptr;
        }
        if (r_z != nullptr) {
            free(r_z);
            r_z = nullptr;
        }
        if (r_z_l != nullptr) {
            free(r_z_l);
            r_z_l = nullptr;
        }
        if (d_1_share != nullptr) {
            free(d_1_share);
            d_1_share = nullptr;
        }
        if (d_2_share != nullptr) {
            free(d_2_share);
            d_2_share = nullptr;
        }
        if (c_0_share != nullptr) {
            free(c_0_share);
            c_0_share = nullptr;
        }
        if (c_2_share != nullptr) {
            free(c_2_share);
            c_2_share = nullptr;
        }
        if (r_x_share != nullptr) {
            free(r_x_share);
            r_x_share = nullptr;
        }
        if (r_x_l_share != nullptr) {
            free(r_x_l_share);
            r_x_l_share = nullptr;
        }
        if (r_x_share_sigmas != nullptr) {
            free(r_x_share_sigmas);
            r_x_share_sigmas = nullptr;
        }
        if (r_x_l_share_sigmas != nullptr) {
            free(r_x_l_share_sigmas);
            r_x_l_share_sigmas = nullptr;
        }
        if (r_x_share_mac != nullptr) {
            free(r_x_share_mac);
            r_x_share_mac = nullptr;
        }
        if (r_x_l_share_mac != nullptr) {
            free(r_x_l_share_mac);
            r_x_l_share_mac = nullptr;
        }
        if (delta != nullptr) {
            free(delta);
            delta = nullptr;
        }
        if (delta_l != nullptr) {
            free(delta_l);
            delta_l = nullptr;
        }
        if (gammas != nullptr) {
            free(gammas);
            gammas = nullptr;
        }
        if (gammas_l != nullptr) {
            free(gammas_l);
            gammas_l = nullptr;
        }
    }

    void set_up(const std::vector<AShareT<T>>& x, std::vector<AShareT<T>>& output, bool need_gen) {
        const size_t WIDTH = output.size();
        omp_set_num_threads(std::min(WIDTH + 3, (size_t)omp_get_max_threads() / 3));
        r_x_i = (Pvlist<LENGTH, LENGTH>*)malloc(sizeof(Pvlist<LENGTH, LENGTH>) * output.size());

        rl = (T*)malloc(sizeof(T) * WIDTH);
        rll = (T*)malloc(sizeof(T) * WIDTH);
        r_z = (T*)malloc(sizeof(T) * WIDTH);
        r_z_l = (T*)malloc(sizeof(T) * WIDTH);
        r_x_share = (T*)malloc(sizeof(T) * WIDTH);
        r_x_share_sigmas = (AShareT<T>*)malloc(sizeof(AShareT<T>) * WIDTH);
        r_x_l_share_sigmas = (AShareT<T>*)malloc(sizeof(AShareT<T>) * WIDTH);
        r_x_l_share = (T*)malloc(sizeof(T) * WIDTH);
        d_1_share = (AShareT<T>*)malloc(sizeof(AShareT<T>) * WIDTH);
        d_2_share = (AShareT<T>*)malloc(sizeof(AShareT<T>) * WIDTH);
        c_0_share = (AShareT<T>*)malloc(sizeof(AShareT<T>) * WIDTH);
        c_2_share = (AShareT<T>*)malloc(sizeof(AShareT<T>) * WIDTH);

        r_x_share_mac = (AShareMAC<T, LAMBDA>*)malloc(sizeof(AShareMAC<T, LAMBDA>) * WIDTH);
        r_x_l_share_mac = (AShareMAC<T, LAMBDA>*)malloc(sizeof(AShareMAC<T, LAMBDA>) * WIDTH);

        r_x_i = (Pvlist<LENGTH, LENGTH>*)malloc(sizeof(Pvlist<LENGTH, LENGTH>) * WIDTH);
        r_x_i_l = (Pvlist<LENGTH, LENGTH>*)malloc(sizeof(Pvlist<LENGTH, LENGTH>) * WIDTH);

        random_T<T>(rl, WIDTH);
        random_T<T>(rll, WIDTH);
        random_T<T>(r_z, WIDTH);
        random_T<T>(r_z_l, WIDTH);
        int8_t* d_1 = (int8_t*)malloc(sizeof(int8_t) * WIDTH);
        int8_t* d_2 = (int8_t*)malloc(sizeof(int8_t) * WIDTH);
        int8_t* c_0 = (int8_t*)malloc(sizeof(int8_t) * WIDTH);
        int8_t* c_2 = (int8_t*)malloc(sizeof(int8_t) * WIDTH);
        random_T<int8_t>(d_1, WIDTH);
        random_T<int8_t>(d_2, WIDTH);
        random_T<int8_t>(c_0, WIDTH);
        random_T<int8_t>(c_2, WIDTH);

        if (Config::myconfig->check("player0")) {
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
        } else {
            receiveVector<uint8_t>(alpha, 0, LAMBDA);
            receiveVector<uint8_t>(alpha_l, 0, LAMBDA);
        }

        // Additional logic continues...
    }

    void online(const std::vector<AShareT<T>>& x, std::vector<AShareT<T>>& output) {
        // Implementation of the online function
    }

    void verify();
};

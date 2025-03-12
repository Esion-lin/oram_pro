#pragma once 
#include <vector>
#include "encode.hpp"
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <inttypes.h>
#include <NTL/ZZX.h>
using namespace NTL;
#define THREADS 2

//random target with lens * sizeof(T) length
template<class T>
void random_T(T* target, uint32_t lens){
    AES_KEY aes_key;
    uint8_t seeds[16] = {0};
    memset(seeds, 0, 16);
    AES_set_encrypt_key(seeds, 128, &(aes_key));
    #pragma omp parallel for
    for(int i = 0; i < lens * sizeof(T) / 16; i ++)
        AES_encrypt(seeds, ((uint8_t*)target) + 16*i, &aes_key);
    #pragma omp parallel for
    for(int i = (lens * sizeof(T) / 16 )*16; i < lens; i++){
        target[i] = 1;
    }

}

template<class T>
std::vector<T> random_T(uint32_t lens){
    std::vector<T> res;
    res.resize(lens);
    random_T(res.date(), lens);
    return res;
}        
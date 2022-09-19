#pragma once
#include <gmp.h>
#include <gmpxx.h>
#include <vector>
//float-Fp encode
template <typename T>
class Encoder{
public:
    Encoder(uint32_t precision, uint32_t data_size, mpz_class p):precision(precision), data_size(data_size), p(p){
        max_data = p / 2;
        scaler = 1 << precision;
    }
    std::vector<mpz_class> encode(T * org, uint32_t arr_size);
    void encode(T * org, mpz_class* dest, uint32_t arr_size);
    void decode(std::vector<mpz_class> &org, T *dest, uint32_t arr_size);
    void erase(T *dest, uint16_t count, uint32_t arr_size);
private:
    uint32_t precision,data_size;
    //precision << data_size
    uint32_t scaler;
    mpz_class p;
    // <= p/2 : pos || > p/2 neg 
    mpz_class max_data;
};
using Float_Encoder = Encoder<float>;
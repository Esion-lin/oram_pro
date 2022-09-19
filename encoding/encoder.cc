#include "encoder.h"
#include <assert.h>
template<>
std::vector<mpz_class> Float_Encoder::encode(float * org, uint32_t arr_size){
    //p > 2**32
    //return p + org * 2**precision % p
    std::vector<mpz_class> ret;
    for(int i = 0; i < arr_size; i++){
        ret.push_back((p + (int)(org[i] * scaler)) % p);
    }
}
template<>
void Float_Encoder::encode(float * org, mpz_class* dest, uint32_t arr_size){
    //p > 2**32
    //return p + org * 2**precision % p
    std::vector<mpz_class> ret;
    for(int i = 0; i < arr_size; i++){
        dest[i] = (p + (int)(org[i] * scaler)) % p;
    }
}
template<>
void Float_Encoder::decode(std::vector<mpz_class> &org, float *dest, uint32_t arr_size){
    assert(org.size() == arr_size);
    for(int i = 0; i < arr_size; i++){
        if(org[i] > max_data){
            mpz_class ele = org[i] - p;
            double temp = ele.get_d();
            dest[i] = temp / scaler;
        }
        else{
            double temp = org[i].get_d();
            dest[i] = temp / scaler;
        }
    }
}
template<>
void Float_Encoder::erase(float *dest, uint16_t count, uint32_t arr_size){
    for( int i = 0; i < count; i++){
        for(int j = 0; j < arr_size; j ++){
            dest[j] /= scaler;
        }
    }
}
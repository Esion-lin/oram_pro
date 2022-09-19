#pragma once
#include "spdz.h"
#include "BMR.h"

class Audit_AY{
public:
    Audit_AY(){
        spdz2 = std::make_shared<SPDZXOR>();
        spdz = std::make_shared<SPDZ>();
    };
    
    //generate pair
    void offline(uint32_t lens);
    //A to B
    std::vector<mpz_class> transfer(std::vector<Amaterial> A_data);
private:
    std::shared_ptr<SPDZXOR> spdz2;
    std::shared_ptr<SPDZ> spdz;
    std::vector<Amaterial> a_data;
    std::vector<Xmaterial> b_data;
    std::shared_ptr<BMR> bmr;
};
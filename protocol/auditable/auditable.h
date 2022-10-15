#pragma once
#include "spdz.h"
#include "BMR.h"

class Audit_AY{
public:
    Audit_AY(){
        spdz2 = std::make_shared<SPDZXOR>();
        spdz = std::make_shared<SPDZ>();
    };
    std::shared_ptr<SPDZXOR> spdz2;
    //generate pair
    void offline(uint32_t lens);
    //A to B
    std::vector<Xmaterial> transfer(std::vector<Amaterial> A_data);
    void run_cir_offline(std::string cir, uint32_t lens);
    std::vector<Xmaterial> run_cir(std::vector<uint8_t> data);
    std::vector<Xmaterial> get_mask(uint32_t len);
private:
    uint32_t data_lens;
    std::shared_ptr<SPDZ> spdz;
    std::vector<Amaterial> a_data;
    std::vector<Xmaterial> b_data;
    std::shared_ptr<BMR> bmr, tbmr;
};

//app
std::vector<Amaterial> relu(std::vector<Amaterial> arr);
std::vector<Amaterial> bit2A(std::vector<Xmaterial> arr, std::shared_ptr<SPDZXOR> temp);

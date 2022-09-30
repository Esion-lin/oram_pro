#pragma once
#include "net.hpp"
#include "emp-tool/emp-tool.h"
#include "emp-ot/emp-ot.h"
#include <openssl/rand.h>
#include <stdexcept>
#include "spdz.h"
//using gc_table=std::array<block, 4>;

struct gc_table{
    block t[4];
};
//template< int Lambda = 128>
class BMR{
    
    public:
    uint32_t in_size;
    BristolFormat *cf;
    std::vector<uint8_t> maskbits;
    std::vector<uint8_t> opened_bits;
    std::vector<uint8_t> Lambda_bits;
    std::vector<block> Ks;
    std::vector<std::vector<block>> Ks_pickeds;
    
    std::vector<gc_table> tabs;
    uint32_t num_gate;
    //std::shared_ptr<SPDZ> spdz = std::make_shared<SPDZ>();
    block R;
    BMR(std::string cir_file);
    BMR(std::string cir_file, uint32_t lens);
    //from share
    void online();
    
    void online(std::map<std::string, std::pair<int, int>>holder, std::vector<uint8_t> data);
    void online(std::map<std::string, std::pair<int, int>>holder, std::vector<std::vector<uint8_t>> datas, uint32_t lens);
    void offline();
    void offline(uint32_t lens);
    template<uint8_t partes_number>
    void fake_offline(std::string owner){
        //random mask bit 
        if(Config::myconfig->check(owner)){

        }
    }
    ~BMR(){
        delete cf;
    }
    private:
    bool contained_ = false;
    uint32_t offline_lens;
    
};
#include "auditable.h"
void Audit_AY::offline(uint32_t lens){
    // ->
    //use mod p circuit 
    bmr = std::make_shared<BMR>("../cir/add_32.txt", lens, "player0");

    bmr -> offline(lens);
    mpz_class temp[lens], rr[lens];
    for(int i = 0; i < lens; i++) temp[i] = 1;
    randomms<>(rr, spdz->get_prime(), lens);
    a_data = spdz->share(temp, rr, lens, "player0");
    uint8_t b[lens * 32];
    std::vector<std::pair<mpz_class, mpz_class>> rr0_p;
    for(int i = 0; i < lens * 32; i ++) {
        if(i % 32 == 0){
            b[i] = 1;
            rr0_p.push_back({11, 23141});
        }else{
            b[i] = 0;
            rr0_p.push_back({12312, 23141});
        }
        
    }
    b_data = spdz2->share(b, rr0_p, lens * 32, "player0");
}
std::vector<Xmaterial> Audit_AY::transfer(std::vector<Amaterial> A_data){
    std::vector<Amaterial> temp = spdz->add(A_data, spdz->neg(a_data));
    std::vector<mpz_class> revealed_data = spdz->reveal(temp, "player0");
    P2Pchannel::mychnl->flush_all();
    //b_data ^ maskbit
    //spdz2->add(b_data, shared_b2);((revealed_data[j].get_ui() >> i) & 1)
    std::vector<std::vector<uint8_t>> datas(A_data.size());
    std::vector<uint8_t> temps(A_data.size() * 64);
    for(int i = 0; i < A_data.size(); i++){
        for(int j = 0; j < 32; j++){
            if(Config::myconfig->check("player0")){
                //std::cout<<((revealed_data[i].get_ui() >> j) & 1)<<std::endl;
                bmr->Lambda_bits[i* bmr->num_gate + j] = bmr->maskbits[i* bmr->num_gate + j] ^ ((revealed_data[i].get_ui() >> j) & 1);
            }else{
                bmr->Lambda_bits[i* bmr->num_gate + j] = bmr->maskbits[i* bmr->num_gate + j];
            }
            bmr->Lambda_bits[i* bmr->num_gate + 32 + j] = bmr->maskbits[i* bmr->num_gate + 32 + j] ^ (b_data[i * 32 + j].b % 2);
            temps[i * 64 + j] = bmr->Lambda_bits[i* bmr->num_gate + j];
            temps[i * 64 + 32 + j] = bmr->Lambda_bits[i* bmr->num_gate + 32 + j];
            // std::cout<<(int)temps[i * 64 + j]<<" "<<(int)temps[i * 64 + 32 + j];
        }
        
    }
    P2Pchannel::mychnl->bloadcast(temps.data(), temps.size());
    std::map<std::string, std::vector<uint8_t>> rets_Lam = P2Pchannel::mychnl->recv_all(temps.size());
    P2Pchannel::mychnl->flush_all();
    for(auto & ele: rets_Lam){
        for(int i = 0; i < A_data.size(); i++){
            for(int j = 0; j < 32; j++){
                bmr->Lambda_bits[i* bmr->num_gate + j] ^= ele.second[i * 64 + j];
                bmr->Lambda_bits[i* bmr->num_gate + 32 + j] ^= ele.second[i * 64 + 32 + j];
            }
        }
    }
    // for(int i = 0; i < A_data.size(); i++){
    //         for(int j = 0; j < 32; j++){
    //             std::cout<<(int)bmr->Lambda_bits[i* bmr->num_gate + j]<<" "<<(int)bmr->Lambda_bits[i* bmr->num_gate + 32 + j];
    //         }
            
    //     }
    bmr->online();
    P2Pchannel::mychnl->flush_all();
    return bmr->res;
    // std::vector<uint8_t> revealed_b = spdz2->reveal(bmr->res, "player0");
    // P2Pchannel::mychnl->flush_all();
    // for(int i = 0; i < A_data.size(); i++){
    //     for(int j = 0; j < 32; j++){
    //         std::cout<<(int)revealed_b[i *32 + j]<<" ";
    //     }
    // }
    // //bmr-> online(temp, b_data)
    // return revealed_data;

}

void Audit_AY::run_cir_offline(std::string cir, uint32_t lens){
    data_lens = lens;
    tbmr = std::make_shared<BMR>(cir, lens, "player0");
    tbmr -> offline(lens);
}

std::vector<Xmaterial> Audit_AY::run_cir(std::vector<uint8_t> data){
    for(int i = 0; i < data_lens; i++){
        
        for(int j = 0; j < tbmr->in_size; j ++){
            tbmr->Lambda_bits[i* bmr->num_gate + j] = data[i * tbmr->in_size + j];
        }
    }
    tbmr->online(true);
    P2Pchannel::mychnl->flush_all();
    return tbmr->res;
}

std::vector<Xmaterial> Audit_AY::get_mask(uint32_t len){
    std::vector<Xmaterial> ret;
    // std::cout<<tbmr->Amaskbits.size()<<" "<<tbmr->in_size<<" "<<len<<std::endl;
    for(int j = 0; j < len; j++){
        for(int i = 0; i < tbmr->in_size; i++){
            //std::cout<<i + j * tbmr->num_gate<<std::endl;
            ret.push_back(tbmr->Amaskbits[i + j * tbmr->num_gate]);
        }
    }
    return ret;
}
std::vector<Amaterial> relu(std::vector<Amaterial> arr){
    std::shared_ptr<Audit_AY> audit = std::make_shared<Audit_AY>();
    mpz_class threshold = Commitment<>::q/2;
    uint32_t threshold_t = threshold.get_ui();
    audit->offline(arr.size());
    std::vector<Xmaterial> res = audit->transfer(arr);
    audit->run_cir_offline("../cir/comparator_32bit_unsigned_lteq.txt", arr.size());
    std::vector<Xmaterial> maskbits = audit->get_mask(arr.size());
    std::vector<Xmaterial> in_data;
    for(int i = 0; i < arr.size(); i++){
        for(int j = 0; j < 32; j++){
            in_data.push_back(audit->spdz2->add(res[i * 32 + j], maskbits[i * 64 + j]));
        }
        for(int j = 0; j < 32; j++){
            if((threshold_t>>j) % 2 == 1 && Config::myconfig->check("player0")){
                in_data.push_back(audit->spdz2->flap(maskbits[i * 64 + 32 + j]));
            }else{
                in_data.push_back(maskbits[i * 64 + 32 + j]);
            }
        }
    }
    std::vector<uint8_t> in_data_x = audit->spdz2->reveal(in_data, "player0");
    std::vector<Xmaterial> last_ret = audit->run_cir(in_data_x);
    std::vector<uint8_t> revealed_b = audit->spdz2->reveal(last_ret, "player0");

    return arr;
}

std::vector<Amaterial> bit2A(std::vector<Xmaterial> arr, std::shared_ptr<SPDZXOR> temp){
    //offline
    Timer::record("bit2A");
    std::vector<Xmaterial> alpha(arr.size());
    std::vector<Amaterial> beta(arr.size());
    std::vector<Xmaterial> gama = temp->add(arr, alpha);
    temp->reveal(gama, "player0");
    Timer::stop("bit2A");
    return beta;

    
}
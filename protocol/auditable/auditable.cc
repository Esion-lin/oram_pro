#include "auditable.h"
void Audit_AY::offline(uint32_t lens){
    // ->
    //use mod p circuit 
    bmr = std::make_shared<BMR>("../cir/add_32.txt", lens);

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
std::vector<mpz_class> Audit_AY::transfer(std::vector<Amaterial> A_data){
    std::vector<Amaterial> temp = spdz->add(A_data, spdz->neg(a_data));
    std::vector<mpz_class> revealed_data = spdz->reveal(temp, "player0");
    //b_data ^ maskbit
    //spdz2->add(b_data, shared_b2);((revealed_data[j].get_ui() >> i) & 1)
    std::vector<std::vector<uint8_t>> datas(A_data.size());
    std::vector<uint8_t> temps(A_data.size() * 32);
    for(int i = 0; i < A_data.size(); i++){
        for(int j = 0; j < 32; j++){
            bmr->Lambda_bits[i* bmr->num_gate + j] = bmr->opened_bits[i* bmr->num_gate + j] ^ ((revealed_data[i].get_ui() >> j) & 1);
            bmr->Lambda_bits[i* bmr->num_gate + 32 + j] = bmr->maskbits[i* bmr->num_gate + j] ^ b_data[i * 32 + j].b;
            temps[i * 32 + j] = bmr->Lambda_bits[i* bmr->num_gate + 32 + j];
        }
        
    }
    P2Pchannel::mychnl->bloadcast(temps.data(), temps.size());
    std::map<std::string, std::vector<uint8_t>> rets_Lam = P2Pchannel::mychnl->recv_all(temps.size());
    P2Pchannel::mychnl->flush_all();
    for(auto & ele: rets_Lam){
        for(int i = 0; i < A_data.size(); i++){
            for(int j = 0; j < 32; j++){
                bmr->Lambda_bits[i* bmr->num_gate + 32 + j] ^= temps[i * 32 + j];
            }
            
        }
    }
    for(int i = 0; i < 32; i++){
        for(int j = 0; j < A_data.size(); j++){
            //b_data[j*32 + i].b ^ bmr->maskbits[j*64 + i];
        }
    }

    //bmr-> online(temp, b_data)
    return revealed_data;

}

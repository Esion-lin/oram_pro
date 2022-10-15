#include "BMR.h"
#include "timer.hpp"
bool check_equal(block a, block b){
    return _mm_test_all_ones(_mm_cmpeq_epi8(a,b));
    // __m128i vcmp = (__m128i)_mm_cmpneq_ps(a, b); // compare a, b for inequality
    // uint16_t test = _mm_movemask_epi8(vcmp); // extract results of comparison
    // return test == 0;
}
BMR::BMR(std::string cir_file){
    cf = new BristolFormat(cir_file.c_str());
    /*cf->n1/n2 -> wires party1/party2 held */
    Ks_pickeds.resize(Config::myconfig->get_players_num());
    maskbits.resize(cf->num_wire);
    opened_bits.resize(cf->num_wire);
    Lambda_bits.resize(cf->num_wire);
    Ks.resize(cf->num_wire*2);
    /*
    random -> maskbits
    random -> Ks[cf.n1]
    */
    RAND_bytes(reinterpret_cast<uint8_t*>(&R), sizeof(block));
    RAND_bytes(maskbits.data(), cf->num_wire);
    
    for(int i = 0; i < maskbits.size(); i++){
        maskbits[i] %= 2;
    }
    
    in_size = cf->n1 + cf->n2;
    for(int i = 0; i < in_size; i++){
        RAND_bytes(reinterpret_cast<uint8_t*>(&Ks[i * 2]), sizeof(block));
        Ks[i * 2 + 1] = R ^ Ks[i * 2];
    }

}
BMR::BMR(std::string cir_file, uint32_t lens){
    contained_ = true;
    cf = new BristolFormat(cir_file.c_str());
    num_gate = cf->num_gate;
    /*cf->n1/n2 -> wires party1/party2 held */
    Ks_pickeds.resize(lens*Config::myconfig->get_players_num());
    maskbits.resize(lens*cf->num_wire);
    opened_bits.resize(lens*cf->num_wire);
    Lambda_bits.resize(lens*cf->num_wire);
    Ks.resize(lens*cf->num_wire*2);
    /*
    random -> maskbits
    random -> Ks[cf.n1]
    */
    RAND_bytes(reinterpret_cast<uint8_t*>(&R), sizeof(block));
    RAND_bytes(maskbits.data(), lens*cf->num_wire);
    
    for(int i = 0; i < maskbits.size(); i++){
        maskbits[i] %= 2;
    }
    
    int itr = cf->n1 + cf->n2;
    for(int k = 0; k < lens; k++)
    for(int i = 0; i < itr; i++){
        RAND_bytes(reinterpret_cast<uint8_t*>(&Ks[2*k*itr + i * 2]), sizeof(block));
        Ks[2*k*cf->num_wire + i * 2 + 1] = R ^ Ks[2*k*cf->num_wire +  i * 2];
    }

}

void BMR::online(std::map<std::string, std::pair<int, int>>holder, std::vector<uint8_t> data){
    //holder
    //broad cast \Lambda
    int itr = cf->n1 + cf->n2;
    // for(int i = 0; i< itr * 2; i ++){
    //     std::cout<<(uint32_t)opened_bits[i]<<" ";
    // }
    for(auto & ele: holder){
        if(Config::myconfig->check(ele.first)){
            assert(data.size() == (ele.second.second - ele.second.first));
            for(int i = ele.second.first; i < ele.second.second; i++){
                Lambda_bits[i] = data[i - ele.second.first] ^ opened_bits[i];
            }
            // for(auto & subele : Config::myconfig->Pmap){
            //     if(subele.first != ele.first)
            //     P2Pchannel::mychnl->send_data_to(subele.first, Lambda_bits.data() + ele.second.first, ele.second.second - ele.second.first);
            // }
            P2Pchannel::mychnl->bloadcast(Lambda_bits.data() + ele.second.first, ele.second.second - ele.second.first);
            
        }
        else{
            P2Pchannel::mychnl->recv_data_from(ele.first,Lambda_bits.data() + ele.second.first, ele.second.second - ele.second.first);
        }
    }
    //broadcast kw
    
    Ks_pickeds[Config::myconfig->get_idex()].resize(cf->num_wire);
    for(int i = 0; i < itr; i ++){
        Ks_pickeds[Config::myconfig->get_idex()][i] = Ks[i*2 + Lambda_bits[i]];
        
        
    }
    // std::cout<<(uint32_t)reinterpret_cast<uint8_t*>(Ks_pickeds[Config::myconfig->get_idex()].data())[0]<<" \n";
    P2Pchannel::mychnl->bloadcast(reinterpret_cast<uint8_t*>(Ks_pickeds[Config::myconfig->get_idex()].data()), itr * sizeof(block));
    std::map<std::string, std::vector<uint8_t>> rets  = P2Pchannel::mychnl->recv_all(itr * sizeof(block));
    for(auto & ele : rets){
        // std::cout<<"idex"<<Config::get_idex(ele.first)<<std::endl;
        Ks_pickeds[Config::get_idex(ele.first)].resize(cf->num_wire);
        
        memcpy(reinterpret_cast<uint8_t*>(Ks_pickeds[Config::get_idex(ele.first)].data()), ele.second.data(), itr * sizeof(block));
        // std::cout<<(uint32_t)ele.second.data()[0]<<" \n";
    }
    //eval
    int itr_tab = 0;
    for(int i = 0; i < cf->num_gate; ++i) {
        if(cf->gates[4*i+3] == AND_GATE) {
            uint8_t arr[40];
            
            for(int j = 0; j < Config::myconfig->get_players_num(); j++){
                block temp = {0,0};
                // std::cout<<tabs[itr_tab].t[Lambda_bits[cf->gates[4*i + 1]] * 2 + Lambda_bits[cf->gates[4*i]]];
                for(int k = 0; k < Config::myconfig->get_players_num(); k++){
                    memcpy(arr, &Ks_pickeds[k][cf->gates[4*i]], 16);
                    memcpy(arr+16, &Ks_pickeds[k][cf->gates[4*i+1]], 16);
                    memcpy(arr+32, &i, 4);
                    memcpy(arr+36, &j, 4);
                    temp ^= Hash::hash_for_block(arr, 40);
                    // std::cout<<"hash "<<Ks_pickeds[k][cf->gates[4*i]]<< " "<< Ks_pickeds[k][cf->gates[4*i+1]]<<std::endl;
                }
                // printf("%d %d\n",Lambda_bits[cf->gates[4*i + 1]] * 2 + Lambda_bits[cf->gates[4*i]], itr_tab);
                Ks_pickeds[j][cf->gates[4*i+2]] = tabs[itr_tab ++].t[Lambda_bits[cf->gates[4*i + 1]] * 2 + Lambda_bits[cf->gates[4*i]]] ^ temp;
                // std::cout<<"res "<<Ks_pickeds[j][cf->gates[4*i+2]]<< " " << Ks[cf->gates[4*i + 2] * 2] << " " << (Ks[cf->gates[4*i + 2] * 2] ^ R) << std::endl;
                
                if(Config::myconfig->get_idex() == j){
                    //judge Lambda
                    if(check_equal(Ks_pickeds[j][cf->gates[4*i+2]], Ks[cf->gates[4*i + 2] * 2])){
                        Lambda_bits[cf->gates[4*i+2]] = 0;
                        
                    }else if(check_equal(Ks_pickeds[j][cf->gates[4*i+2]], Ks[cf->gates[4*i + 2] * 2] ^ R)){
                        Lambda_bits[cf->gates[4*i+2]] = 1;
                    }else{
                        
                        // std::cout<<Ks_pickeds[j][cf->gates[4*i+2]]<< " " << Ks[cf->gates[4*i + 2] * 2] << " " << (Ks[cf->gates[4*i + 2] * 2] ^ R) << std::endl;
                        // std::cout<<"detected croupted!!! "<<i<<" "<<cf->gates[4*i]<<" "<<Ks_pickeds[j][cf->gates[4*i]]<<" "<<cf->gates[4*i+1]<<" "<<Ks_pickeds[j][cf->gates[4*i+1]]<<" "<<Hash::hash_for_block(arr, 40)<<std::endl;
                    std::cout<<"detected croupted!!! "<< i <<" \n";
                    throw std::invalid_argument( "received negative value" ); 
                    }
                }
            }
        }else if (cf->gates[4*i+3] == XOR_GATE){
            Lambda_bits[cf->gates[4*i+2]] = Lambda_bits[cf->gates[4*i]] ^ Lambda_bits[cf->gates[4*i+1]];
            for(int j = 0; j < Config::myconfig->get_players_num(); j++){
                Ks_pickeds[j][cf->gates[4*i+2]] = Ks_pickeds[j][cf->gates[4*i]] ^ Ks_pickeds[j][cf->gates[4*i+1]];
            }
        }else{
            
            //INV
            Lambda_bits[cf->gates[4*i+2]] = Lambda_bits[cf->gates[4*i]];
            // std::cout<<(uint32_t)(Lambda_bits[cf->gates[4*i+2]] ^ opened_bits[cf->gates[4*i+2]])<<" ";
            for(int j = 0; j < Config::myconfig->get_players_num(); j++){
                Ks_pickeds[j][cf->gates[4*i+2]] = Ks_pickeds[j][cf->gates[4*i]];
            }
        }
    }
    for(int i = cf->num_wire - cf->n3; i < cf->num_wire; i++){
        std::cout<<(uint32_t)(Lambda_bits[i] ^ opened_bits[i])<<" ";
    }
    std::cout<<std::endl;
    // for(int i = cf->num_wire - cf->n3 - 1; i < cf->num_wire; i++){
    //     std::cout<<(uint32_t)opened_bits[i]<<" ";
    // }

}


void BMR::online(std::map<std::string, std::pair<int, int>>holder, std::vector<std::vector<uint8_t>> datas, uint32_t lens){
    //holder
    //broad cast \Lambda
    int itr = cf->n1 + cf->n2;
    // for(int i = 0; i< itr * 2; i ++){
    //     std::cout<<(uint32_t)opened_bits[i]<<" ";
    // }
    for(auto & ele: holder){
        if(Config::myconfig->check(ele.first)){
            //assert(data.size() == (ele.second.second - ele.second.first));
            std::vector<uint8_t> temp;
            for(int k = 0; k < lens; k ++){
                for(int i = ele.second.first; i < ele.second.second; i++){
                    Lambda_bits[k * num_gate + i] = datas[k][i - ele.second.first] ^ opened_bits[k * num_gate + i];
                    temp.push_back(Lambda_bits[k * num_gate + i]);
                }

            }
            // for(auto & subele : Config::myconfig->Pmap){
            //     if(subele.first != ele.first)
            //     P2Pchannel::mychnl->send_data_to(subele.first, Lambda_bits.data() + ele.second.first, ele.second.second - ele.second.first);
            // }
            P2Pchannel::mychnl->bloadcast(temp.data(), temp.size());
            std::cout<<"send data"<<temp.size()<<std::endl;
        }
        else{
            std::vector<uint8_t> temp; int itra = 0;
            temp.resize(lens * (ele.second.second - ele.second.first));
            P2Pchannel::mychnl->recv_data_from(ele.first,temp.data(), temp.size());
            std::cout<<"recv data"<<temp.size()<<std::endl;
            for(int k = 0; k < lens; k ++){
                for(int i = ele.second.first; i < ele.second.second; i++){
                    Lambda_bits[k * num_gate + i] = temp[itra++];
                }

            }
        }
    }
    P2Pchannel::mychnl->flush_all();
    //broadcast kw
    block temp_blk[itr * lens];
    Ks_pickeds[Config::myconfig->get_idex()].resize(cf->num_wire * lens);

    for(int k = 0; k < lens; k++)
    for(int i = 0; i < itr; i ++){
        Ks_pickeds[Config::myconfig->get_idex()][i + k*num_gate] = Ks[i*2 + Lambda_bits[i] + k*num_gate * 2]; 
        temp_blk[k * itr + i] = Ks[i*2 + Lambda_bits[i] + k*num_gate * 2];
    }
    // std::cout<<(uint32_t)reinterpret_cast<uint8_t*>(Ks_pickeds[Config::myconfig->get_idex()].data())[0]<<" \n";
    P2Pchannel::mychnl->bloadcast(reinterpret_cast<uint8_t*>(temp_blk), lens*itr * sizeof(block));
    std::map<std::string, std::vector<uint8_t>> rets  = P2Pchannel::mychnl->recv_all(lens*itr * sizeof(block));
    for(auto & ele : rets){
        // std::cout<<"idex"<<Config::get_idex(ele.first)<<std::endl;
        Ks_pickeds[Config::get_idex(ele.first)].resize(cf->num_wire * lens);
        for(int i = 0; i < lens; i++){  
            memcpy(reinterpret_cast<uint8_t*>(Ks_pickeds[Config::get_idex(ele.first)].data() + i * num_gate), ele.second.data() + i * itr, itr * sizeof(block));
        }
        
        // std::cout<<(uint32_t)ele.second.data()[0]<<" \n";
    }
    //eval
    int itr_tab = 0;
    for(int w = 0; w < lens; w++)
    for(int i = 0; i < cf->num_gate; ++i) {
        if(cf->gates[4*i+3] == AND_GATE) {
            uint8_t arr[40];
            
            for(int j = 0; j < Config::myconfig->get_players_num(); j++){
                block temp = {0,0};
                // std::cout<<tabs[itr_tab].t[Lambda_bits[cf->gates[4*i + 1]] * 2 + Lambda_bits[cf->gates[4*i]]];
                for(int k = 0; k < Config::myconfig->get_players_num(); k++){
                    memcpy(arr, &Ks_pickeds[k][w*num_gate + cf->gates[4*i]], 16);
                    memcpy(arr+16, &Ks_pickeds[k][w*num_gate + cf->gates[4*i+1]], 16);
                    memcpy(arr+32, &i, 4);
                    memcpy(arr+36, &j, 4);
                    temp ^= Hash::hash_for_block(arr, 40);
                    // std::cout<<"hash "<<Ks_pickeds[k][cf->gates[4*i]]<< " "<< Ks_pickeds[k][cf->gates[4*i+1]]<<std::endl;
                }
                // printf("%d %d\n",Lambda_bits[cf->gates[4*i + 1]] * 2 + Lambda_bits[cf->gates[4*i]], itr_tab);
                Ks_pickeds[j][w*num_gate + cf->gates[4*i+2]] = tabs[itr_tab ++].t[Lambda_bits[w*num_gate + cf->gates[4*i + 1]] * 2 + Lambda_bits[w*num_gate + cf->gates[4*i]]] ^ temp;
                // std::cout<<"res "<<Ks_pickeds[j][cf->gates[4*i+2]]<< " " << Ks[cf->gates[4*i + 2] * 2] << " " << (Ks[cf->gates[4*i + 2] * 2] ^ R) << std::endl;
                
                if(Config::myconfig->get_idex() == j){
                    //judge Lambda
                    if(check_equal(Ks_pickeds[j][w*num_gate + cf->gates[4*i+2]], Ks[w*num_gate * 2 + cf->gates[4*i + 2] * 2])){
                        Lambda_bits[w*num_gate + cf->gates[4*i+2]] = 0;
                        
                    }else if(check_equal(Ks_pickeds[j][w*num_gate + cf->gates[4*i+2]], Ks[w*num_gate * 2 + cf->gates[4*i + 2] * 2] ^ R)){
                        Lambda_bits[w*num_gate + cf->gates[4*i+2]] = 1;
                    }else{
                        
                        // std::cout<<Ks_pickeds[j][cf->gates[4*i+2]]<< " " << Ks[cf->gates[4*i + 2] * 2] << " " << (Ks[cf->gates[4*i + 2] * 2] ^ R) << std::endl;
                        // std::cout<<"detected croupted!!! "<<i<<" "<<cf->gates[4*i]<<" "<<Ks_pickeds[j][cf->gates[4*i]]<<" "<<cf->gates[4*i+1]<<" "<<Ks_pickeds[j][cf->gates[4*i+1]]<<" "<<Hash::hash_for_block(arr, 40)<<std::endl;
                    std::cout<<"detected croupted!!! "<< w<< " " << i <<" \n";
                    throw std::invalid_argument( "received negative value" ); 
                    }
                }
            }
        }else if (cf->gates[4*i+3] == XOR_GATE){
            Lambda_bits[w*num_gate + cf->gates[4*i+2]] = Lambda_bits[w*num_gate + cf->gates[4*i]] ^ Lambda_bits[w*num_gate + cf->gates[4*i+1]];
            for(int j = 0; j < Config::myconfig->get_players_num(); j++){
                Ks_pickeds[j][w*num_gate + cf->gates[4*i+2]] = Ks_pickeds[j][w*num_gate + cf->gates[4*i]] ^ Ks_pickeds[j][w*num_gate + cf->gates[4*i+1]];
            }
        }else{
            
            //INV
            Lambda_bits[w*num_gate + cf->gates[4*i+2]] = Lambda_bits[w*num_gate + cf->gates[4*i]];
            // std::cout<<(uint32_t)(Lambda_bits[cf->gates[4*i+2]] ^ opened_bits[cf->gates[4*i+2]])<<" ";
            for(int j = 0; j < Config::myconfig->get_players_num(); j++){
                Ks_pickeds[j][w*num_gate + cf->gates[4*i+2]] = Ks_pickeds[j][w*num_gate + cf->gates[4*i]];
            }
        }
    }
    for(int w = 0; w < lens; w++){
        for(int i = cf->num_wire - cf->n3; i < cf->num_wire; i++){
            std::cout<<(uint32_t)(Lambda_bits[w*num_gate + i] ^ opened_bits[w*num_gate + i])<<" ";
        }
        std::cout<<std::endl;
    }
    // for(int i = cf->num_wire - cf->n3 - 1; i < cf->num_wire; i++){
    //     std::cout<<(uint32_t)opened_bits[i]<<" ";
    // }

}
void BMR::online(){
    int itr = cf->n1 + cf->n2;
    // for(int i = 0; i< itr * 2; i ++){
    //     std::cout<<(uint32_t)opened_bits[i]<<" ";
    // }
    //broadcast kw
    block temp_blk[itr * offline_lens];
    Ks_pickeds[Config::myconfig->get_idex()].resize(cf->num_wire * offline_lens);

    for(int k = 0; k < offline_lens; k++)
    for(int i = 0; i < itr; i ++){
        Ks_pickeds[Config::myconfig->get_idex()][i + k*num_gate] = Ks[i*2 + Lambda_bits[i] + k*num_gate * 2]; 
        temp_blk[k * itr + i] = Ks[i*2 + Lambda_bits[i] + k*num_gate * 2];
    }
    // std::cout<<(uint32_t)reinterpret_cast<uint8_t*>(Ks_pickeds[Config::myconfig->get_idex()].data())[0]<<" \n";
    P2Pchannel::mychnl->bloadcast(reinterpret_cast<uint8_t*>(temp_blk), offline_lens*itr * sizeof(block));
    std::map<std::string, std::vector<uint8_t>> rets  = P2Pchannel::mychnl->recv_all(offline_lens*itr * sizeof(block));
    
    for(auto & ele : rets){
        // std::cout<<"idex"<<Config::get_idex(ele.first)<<std::endl;
        Ks_pickeds[Config::get_idex(ele.first)].resize(cf->num_wire * offline_lens);
        for(int i = 0; i < offline_lens; i++){  
            memcpy(reinterpret_cast<uint8_t*>(Ks_pickeds[Config::get_idex(ele.first)].data() + i * num_gate), ele.second.data() + i * itr, itr * sizeof(block));
        }
        
        // std::cout<<(uint32_t)ele.second.data()[0]<<" \n";
    }
    //eval
    int itr_tab = 0;
    for(int w = 0; w < offline_lens; w++)
    for(int i = 0; i < cf->num_gate; ++i) {
        if(cf->gates[4*i+3] == AND_GATE) {
            uint8_t arr[40];
            
            for(int j = 0; j < Config::myconfig->get_players_num(); j++){
                block temp = {0,0};
                // std::cout<<tabs[itr_tab].t[Lambda_bits[cf->gates[4*i + 1]] * 2 + Lambda_bits[cf->gates[4*i]]];
                for(int k = 0; k < Config::myconfig->get_players_num(); k++){
                    memcpy(arr, &Ks_pickeds[k][w*num_gate + cf->gates[4*i]], 16);
                    memcpy(arr+16, &Ks_pickeds[k][w*num_gate + cf->gates[4*i+1]], 16);
                    memcpy(arr+32, &i, 4);
                    memcpy(arr+36, &j, 4);
                    temp ^= Hash::hash_for_block(arr, 40);
                    // std::cout<<"hash "<<Ks_pickeds[k][cf->gates[4*i]]<< " "<< Ks_pickeds[k][cf->gates[4*i+1]]<<std::endl;
                }
                // printf("%d %d\n",Lambda_bits[cf->gates[4*i + 1]] * 2 + Lambda_bits[cf->gates[4*i]], itr_tab);
                Ks_pickeds[j][w*num_gate + cf->gates[4*i+2]] = tabs[itr_tab ++].t[Lambda_bits[w*num_gate + cf->gates[4*i + 1]] * 2 + Lambda_bits[w*num_gate + cf->gates[4*i]]] ^ temp;
                // std::cout<<"res "<<Ks_pickeds[j][cf->gates[4*i+2]]<< " " << Ks[cf->gates[4*i + 2] * 2] << " " << (Ks[cf->gates[4*i + 2] * 2] ^ R) << std::endl;
                
                if(Config::myconfig->get_idex() == j){
                    //judge Lambda
                    if(check_equal(Ks_pickeds[j][w*num_gate + cf->gates[4*i+2]], Ks[w*num_gate * 2 + cf->gates[4*i + 2] * 2])){
                        Lambda_bits[w*num_gate + cf->gates[4*i+2]] = 0;
                        
                    }else if(check_equal(Ks_pickeds[j][w*num_gate + cf->gates[4*i+2]], Ks[w*num_gate * 2 + cf->gates[4*i + 2] * 2] ^ R)){
                        Lambda_bits[w*num_gate + cf->gates[4*i+2]] = 1;
                    }else{
                        
                        // std::cout<<Ks_pickeds[j][cf->gates[4*i+2]]<< " " << Ks[cf->gates[4*i + 2] * 2] << " " << (Ks[cf->gates[4*i + 2] * 2] ^ R) << std::endl;
                        // std::cout<<"detected croupted!!! "<<i<<" "<<cf->gates[4*i]<<" "<<Ks_pickeds[j][cf->gates[4*i]]<<" "<<cf->gates[4*i+1]<<" "<<Ks_pickeds[j][cf->gates[4*i+1]]<<" "<<Hash::hash_for_block(arr, 40)<<std::endl;
                    std::cout<<"detected croupted!!! "<< w<< " " << i <<" \n";
                    throw std::invalid_argument( "received negative value" ); 
                    }
                }
            }
        }else if (cf->gates[4*i+3] == XOR_GATE){
            Lambda_bits[w*num_gate + cf->gates[4*i+2]] = Lambda_bits[w*num_gate + cf->gates[4*i]] ^ Lambda_bits[w*num_gate + cf->gates[4*i+1]];
            for(int j = 0; j < Config::myconfig->get_players_num(); j++){
                Ks_pickeds[j][w*num_gate + cf->gates[4*i+2]] = Ks_pickeds[j][w*num_gate + cf->gates[4*i]] ^ Ks_pickeds[j][w*num_gate + cf->gates[4*i+1]];
            }
        }else{
            
            //INV
            Lambda_bits[w*num_gate + cf->gates[4*i+2]] = Lambda_bits[w*num_gate + cf->gates[4*i]];
            // std::cout<<(uint32_t)(Lambda_bits[cf->gates[4*i+2]] ^ opened_bits[cf->gates[4*i+2]])<<" ";
            for(int j = 0; j < Config::myconfig->get_players_num(); j++){
                Ks_pickeds[j][w*num_gate + cf->gates[4*i+2]] = Ks_pickeds[j][w*num_gate + cf->gates[4*i]];
            }
        }
    }
    std::vector<uint8_t> res;
    for(int w = 0; w < offline_lens; w++){
        for(int i = cf->num_wire - cf->n3; i < cf->num_wire; i++){
            
            std::cout<<(uint32_t)(Lambda_bits[w*num_gate + i] ^ opened_bits[w*num_gate + i])<<" ";
        }
        std::cout<<std::endl;
    }
}

void BMR::offline(){
    //std::vector<uint8_t> opened_bits(maskbits);
    for(int i = 0; i < cf->num_gate; ++i) {
        if (cf->gates[4*i+3] == XOR_GATE){
            maskbits[cf->gates[4*i+2]] = maskbits[cf->gates[4*i]] ^ maskbits[cf->gates[4*i+1]];
        }
        else if(cf->gates[4*i+3] == NOT_GATE){
            if(Config::myconfig->check("player0"))
                maskbits[cf->gates[4*i+2]] = maskbits[cf->gates[4*i]]^1;
            else
                maskbits[cf->gates[4*i+2]] = maskbits[cf->gates[4*i]];
        }
    }
    std::copy(maskbits.begin(), maskbits.end(), opened_bits.begin());
    P2Pchannel::mychnl->bloadcast(maskbits.data(), maskbits.size());
    std::map<std::string, std::vector<uint8_t>> rets  = P2Pchannel::mychnl->recv_all(maskbits.size());
    P2Pchannel::mychnl->flush_all();
    for(auto & ele: rets){
        for(int i = 0; i < ele.second.size(); i++){
            opened_bits[i] = opened_bits[i] ^ ele.second[i];
            
        }
    }
    
    for(int i = 0; i < cf->num_gate; ++i) {
        if(cf->gates[4*i+3] == AND_GATE) {
            //maskbits[cf->gates[4*i+2]];
            RAND_bytes(reinterpret_cast<uint8_t*>(&Ks[cf->gates[4*i+2] * 2]), sizeof(block));
            Ks[cf->gates[4*i+2] * 2 + 1] = Ks[cf->gates[4*i+2] * 2] ^ R;
            uint8_t arr[40];
            /*1.share k_{w_0}, R*/
            /*2.calculate garble table*/
            //{0, 0}
            // xy+z = 1
            // (x + 1)y + z = 1
            for(int j = 0; j < Config::myconfig->get_players_num(); j++){
                gc_table temp;
                for(int k = 0; k < 4; k++){             
                    //k%2 u, k/2 v           
                    uint8_t sbit = (opened_bits[cf->gates[4*i]] ^ (k % 2)) & (opened_bits[cf->gates[4*i+1]] ^ (k / 2)) ^ opened_bits[cf->gates[4*i+2]];
                    memcpy(arr, &Ks[cf->gates[4*i] * 2 + k % 2], 16);
                    memcpy(arr+16, &Ks[cf->gates[4*i + 1] * 2 + (k / 2)], 16);
                    memcpy(arr+32, &i, 4);
                    memcpy(arr+36, &j, 4);
                    temp.t[k] = Hash::hash_for_block(arr, 40);
                    // if(i == 64){
                    //     std::cout<<"k0 "<<Ks[cf->gates[4*i] * 2 + k % 2]<<" k1 "<<Ks[cf->gates[4*i + 1] * 2 + (k / 2)]<<" res "<<temp.t[k];
                    // }
                    if(Config::myconfig->get_idex() == j){
                        //^ k_{w, 0}
                        if(sbit)
                            temp.t[k] = temp.t[k] ^ Ks[cf->gates[4*i + 2] * 2] ^ R;
                        else
                            temp.t[k] = temp.t[k] ^ Ks[cf->gates[4*i + 2] * 2];
                    }
                }
                tabs.push_back(temp);
            }
            

        }else if (cf->gates[4*i+3] == XOR_GATE){
            // maskbits[cf->gates[4*i+2]] = maskbits[cf->gates[4*i]] ^ maskbits[cf->gates[4*i+1]];
            Ks[cf->gates[4*i+2] * 2] = Ks[cf->gates[4*i] * 2] ^ Ks[cf->gates[4*i+1] * 2] ;
            Ks[cf->gates[4*i+2] * 2 + 1] = Ks[cf->gates[4*i+2] * 2] ^ R;
        }else{
            //INV
            Ks[cf->gates[4*i+2] * 2] = Ks[cf->gates[4*i] * 2];
            Ks[cf->gates[4*i+2] * 2 + 1] = Ks[cf->gates[4*i] * 2 + 1];
        }
    }
    //reveal tab
    P2Pchannel::mychnl->bloadcast(reinterpret_cast<uint8_t*>(tabs.data()), sizeof(gc_table)*tabs.size());
    std::map<std::string, std::vector<uint8_t>> rets2  = P2Pchannel::mychnl->recv_all(sizeof(gc_table)*tabs.size());
    P2Pchannel::mychnl->flush_all();
    for(auto & ele: rets2){
        gc_table temp[tabs.size()];
        memcpy(temp, ele.second.data(), sizeof(gc_table)*tabs.size());
        for(int i = 0; i < tabs.size(); i++){
            tabs[i].t[0] = tabs[i].t[0] ^ temp[i].t[0];
            tabs[i].t[1] = tabs[i].t[1] ^ temp[i].t[1];
            tabs[i].t[2] = tabs[i].t[2] ^ temp[i].t[2];
            tabs[i].t[3] = tabs[i].t[3] ^ temp[i].t[3];
        }
    }
}

void BMR::offline(uint32_t lens){
    assert(contained_);
    offline_lens = lens;
    //std::vector<uint8_t> opened_bits(maskbits);
    for(int k = 0; k < lens; k ++)
    for(int i = 0; i < cf->num_gate; ++i) {
        if (cf->gates[4*i+3] == XOR_GATE){
            maskbits[num_gate * k + cf->gates[4*i+2]] = maskbits[num_gate * k + cf->gates[4*i]] ^ maskbits[num_gate * k + cf->gates[4*i+1]];
        }
        else if(cf->gates[4*i+3] == NOT_GATE){
            if(Config::myconfig->check("player0")){
                maskbits[num_gate * k + cf->gates[4*i+2]] = maskbits[num_gate * k + cf->gates[4*i]]^1;
                }
            else
                maskbits[num_gate * k + cf->gates[4*i+2]] = maskbits[num_gate * k + cf->gates[4*i]];
        }
    }
    std::copy(maskbits.begin(), maskbits.end(), opened_bits.begin());
    P2Pchannel::mychnl->bloadcast(maskbits.data(), maskbits.size());
    std::map<std::string, std::vector<uint8_t>> rets  = P2Pchannel::mychnl->recv_all(maskbits.size());
    P2Pchannel::mychnl->flush_all();
    for(auto & ele: rets){
        for(int i = 0; i < ele.second.size(); i++){
            opened_bits[i] = opened_bits[i] ^ ele.second[i];
            
        }
    }
    for(int w = 0; w < lens; w++)
    for(int i = 0; i < cf->num_gate; ++i) {
        if(cf->gates[4*i+3] == AND_GATE) {
            //maskbits[cf->gates[4*i+2]];
            RAND_bytes(reinterpret_cast<uint8_t*>(&Ks[w * num_gate * 2 + cf->gates[4*i+2] * 2]), sizeof(block));
            Ks[w * num_gate * 2 + cf->gates[4*i+2] * 2 + 1] = Ks[w * num_gate * 2 + cf->gates[4*i+2] * 2] ^ R;
            uint8_t arr[40];
            /*1.share k_{w_0}, R*/
            /*2.calculate garble table*/
            //{0, 0}
            // xy+z = 1
            // (x + 1)y + z = 1
            for(int j = 0; j < Config::myconfig->get_players_num(); j++){
                gc_table temp;
                for(int k = 0; k < 4; k++){             
                    //k%2 u, k/2 v           
                    uint8_t sbit = (opened_bits[w * num_gate + cf->gates[4*i]] ^ (k % 2)) & (opened_bits[w * num_gate + cf->gates[4*i+1]] ^ (k / 2)) ^ opened_bits[w * num_gate + cf->gates[4*i+2]];
                    memcpy(arr, &Ks[w * num_gate * 2 + cf->gates[4*i] * 2 + k % 2], 16);
                    memcpy(arr+16, &Ks[w * num_gate * 2 + cf->gates[4*i + 1] * 2 + (k / 2)], 16);
                    memcpy(arr+32, &i, 4);
                    memcpy(arr+36, &j, 4);
                    temp.t[k] = Hash::hash_for_block(arr, 40);
                    // if(i == 64){
                    //     std::cout<<"k0 "<<Ks[cf->gates[4*i] * 2 + k % 2]<<" k1 "<<Ks[cf->gates[4*i + 1] * 2 + (k / 2)]<<" res "<<temp.t[k];
                    // }
                    if(Config::myconfig->get_idex() == j){
                        //^ k_{w, 0}
                        if(sbit)
                            temp.t[k] = temp.t[k] ^ Ks[w * num_gate * 2 + cf->gates[4*i + 2] * 2] ^ R;
                        else
                            temp.t[k] = temp.t[k] ^ Ks[w * num_gate * 2 + cf->gates[4*i + 2] * 2];
                    }
                }
                tabs.push_back(temp);
            }
            

        }else if (cf->gates[4*i+3] == XOR_GATE){
            // maskbits[cf->gates[4*i+2]] = maskbits[cf->gates[4*i]] ^ maskbits[cf->gates[4*i+1]];
            Ks[w * num_gate * 2 + cf->gates[4*i+2] * 2] = Ks[w * num_gate * 2 + cf->gates[4*i] * 2] ^ Ks[w * num_gate * 2 + cf->gates[4*i+1] * 2] ;
            Ks[w * num_gate * 2 + cf->gates[4*i+2] * 2 + 1] = Ks[w * num_gate * 2 + cf->gates[4*i+2] * 2] ^ R;
        }else{
            //INV
            Ks[w * num_gate * 2 + cf->gates[4*i+2] * 2] = Ks[w * num_gate * 2 + cf->gates[4*i] * 2];
            Ks[w * num_gate * 2 + cf->gates[4*i+2] * 2 + 1] = Ks[w * num_gate * 2 + cf->gates[4*i] * 2 + 1];
        }
    }
    //reveal tab
    P2Pchannel::mychnl->bloadcast(reinterpret_cast<uint8_t*>(tabs.data()), sizeof(gc_table)*tabs.size());
    std::map<std::string, std::vector<uint8_t>> rets2  = P2Pchannel::mychnl->recv_all(sizeof(gc_table)*tabs.size());
    P2Pchannel::mychnl->flush_all();
    for(auto & ele: rets2){
        gc_table temp[tabs.size()];
        memcpy(temp, ele.second.data(), sizeof(gc_table)*tabs.size());
        for(int i = 0; i < tabs.size(); i++){
            tabs[i].t[0] = tabs[i].t[0] ^ temp[i].t[0];
            tabs[i].t[1] = tabs[i].t[1] ^ temp[i].t[1];
            tabs[i].t[2] = tabs[i].t[2] ^ temp[i].t[2];
            tabs[i].t[3] = tabs[i].t[3] ^ temp[i].t[3];
        }
    }
}
#pragma once
#include "net.hpp"
#include "emp-tool/emp-tool.h"
#include "emp-ot/emp-ot.h"
#include <openssl/rand.h>
//using gc_table=std::array<block, 4>;
struct gc_table{
    block t[4];
}
//template< int Lambda = 128>
class BMR{
    
    public:
    BristolFormat *cf;
    std::vector<uint8_t> maskbits;
    std::vector<block> Ks;
    std::vector<gc_table> tabs;
    std::shared_ptr<SPDZ> spdz = std::make_shared<SPDZ>();
    block R;
    BMR(std::string cir_file){
        cf = new BristolFormat("../cir/Ghash.txt");
        /*cf->n1/n2 -> wires party1/party2 held */

        maskbits.resize(cf->num_wire);
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
        int itr = cf->n1 + cf->n2;
        for(int i = 0; i < itr; i++){
            RAND_bytes(reinterpret_cast<uint8_t*>(&Ks[i * 2]), sizeof(block));
            Ks[i * 2 + 1] = R ^ Ks[i * 2];
        }

    }
    void online(){
        //broad cast \Alpha

    }
    void offline(){
        std::vector<uint8_t> opened_bits(maskbits);
        P2Pchannel::mychnl->bloadcast(maskbits.data(), maskbits.size());
        std::map<std::string, std::vector<uint8_t>> rets  = P2Pchannel::mychnl->recv_all(maskbits.size());
        for(auto & ele: rets){
            for(int i = 0; i < a.size() * 2; i++){
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
                gc_table temp;
                for(int k = 0; k < 4; k++){
                    uint8_t sbit = (maskbits[cf->gates[4*i]] ^ (k % 2)) & (maskbits[cf->gates[4*i+1]] ^ (k / 2)) ^ maskbits[cf->gates[4*i+2]]
                    memcpy(arr, &Ks[cf->gates[4*i] * 2 + k % 2], 16);
                    memcpy(arr+16, &Ks[cf->gates[4*i + 1] * 2 + (k / 2)], 16);
                    memcpy(arr+32, &i, 4);
                    for(int j = 0; j < Config::myconfig->get_players_num(); i++){
                        memcpy(arr+36, &j, 4);
                        temp.t[k] = Hash::hash_for_block(arr, 40);
                        if(Config::myconfig->get_idex() == j){
                            //^ k_{w, 0}
                            if(sbit)
                                temp.t[k] = temp.t[k] ^ Ks[cf->gates[4*i + 2] * 2] ^ R;
                            else
                                temp.t[k] = temp.t[k] ^ Ks[cf->gates[4*i + 2] * 2];
                        }
                            
                    }
                }
                tabs.push_back(temp);

                

            }else if (cf->gates[4*i+3] == XOR_GATE){
                maskbits[cf->gates[4*i+2]] = maskbits[cf->gates[4*i]] ^ maskbits[cf->gates[4*i+1]];
                Ks[cf->gates[4*i+2] * 2] = Ks[cf->gates[4*i] * 2] ^ Ks[cf->gates[4*i+1] * 2] ;
                Ks[cf->gates[4*i+2] * 2 + 1] = Ks[cf->gates[4*i+2] * 2] ^ R;
            }
        }
        //reveal tab
        P2Pchannel::mychnl->bloadcast(reinterpret_cast<uint8_t*>(tabs.data()), sizeof(gc_table)*tabs.size());
        std::map<std::string, std::vector<uint8_t>> rets  = P2Pchannel::mychnl->recv_all(sizeof(gc_table)*tabs.size());
        for(auto & ele: rets){
            gc_table temp[tabs.size()];
            memcpy(temp, ele.second.data(), sizeof(gc_table)*tabs.size());
            for(int i = 0; i < a.size() * 2; i++){
                tabs[i].t[0] = tabs[i].t[0] ^ temp.t[0];
                tabs[i].t[1] = tabs[i].t[1] ^ temp.t[1];
                tabs[i].t[2] = tabs[i].t[2] ^ temp.t[2];
                tabs[i].t[3] = tabs[i].t[3] ^ temp.t[3];
            }
        }
    }
    template<uint8_t partes_number>
    void fake_offline(std::string owner){
        //random mask bit 
        if(Config::myconfig->check(owner)){

        }
    }
    ~BMR(){
        delete cf;
    }
};
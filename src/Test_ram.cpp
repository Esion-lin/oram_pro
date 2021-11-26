#include "preliminaries.hpp"
#include "easylogging++.h"
#include "risc.h"
#include "convert.h"
INITIALIZE_EASYLOGGINGPP
int main(int argc, char** argv){
    Config* cfg = new Config("./config.json");
    
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, argv[1]);
    std::string st = argv[1];
    uint32_t arr[15] = {1,2,3,4,5,6,7,8,9,10,1,2,3,4,5};
    /**/
    std::cout<<"-----------------------test four pc share and reveal----------------\n";
    uint64_t brr[15] = {1,2,3,4,5,6,7,8,9,10,1,2,3,4,5};
    uint64_t crr[15] = {};
    fourpc_share<uint64_t>(brr, 15, st, p2pchnl, 
                    [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);},
                    [](uint64_t* data, uint16_t lens)->void{for(int i = 0; i < lens; i++)data[i] = rand();});
    for(auto& ele:brr){
        std::cout<<ele<<" ";
    }
    fourpc_reveal<uint64_t>(brr, crr, 15, {"player0","player1"}, st, p2pchnl, 
                    [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);});
    for(auto& ele:crr){
        std::cout<<ele<<" ";
    }
    std::cout<<"-----------------------test done----------------\n";
    Ram<uint32_t>* test_ram = new Ram<uint32_t>(arr, 15, argv[1], p2pchnl);
    Convert* conv = new Convert(argv[1], p2pchnl);
    conv->fourpc_zeroshare<uint32_t>(1);
    test_ram->init();
    test_ram->prepare_read(1);
    test_ram->prepare_write(1);
    uint32_t tmp = test_ram->read(2, false);
    
    std::cout<<"tmp is "<< tmp <<std::endl;
    if(st == "player1" || st == "player3") tmp = - tmp;
    conv->fourpc_share_2_replicated_share<uint32_t>(&tmp, 1);
    std::cout<<"tmp is "<< tmp <<std::endl;
    p2pchnl->flush_all();
    test_ram->write(2, 6, tmp, false);
    std::cout<<"write"<<test_ram->data_ptr[8]<<std::endl;
    std::cout<<"write"<<test_ram->data_ptr[0]<<std::endl;
    delete conv;
    delete cfg;
    delete p2pchnl;
    delete test_ram;
}
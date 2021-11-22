#include "preliminaries.hpp"
#include "easylogging++.h"
#include "risc.h"
INITIALIZE_EASYLOGGINGPP
int main(int argc, char** argv){
    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, argv[1]);
    uint32_t arr[15] = {1,2,3,4,5,6,7,8,9,10,1,2,3,4,5};
    Ram<uint32_t>* test_ram = new Ram<uint32_t>(arr, 15, argv[1], p2pchnl);
    test_ram->init();
    test_ram->prepare_read(1);
    test_ram->prepare_write(1);
    uint32_t tmp = test_ram->read(2, false);
    std::cout<<"tmp is "<< tmp <<std::endl;
    p2pchnl->flush_all();
    test_ram->write(2, 6, 9, false);
    std::cout<<"write"<<test_ram->data_ptr[8]<<std::endl;
    std::cout<<"write"<<test_ram->data_ptr[0]<<std::endl;
    delete cfg;
    delete p2pchnl;
    delete test_ram;
}
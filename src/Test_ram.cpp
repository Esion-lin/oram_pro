#include "preliminaries.h"
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP
int main(argc, char** argv){
    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, argv[1]);
    uint32_t arr[15] = {1,2,3,4,5,6,7,8,9,10,1,2,3,4,5};
    Ram<uint32_t>* test_ram = Ram<uint32_t>(arr, 15, argv[1], p2pchnl);
    test_ram->prepare_read(1);
    std::cout<<test_ram->read(2)<<std::endl;
}
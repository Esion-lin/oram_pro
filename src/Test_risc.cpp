#include "preliminaries.hpp"
#include "easylogging++.h"
#include "risc.h"
#include <bitset>
INITIALIZE_EASYLOGGINGPP
int main(int argc, char** argv){
    Ins testins = {2,3,1,1,12,12312};
    uint64_t tmp = i2m(testins), tmp2;
    Env testenv;
    set_T<uint64_t>(testenv.mem, 0, tmp);
    tmp2 = load_T<uint64_t>(testenv.mem, 0);
    testenv.pc = 0;
    Ins ans = m2i(tmp2);
    std::cout<<ans;

    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, argv[1]);
    
    Mechine * now_mechine;
    if(argv[1] == "aid"){
        now_mechine = new Mechine(argv[1], p2pchnl, testenv);
        
    }
    else{
        now_mechine = new Mechine(argv[1], p2pchnl);
    }
    now_mechine->load_env();
    Ram<uint64_t>* ins_ram = new Ram<uint64_t>(reinterpret_cast<uint64_t*>(testenv.mem), 15, argv[1], p2pchnl);
}
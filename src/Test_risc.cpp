#include "preliminaries.hpp"
#include "easylogging++.h"
#include "risc.h"
#include <bitset>
INITIALIZE_EASYLOGGINGPP
int main(int argc, char** argv){
    Ins testins = {2,3,1,1,12,12312};
    std::string st = argv[1];
    uint64_t tmp = i2m(testins), tmp2;
    Env testenv;
    set_T<uint64_t>(testenv.mem, 0, tmp);
    tmp2 = load_T<uint64_t>(testenv.mem, 0);
    testenv.pc = 0;
    testenv.flag = 1;
    for(int i = 0; i < M_LEN; i++){
        testenv.m[i] = i;
    }
    Ins ans = m2i(tmp2);
    std::cout<<ans;

    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, argv[1]);
    
    Mechine * now_mechine;
    if(st == "aid"){
        now_mechine = new Mechine(argv[1], p2pchnl, testenv);
        //std::cout<<m2i(load_T<uint64_t>(now_mechine->myenv.mem, 0))<<std::endl;
    }
    else{
        now_mechine = new Mechine(argv[1], p2pchnl);
    }
    now_mechine->load_env();
    Ins a = now_mechine->load_ins();
    std::cout<<a<<std::endl;
    delete cfg;
    delete p2pchnl;
    delete now_mechine;
}
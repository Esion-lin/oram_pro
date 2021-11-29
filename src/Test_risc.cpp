#include "preliminaries.hpp"
#include "easylogging++.h"
#include "scmp.hpp"
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


    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, argv[1]);
    
    /**/
    
    std::cout<<"-----------------------test overflow-------------------\n";
    
    Compare* cmp = new Compare();
    uint32_t res[2],res2[2];
    cmp->scmp_off(st, p2pchnl, 4);
    if(st == "player0"){
        uint32_t arr[2] = {((uint32_t)1<<31)+22,3};
        uint32_t brr[2] = {((uint32_t)1<<31)+22,123};
        cmp->overflow(st, p2pchnl, arr, brr, 2,res);
        cmp->overflow(st, p2pchnl, arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }else{
        uint32_t arr[2] = {0,0};
        uint32_t brr[2] = {0,123};
        cmp->overflow(st, p2pchnl, arr, brr, 2,res);
        cmp->overflow(st, p2pchnl, arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }
    for(int i = 0; i < 2; i++){
        std::cout<<"res "<<res[i]<<std::endl;
    }
    for(int i = 0; i < 2; i++){
        std::cout<<"res "<<res2[i]<<std::endl;
    }
    delete cmp;
    std::cout<<"-----------------------test done-------------------\n";
    
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
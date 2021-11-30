#include "preliminaries.hpp"
#include "easylogging++.h"
#include "scmp.hpp"
#include "risc.h"
#include <bitset>
#include "operation.h"
#include "timer.hpp"
#include <functional>
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
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
    
    Compare* cmp = new Compare(st, p2pchnl);
    uint32_t res[2],res2[2];
    cmp->scmp_off( 4);
    Timer::record("overflow");
    if(st == "player0"){
        uint32_t arr[2] = {((uint32_t)1<<31)+22,3};
        uint32_t brr[2] = {((uint32_t)1<<31)+22,123};
        cmp->overflow(arr, brr, 2,res);
        cmp->overflow(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }else{
        uint32_t arr[2] = {0,0};
        uint32_t brr[2] = {0,123};
        cmp->overflow(arr, brr, 2,res);
        cmp->overflow(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }
    Timer::stop("overflow");
    for(int i = 0; i < 2; i++){
        std::cout<<"+ res "<<res[i]<<std::endl;
    }
    for(int i = 0; i < 2; i++){
        std::cout<<"- res "<<res2[i]<<std::endl;
    }
    delete cmp;
    cmp = new Compare(st, p2pchnl);
    Compare* cmp2 = new Compare(st, p2pchnl);
    cmp->scmp_off(2, 64);
    cmp2->scmp_off(2);
    Timer::record("overflow2");
    std::array<uint64_t,3> tmp_arr = {1,1,4};
    auto multiple = std::bind(mul_t<uint64_t>, std::placeholders::_1, std::placeholders::_2, st, p2pchnl, tmp_arr);
    if(st == "player0"){
        uint32_t arr[2] = {((uint32_t)1<<31)+22,3};
        uint32_t brr[2] = {((uint32_t)1<<31)+22,123};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->overflow_1(arr, brr, 2,res2, 64, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->overflow_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_end(arr, brr, 2,res);
        cmp2->overflow_end(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }else{
        uint32_t arr[2] = {0,0};
        uint32_t brr[2] = {0,123};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->overflow_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->overflow_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_end(arr, brr, 2,res);
        cmp2->overflow_end(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }
    Timer::stop("overflow2");
    for(int i = 0; i < 2; i++){
        std::cout<<"+ split res "<<res[i]<<std::endl;
    }
    for(int i = 0; i < 2; i++){
        std::cout<<"- split res "<<res2[i]<<std::endl;
    }
    delete cmp2;
    delete cmp;

    cmp = new Compare(st, p2pchnl);
    cmp->eq_off(2);
    Timer::record("equal");
    if(st == "player0"){
        uint32_t arr[2] = {((uint32_t)1<<22)+22,3};
        uint32_t brr[2] = {((uint32_t)1<<22)+22,123};
        cmp->equal(arr, brr, 2, res);
    }else{
        uint32_t arr[2] = {0,0};
        uint32_t brr[2] = {0,123};
        cmp->equal(arr, brr, 2, res);
    }
    Timer::stop("equal");
    for(int i = 0; i < 2; i++){
        std::cout<<"== res "<<res[i]<<std::endl;
    }
    delete cmp;
    Timer::test_print();
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
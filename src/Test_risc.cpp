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
    Ins testins = {ADD,3,1,1,12,501};
    std::string st = argv[1];
    uint64_t tmp = i2m(testins), tmp2;
    Env testenv;
    set_T<uint64_t>(testenv.mem, 0, tmp);
    tmp2 = load_T<uint64_t>(testenv.mem, 0);
    testenv.pc = 0;
    testenv.flag = 1;
    for(int i = 0; i < M_LEN; i++){
        testenv.m[i] = 2*i;
    }
    testenv.mem[501] = 2020;


    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, argv[1]);
    
    /**/
    uint32_t res[4],res2[4];
    std::cout<<"-----------------------test msb-------------------\n";
    Compare* cmp = new Compare(st, p2pchnl);

    cmp->msb_off(4, 33, true);
    /*4294967297, 8589946904, 1613827447, 6710252978*/
    /*1,        0,          0,          1*/
    uint64_t buffer[4];
    if(st == "player0"){
        uint32_t brr[4] = {1943807975,600768653,1433695274,72837182};
        
        cmp->msb_1(brr, 4,res,buffer);
        cmp->msb_2(brr, 4,res,buffer, 33, 0, true);
    }else if(st == "player1"){
        
        uint32_t brr[4] = {914631543,1433695274,87876123,2420488357};
        cmp->msb_1(brr, 4,res,buffer);
        cmp->msb_2(brr, 4,res,buffer, 33, 0, true);
    }
    else if(st == "player2"){
        uint32_t brr[4] = {1375911125,2420488357,10323231,4134994620};
        cmp->msb_1( brr, 4,res,buffer);
        cmp->msb_2(brr, 4,res,buffer, 33, 0, true);
    }
    else if(st == "player3"){
        uint32_t brr[4] = {60616654,4134994620,81932819,81932819};
        cmp->msb_1(brr, 4,res,buffer);
        cmp->msb_2(brr, 4,res,buffer, 33, 0, true);
    }
    //fourpc_reveal<uint32_t>(res, 4, {"player0"}, st, p2pchnl);
    for(int i = 0; i < 4; i++){
        std::cout<<"+ res "<<res[i]<<std::endl;
    }
    delete cmp;
    std::cout<<"-----------------------test done-------------------\n";

    std::cout<<"-----------------------test overflow-------------------\n";
    
    cmp = new Compare(st, p2pchnl);
    
    cmp->scmp_off( 4);
    Timer::record("overflow");
    if(st == "player0"){
        uint32_t arr[2] = {((uint32_t)1<<31)+22,1438719804};
        uint32_t brr[2] = {((uint32_t)1<<31)+22,4090056186};
        cmp->overflow(arr, brr, 2,res);
        cmp->overflow(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }else if(st == "player1"){
        
        uint32_t arr[2] = {0,1047901216};
        uint32_t brr[2] = {0,2964804424};
        cmp->overflow(arr, brr, 2,res);
        cmp->overflow(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }
    else if(st == "player2"){
        
        uint32_t arr[2] = {0,628188141};
        uint32_t brr[2] = {0,210757332};
        cmp->overflow(arr, brr, 2,res);
        cmp->overflow(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }
    else if(st == "player3"){
        
        uint32_t arr[2] = {0,1180158136};
        uint32_t brr[2] = {0,1324328962};
        cmp->overflow(arr, brr, 2,res);
        cmp->overflow(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }
    Timer::stop("overflow");
    fourpc_reveal<uint32_t>(res, 2, {"player0"}, st, p2pchnl);
    fourpc_reveal<uint32_t>(res2, 2, {"player0"}, st, p2pchnl);
    for(int i = 0; i < 2; i++){
        std::cout<<"+ res "<<res[i]<<std::endl;
    }
    for(int i = 0; i < 2; i++){
        std::cout<<"- res "<<res2[i]<<std::endl;
    }
    delete cmp;
    cmp = new Compare(st, p2pchnl);
    Compare* cmp2 = new Compare(st, p2pchnl);
    cmp->scmp_off(2);
    cmp2->scmp_off(2);
    Timer::record("overflow2");
    std::array<uint64_t,3> tmp_arr = {1,1,4};
    auto multiple = std::bind(mul_t<uint64_t>, std::placeholders::_1, std::placeholders::_2, st, p2pchnl, tmp_arr);
    if(st == "player0"){
        uint32_t arr[2] = {824693088,1438719804};
        uint32_t brr[2] = {3071257272,4090056186};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->overflow_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->overflow_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_end(arr, brr, 2,res);
        cmp2->overflow_end(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }else if(st == "player1"){
        uint32_t arr[2] = {2182846789,1047901216};
        uint32_t brr[2] = {3575917287,2964804424};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->overflow_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->overflow_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_end(arr, brr, 2,res);
        cmp2->overflow_end(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }else if(st == "player2"){
        uint32_t arr[2] = {983653189,628188141};
        uint32_t brr[2] = {2977866476,210757332};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->overflow_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->overflow_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        cmp->overflow_end(arr, brr, 2,res);
        cmp2->overflow_end(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }else if (st == "player3"){
        uint32_t arr[2] = {824693088,1180158136};
        uint32_t brr[2] = {3259873165,1324328962};
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
    fourpc_reveal<uint32_t>(res, 2, {"player0"}, st, p2pchnl);
    for(int i = 0; i < 2; i++){
        std::cout<<"+ split res "<<res[i]<<std::endl;
    }
    fourpc_reveal<uint32_t>(res2, 2, {"player0"}, st, p2pchnl);
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
        cmp->equal_1(arr, brr, 2, res);
        cmp->equal_2(arr, brr, 2, res);
        cmp->equal_3(arr, brr, 2, res);
    }else{
        uint32_t arr[2] = {0,0};
        uint32_t brr[2] = {0,123};
        cmp->equal_1(arr, brr, 2, res);
        cmp->equal_2(arr, brr, 2, res);
        cmp->equal_3(arr, brr, 2, res);
    }
    Timer::stop("equal");
    fourpc_reveal<uint32_t>(res, 2, {"player0"}, st, p2pchnl);
    for(int i = 0; i < 2; i++){
        std::cout<<"== res "<<res[i]<<std::endl;
    }
    delete cmp;
    
    std::cout<<"-----------------------test done-------------------\n";
    std::cout<<"-----------------------test load ins-------------------\n";
    Mechine * now_mechine;
    if(st == "aid"){
        now_mechine = new Mechine(argv[1], p2pchnl, testenv, 2);
        //std::cout<<m2i(load_T<uint64_t>(now_mechine->myenv.mem, 0))<<std::endl;
    }
    else{
        now_mechine = new Mechine(argv[1], p2pchnl, 2);
    }
    now_mechine->load_env();
    Timer::record("load_ins");
    Ins a = now_mechine->load_ins();
    Timer::stop("load_ins");
    uint32_t re_list[3] = {now_mechine->Ri, now_mechine->Rj, now_mechine->A};
    fourpc_reveal<uint32_t>(re_list, 3, {"player0"}, st, p2pchnl);
    if(st == "player0") std::cout<<"Ri "<<re_list[0]<<"; Rj "<<re_list[1]<<"; A "<<re_list[2]<<";\n";
    std::cout<<"-----------------------test done-------------------\n";

    std::cout<<"-----------------------test run_op-------------------\n";
    Timer::record("run_ins");
    now_mechine->run_op();
    now_mechine->ret_res();
    now_mechine->print_res_flag(501);
    Timer::stop("run_ins");
    std::cout<<"-----------------------test done-------------------\n";
    
    Timer::test_print();
    delete cfg;
    delete p2pchnl;
    delete now_mechine;
}
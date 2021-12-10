#include "preliminaries.hpp"
#include "easylogging++.h"
#include "scmp.hpp"
#include "risc.h"
#include <bitset>
#include "operation.h"
#include "timer.hpp"
#include <functional>
#include <stdio.h>
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
INITIALIZE_EASYLOGGINGPP
int main(int argc, char** argv){
    ifstream infile("ins.ins");
    Ins testins;
    uint32_t char_int;
    if (infile.is_open()){
        infile>>char_int;
        testins.optr = char_int;
        infile>>char_int;
        testins.idic = char_int;
        infile>>char_int;
        testins.i = char_int;
        infile>>char_int;
        testins.j = char_int;
        infile>>char_int;
        testins.pad = char_int;
        infile>>char_int;
        testins.imme = char_int;
    }
    std::string st = argv[1];
    uint64_t tmp = i2m(testins), tmp2;
    std::cout<<testins<<std::endl;
    Env testenv;
    set_T<uint64_t>(testenv.mem, 0, tmp);
    tmp2 = load_T<uint64_t>(testenv.mem, 0);
    
    testenv.pc = 0;
    testenv.flag = 0;
    testenv.rc0 = 2;
    testenv.rc1 = 2;
    testenv.num0 = 2;
    testenv.num1 = 2;
    for(int i = 0; i < TAPE_LEN; i++){
        testenv.tape1[i] = 100+i;
        testenv.tape2[i] = 200+i;
        
    }
    for(int i = 0; i < M_LEN; i++){
        testenv.m[i] = i;
    }
    for(int i = 2; i < MEM_LEN; i++){
        testenv.mem[i] = 2000 + 2*i;
    }
    testenv.mem[501] = 2020;


    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, argv[1]);
    
    /**/
    uint32_t res[4],res2[4];
    std::cout<<"-----------------------test msb-------------------\n";

    Compare* cmp = new Compare(st, p2pchnl);
    //cmp->msb_off(4, 34, true);
    //cmp->twopc_ic_off(4, 1521571397, 2493325539, (uint64_t)1<<32, 32);
    /*4295912975, 8589946904, 1613827447, 6710252978*/
    /*1,        0,          0,          1*/
    uint64_t buffer[4];
    if(st == "aid"){
        Mul * mmul = new Mul(st, p2pchnl);
        mmul->offline();
        delete mmul;
    } 
    if(st == "player0"){
        uint32_t brr[5] = {1943807975,600768653,1433695274,72837182};
        Mul * mmul = new Mul(brr[0], brr[1], &brr[2], &brr[3], st, p2pchnl);
        mmul->offline();
        mmul->round1();
        mmul->round2();
        mmul->round3();
        mmul->roundend();
        delete mmul;
        // cmp->msb_1<uint32_t>(brr, 4,res,buffer, 34);
        // cmp->msb_diag_2<uint32_t>(brr, 4,res,buffer, 34, 0, true);
    }else if(st == "player1"){
        
        uint32_t brr[5] = {914631543,1433695274,87876123,2420488357};
        Mul * mmul = new Mul(brr[0], brr[1], &brr[2], &brr[3], st, p2pchnl);
        mmul->offline();
        mmul->round1();
        mmul->round2();
        mmul->round3();
        mmul->roundend();
        delete mmul;
        // cmp->msb_1<uint32_t>(brr, 4,res,buffer, 34);
        // cmp->msb_diag_2<uint32_t>(brr, 4,res,buffer, 34, 0, true);
    }
    else if(st == "player2"){
        uint32_t brr[5] = {1375911125,2420488357,10323231,4134994620};
        Mul * mmul = new Mul(brr[0], brr[1], &brr[2], &brr[3], st, p2pchnl);
        mmul->offline();
        mmul->round1();
        mmul->round2();
        mmul->round3();
        mmul->roundend();
        delete mmul;
        // cmp->msb_1<uint32_t>( brr, 4,res,buffer, 34);
        // cmp->msb_diag_2<uint32_t>(brr, 4,res,buffer, 34, 0, true);
    }
    else if(st == "player3"){
        uint32_t brr[5] = {60616654,4134994620,81932819,81932819};
        Mul * mmul = new Mul(brr[0], brr[1], &brr[2], &brr[3], st, p2pchnl);
        mmul->offline();
        mmul->round1();
        mmul->round2();
        mmul->round3();
        mmul->roundend();
        delete mmul;
        // cmp->msb_1<uint32_t>(brr, 4,res,buffer, 34);
        // cmp->msb_diag_2<uint32_t>(brr, 4,res,buffer, 34, 0, true);
    }
    twopc_reveal<uint32_t>(res, res2, 4, st, p2pchnl);
    for(int i = 0; i < 4; i++){
        std::cout<<"+ res "<<res2[i]<<std::endl;
    }
    delete cmp;
    std::cout<<"-----------------------test done-------------------\n";

    std::cout<<"-----------------------test overflow-------------------\n";
    cmp = new Compare(st, p2pchnl);
    Compare* cmp2 = new Compare(st, p2pchnl);
    cmp->scmp_off(2);
    cmp2->mul_off(2);
    Timer::record("overflow2");
    /*  520918858 1
        12312   12312
    */
    if(st == "player0"){
        uint32_t arr[2] = {824693088,1438719804};
        uint32_t brr[2] = {3071257272,4090056186};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->mul_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->mul_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);},true);
        cmp->overflow_end(arr, brr, 2,res);
    }else if(st == "player1"){
        uint32_t arr[2] = {2182846789,1047901216};
        uint32_t brr[2] = {3575917287,2964804424};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->mul_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->mul_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);},true);
        cmp->overflow_end(arr, brr, 2,res);
    }else if(st == "player2"){
        uint32_t arr[2] = {983653189,628188141};
        uint32_t brr[2] = {2977866476,210757332};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->mul_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->mul_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);},true);
        cmp->overflow_end(arr, brr, 2,res);
    }else if (st == "player3"){
        uint32_t arr[2] = {824693088,1180158136};
        uint32_t brr[2] = {3259873165,1324328962};
        cmp->overflow_1(arr, brr, 2,res);
        cmp2->mul_1(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_2(arr, brr, 2,res);
        cmp2->mul_2(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);});
        cmp->overflow_3(arr, brr, 2,res);
        cmp2->overflow_3(arr, brr, 2,res2, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a * b) % ((uint64_t)1<<33);},true);
        cmp->overflow_end(arr, brr, 2,res);
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
    std::cout<<"data:"<<testenv.mem[1]<<std::endl;
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
    now_mechine->print_res_flag(1);
    Timer::stop("run_ins");
    std::cout<<"-----------------------test done-------------------\n";
    
    Timer::test_print();
    delete cfg;
    delete p2pchnl;
    delete now_mechine;
}
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
    Env testenv;
    for (int i = 0; i < 18; i++){
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
        
        uint64_t tmp = i2m(testins);
        set_T<uint64_t>(testenv.mem, i, tmp);
    }
    std::string st = argv[1];
    
    testenv.pc = 0;
    testenv.flag = 0;
    testenv.rc0 = 0;
    testenv.rc1 = 0;
    testenv.num0 = 31;
    testenv.num1 = 31;
    for(int i = 0; i < TAPE_LEN; i++){
        testenv.tape1[i] = 100+i;
        testenv.tape2[i] = 200+i;
        
    }
    testenv.tape1[0] = 3030;
    for(int i = 0; i < M_LEN; i++){
        testenv.m[i] = i;
    }
    for(int i = 512; i < MEM_LEN; i++){
        testenv.mem[i] = 2000 + 2*i;
    }
    testenv.mem[512] = 0;
    testenv.mem[513] = 100;



    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, argv[1]);
    
    /**/
    
    Mechine * now_mechine;
    if(st == "aid"){
        now_mechine = new Mechine(argv[1], p2pchnl, testenv, 32);
        //std::cout<<m2i(load_T<uint64_t>(now_mechine->myenv.mem, 0))<<std::endl;
    }
    else{
        now_mechine = new Mechine(argv[1], p2pchnl, 32);
    }
    
    now_mechine->print_res_flag(1);
    now_mechine->load_env();
    
    while(true){
        Timer::record("load_ins");
        //std::cout<<"NO."<<i<<" -----------------------test load ins-------------------\n";
        Ins a = now_mechine->load_ins();
        if(now_mechine->done) break;
        Timer::stop("load_ins");
        uint32_t re_list[3] = {now_mechine->Ri, now_mechine->Rj, now_mechine->A};
        fourpc_reveal<uint32_t>(re_list, 3, {"player0"}, st, p2pchnl);
        //if(st == "player0") std::cout<<"Ri "<<re_list[0]<<"; Rj "<<re_list[1]<<"; A "<<re_list[2]<<";\n";
        //std::cout<<"-----------------------test done-------------------\n";
        
        std::cout<<"-----------------------test run_op-------------------\n";
        Timer::record("run_ins");
        now_mechine->run_op();
        now_mechine->ret_res();
        now_mechine->print_res_flag(8);
        Timer::stop("run_ins");
        std::cout<<"-----------------------test done-------------------\n";
        }
    now_mechine->print_res_flag(8);
    Timer::test_print();
    delete cfg;
    delete p2pchnl;
    delete now_mechine;
}
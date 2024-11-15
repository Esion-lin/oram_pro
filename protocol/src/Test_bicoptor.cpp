
#include <stdlib.h>   // 提供 malloc 和 free
#include <stdint.h> 
#include <iostream>
#include "bicoptor.h"
#include "config.hpp"
#include "timer.hpp"
Config* Config::myconfig;
int partyNum;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
int main(int argc, char** argv){
    srand((unsigned)time(NULL)); 
    Config::myconfig = new Config("./3_p_config.json");
    
	
    Config::myconfig->set_player(argv[1]);
    partyNum = Config::myconfig->get_idex();
    initializeCommunication(partyNum);
    synchronize(2000000);
    int number = atoi(argv[2]);
    Bicoptor<uint64_t>* bicoptor = new Bicoptor<uint64_t>(number);
    std::vector<uint64_t> temp1(number), temp_z(number);
    // init_ashare<uint64_t>(temp1, 1024);
    // init_ashare<uint64_t>(temp_z, 1024);
    start_communication();
    Timer::record("online");
    bicoptor->online(temp1, temp_z);
    Timer::stop("online");
    Timer::test_print();
    end_communication("test");
    delete bicoptor;
}
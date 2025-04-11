#include "connect.h"
#include "timer.hpp"
#include <iostream>
#include "signv.h"
#include <stdint.h>
#include <inttypes.h>

Config* Config::myconfig;
int partyNum;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
int main(int argc, char** argv){
    srand((unsigned)time(NULL)); 
    Config::myconfig = new Config("../test/3_p_config.json");
    
	
    Config::myconfig->set_player(argv[1]);
    partyNum = Config::myconfig->get_idex();
    initializeCommunication(partyNum);
    synchronize(2000000);
    int number = atoi(argv[2]);
    Signv<uint64_t>* signv = new Signv<uint64_t>(number);
    std::vector<AShareT<uint64_t>> x(number), x_sign(number);
    printf("datasize=%d\n", number);
    start_communication();
    Timer::record("setup");
    Timer::record("all");
    signv->set_up(x, x_sign, true);
    Timer::stop("setup");
    Timer::record("online");
    signv->online(x, x_sign);
    Timer::stop("online");

    Timer::stop("all");

    Timer::test_print();
    end_communication("test");


    delete signv;
}

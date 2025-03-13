#include "connect.h"
#include "timer.hpp"
#include <iostream>
#include "sign.h"
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
    Config::myconfig = new Config("./3_p_config.json");
    
	
    Config::myconfig->set_player(argv[1]);
    partyNum = Config::myconfig->get_idex();
    initializeCommunication(partyNum);
    synchronize(2000000);
    int number = atoi(argv[2]);
    Sign<uint64_t>* sign = new Sign<uint64_t>(number);
    Signv<uint64_t>* signv = new Signv<uint64_t>(number);
    std::vector<AShareT<uint64_t>> x(2*number), x_sign(2*number);
    x[0].r = (1<<64)-1;
    start_communication();
    Timer::record("setup");
    sign->set_up(x, x_sign, true);
    Timer::stop("setup");
    Timer::record("online");
    sign->online(x, x_sign);
    Timer::stop("online");

    Timer::test_print();
    end_communication("test");

    printf("x[0].r=0x%" PRIx64 "\n", x[0].r);
    printf("x_sign[0].r=0x%" PRIx64 "\n", x_sign[0].r);

    delete sign;
    delete signv;
}
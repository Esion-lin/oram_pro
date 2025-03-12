
#include <inttypes.h>
#include "connect.h"
#include "timer.hpp"
#include <iostream>
#include "sign.h"
#include "signv.h"
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
    std::vector<AShareT<uint64_t>> temp1(2*number), temp_z(2*number);
    // init_ashare<uint64_t>(temp1, 1024);
    // init_ashare<uint64_t>(temp_z, 1024);
    start_communication();
    Timer::record("setup");
    sign->set_up(temp1, temp_z, true);
    //sign->set_up(temp1, temp_z, true);sign->set_up(temp1, temp_z, true);sign->set_up(temp1, temp_z, true);
    Timer::stop("setup");
    Timer::record("online");
    sign->online(temp1, temp_z);
    // sign->online(temp1, temp_z);
    // sign->online(temp1, temp_z);
    // sign->online(temp1, temp_z);
    // sign->online(temp1, temp_z);
    // sign->online(temp1, temp_z);
    // sign->online(temp1, temp_z);
    // sign->online(temp1, temp_z);
    // sign->online(temp1, temp_z);
    // sign->online(temp1, temp_z);
    Timer::stop("online");
    // Timer::record("verify setup");
    // signv->set_up(temp1, temp_z, true);
    // Timer::stop("verify setup");
    // Timer::record("verify online");
    // signv->online(temp1, temp_z);
    // Timer::stop("verify online");
    Timer::test_print();
    end_communication("test");
    delete sign;
    delete signv;
}
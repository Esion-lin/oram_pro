#include "connect.h"
#include "timer.hpp"
#include <iostream>
#include "sign.h"
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
    Sign<uint64_t>* sign = new Sign<uint64_t>(number);
    std::vector<AShareT<uint64_t>> x(number), x_sign(number);
    printf("datasize=%d\n", number);
    start_communication();
    Timer::record("setup");
    Timer::record("all");
    sign->set_up(x, x_sign, true);
    Timer::stop("setup");
    Timer::record("online");
    sign->online(x, x_sign);
    Timer::stop("all");
    Timer::stop("online");

    Timer::test_print();
    end_communication("test");


    delete sign;
}

/*

    x[0].r = (1<<64)-1;
    x[0].r_1 = 0;
    x[0].r_2 = x[0].r;
    if(Config::myconfig->check("player0")){
        x[0].r_1=1;
        x[0].r_2=1;
    }else{
        x[0].r_1=0x0000000000000000+2;
        x[0].r_2=1;
    }
    printf("x[0].r_1=%" PRIx64 "\n", x[0].r_1);
    printf("x[0].r_2=%" PRIx64 "\n", x[0].r_2);
    
    printf("sign_x[0].r_1=%" PRIx64 "\n", x_sign[0].r_1);
    printf("sign_x[0].r_2=%" PRIx64 "\n", x_sign[0].r_2);
    if(Config::myconfig->check("player0")){
        sendVector<uint64_t>(&x_sign[0].r_2, 1, 1);
        sendVector<uint64_t>(&x_sign[0].r_1, 2, 1);
    }else{
        uint64_t r_share;
        receiveVector<uint64_t>(&r_share, 0, 1);
        printf("sign_x_reveal=%" PRIx64 "\n", x_sign[0].r_1-x_sign[0].r_2-r_share);
    }
*/
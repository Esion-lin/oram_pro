#include "semi.h"
#include <inttypes.h>
#include "net.hpp"
#include "verification.h"
#include <time.h> 
#include <iostream>
#include "timer.hpp"
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
/**
 * @brief test share and recostruct
 * 
 */
int main(int argc, char** argv){
    srand((unsigned)time(NULL)); 
    Config::myconfig = new Config("./3_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    uint32_t lens = atoi(argv[2]);
    uint32_t tlens = lens;
    uint32_t R = atoi(argv[3]);
    uint64_t data[lens];

    for(int i = 0; i < lens; i++){
        data[i] = (1<<63)-1;
    }
    AShare<uint64_t, 1> *x = new AShare<uint64_t, 1>[lens], *y = new AShare<uint64_t, 1>[lens], z;
    z.lens = 1;

    Timer::record("total");
    for(int i = 0; i < R; i++){
        lens /= 2;
        AShare<uint64_t, 1> *x_p = new AShare<uint64_t, 1>[lens], *y_p = new AShare<uint64_t, 1>[lens], z_p;
        z_p.lens = 1;
        
        PolyVerify<uint64_t, 1> ver(lens*2);

        ver.verify(x, y, z, x_p, y_p, z_p);
       delete[] x;
        delete[] y;
        x = x_p;
        y = y_p;
        z = z_p;
        
        
        
    }
    Dot<uint64_t, 1,32> dotv;
    
    dotv.set_up(x,y,z, lens, false);
    dotv.online(x,y,z, lens);

    Timer::stop("total");
    Timer::test_print(R, tlens);
    delete[] x;
    delete[] y;
   
    
}
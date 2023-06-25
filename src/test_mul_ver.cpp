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
    AShareRing<1>* x_z = new AShareRing<1>[lens], *y_z = new AShareRing<1>[lens], z_z;
    for(int i = 0; i < lens; i ++){
        dummy_transfer<1>(x_z[i], 32);
        dummy_transfer<1>(y_z[i], 32);
        
    }
    dummy_transfer(z_z, 32);
    Timer::record("total");
    for(int i = 0; i < R; i++){
        lens /= 2;
        AShareRing<1>*x_p_z = new AShareRing<1>[lens], *y_p_z = new AShareRing<1>[lens], z_p_z;
        // PolyVerify<uint64_t, 1024> ver;
        PolyRingVerify<1, 32> ver(lens*2);
        ver.verify(x_z, y_z, z_z, x_p_z, y_p_z, z_p_z);
        delete[] x_z;
        delete[] y_z;
        x_z = x_p_z;
        y_z = y_p_z;
        z_z = z_p_z;
        
    }
    Dotverify<1> dotv(lens);
    dotv.set_up(x_z,y_z,z_z);
    dotv.verify(x_z,y_z,z_z);
    Timer::stop("total");
    Timer::test_print(R, tlens);
    delete[] x_z;
    delete[] y_z;
   
    
}
#include "semi.h"
#include <inttypes.h>
#include "net.hpp"
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
/**
 * @brief test share and recostruct
 * 
 */
int main(int argc, char** argv){
    Config::myconfig = new Config("./3_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    float origin[1024];
    uint64_t data[1024];
    uint64_t tar_data[1024];
    float flip = -1;

    FloatEncode<uint64_t, 16> enc;
    Mult<uint64_t> mult;
    Trunc<uint64_t,16> trunc;
    for(int i = 0; i < 1024; i++){
        flip *= -1;
        origin[i] = 1.22 * flip * i;
        data[i] = enc.encode(origin[i]);
        if(Config::myconfig->check("player0"))
        printf("%" PRIu64 " ", data[i]);
    }
    AShare<uint64_t> temp1, temp_y, temp_z,temp_z2; 
    init_ashare<uint64_t>(temp1, 1024);
    init_ashare<uint64_t>(temp_y, 1024);
    init_ashare<uint64_t>(temp_z, 1024);
    init_ashare<uint64_t>(temp_z2, 1024);
    AShare<uint32_t> temp2;
    init_ashare<uint32_t>(temp2, 1024);
    AShare<uint8_t> temp3;
    init_ashare<uint8_t>(temp3, 1024);
    ShareA<uint64_t>(data, temp1);
    ShareA<uint64_t>(data, temp_y);
    mult.set_up(temp1, temp_y, temp_z, true);
    mult.online(temp1, temp_y, temp_z);
    trunc.set_up(temp_z, temp_z2);
    trunc.online(temp_z, temp_z2);
    RevealA<uint64_t>(tar_data, temp_z2);
    printf("\n ---------result--------- \n");
    if(Config::myconfig->check("player0")){
        // for(int i = 0; i < 1024; i++){
        //     printf("%" PRIu64 " ", tar_data[i]);
            
        // }
        // printf("\n ---------result--------- \n");
        for(int i = 0; i < 1024; i++){
            printf("%f ", enc.decode(tar_data[i]));
            
        }
    }

    release_ashare<uint64_t>(temp1);
    release_ashare<uint32_t>(temp2);
    release_ashare<uint8_t>(temp3);
    release_ashare<uint64_t>(temp_y);
    release_ashare<uint64_t>(temp_z);
    release_ashare<uint64_t>(temp_z2);
    
}

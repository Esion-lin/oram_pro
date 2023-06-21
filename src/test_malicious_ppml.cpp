#include "semi.h"
#include <inttypes.h>
#include "net.hpp"
#include "verification.h"
#include <time.h> 
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
    float origin[1024];
    uint64_t data[1024];
    uint64_t tar_data[1024];
    uint64_t tar_data2[1024];
    float flip = -1;

    FloatEncode<uint64_t, 16> enc;
    Mult<uint64_t> mult;
    Trunc<uint64_t,16> trunc;
    Sign<uint64_t> sign;
    for(int i = 0; i < 1024; i++){
        flip *= -1;
        origin[i] = 1.22 * flip * i;
        data[i] = (1<<63)-1;//enc.encode(origin[i]);
        // if(Config::myconfig->check("player0"))
        // printf("%" PRIu64 " ", data[i]);
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
    // ShareA<uint64_t>(data, temp1);
    // ShareA<uint64_t>(data, temp_y);
    // mult.set_up(temp1, temp_y, temp_z, true);
    // mult.online(temp1, temp_y, temp_z);
    // // trunc.set_up(temp_z, temp_z2);
    // // trunc.online(temp_z, temp_z2);
    // sign.set_up(temp1, temp_z2, true);
    // sign.online(temp1, temp_z2);

    // RevealA<uint64_t>(tar_data, temp_z2);
    // printf("\n ---------result--------- \n");
    // if(Config::myconfig->check("player0")){
    //     for(int i = 0; i < 1024; i++){
    //         printf("%" PRIu64 " ", tar_data[i]);
            
    //     }
    //     // printf("\n ---------result--------- \n");
    //     // for(int i = 0; i < 1024; i++){
    //     //     printf("%f ", enc.decode(tar_data[i]));
            
    //     // }
    // }
    AShare<uint64_t> x[1024], y[1024], z, x_p[512], y_p[512], z_p, z_p_s;
    uint64_t ite = 0;
    for(int i = 0; i < 1024; i++){
        
        init_ashare<uint64_t>(x[i], 1024);
        init_ashare<uint64_t>(y[i], 1024);
        for(int i = 0; i < 1024; i++){
            data[i] = rand();
        }
        ShareA<uint64_t>(data, x[i]);
        for(int i = 0; i < 1024; i++){
            data[i] = rand();
        }
        ShareA<uint64_t>(data, y[i]);
        if(i < 512){
            init_ashare<uint64_t>(x_p[i], 1024);
            init_ashare<uint64_t>(y_p[i], 1024);
        }
    }
    init_ashare<uint64_t>(z, 1024);
    init_ashare<uint64_t>(z_p, 1024);
    init_ashare<uint64_t>(z_p_s, 1024);
    Dot<uint64_t> dot1,dot2;
    PolyVerify<uint64_t, 1024> ver;
    dot1.set_up(x, y, z, 1024, true);
    dot1.online(x, y, z, 1024);
    ver.verify(x, y, z, x_p, y_p, z_p);
    dot2.set_up(x_p, y_p, z_p_s, 512, true);
    dot2.online(x_p, y_p, z_p_s, 512);
    RevealA<uint64_t>(tar_data, z_p_s);
    RevealA<uint64_t>(tar_data2, z_p);
    printf("\n ---------result--------- \n");
    if(Config::myconfig->check("player0")){
        for(int i = 0; i < 512; i++){
            if(tar_data[i] != tar_data2[i])
            printf("%" PRIu64 " %" PRIu64 " \n", tar_data[i],tar_data2[i]);
            
        }
        // printf("\n ---------result--------- \n");
        // for(int i = 0; i < 1024; i++){
        //     printf("%f ", enc.decode(tar_data[i]));
            
        // }
    }
    

    for(int i = 0; i < 1024; i++){
        release_ashare<uint64_t>(x[i]);
        release_ashare<uint64_t>(y[i]);
        if(i < 512){
            release_ashare<uint64_t>(x_p[i]);
            release_ashare<uint64_t>(y_p[i]);
        }
    }
    release_ashare<uint64_t>(z);
    release_ashare<uint64_t>(z_p);
    release_ashare<uint64_t>(z_p_s);
    release_ashare<uint64_t>(temp1);
    release_ashare<uint32_t>(temp2);
    release_ashare<uint8_t>(temp3);
    release_ashare<uint64_t>(temp_y);
    release_ashare<uint64_t>(temp_z);
    release_ashare<uint64_t>(temp_z2);
    
}

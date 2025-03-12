#include <inttypes.h>
#include <stdlib.h>   // 提供 malloc 和 free
#include <stdint.h> 
#include "fss-server.h"
#include "fss-common.h"
#include "fss-client.h"
#include "fss_help.hpp"
#include "timer.hpp"
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
int main(int argc, char** argv){
    srand((unsigned)time(NULL)); 
    Config::myconfig = new Config("./3_p_config.json");
    Config::myconfig->set_player(argv[1]);
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);

    int number = atoi(argv[2]);

    Fss server_key;
    ServerKeyLt k0, k1;
    
    uint32_t datalens = 2048 ;
    uint32_t res[datalens],res2[datalens];
    Timer::record("setup");
    initializeClient(&server_key, 4, 2);
    uint32_t b = 1;
    //#pragma omp parallel for
    
    
    uint8_t* testdata = (uint8_t*)malloc(number * (34 * 63 + 34));
    if(Config::myconfig->check("player0")){
        //send key to P1 and P2
        for(int i = 0; i < number; i++){
            generateTreeLt(&server_key, &k0, &k1, 3, 4);
        }
        P2Pchannel::mychnl->send_data_to("player1",testdata,number * (34 * 63 + 34));
        P2Pchannel::mychnl->send_data_to("player2",testdata,number * (34 * 63 + 34));
        
        
    }else{
        P2Pchannel::mychnl->recv_data_from("player0",testdata,number * (34 * 63 + 34));
    }
    uint8_t* testdata2 = (uint8_t*)malloc(number * 8);
    if(Config::myconfig->check("player1")){
        //send key to P1 and P2
        
        P2Pchannel::mychnl->send_data_to("player2",testdata2,number * 8);
        P2Pchannel::mychnl->recv_data_from("player2",testdata,number * 8);
        
        
    }else if(Config::myconfig->check("player2")){
        P2Pchannel::mychnl->send_data_to("player1",testdata2,number * 8);
        P2Pchannel::mychnl->recv_data_from("player1",testdata,number * 8);
    }
    //online
    if(Config::myconfig->check("player0")){
    #pragma omp parallel for
    for(int i = 0; i < number; i ++)
        evaluateLt(&server_key, &k0, 1);
    }
    Timer::stop("setup");
    //evaluateEq_full(&server_key, &k0, res2, 16);
    //evaluateEq(&server_key, &k1, res2, datalens);

    // for(int i = datalens; i > datalens - 100; i--){
    //     std::cout<<res[i]<<" "<<res2[i]<<std::endl;
    // }    
}
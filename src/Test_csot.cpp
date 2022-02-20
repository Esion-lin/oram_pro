#include "csot.hpp"
#include "io.hpp"
#include "easylogging++.h"
INITIALIZE_EASYLOGGINGPP
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
int main(int argc, char** argv){
    Config::myconfig = new Config("./3_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    uint32_t datas[2048], rep[2048];
    for(int i = 0;i < 2048; i++){
        datas[i] = i;
    }
    replicated_share<uint32_t>("player0", {"player0", "player1", "player2"}, datas, rep, 2048);
    std::cout<<datas[0] << " "<<rep[0]<<std::endl;
    CSOT<uint32_t> * csot = new CSOT<uint32_t>();
    csot->offline();
    uint32_t block[2] = {120,1333}, m[2] = {200,1};

    std::cout<<csot->online<uint32_t>(block, m)<<std::endl;
    delete csot;
    return 0;
}
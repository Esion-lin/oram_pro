#include "scmp.hpp"
#include "io.hpp"
#include "easylogging++.h"
#include "timer.hpp"
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
INITIALIZE_EASYLOGGINGPP
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
int main(int argc, char** argv){
    Config::myconfig = new Config("./3_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    uint32_t datas[2048] = {1, 10, 55, 100, 1230, 2049, 2099}, rep[2048];
    add_share<uint32_t>("player0", {"player1", "player2"}, datas, 2048);
    Thr_pc_ic_id<uint32_t> *ic = new Thr_pc_ic_id<uint32_t>(32, "player2");
    uint32_t tao = (uint32_t)1 << (32 - 1) - 1;
    uint32_t rou = 2691244714;
    ic->twopc_ic_off(1, 0, tao, 0, &rou);
    datas[0] = 2691245311;
    Timer::record("IC");
    for(int i = 0; i < 10000; i++){
        ic->twopc_ic(datas, 1, rep);
    }
    Timer::stop("IC");
    Timer::test_print();
    std::cout<<rep[0]<<std::endl;
    // for(int i = 0; i < 7; i++){
    //     std::cout<<rep[i]<<std::endl;
    // }
    delete ic;
    return 0;
}
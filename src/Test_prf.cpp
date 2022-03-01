#include "fss-common.h"
#include "fss-server.h"
#include "fss-client.h"
#include "io.hpp"
#include "pre_op.h"
#include "easylogging++.h"
#include "fss_mpc.h"
#include "timer.hpp"
using namespace std;
INITIALIZE_EASYLOGGINGPP
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
int main(int argc, char** argv){
    Config::myconfig = new Config("./config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    uint32_t data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    add_share<uint32_t>("aid", {"player0", "player1"},data, 12);
    for(int i = 0; i < 12; i++){
        cout<<data[i]<< " ";
    }
    cout<<endl;
    add_share<uint32_t>("player2", {"player2", "player3"},data, 12);
    for(int i = 0; i < 12; i++){
        cout<<data[i]<< " ";
    }cout<<endl;
    add_reveal<uint32_t>({"player2"}, {"player2", "player3"},data, 12);
    for(int i = 0; i < 12; i++){
        cout<<data[i]<< " ";
    }cout<<endl;
    uint8_t data0[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    uint8_t data1[16] = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    uint8_t res[16] = {0};
    std::cout<< "----------------test xor sharing select----------------\n";
    add_share<uint8_t>("player2", {"player2", "player3"},data0, 16, 
                [](uint8_t a, uint8_t b)->uint8_t{return a^b;}, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    add_share<uint8_t>("player2", {"player2", "player3"},data1, 16, 
                [](uint8_t a, uint8_t b)->uint8_t{return a^b;}, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    bool select = false;
    if(Config::myconfig->check("player2")) select = false;
    shared_select<uint8_t>(data0, data1, select, res, 16, {"player2", "player3"});
    add_reveal<uint8_t>({"player2"}, {"player2", "player3"}, res, 16, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    for(int i = 0; i < 16; i++){
        cout<<(uint32_t)res[i]<< " ";
    }cout<<endl;
    std::cout<< "----------------test add sharing select----------------\n";
    uint32_t datas[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    uint32_t datas1[16] = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};
    uint32_t res_32[32];
    add_share<uint32_t>("player2", {"player2", "player3"},datas, 16);
    half_select<uint32_t>(datas, datas1, select, res_32, 16, {"player2", "player3"});
    add_reveal<uint32_t>({"player2"}, {"player2", "player3"}, res_32, 16);
    for(int i = 0; i < 16; i++){
        cout<<(uint32_t)res[i]<< " ";
    }cout<<endl;
    std::cout<< "----------------test Fss mpc versions------------------\n";
    FSS_MPC<uint32_t, 32> *fss_mpc = new FSS_MPC<uint32_t, 32>({"player0", "player1"});
    if(Config::myconfig->check("player0")){
        Timer::record("fss mpc gen");
        fss_mpc->gen(3, 1, 0);
        Timer::stop("fss mpc gen");
        for(int i = 0; i < 8; i++)
        fss_mpc->evl(i,0);
    }
    else if(Config::myconfig->check("player1")){
        Timer::record("fss mpc gen");
        fss_mpc->gen(0, 0, 1);
        Timer::stop("fss mpc gen");
        for(int i = 0; i < 8; i++)
        fss_mpc->evl(i,1);
    }
        std::cout<< "----------------test Dcf mpc versions------------------\n";
    
    // if(Config::myconfig->check("player0")){
    //     DCF_MPC<uint32_t, uint32_t> *dcf_mpc = new DCF_MPC<uint32_t, uint32_t>({"player0", "player1"});
    //     Timer::record("dcf mpc gen");
    //     dcf_mpc->gen(100, 144);
    //     Timer::stop("dcf mpc gen");
    //     for(int i = 0; i < 8; i++)
    //     dcf_mpc->evl(i+95);
    //     delete dcf_mpc;
    // }
    // else if(Config::myconfig->check("player1")){
    //     DCF_MPC<uint32_t, uint32_t> *dcf_mpc = new DCF_MPC<uint32_t, uint32_t>({"player0", "player1"});
    //     Timer::record("dcf mpc gen");
    //     dcf_mpc->gen(0, 0);
    //     Timer::stop("dcf mpc gen");
    //     for(int i = 0; i < 8; i++)
    //     dcf_mpc->evl(i+95);
    //     delete dcf_mpc;
    // }
    Timer::test_print();
    delete fss_mpc;
    
    // /*test full domain*/
    // Fss keys_full;
    // ServerKeyEq k0, k1;
    // mpz_class res[4], res2[4];
    // initializeClient(&keys_full, 3, 2);
    // generateTreeEq(&keys_full, &k0, &k1, 3, 1);
    // evaluateEq(&keys_full, &k0, res, 4);
    
    // evaluateEq(&keys_full, &k1, res2, 4);
    // for(int i = 0; i < 4; i++){
    //     std::cout<<res[i]<<" "<<res2[i]<<std::endl;
    // }
    
    // free(k0.cw[0]);free(k0.cw[1]);
    // free(k1.cw[0]);free(k1.cw[1]);
    // free(keys_full.aes_keys);
    return 0;
}
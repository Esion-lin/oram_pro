#include "fss-common.h"
#include "io.hpp"
#include "pre_op.h"
#include "easylogging++.h"
using namespace std;
INITIALIZE_EASYLOGGINGPP
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
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
    add_share<uint8_t>("player2", {"player2", "player3"},data0, 16, 
                [](uint8_t a, uint8_t b)->uint8_t{return a^b;}, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    add_share<uint8_t>("player2", {"player2", "player3"},data1, 16, 
                [](uint8_t a, uint8_t b)->uint8_t{return a^b;}, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    bool select = false;
    if(Config::myconfig->check("player2")) select = true;
    shared_select<uint8_t>(data0, data1, select, res, 16, {"player2", "player3"});
    add_reveal<uint8_t>({"player2"}, {"player2", "player3"}, res, 16, [](uint8_t a, uint8_t b)->uint8_t{return a^b;});
    for(int i = 0; i < 16; i++){
        cout<<(uint32_t)res[i]<< " ";
    }cout<<endl;
    return 0;
}
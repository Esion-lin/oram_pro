#include "outsource_dt.h"
#include "io.hpp"
#include "easylogging++.h"
#define Cons false
INITIALIZE_EASYLOGGINGPP
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
int main(int argc, char** argv){
    Config::myconfig = new Config("./3_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    if(Cons){
        uint32_t data[100],rep[100];
        Node<uint32_t> Ps[7];
        uint32_t leaves[8];
        for(int i = 0; i < 8 ;i ++){
            if(i < 7){
                Ps[i] = {i, i + 1};
                data[i] = i + 2;
            }
            leaves[i] = i;
        } 

        Dt<uint32_t, Node<T>> *dt = new Dt<uint32_t, Node<T>>(Ps, leaves, 100, 7, 8, "player2", "player0", "player1");
        replicated_share<uint32_t>("player2", {"player0", "player1", "player2"}, data, rep, 100);
        ConstDtEval<uint32_t> *dt_evl =  new ConstDtEval<uint32_t>(dt, rep, data, 100);
        dt_evl->offline();
        uint32_t res = dt_evl->online();
        add_reveal<uint32_t>({"player0", "player1", "player2"}, {"player0", "player1", "player2"}, &res, 1);
        std::cout << res<<std::endl;
        delete dt_evl;
        delete dt;
    }else{
        uint32_t data[100],rep[100];
        for(int i = 0; i < 100 ;i ++){
            data[i] = i + 2;
        } 
        Dt<uint32_t, Node_el<uint32_t>>*dt = new Dt<uint32_t, Node_el<uint32_t>>("../dt/boston", 100, "player2", "player0", "player1");
        replicated_share<uint32_t>("player2", {"player0", "player1", "player2"}, data, rep, 100);
        PolyDtEval<uint32_t> *dt_evl = new PolyDtEval<uint32_t>(dt, rep, data, 100);
        uint32_t res = dt_evl->online();
        add_reveal<uint32_t>({"player0", "player1", "player2"}, {"player0", "player1", "player2"}, &res, 1);
        std::cout << res<<std::endl;
        delete dt_evl;
        delete dt;
    }
    return 0;
}
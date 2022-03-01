#include "sot.hpp"
#include "outsource_dt.h"
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
    replicated_share<uint32_t>("player0", {"player0", "player1", "player2"}, datas, rep, 100);
    Node_el<uint32_t> Ps[15];
    for(int i = 0; i < 15 ;i ++){
        uint32_t tmp[8] = {(i * 2 + 1) % 15, (i * 2 + 2)%15, i * 2 + 1, i * 2 + 2, i+1, i >= 7?i:0, 15, 100};
        memcpy(&Ps[i], tmp, 8*sizeof(uint32_t));
    } 
    Dt<uint32_t, Node_el<uint32_t>>*dt = new Dt<uint32_t, Node_el<uint32_t>>(Ps, 15, 100, 0, "player2", 4, "player0", "player1");
    
    uint32_t idx[3] = {3, 2, 1};
    Node_el<uint32_t> res[3];
    for(int i = 0; i < 15 ;i ++){
        dt->Ps[i].t = rep[i];
        dt->Ps_rep[i].t = datas[i];
    }
    SOT<Node_el<uint32_t>> * ot = new SOT<Node_el<uint32_t>>(dt->Ps, dt->Ps_rep, 15);
    ot->offline(1);
    std::cout<<ot->online(idx[0], 0)<<std::endl;
   
    SOT<> * ot2 = new SOT<>(rep, datas, 100);
    ot2->offline(1);
    std::cout<<ot2->online(idx[0], 0)<<std::endl;
    delete dt;
    delete ot;
    delete ot2;
    return 0;
}
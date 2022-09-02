#include "GCM.h"
#include "timer.hpp"
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
INITIALIZE_EASYLOGGINGPP
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
int main(int argc, char** argv){
    Config::myconfig = new Config("./2_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    GCM* gcm = new GCM();
    // Pratical_OT* ot = new Pratical_OT(P2Pchannel::mychnl, Config::myconfig->get_player());
    // block a[10],b[10];
    // bool select[10] = {1,0,1,1,0,1,1,0, 0, 1};
    // ot->gen_rot(10, "player0", "player1");
    // std::cout<<"gen done\n";
    // ot->prg.random_block(a,10); ot->prg.random_block(b,10);
    // if(Config::myconfig->check("player0")){
    //     ot->send_with_rot(a, b, 10, "player0", "player1");
    //     for(int i = 0; i < 10 ;i ++){
    //         std::cout<<a[i]<<" "<<b[i]<<std::endl;
    //     }
    // }
    // if(Config::myconfig->check("player1")){
    //     ot->recv_with_rot(a, select, 10, "player0", "player1");
    //     for(int i = 0; i < 10 ;i ++){
    //         std::cout<<a[i]<<std::endl;
    //     }
    // }
    block*gcm_text;
    block msg = {2734782384,12834728374};
    block key = {197349837,721834727};
    if(Config::myconfig->check("player1")){
        key[0] = 12343523;
        gcm_text = new block[1];
    }else{
        gcm_text = new block[8];
    }
    gcm->ot->gen_rot(128, "player0", "player1");
    Timer::record("aes");
    P2Pchannel::mychnl->record("aes");
    gcm->do_aes(key, msg);
    P2Pchannel::mychnl->stop("aes");
    Timer::stop("aes");
    printf("\n------------------\n");
    Timer::record("ghash");
    P2Pchannel::mychnl->record("ghash");
    gcm->do_Ghash(key, msg);
    P2Pchannel::mychnl->stop("ghash");
    Timer::stop("ghash");
    printf("\n------------------\n");
    Timer::record("GC_total");
    P2Pchannel::mychnl->record("GC_total");
    gcm->runGC(gcm_text, 8, 1, "../cir/GCM.txt");
    P2Pchannel::mychnl->stop("GC_total");
    Timer::stop("GC_total");
    Timer::record("offline");
    gcm->sim_gcm_offline(key, msg);
    Timer::stop("offline");
    Timer::record("online");
    gcm->sim_gcm_online(key, msg);
    Timer::stop("online");
    Timer::test_print();
    P2Pchannel::mychnl->test_print();
    delete gcm;
    // delete ot;
    delete[] gcm_text;
}
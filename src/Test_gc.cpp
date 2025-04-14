#include "gc_op.h"
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
int main(int argc, const char** argv) {
    std::string st = argv[1];
    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, st);
    Pratical_OT* ot = new Pratical_OT(p2pchnl, st);
    Bitwise * bitwise = new Bitwise(ot, st, p2pchnl);
    uint32_t R, A;

    if(st == "player0"){
        R = 1;
        A = 2;
    }
    if(st == "player2"){
        R = 0;
        A = 0;
    }
    std::vector<std::string> files = {"../cir/left_shift.txt", "../cir/right_shift.txt", "../cir/and_32.txt", "../cir/or_32.txt", "../cir/xor_32.txt", "../cir/not_32.txt", "../cir/zero_check.txt"};
    bitwise->to_Y("player0", "player2", R, A, 32);
    bitwise->runs("player0", "player2",files,32);
    bitwise->to_As("player0", "player2",32);
    std::cout<<bitwise->r<<std::endl;
    for(auto & dat : bitwise->r_set){
        std::cout<<dat<<" ";
    }
    // delete[] c;
    delete bitwise;
    delete cfg;
    delete p2pchnl;
    delete ot;
}
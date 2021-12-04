#include "gc_op.h"
INITIALIZE_EASYLOGGINGPP
int main(int argc, const char** argv) {
    std::string st = argv[1];
    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, st);
    Pratical_OT* ot = new Pratical_OT(p2pchnl, st);
    Bitwise * bitwise = new Bitwise(ot, st, p2pchnl);
    // PRG prg;
    // block a[64],b[64],*c = new block[64],d;
    // bool select[64];
    // for(int i = 0; i < 64; i++){
    //     select[i] = false;
    // }
    // prg.random_block(a, 64);
    // prg.random_block(b, 64);
    // ot->send(a, b, 64, "player0", "player1");
    // ot->recv(c, select, 64, "player1", "player0");
    // std::cout<<a<<" "<<b<<std::endl;
    // std::cout<<c<<std::endl;
    // d = c;
    uint64_t R, A;
    if(st == "player0"){
        R = 23123;
        A = 2;
    }
    if(st == "player1"){
        R = 1231;
        A = 1;
    }
    bitwise->to_Y("player0", "player1", R, A, 32);
    bitwise->run("player0", "player1","../cir/adder64.txt",32);
    bitwise->to_A("player0", "player1",32);
    std::cout<<bitwise->r<<std::endl;
    // delete[] c;
    delete bitwise;
    delete cfg;
    delete p2pchnl;
    delete ot;
}
#include "gc_op.h"
#include "timer.hpp"
#include <omp.h>
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
int main(int argc, const char** argv) {
    std::string st = argv[1];
    int number = atoi(argv[2]);
    omp_set_num_threads((size_t)omp_get_max_threads()/6);
    Config::myconfig = new Config("../test/3_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, st);
    Config::myconfig->set_player(st);
    Pratical_OT* ot = new Pratical_OT( P2Pchannel::mychnl, st);
    Bitwise * bitwise = new Bitwise(ot, st, P2Pchannel::mychnl);
    uint64_t R[number], A[number], C[number];
    printf("start\n");
    Timer::record("runtime");
    bitwise->bitextract(R, A, C, 64, number);
    Timer::stop("runtime");
    Timer::test_print();
    // delete[] c;
    delete bitwise;
    //delete p2pchnl;
    delete ot;
}
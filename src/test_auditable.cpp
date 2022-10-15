#include <spdz.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include "BMR.h"
#include "encoder.h" 
#include "auditable.h"
#include "timer.hpp"

Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
INITIALIZE_EASYLOGGINGPP

std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;

int main(int argc, char** argv){
    //for commitment
    init_<>();
    Config::myconfig = new Config("./3_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    std::shared_ptr<SPDZ> spdz = std::make_shared<SPDZ>();
    std::shared_ptr<SPDZXOR> spdz2 = std::make_shared<SPDZXOR>();
    int tree_deep = 2;
    int tree_node = (2 << (tree_deep - 1)) - 1;
    int data_len = 1024 * 16;

    uint8_t *data = new uint8_t[data_len];
    
    mpz_class *rr = new mpz_class[data_len];
    mpz_class *rr_plus = new mpz_class[data_len];
    std::vector<std::pair<mpz_class, mpz_class>> rr_p;
    for(int i = 0; i < data_len; i++) rr_p.push_back({rr[i], rr_plus[i]});
    std::vector<Xmaterial> shared_data1 = spdz2->share(data, rr_p, data_len, "player0");
    bit2A(shared_data1, spdz2);
    delete[] data;
    delete[] rr;
    delete[] rr_plus;
    
}
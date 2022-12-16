#include <spdz.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include "BMR.h"
#include "encoder.h" 
#include "auditable.h"
#include "timer.hpp"

#include "argparse/argparse.hpp"
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
INITIALIZE_EASYLOGGINGPP
using namespace argparse;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
int main(int argc, const char** argv){
    int tree_deep = 2;
    ArgumentParser parser("Auditable_TEST", "Argument parser example");
    parser.add_argument()
        .names({"-p", "--role"})
        .description("party's role")
        .required(true);
    parser.add_argument()
        .names({"-s", "--element_size"})
        .description("oram element size(x 32 bits)")
        .required(true);
    
    parser.enable_help();
    auto err = parser.parse(argc, argv);
    std::string st = parser.get<std::string>("role");
    int data_len = parser.get<int>("element_size");
    
    init_<>();
    Config::myconfig = new Config("./2_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, st);
    Config::myconfig->set_player(st);



    std::shared_ptr<SPDZ> spdz = std::make_shared<SPDZ>();
    std::shared_ptr<Float_Encoder> enc = std::make_shared<Float_Encoder>(8, 32, spdz->get_prime());
   
    float test_spdz1[data_len], test_spdz2[data_len], test_spdz3[data_len],test_spdz4[data_len];
    mpz_class *data = new mpz_class[data_len];
    mpz_class *data2 = new mpz_class[data_len];
    mpz_class *rr = new mpz_class[data_len];
    mpz_class *rr2 = new mpz_class[data_len];


    std::vector<Amaterial> shared_data1 = spdz->share(data, rr, data_len, "player0");
    std::vector<Amaterial> shared_data2 = spdz->share(data2, rr2, data_len, "player0");
    std::cout<<"share done\n";
    spdz->prepare(data_len);
    P2Pchannel::mychnl->flush_all();
    Timer::record("Amult");
    
    std::vector<Amaterial> shared_data3 = spdz->mult<>(shared_data1, shared_data2);
    Timer::stop("Amult");
    Timer::record("reveal");
    spdz->reveal(shared_data3, "player0");
    Timer::stop("reveal");
    // std::cout<< "---------------test SPDZ2-------------\n";

    std::shared_ptr<SPDZXOR> spdz2 = std::make_shared<SPDZXOR>();
    
    
    
    std::cout<<"------------------test bit2A---------------\n";
    // uint8_t *datayttt = new uint8_t[data_len];
    // uint8_t *datayttt2 = new uint8_t[data_len];
    // mpz_class *rr_plus = new mpz_class[data_len];
    // std::vector<std::pair<mpz_class, mpz_class>> rr_p, rr_ps;
    // for(int i = 0; i < data_len; i++) rr_p.push_back({rr[i], rr_plus[i]});
    // for(int i = 0; i < data_len; i++) rr_ps.push_back({rr[i], rr_plus[i]});
    
    // std::vector<Xmaterial> shared_data122 = spdz2->share(datayttt, rr_p, data_len, "player0");
    // std::vector<Xmaterial> shared_data123 = spdz2->share(datayttt2, rr_ps, data_len, "player0");
    // bit2A(shared_data122, spdz2);
    // spdz2->prepare(data_len);
    // Timer::record("Xmult");
    // std::vector<Xmaterial> shared_data124 = spdz2->mult<>(shared_data122, shared_data123);
    // Timer::stop("Xmult");
    

   
    Timer::test_print();
    delete[] data;
    delete[] data2;
    delete[] rr;
    delete[] rr2;
    // delete[] datayttt;
    // delete[] datayttt2;
    // delete[] rr_plus;

}
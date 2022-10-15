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
    tree_deep = parser.get<int>("element_size");
    
    init_<>();
    Config::myconfig = new Config("./3_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, st);
    Config::myconfig->set_player(st);

    // std::shared_ptr<Commitment<32>> pcom = std::make_shared<Commitment<32>>();
    // std::shared_ptr<XORCommitment<32>> xcom = std::make_shared<XORCommitment<32>>();
    // mpz_class m1 = 12, m2 = 110, m3 = (m1 + pcom->q - m2) % pcom->q;
    // uint8_t b1 = 1, b2 = 0, b3 = 1;
    // mpz_class r1 = 321421, r2 = 123102, r3 = r1 - r2;
    // mpz_class r1_ = 123102, r2_ = 321421, r3_ = r1_  + r2_  + b1 * b2;
    // mpz_class t1,t2,t3;

    std::shared_ptr<SPDZ> spdz = std::make_shared<SPDZ>();
    std::shared_ptr<Float_Encoder> enc = std::make_shared<Float_Encoder>(8, 32, spdz->get_prime());
    // std::cout<< "---------------test commitment-------------\n";
    // //test pcom
    // pcom->comit(t1, m1, r1);
    // pcom->comit(t2, m2, r2);
    // mpz_class inv_temp;
    // pcom->inv(inv_temp, t2);
    // pcom->cadd(t3, t1, inv_temp);
    // printf("%d\n", pcom->decomit(t3, m3, r3));
    // //m1 * 11
    // pcom->scale(t3, t1, 11);
    // printf("%d\n", pcom->decomit(t3, m1*11, r1*11));

    // //m1 + 1123
    // pcom->padd(t3, t1, r2);
    // printf("%d\n", pcom->decomit(t3, m1+r2, r1));
    
    // //test xcom
    // xcom->comit(t1, b1, {r1_,r1});
    // xcom->comit(t2, b2, {r2_,r2});
    // pcom->cadd(t3, t1, t2);
    // printf("%d\n", xcom->decomit(t3, b3, {r3_,r3}));

    // //m1 * 11
    // // xcom->scale(t3, t1, 1);
    // // printf("%d\n", xcom->decomit(t3, b1*11, r1*11));

    // //m1 + 1123
    // xcom->padd(t3, t1, 1);
    // printf("%d\n", xcom->decomit(t3, b1 ^ 1, {r1_ + b1*1,r1}));

    // std::cout<< "---------------test SPDZ-------------\n";
    int data_len = 1024*16;
    float test_spdz1[data_len], test_spdz2[data_len], test_spdz3[data_len],test_spdz4[data_len];
    mpz_class *data = new mpz_class[data_len];
    mpz_class *data2 = new mpz_class[data_len];
    mpz_class *rr = new mpz_class[data_len];
    mpz_class *rr2 = new mpz_class[data_len];
    // //mpz_class data[data_len], data2[data_len], rr[data_len], rr2[data_len];
    // test_spdz2[0] = 1;
    // test_spdz1[0] = -1;
    // test_spdz3[0] = -1;
    // for(int i = 1; i < data_len; i ++){
    //     test_spdz1[i] = i * 1.0001;
    //     test_spdz2[i] =  -1 * test_spdz2[i - 1] + (-1.321);
    //     test_spdz3[i] = test_spdz1[i] * test_spdz2[i];
    // }
    // randomms<>(rr, spdz->get_prime(), data_len);
    // randomms<>(rr2, spdz->get_prime(), data_len);
    // enc->encode(test_spdz1, data, data_len);
    // enc->encode(test_spdz2, data2, data_len);
    // std::cout<<"share\n";
    std::vector<Amaterial> shared_data1 = spdz->share(data, rr, data_len, "player0");
    std::vector<Amaterial> shared_data2 = spdz->share(data2, rr2, data_len, "player0");
    std::cout<<"share done\n";
    // spdz->prepare(data_len);
    // P2Pchannel::mychnl->flush_all();
    // Timer::record("Conv");
    // std::vector<Amaterial> shared_data3 = spdz->mult<>(shared_data1, shared_data2);
    // Timer::stop("Conv");
    // std::vector<mpz_class> revealed_data = spdz->reveal(shared_data3, "player0");
    //decode
    // if(Config::myconfig->check("player0")) {
    //     enc->decode(revealed_data, test_spdz4, data_len);
    //     enc->erase(test_spdz4, 1, data_len);
    //     // std::cout<<revealed_data[0]<<std::endl;
    //     // for(int i = 0; i < data_len; i ++){
    //     //     std::cout<<test_spdz1[i]<< " " << test_spdz2[i] << " " << test_spdz3[i]<<" "<<test_spdz4[i]<<std::endl;
    //     // }
    // }
    
    // std::cout<< "---------------test BMR-------------\n";
    // std::shared_ptr<BMR> bmr = std::make_shared<BMR>("../cir/or_32.txt", 16);
    // bmr->offline(16);
    // P2Pchannel::mychnl->flush_all();
    // std::cout<< "---------------test offline done-------------\n";
    // std::map<std::string, std::pair<int, int>>holder;
    // holder["player0"] = {0, 32};
    // holder["player1"] = {32, 64};
    // holder["player2"] = {0, 0};
    // std::vector<std::vector<uint8_t>> bmr_data;
    // if(Config::myconfig->check("player0")){
    //     for(int j = 0; j < 16; j++){
    //         std::vector<uint8_t> temp_bmr;
    //         for(int i = 0; i < 32; i ++) temp_bmr.push_back(0);
    //         temp_bmr[j] = 1;
    //         bmr_data.push_back(temp_bmr);
    //     }
        
    // }
    // if(Config::myconfig->check("player1") ){
    //     for(int j = 0; j < 16; j++){
    //         std::vector<uint8_t> temp_bmr;
    //         for(int i = 0; i < 32; i ++) temp_bmr.push_back(0);
    //         temp_bmr[j] = 1;
    //         bmr_data.push_back(temp_bmr);
    //     }
    // }
    // bmr->online(holder, bmr_data, 16);
    // P2Pchannel::mychnl->flush_all();

    // std::cout<< "---------------test SPDZ2-------------\n";

    std::shared_ptr<SPDZXOR> spdz2 = std::make_shared<SPDZXOR>();
    // uint8_t bb0[2] = {1, 0}, bb1[2] = {1, 1};
    // mpz_class rr0_plus[2] = {1231,14552}, rr1_plus[2] = {15513,74577},spd_rr[2] = {1231,14551},spd_rr2[2] = {15512,74576};
    // std::vector<std::pair<mpz_class, mpz_class>> rr0_p,rr1_p;
    // for(int i = 0; i < 2; i++){
    //     rr0_p.push_back({rr0_plus[i], spd_rr[i]});
    //     rr1_p.push_back({rr1_plus[i], spd_rr2[i]});
    // }
    // std::vector<Xmaterial> shared_b1 = spdz2->share(bb0, rr0_p, 2, "player0");
    // //std::vector<Xmaterial> shared_b2 = spdz2->share(bb1, rr1_p, 2, "player0");
    // std::vector<Xmaterial> shared_b2;
    // if(Config::myconfig->check("player0")){
    //     shared_b2.push_back(spdz2->flap(shared_b1[0]));
    //     shared_b2.push_back(spdz2->flap(shared_b1[1]));
    // }else{
    //     shared_b2.push_back(shared_b1[0]);
    //     shared_b2.push_back(shared_b1[1]);
    // }
    
    // std::vector<Xmaterial> shared_b3 = spdz2->add(shared_b1, shared_b2);
    // std::vector<uint8_t> revealed_b = spdz2->reveal(shared_b2, "player0");
    // std::cout<<(uint32_t)revealed_b[0]<< " " <<(uint32_t)revealed_b[1]<<std::endl;
    
    int tree_node = (2 << (tree_deep - 1)) - 1;
    int leafs = (2 << (tree_deep - 1));
    std::cout<<"------------------test auditable---------------\n";
    std::vector<uint8_t> revealed_b;
    data_len = (2 << (tree_deep - 1)) - 1;;
    std::shared_ptr<Audit_AY> audit = std::make_shared<Audit_AY>();
    mpz_class threshold = Commitment<>::q/2;
    uint32_t threshold_t = threshold.get_ui();
    audit->offline(data_len);
    for(int i = 0; i < data_len; i ++){
        data[i] = i + 16;
        //std::cout<<data[i]<<" ";
    }
    //std::cout<<threshold<<std::endl;
    shared_data1 = spdz->share(data, rr, data_len, "player0");
    std::vector<Xmaterial> res = audit->transfer(shared_data1);
    std::cout<<"------------------test compare---------------\n";
    audit->run_cir_offline("../cir/comparator_32bit_unsigned_lteq.txt", data_len);
    
    // std::cout<<"p/2 "<<threshold_t<<std::endl;
    std::vector<Xmaterial> maskbits = audit->get_mask(data_len);
    std::vector<Xmaterial> in_data;
    
    for(int i = 0; i < data_len; i++){
        for(int j = 0; j < 32; j++){
            in_data.push_back(audit->spdz2->add(res[i * 32 + j], maskbits[i * 64 + j]));
        }
        for(int j = 0; j < 32; j++){
            if((threshold_t>>j) % 2 == 1 && Config::myconfig->check("player0")){
                in_data.push_back(audit->spdz2->flap(maskbits[i * 64 + 32 + j]));
            }else{
                in_data.push_back(maskbits[i * 64 + 32 + j]);
            }
        }
        
    }
    
    std::vector<uint8_t> in_data_x = audit->spdz2->reveal(in_data, "player0");
    P2Pchannel::mychnl->flush_all();
    // Timer::record("CMP");
    
    // std::vector<Xmaterial> last_ret = audit->run_cir(in_data_x);
    // Timer::stop("CMP");
    std::cout<<"---------done---------\n";
    // revealed_b = spdz2->reveal(last_ret, "player0");
    // for(auto & ele : revealed_b){
    //     std::cout<<(int)ele<<" ";
    // }
    // std::cout<<std::endl;
    // EC_KEY *key=EC_KEY_new();
    // EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_sm2);
    std::cout<<"------------------test bit2A---------------\n";
    uint8_t *datayttt = new uint8_t[data_len];
    mpz_class *rr_plus = new mpz_class[data_len];
    std::vector<std::pair<mpz_class, mpz_class>> rr_p;
    for(int i = 0; i < data_len; i++) rr_p.push_back({rr[i], rr_plus[i]});
    std::vector<Xmaterial> shared_data122 = spdz2->share(datayttt, rr_p, data_len, "player0");
    bit2A(shared_data122, spdz2);

    //lr y = wx + b
    // y = (wx + b - y')x

    //test logistic
    // data_len = 12*10;
    // std::vector<Amaterial> weight = spdz->share(data, rr, data_len, "player0");
    // std::vector<Amaterial> bias = spdz->share(data, rr, 10, "player0");
    // std::vector<Amaterial> x = spdz->share(data, rr, data_len, "player0");
    // spdz->prepare(data_len);
    // P2Pchannel::mychnl->flush_all();
    // Timer::record("logistic");
    // std::vector<Amaterial> shared_data3 = spdz->mult<>(x, weight);
    // std::vector<Amaterial> mulres,addres;
    // for(int i = 0; i < 10; i++){
    //     mulres.push_back(spdz->sum(shared_data3.data() + 12 * i, 12));
    // }
    // addres = spdz->add(mulres, bias);
    // std::vector<Xmaterial> last_ret = audit->run_cir(in_data_x);

    
    // shared_data3 = spdz->mult<>(x, weight);
    // Timer::stop("logistic");

    std::cout<<"------------------test DT---------------\n";
    std::shared_ptr<SPDZ> spdz_fortest = std::make_shared<SPDZ>();
    spdz_fortest->prepare(leafs);
    Timer::record("dt");
    std::vector<Xmaterial> last_ret = audit->run_cir(in_data_x);
    std::vector<Amaterial> element = bit2A(last_ret, spdz2);
    std::vector<Amaterial> leafs_ele(leafs);
    for(int i = 0; i <tree_deep; i++){
        std::vector<Amaterial> temp_node(2 << i);
        std::vector<Amaterial> temp_node2(2 << i);
        spdz_fortest->mult<>(temp_node, temp_node2);
    }
    spdz_fortest->sum(leafs_ele.data(), leafs_ele.size());
    Timer::stop("dt");
    Timer::test_print();
    delete[] data;
    delete[] data2;
    delete[] rr;
    delete[] rr2;
    delete[] datayttt;
    delete[] rr_plus;

}
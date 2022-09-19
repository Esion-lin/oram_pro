#include <spdz.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include "BMR.h"
#include "encoder.h" 
#include "auditable.h"
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
INITIALIZE_EASYLOGGINGPP
int main(int argc, char** argv){
    Config::myconfig = new Config("./3_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    std::shared_ptr<Commitment<32>> pcom = std::make_shared<Commitment<32>>();
    std::shared_ptr<XORCommitment<32>> xcom = std::make_shared<XORCommitment<32>>();
    mpz_class m1 = 12, m2 = 110, m3 = (m1 + pcom->q - m2) % pcom->q;
    uint8_t b1 = 1, b2 = 0, b3 = 1;
    mpz_class r1 = 321421, r2 = 123102, r3 = r1 - r2;
    mpz_class r1_ = 123102, r2_ = 321421, r3_ = r1_  + r2_  + b1 * b2;
    mpz_class t1,t2,t3;

    std::shared_ptr<SPDZ> spdz = std::make_shared<SPDZ>();
    std::shared_ptr<Float_Encoder> enc = std::make_shared<Float_Encoder>(8, 32, spdz->get_prime());
    std::cout<< "---------------test commitment-------------\n";
    //test pcom
    pcom->comit(t1, m1, r1);
    pcom->comit(t2, m2, r2);
    mpz_class inv_temp;
    pcom->inv(inv_temp, t2);
    pcom->cadd(t3, t1, inv_temp);
    printf("%d\n", pcom->decomit(t3, m3, r3));
    //m1 * 11
    pcom->scale(t3, t1, 11);
    printf("%d\n", pcom->decomit(t3, m1*11, r1*11));

    //m1 + 1123
    pcom->padd(t3, t1, r2);
    printf("%d\n", pcom->decomit(t3, m1+r2, r1));
    
    //test xcom
    xcom->comit(t1, b1, {r1_,r1});
    xcom->comit(t2, b2, {r2_,r2});
    pcom->cadd(t3, t1, t2);
    printf("%d\n", xcom->decomit(t3, b3, {r3_,r3}));

    //m1 * 11
    // xcom->scale(t3, t1, 1);
    // printf("%d\n", xcom->decomit(t3, b1*11, r1*11));

    //m1 + 1123
    xcom->padd(t3, t1, 1);
    printf("%d\n", xcom->decomit(t3, b1 ^ 1, {r1_ + b1*1,r1}));

    std::cout<< "---------------test SPDZ-------------\n";
    int data_len = 100;
    float test_spdz1[data_len], test_spdz2[data_len], test_spdz3[data_len],test_spdz4[data_len];
    mpz_class data[data_len], data2[data_len], rr[data_len], rr2[data_len];
    test_spdz2[0] = 1;
    test_spdz1[0] = -1;
    test_spdz3[0] = -1;
    for(int i = 1; i < data_len; i ++){
        test_spdz1[i] = i * 1.0001;
        test_spdz2[i] =  -1 * test_spdz2[i - 1] + (-1.321);
        test_spdz3[i] = test_spdz1[i] * test_spdz2[i];
    }
    randomms<>(rr, spdz->get_prime(), data_len);
    randomms<>(rr2, spdz->get_prime(), data_len);
    enc->encode(test_spdz1, data, data_len);
    enc->encode(test_spdz2, data2, data_len);
    std::cout<<"share\n";
    std::vector<Amaterial> shared_data1 = spdz->share(data, rr, data_len, "player0");
    std::vector<Amaterial> shared_data2 = spdz->share(data2, rr2, data_len, "player0");
    std::cout<<"share done\n";
    spdz->prepare(data_len);
    std::vector<Amaterial> shared_data3 = spdz->mult<>(shared_data1, shared_data2);
    std::vector<mpz_class> revealed_data = spdz->reveal(shared_data3, "player0");
    //decode
    if(Config::myconfig->check("player0")) {
        enc->decode(revealed_data, test_spdz4, data_len);
        enc->erase(test_spdz4, 1, data_len);
        // std::cout<<revealed_data[0]<<std::endl;
        for(int i = 0; i < data_len; i ++){
            std::cout<<test_spdz1[i]<< " " << test_spdz2[i] << " " << test_spdz3[i]<<" "<<test_spdz4[i]<<std::endl;
        }
    }
    
    std::cout<< "---------------test BMR-------------\n";
    std::shared_ptr<BMR> bmr = std::make_shared<BMR>("../cir/zero_check.txt", 16);
    bmr->offline(16);
    P2Pchannel::mychnl->flush_all();
    std::cout<< "---------------test offline done-------------\n";
    std::map<std::string, std::pair<int, int>>holder;
    holder["player0"] = {0, 32};
    holder["player1"] = {0, 0};
    holder["player2"] = {0, 0};
    std::vector<std::vector<uint8_t>> bmr_data;
    if(Config::myconfig->check("player0")){
        for(int j = 0; j < 16; j++){
            std::vector<uint8_t> temp_bmr;
            for(int i = 0; i < 32; i ++) temp_bmr.push_back(0);
            temp_bmr[j] = 1;
            bmr_data.push_back(temp_bmr);
        }
        
    }
    if(Config::myconfig->check("player1") ){
        // for(int i = 0; i < 32; i ++) bmr_data.push_back(0);
        // bmr_data[1] = 1;
    }
    bmr->online(holder, bmr_data, 16);
    P2Pchannel::mychnl->flush_all();

    std::cout<< "---------------test SPDZ2-------------\n";

    std::shared_ptr<SPDZXOR> spdz2 = std::make_shared<SPDZXOR>();
    uint8_t bb0[2] = {1, 0}, bb1[2] = {1, 1};
    mpz_class rr0_plus[2] = {1231,14552}, rr1_plus[2] = {15513,74577},spd_rr[2] = {1231,14551},spd_rr2[2] = {15512,74576};
    std::vector<std::pair<mpz_class, mpz_class>> rr0_p,rr1_p;
    for(int i = 0; i < 2; i++){
        rr0_p.push_back({rr0_plus[i], spd_rr[i]});
        rr1_p.push_back({rr1_plus[i], spd_rr2[i]});
    }
    std::vector<Xmaterial> shared_b1 = spdz2->share(bb0, rr0_p, 2, "player0");
    std::vector<Xmaterial> shared_b2 = spdz2->share(bb1, rr1_p, 2, "player0");
    std::vector<Xmaterial> shared_b3 = spdz2->add(shared_b1, shared_b2);
    std::vector<uint8_t> revealed_b = spdz2->reveal(shared_b3, "player0");
    std::cout<<(uint32_t)revealed_b[0]<< " " <<(uint32_t)revealed_b[1]<<std::endl;


    std::cout<<"------------------test auditable---------------\n";
    std::shared_ptr<Audit_AY> audit = std::make_shared<Audit_AY>();
    audit->offline(data_len);
    for(int i = 0; i < data_len; i ++){
        test_spdz1[i] = 1<<i;
        shared_data1 = spdz->share(data, rr, data_len, "player0");
    }
    audit->transfer(shared_data1);
    // EC_KEY *key=EC_KEY_new();
    // EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_sm2);


}
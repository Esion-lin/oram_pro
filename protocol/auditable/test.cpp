#include <spdz.h>
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
INITIALIZE_EASYLOGGINGPP
int main(int argc, char** argv){
    Config::myconfig = new Config("./3_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    std::shared_ptr<Commitment<>> pcom = std::make_shared<Commitment<>>();
    std::shared_ptr<XORCommitment<>> xcom = std::make_shared<XORCommitment<>>();
    mpz_class m1 = 12, m2 = 110, m3 = (m1 + pcom->q - m2) % pcom->q;
    uint8_t b1 = 1, b2 = 0, b3 = 1;
    mpz_class r1 = 321421, r2 = 123102, r3 = r1 - r2;
    mpz_class r1_ = 123102, r2_ = 321421, r3_ = r1_  + r2_  + b1 * b2;
    mpz_class t1,t2,t3;

    std::shared_ptr<SPDZ> spdz = std::make_shared<SPDZ>();

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
    mpz_class data[2] = {3,19}, rr[2] = {1231,14551};
    mpz_class data2[2] = {123,41}, rr2[2] = {15512,74576};
    std::vector<Amaterial> shared_data1 = spdz->share(data, rr, 2, "player0");
    std::vector<Amaterial> shared_data2 = spdz->share(data2, rr2, 2, "player0");
    spdz->prepare(2);
    std::vector<Amaterial> shared_data3 = spdz->mult<>(shared_data1, shared_data2);
    std::vector<mpz_class> revealed_data = spdz->reveal(shared_data3, "player0");
    // std::cout<<revealed_data[0]<<std::endl;
    for(auto& ele : revealed_data){
        std::cout<<ele<<" ";
    }

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
}
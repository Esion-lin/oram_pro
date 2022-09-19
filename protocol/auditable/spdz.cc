#include "spdz.h"
void randomm(mpz_class& target, mpz_class& pri){
    unsigned char intArray[8];
    RAND_bytes(intArray, 8);
    mpz_import(target.get_mpz_t(), 8, 1, sizeof(intArray[0]), 0, 0, intArray);
    target = target % pri;
}
void commit(mpz_class g, mpz_class h, mpz_class m, mpz_class r, mpz_class& target, mpz_class p){
    mpz_class a, b;
    mpz_powm(a.get_mpz_t(), g.get_mpz_t(), m.get_mpz_t(), p.get_mpz_t());
    mpz_powm(b.get_mpz_t(), h.get_mpz_t(), r.get_mpz_t(), p.get_mpz_t());
    target = a * b % p;
}
bool decommit(mpz_class g, mpz_class h, mpz_class m, mpz_class r, mpz_class target, mpz_class p){
    mpz_class a, b;
    mpz_powm(a.get_mpz_t(), g.get_mpz_t(), m.get_mpz_t(), p.get_mpz_t());
    mpz_powm(b.get_mpz_t(), h.get_mpz_t(), r.get_mpz_t(), p.get_mpz_t());
    // std::cout<< a * b % p << " target "<<target<<std::endl;
    return target == a * b % p;
}
template<uint32_t mpzsize = 8>
void randomms(mpz_class* target, mpz_class pri, uint32_t lens){

    unsigned char intArray[mpzsize * lens];
    RAND_bytes(intArray, mpzsize * lens);
    for(int i = 0; i < lens; i++){
        mpz_import(target[i].get_mpz_t(), mpzsize, 1, sizeof(intArray[0]), 0, 0, intArray + i * mpzsize);
        target[i] = target[i] % pri;
    }
}


SPDZ::SPDZ(std::string verifier){
    COM = new Commitment<32>();
    prime = COM->q;
    this->verifier = verifier;
}
SPDZ::~SPDZ(){
    delete COM;
}
mpz_class SPDZ::get_prime(){return prime;}
std::vector<Amaterial> SPDZ::share(mpz_class* data, mpz_class* r, const uint32_t lens, std::string owner){
    /*该方法可能会改写data*/
    std::vector<mpz_class> comts(lens);
    std::vector<Amaterial> shared_data;
    if(Config::myconfig->check(owner)){
        for(int i = 0; i < lens; i++){
            mpz_class target;
            COM->comit(target, data[i], r[i]);
            comts[i] = target;
        }
        //send comts to verifier
    }
    add_share(owner, Config::myconfig->Ps, data, prime, lens);
    add_share(owner, Config::myconfig->Ps, r, prime, lens);
    for(int i = 0; i < lens; i++){
        Amaterial tmp = {data[i],r[i],comts[i]};
        //std::cout<<data[i]<<" "<<shared_data.size()<<std::endl;
        shared_data.push_back(tmp);
    }
    return shared_data;
}
std::vector<mpz_class> SPDZ::reveal(const std::vector<Amaterial> &data, std::string owner){
    mpz_class ms_rs[data.size() * 2],comts[data.size()];
    std::transform(data.begin(), data.end(), ms_rs, [](Amaterial a) -> mpz_class{return a.m;});
    std::transform(data.begin(), data.end(), ms_rs+data.size(), [](Amaterial a) -> mpz_class{return a.r;});
    std::transform(data.begin(), data.end(), comts, [](Amaterial a) -> mpz_class{return a.c;});
    
    //reveal mr
    add_reveal<>(owner, Config::myconfig->Ps, ms_rs, prime, data.size() * 2);
    //check commit
    if(Config::myconfig->check(owner)){
        std::vector<mpz_class> res(ms_rs, ms_rs + data.size());
        printf("%d\n", COM->decomits(comts, ms_rs, ms_rs + data.size(), data.size()));

        return res;
    }else{
        std::vector<mpz_class> res;
        return res;
    }
}
std::vector<Amaterial> SPDZ::add(const std::vector<Amaterial> &a, const std::vector<Amaterial>& b){
    std::vector<Amaterial> rets;
    for(int i = 0; i < a.size(); i++){
        mpz_class cc;
        COM->cadd(cc, a[i].c, b[i].c);
        Amaterial tmp = {(a[i].m + b[i].m) % prime, (a[i].r + b[i].r) % prime,cc};
        rets.push_back(tmp);
    }
    return rets;
}
std::vector<Amaterial> SPDZ::neg(const std::vector<Amaterial> &a){
    std::vector<Amaterial> rets;
    for(int i = 0; i < a.size(); i++){
        mpz_class cc;
        COM->neg(cc, a[i].c);
        Amaterial tmp = {prime - a[i].m, prime - a[i].r ,cc};
        rets.push_back(tmp);
    }
    return rets;
}
    

void SPDZ::prepare(uint32_t lens){
    //for test
    mpz_class a[lens], b[lens], c[lens];
    mpz_class a_r[lens], b_r[lens], c_r[lens];
    if(Config::myconfig->check(verifier)){
        randomms<>(a, prime, lens);
        randomms<>(b, prime, lens);
        randomms<>(a_r, prime, lens);
        randomms<>(b_r, prime, lens);
        randomms<>(c_r, prime, lens);
        for(int i = 0; i < lens; i++){
            //triples.push_back();
            a[i] = 1;b[i] = 1;
            c[i] = a[i] * b[i] % prime;
            //std::cout<<"triple "<<(prime + 3 - a[i])%prime<<" "<<(prime + 123 - b[i])%prime<<" ";
        }
    }
    std::vector<Amaterial> shared_data1 = share(a, a_r, lens, verifier);
    std::vector<Amaterial> shared_data2 = share(b, b_r, lens, verifier);
    std::vector<Amaterial> shared_data3 = share(c, c_r, lens, verifier);
    for(int i = 0; i < lens; i++){
        //triples.push_back();
        triples.push_back({shared_data1[i],shared_data2[i],shared_data3[i]});
    }
}   


SPDZXOR::SPDZXOR(std::string verifier){
    COM = new XORCommitment<32>();
    prime = COM->get_prime();
    this->verifier = verifier;
}    
SPDZXOR::~SPDZXOR(){
    delete COM;
}
std::vector<Xmaterial> SPDZXOR::share(uint8_t* data, std::vector<std::pair<mpz_class,mpz_class>> rs, const uint32_t lens, std::string owner){
    //rs -> r'
    /*该方法可能会改写data*/
    assert(lens == rs.size());
    std::vector<mpz_class> comts(lens), rs_p(lens * 2);
    std::vector<Xmaterial> shared_data;
    if(Config::myconfig->check(owner)){
        for(int i = 0; i < lens; i++){
            COM->comit(comts[i], data[i], {rs[i].first / 2, rs[i].second});
            // std::cout<<"commit:"<<comts[i]<<" "<<(uint32_t)data[i]<<" "<<rs[i].first / 2<< " "<< rs[i].second<<std::endl;
            
        }
    }
    for(int i = 0; i < lens; i++){
        rs_p[i] = rs[i].first;
        rs_p[i + rs.size()] = rs[i].second;
    }
    add_share(owner, Config::myconfig->Ps, rs_p.data(), prime, 2*lens);
    add_share<uint8_t>(owner, Config::myconfig->Ps, data, lens, 
        [](uint8_t a, uint8_t b) -> uint8_t{return a ^ b;}, [](uint8_t a, uint8_t b) -> uint8_t{return a ^ b;});
    for(int i = 0; i < lens; i++){
        Xmaterial tmp = {data[i],{rs_p[i], rs_p[i + lens]},comts[i]};
        //std::cout<<data[i]<<" "<<shared_data.size()<<std::endl;
        shared_data.push_back(tmp);
    }
    return shared_data;
}
std::vector<uint8_t> SPDZXOR::reveal(const std::vector<Xmaterial> &data, std::string owner){
    uint8_t bs[data.size()];
    mpz_class comts[data.size()], rs[data.size() * 2];
    for(int i = 0; i < data.size(); i++){
        bs[i] = data[i].b;
        rs[i] = data[i].r_plus.first;
        rs[i + data.size()] = data[i].r_plus.second;
        comts[i] = data[i].c;
    }
    //reveal mr
    add_reveal<>(owner, Config::myconfig->Ps, rs, prime, data.size() * 2);
    add_reveal<uint8_t>(Config::myconfig->Ps, Config::myconfig->Ps, bs, data.size(), [](uint8_t a, uint8_t b) -> uint8_t{return a ^ b;});
    //check commit
    if(Config::myconfig->check(owner)){
        for(int i = 0; i < data.size(); i++){
            //r1'[last] == b
            if(rs[i] % 2 == bs[i] && COM->decomit(comts[i], bs[i], {rs[i] / 2, rs[i + data.size()]})){
                printf("pass\n");
            }else{
                std::cout<<"conflicted commit:"<<comts[i]<<" "<<(uint32_t)bs[i]<<" "<<rs[i] / 2<< " "<< rs[i + data.size()]<<std::endl;
            }
        }
    }
    std::vector<uint8_t> res(bs, bs + data.size());
    return res;
}
std::vector<Xmaterial> SPDZXOR::add(const std::vector<Xmaterial> &a, const std::vector<Xmaterial>& b){
    assert(a.size() == b.size());
    std::vector<Xmaterial> rets;
    for(int i = 0; i < a.size(); i++){
        mpz_class cc;
        COM->cadd(cc, a[i].c, b[i].c);
        Xmaterial tmp = {(a[i].b ^ b[i].b), {(a[i].r_plus.first + b[i].r_plus.first) % prime, (a[i].r_plus.second + b[i].r_plus.second) % prime},cc};
        rets.push_back(tmp);
    }
    return rets;
}

std::vector<uint8_t> SPDZXOR::mult(const std::vector<uint8_t> &a, const std::vector<uint8_t> &b){
    assert(a.size() == b.size());
    assert(triples.size() >= a.size());
    std::vector<uint8_t> res(a.size());
    std::vector<uint8_t> alphas_beta(a.size() * 2);
    mpz_class comits[a.size() * 2];
    for(int i = 0; i < a.size(); i++){
        alphas_beta[i] = a[i] ^ std::get<0>(triples[i]).b;
        alphas_beta[i + a.size()] =b[i] ^ std::get<1>(triples[i]).b;
    }
    P2Pchannel::mychnl->bloadcast(alphas_beta.data(), 2 * a.size());
    std::map<std::string, std::vector<uint8_t>> rets  = P2Pchannel::mychnl->recv_all(2 * a.size());
    for(auto & ele: rets){
        for(int i = 0; i < a.size() * 2; i++){
            alphas_beta[i] = alphas_beta[i] ^ ele.second[i];
            
        }
    }
    for(int i = 0; i < a.size(); i++){
        res[i] = (b[i] * alphas_beta[i] ^ a[i] * alphas_beta[i + a.size()] ^ std::get<2>(triples[i]).b);
    }
    return res;
}
void SPDZXOR::prepare(uint32_t lens){
    //for test
    uint8_t a[lens], b[lens], c[lens];
    mpz_class a_r[lens * 2], b_r[lens * 2], c_r[lens * 2];
    std::vector<std::pair<mpz_class, mpz_class>> a_rr,b_rr,c_rr;
    if(Config::myconfig->check(verifier)){
        RAND_bytes(a, lens);
        RAND_bytes(b, lens);
        randomms<>(a_r, prime, lens);
        randomms<>(b_r, prime, lens);
        randomms<>(c_r, prime, lens);
        for(int i = 0; i < lens; i++){
            //triples.push_back();
            a[i] = 1;b[i] = 1;
            c[i] = a[i] * b[i];
            a_rr.push_back({a_r[2*i], a_r[2*i + 1]});
            b_rr.push_back({b_r[2*i], b_r[2*i + 1]});
            c_rr.push_back({c_r[2*i], c_r[2*i + 1]});
            //std::cout<<"triple "<<(prime + 3 - a[i])%prime<<" "<<(prime + 123 - b[i])%prime<<" ";
        }
    }
    std::vector<Xmaterial> shared_data1 = share(a, a_rr, lens, verifier);
    std::vector<Xmaterial> shared_data2 = share(b, b_rr, lens, verifier);
    std::vector<Xmaterial> shared_data3 = share(c, c_rr, lens, verifier);
    for(int i = 0; i < lens; i++){
        //triples.push_back();
        triples.push_back({shared_data1[i],shared_data2[i],shared_data3[i]});
    }
}   

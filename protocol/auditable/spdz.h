#pragma once
#include <gmp.h>
#include <gmpxx.h>
#include <utility>
#include <openssl/rand.h>
#include <stack>
#include "io.hpp"
#define MOD_SUB(a, b, pri) a + pri - b % pri
#define MOD_ADD(a, b, pri) a + b % pri
inline void randomm(mpz_class& target, mpz_class& pri){
    unsigned char intArray[8];
    RAND_bytes(intArray, 8);
    mpz_import(target.get_mpz_t(), 8, 1, sizeof(intArray[0]), 0, 0, intArray);
    target = target % pri;
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
inline void commit(mpz_class g, mpz_class h, mpz_class m, mpz_class r, mpz_class& target, mpz_class p){
    mpz_class a, b;
    mpz_powm(a.get_mpz_t(), g.get_mpz_t(), m.get_mpz_t(), p.get_mpz_t());
    mpz_powm(b.get_mpz_t(), h.get_mpz_t(), r.get_mpz_t(), p.get_mpz_t());
    target = a * b % p;
}
inline bool decommit(mpz_class g, mpz_class h, mpz_class m, mpz_class r, mpz_class target, mpz_class p){
    mpz_class a, b;
    mpz_powm(a.get_mpz_t(), g.get_mpz_t(), m.get_mpz_t(), p.get_mpz_t());
    mpz_powm(b.get_mpz_t(), h.get_mpz_t(), r.get_mpz_t(), p.get_mpz_t());
    // std::cout<< a * b % p << " target "<<target<<std::endl;
    return target == a * b % p;
}

template<int FIELD_LEN = 32>
class Commitment{
public:
    static bool flag;
    static mpz_class prime, q;
    static std::pair<mpz_class, mpz_class> ele;
    Commitment(){
        if(!flag) std::cout<<"run init_ first\n";
    }
    void inv(mpz_class& target, mpz_class m){
        mpz_invert(target.get_mpz_t(), m.get_mpz_t(), prime.get_mpz_t());
    }
    void neg(mpz_class& target, mpz_class m){
        target = prime - m;
    }
    void comit(mpz_class& target, mpz_class m, mpz_class r){
        commit(ele.first, ele.second, m, r, target, prime);
    }
    void comits(mpz_class* target, mpz_class* m, mpz_class* r, uint32_t lens){
        for(int i = 0; i < lens; i++)
            commit(ele.first, ele.second, m[i], r[i], target[i], prime);
    }
    
    bool decomit(mpz_class& target, mpz_class m, mpz_class r){
        return decommit(ele.first, ele.second, m, r, target, prime);
    }
    bool decomits(mpz_class* target, mpz_class* m, mpz_class* r, uint32_t lens){
        for(int i = 0; i < lens; i++)
            if(decommit(ele.first, ele.second, m[i], r[i], target[i], prime) == false)
                return false;
        return true;
    }
    void cadd(mpz_class& target, mpz_class c1, mpz_class c2){
        target = c1 * c2 % prime;
    }
    mpz_class cadd(mpz_class c1, mpz_class c2){
        return c1 * c2 % prime;
    }
    void cadds(mpz_class* target, mpz_class* c1, mpz_class* c2, uint32_t lens){
        for(int i = 0; i < lens; i++)
            target[i] = c1[i] * c2[i] % prime;
    }
    void scale(mpz_class& target, mpz_class c, mpz_class e){
        mpz_powm(target.get_mpz_t(), c.get_mpz_t(), e.get_mpz_t(), prime.get_mpz_t());
    }
    mpz_class scale(mpz_class c, mpz_class e){
        mpz_class target;
        mpz_powm(target.get_mpz_t(), c.get_mpz_t(), e.get_mpz_t(), prime.get_mpz_t());
        return target;
    }
    void scale(mpz_class* target, mpz_class* c, mpz_class* e, uint32_t lens){
        for(int i = 0; i < lens; i++)
            mpz_powm(target[i].get_mpz_t(), c[i].get_mpz_t(), e[i].get_mpz_t(), prime.get_mpz_t());
    }
    void padd(mpz_class& target, mpz_class c, mpz_class p){
        mpz_class a;
        commit(ele.first, ele.second, p, 0, a, prime);
        target = c * a % prime;
    }
    mpz_class padd(mpz_class c, mpz_class p){
        mpz_class a;
        commit(ele.first, ele.second, p, 0, a, prime);
        a = c * a % prime;
        return a;
    }
    void padd(mpz_class* target, mpz_class* c, mpz_class* p, uint32_t lens){
        
        for(int i = 0; i < lens; i++){
            mpz_class a;
            commit(ele.first, ele.second, p[i], 0, a, prime);
            target[i] = c[i] * a % prime;
        }
    }

};
template<int FIELD_LEN>
bool Commitment<FIELD_LEN>::flag;
template<int FIELD_LEN>
mpz_class Commitment<FIELD_LEN>::prime;
template<int FIELD_LEN>
mpz_class Commitment<FIELD_LEN>::q;
template<int FIELD_LEN>
std::pair<mpz_class, mpz_class> Commitment<FIELD_LEN>::ele;
template<int FIELD_LEN = 32>
class XORCommitment{
public:
    XORCommitment(){
        pcomm = new Commitment<FIELD_LEN>();
        prime = pcomm->q;
    }
    void comit(mpz_class& target, uint8_t m, std::pair<mpz_class,mpz_class> r){
        mpz_class f = r.first * 2 + m;
        pcomm->comit(target, f, r.second);
    }
    bool decomit(mpz_class& target, uint8_t m, std::pair<mpz_class,mpz_class> r){
        mpz_class f = r.first * 2 + m;
        return pcomm->decomit(target, f, r.second);
    }

    void cadd(mpz_class& target, mpz_class c1, mpz_class c2){
        pcomm->cadd(target, c1, c2);
    }
    void flap(mpz_class& target, mpz_class c1){
        mpz_class temp;
        pcomm->comit(temp, 1, 0);
        pcomm->cadd(target, c1, temp);
    }
    // void scale(mpz_class& target, mpz_class c, mpz_class e){
    //     pcomm->scale(target, c, e);
    // }
    void padd(mpz_class& target, mpz_class c, mpz_class p){
        pcomm->padd(target, c, p);
    }
    ~XORCommitment(){
        delete pcomm;
    }
    mpz_class get_prime(){
        return prime;
    }
private:
    Commitment<FIELD_LEN>* pcomm;
    mpz_class prime;
    
};
template<int FIELD_LEN = 32>
static void init_(){
    mpz_class k;
    mpz_ui_pow_ui(Commitment<FIELD_LEN>::q.get_mpz_t(), 2, FIELD_LEN);
    mpz_nextprime(Commitment<FIELD_LEN>::q.get_mpz_t(), Commitment<FIELD_LEN>::q.get_mpz_t());
    for(int i = 2;;i++){
        Commitment<FIELD_LEN>::prime = Commitment<FIELD_LEN>::q * i + 1;
        if(mpz_probab_prime_p(Commitment<FIELD_LEN>::prime.get_mpz_t(), 50) == 2){
            k = i;
            break;
        }
            
    }
    randomm(Commitment<FIELD_LEN>::ele.first, Commitment<FIELD_LEN>::q);randomm(Commitment<FIELD_LEN>::ele.second, Commitment<FIELD_LEN>::q);
    mpz_powm(Commitment<FIELD_LEN>::ele.first.get_mpz_t(), Commitment<FIELD_LEN>::ele.first.get_mpz_t(), k.get_mpz_t(), Commitment<FIELD_LEN>::prime.get_mpz_t());
    mpz_powm(Commitment<FIELD_LEN>::ele.second.get_mpz_t(), Commitment<FIELD_LEN>::ele.second.get_mpz_t(), k.get_mpz_t(), Commitment<FIELD_LEN>::prime.get_mpz_t());
    Commitment<FIELD_LEN>::flag = true;
}
struct Amaterial{
    mpz_class m,r,c;
};
struct Xmaterial{
    uint8_t b;
    std::pair<mpz_class, mpz_class>r_plus;
    mpz_class c;
};
using multtriple = std::tuple<Amaterial, Amaterial,Amaterial>;
using xortriple = std::tuple<Xmaterial, Xmaterial, Xmaterial>;
class SPDZ{
    public:
    SPDZ(std::string verifier = "player0"){
        COM = new Commitment<>();
        prime = COM->q;
        this->verifier = verifier;
    }
    ~SPDZ(){
        delete COM;
    }
    mpz_class get_prime(){return prime;}
    std::vector<Amaterial> share(mpz_class* data, mpz_class* r, const uint32_t lens, std::string owner){
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
    std::vector<mpz_class> reveal(const std::vector<Amaterial> &data, std::string owner){
        mpz_class ms_rs[data.size() * 2],comts[data.size()];
        std::transform(data.begin(), data.end(), ms_rs, [](Amaterial a) -> mpz_class{return a.m;});
        std::transform(data.begin(), data.end(), ms_rs+data.size(), [](Amaterial a) -> mpz_class{return a.r;});
        std::transform(data.begin(), data.end(), comts, [](Amaterial a) -> mpz_class{return a.c;});
        
        //reveal mr
        add_reveal<>(owner, Config::myconfig->Ps, ms_rs, prime, data.size() * 2);
        //check commit
        if(Config::myconfig->check(owner)){
            std::vector<mpz_class> res(ms_rs, ms_rs + data.size());
            // printf("%d\n", COM->decomits(comts, ms_rs, ms_rs + data.size(), data.size()));

            return res;
        }else{
            std::vector<mpz_class> res;
            return res;
        }
    }
    std::vector<Amaterial> add(const std::vector<Amaterial> &a, const std::vector<Amaterial>& b){
        std::vector<Amaterial> rets;
        for(int i = 0; i < a.size(); i++){
            mpz_class cc;
            COM->cadd(cc, a[i].c, b[i].c);
            Amaterial tmp = {(a[i].m + b[i].m) % prime, (a[i].r + b[i].r) % prime,cc};
            rets.push_back(tmp);
        }
        return rets;
    }
    std::vector<Amaterial> neg(const std::vector<Amaterial> &a){
        std::vector<Amaterial> rets;
        for(int i = 0; i < a.size(); i++){
            mpz_class cc;
            COM->neg(cc, a[i].c);
            Amaterial tmp = {prime - a[i].m, prime - a[i].r ,cc};
            rets.push_back(tmp);
        }
        return rets;
    }
    
    template<uint32_t mpzsize = 8>
    std::vector<Amaterial> mult(const std::vector<Amaterial> &a, const std::vector<Amaterial> &b){
        assert(a.size() == b.size());
        assert(triples.size() >= a.size());
        //return value
        std::vector<Amaterial> res(a.size());

        std::vector<mpz_class> alphas_beta(a.size() * 2), alphas_beta_r(a.size() * 2);
        //a - \alpha's comits and b- \beta's comits
        mpz_class comits[a.size() * 2];
        for(int i = 0; i < a.size(); i++){
            alphas_beta[i] = MOD_SUB(a[i].m, std::get<0>(triples[i]).m, prime);
            alphas_beta[i + a.size()] = MOD_SUB(b[i].m, std::get<1>(triples[i]).m, prime);
            alphas_beta_r[i] = MOD_SUB(a[i].r, std::get<0>(triples[i]).r, prime);
            alphas_beta_r[i + a.size()] = MOD_SUB(b[i].r, std::get<1>(triples[i]).r, prime);
            if(Config::myconfig->check(verifier)){
                mpz_class inv_temp,inv_temp2;
                COM->inv(inv_temp, std::get<0>(triples[i]).c);
                COM->cadd(comits[i], a[i].c, inv_temp);
                COM->inv(inv_temp2, std::get<1>(triples[i]).c);
                COM->cadd(comits[i + a.size()], b[i].c, inv_temp2);
            }
        }
        //open
        // std::vector<Amaterial> tttt;
        // for(int i = 0; i < 2 * a.size(); i++){
        //     //tttt.push_back({std::get<0>(triples[i]).m, std::get<0>(triples[i]).r, std::get<0>(triples[i]).c});
        //     tttt.push_back({alphas_beta[i], alphas_beta_r[i], comits[i]});
        // }
        // std::cout<<"opend "<<reveal(tttt, verifier)[0]<<std::endl;
        uint8_t temp[alphas_beta.size() * mpzsize] = {0};
        uint8_t temp2[alphas_beta_r.size() * mpzsize] = {0};
        mpz2byte<mpzsize>(temp, alphas_beta.data(), 2 * a.size());
        mpz2byte<mpzsize>(temp2, alphas_beta_r.data(), 2 * a.size());
        P2Pchannel::mychnl->bloadcast(temp, 2 * a.size()* mpzsize);
        
        std::map<std::string, std::vector<uint8_t>> rets = P2Pchannel::mychnl->recv_all(2 * a.size()* mpzsize);
        P2Pchannel::mychnl->bloadcast(temp2, 2 * a.size()* mpzsize);
        std::map<std::string, std::vector<uint8_t>> ret_rs = P2Pchannel::mychnl->recv_all(2 * a.size()* mpzsize);
        
        for(auto & ele: rets){
            mpz_class temp_mpz[a.size() * 2];
            byte2mpz<mpzsize>(temp_mpz, ele.second.data(), 2 * a.size());
            for(int i = 0; i < a.size() * 2; i++){
                alphas_beta[i] = (alphas_beta[i] + temp_mpz[i]) % prime;
                
            }
            
        }
        for(auto & ele: ret_rs){
            mpz_class temp_mpz[a.size() * 2];
            byte2mpz<mpzsize>(temp_mpz, ele.second.data(), 2 * a.size());
            for(int i = 0; i < a.size() * 2; i++){
                alphas_beta_r[i] = (alphas_beta_r[i] + temp_mpz[i]) % prime;
            }
            
        }
        // for(int i = 0; i < a.size() * 2; i++){
        //     std::cout<<"opend "<<alphas_beta[i]<<" "<<alphas_beta_r[i]<<std::endl;
        // }
        //decommit
        if(Config::myconfig->check(verifier)){
            printf("%d\n", COM->decomits(comits, alphas_beta.data(), alphas_beta_r.data(), a.size() * 2));
        }
        //y(x-a) + x(y-b) + ab - (x -a )(y-b)
        for(int i = 0; i < a.size(); i++){
            //for data
            res[i].m = (b[i].m * alphas_beta[i] + a[i].m * alphas_beta[i + a.size()] + std::get<2>(triples[i]).m) % prime;
            if(Config::myconfig->check("player1")){
                res[i].m = MOD_SUB(res[i].m, alphas_beta[i] * alphas_beta[i + a.size()], prime);
            }
            //for r
            res[i].r = (b[i].r * alphas_beta[i] + a[i].r * alphas_beta[i + a.size()] + std::get<2>(triples[i]).r) % prime;
            // if(Config::myconfig->check("player1")){
            //     res[i].r = MOD_SUB(res[i].r, alphas_beta[i] * alphas_beta[i + a.size()], prime);
            // }
            //for comits
            res[i].c = COM->cadd(
                COM->cadd(COM->scale(b[i].c, alphas_beta[i]), COM->scale(a[i].c, alphas_beta[i + a.size()])), 
                COM->padd( std::get<2>(triples[i]).c, prime - (alphas_beta[i] * alphas_beta[i + a.size()] % prime))
                );
        }
        P2Pchannel::mychnl->flush_all();
        return res;
    }
    void prepare(uint32_t lens){
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

private:
    Commitment<> *COM; 
    mpz_class prime;
    std::vector<multtriple> triples; 
    std::string verifier;
};

class SPDZXOR{

public:
    SPDZXOR(std::string verifier = "player0"){
        COM = new XORCommitment<>();
        prime = COM->get_prime();
        this->verifier = verifier;
    }    
    ~SPDZXOR(){
        delete COM;
    }
    std::vector<Xmaterial> share(uint8_t* data, std::vector<std::pair<mpz_class,mpz_class>> rs, const uint32_t lens, std::string owner){
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
    std::vector<uint8_t> reveal(const std::vector<Xmaterial> &data, std::string owner){
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
                if(rs[i] % 2 != bs[i]){
                    std::cout<<"conflicted rs b"<<rs[i]<<" "<<bs[i]<<std::endl;
                }
                else if(!COM->decomit(comts[i], bs[i], {rs[i] / 2, rs[i + data.size()]})){
                    std::cout<<"conflicted commit:"<<comts[i]<<" "<<(uint32_t)bs[i]<<" "<<rs[i]<< " "<< rs[i + data.size()]<<std::endl;
                    // printf("pass\n");
                }
            }
        }
        std::vector<uint8_t> res(bs, bs + data.size());
        return res;
    }
    std::vector<Xmaterial> add(const std::vector<Xmaterial> &a, const std::vector<Xmaterial>& b){
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
    Xmaterial add(const Xmaterial a, const Xmaterial b){
        mpz_class cc;
        COM->cadd(cc, a.c, b.c);
        Xmaterial tmp = {(a.b ^ b.b), {(a.r_plus.first + b.r_plus.first) % prime, (a.r_plus.second + b.r_plus.second) % prime},cc};
        return tmp;
    }
    Xmaterial flap(const Xmaterial a){
        mpz_class cc;
        COM->flap(cc, a.c);
        Xmaterial tmp = {(a.b ^ 1), {(a.r_plus.first + 1) % prime, a.r_plus.second },cc};
        return tmp;
    }
    template<uint32_t mpzsize = 8>
    std::vector<Xmaterial> mult(const std::vector<Xmaterial> &a, const std::vector<Xmaterial> &b){
        assert(a.size() == b.size());
        assert(triples.size() >= a.size());
        std::vector<Xmaterial> res(a.size());
        std::vector<uint8_t> alphas_beta(a.size() * 2);
        std::vector<std::pair<mpz_class, mpz_class>> alphas_beta_r(a.size() * 2);
        mpz_class comits[a.size() * 2];
        for(int i = 0; i < a.size(); i++){
            alphas_beta[i] = a[i].b ^ std::get<0>(triples[i]).b;
            alphas_beta[i + a.size()] =b[i].b ^ std::get<1>(triples[i]).b;
            alphas_beta_r[i] = {MOD_SUB(a[i].r_plus.first, std::get<0>(triples[i]).r_plus.first, prime), MOD_SUB(a[i].r_plus.second, std::get<0>(triples[i]).r_plus.second, prime) };
            alphas_beta_r[i + a.size()] = {MOD_SUB(b[i].r_plus.first, std::get<1>(triples[i]).r_plus.first, prime), MOD_SUB(b[i].r_plus.second, std::get<1>(triples[i]).r_plus.second, prime) };
            if(Config::myconfig->check(verifier)){
                COM->cadd(comits[i], a[i].c, std::get<0>(triples[i]).c);
                COM->cadd(comits[i + a.size()], b[i].c, std::get<1>(triples[i]).c);
            }
        }
        P2Pchannel::mychnl->bloadcast(alphas_beta.data(), 2 * a.size());
        std::map<std::string, std::vector<uint8_t>> rets  = P2Pchannel::mychnl->recv_all(2 * a.size());
        for(auto & ele: rets){
            for(int i = 0; i < a.size() * 2; i++){
                alphas_beta[i] = alphas_beta[i] ^ ele.second[i];
                
            }
        }
        for(int i = 0; i < a.size(); i++){
            res[i].b = (b[i].b * alphas_beta[i] ^ a[i].b * alphas_beta[i + a.size()] ^ std::get<2>(triples[i]).b);
        }
        return res;
    }
    std::vector<uint8_t> mult(const std::vector<uint8_t> &a, const std::vector<uint8_t> &b){
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
    void prepare(uint32_t lens){
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
private:
    XORCommitment<> *COM;
    mpz_class prime;
    std::string verifier;
    std::vector<xortriple> triples; 
};
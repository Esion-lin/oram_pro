#pragma once
#include <gmp.h>
#include <gmpxx.h>
#include <utility>
#include <openssl/rand.h>
#include <stack>
#include "io.hpp"
#define MOD_SUB(a, b, pri) a + pri - b % pri
#define MOD_ADD(a, b, pri) a + b % pri
void randomm(mpz_class& target, mpz_class& pri);
template<uint32_t mpzsize = 8>
void randomms(mpz_class* target, mpz_class pri, uint32_t lens);
void commit(mpz_class g, mpz_class h, mpz_class m, mpz_class r, mpz_class& target, mpz_class p);
bool decommit(mpz_class g, mpz_class h, mpz_class m, mpz_class r, mpz_class target, mpz_class p);
template<int FIELD_LEN>
class Commitment{
public:
    mpz_class prime, q;
    std::pair<mpz_class, mpz_class> ele;
    Commitment(){
        init();
    }
    void init(){
        mpz_class k;
        mpz_ui_pow_ui(q.get_mpz_t(), 2, FIELD_LEN);
        mpz_nextprime(q.get_mpz_t(), q.get_mpz_t());
        for(int i = 2;;i++){
            prime = q * i + 1;
            if(mpz_probab_prime_p(prime.get_mpz_t(), 50) == 2){
                k = i;
                break;
            }
                
        }
        randomm(ele.first, q);randomm(ele.second, q);
        mpz_powm(ele.first.get_mpz_t(), ele.first.get_mpz_t(), k.get_mpz_t(), prime.get_mpz_t());
        mpz_powm(ele.second.get_mpz_t(), ele.second.get_mpz_t(), k.get_mpz_t(), prime.get_mpz_t());

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
    SPDZ(std::string verifier = "player0");
    ~SPDZ();
    mpz_class get_prime();
    std::vector<Amaterial> share(mpz_class* data, mpz_class* r, const uint32_t lens, std::string owner);
    std::vector<mpz_class> reveal(const std::vector<Amaterial> &data, std::string owner);
    std::vector<Amaterial> add(const std::vector<Amaterial> &a, const std::vector<Amaterial>& b);
    std::vector<Amaterial> neg(const std::vector<Amaterial> &a);
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
    void prepare(uint32_t lens);
private:
    Commitment<32> *COM; 
    mpz_class prime;
    std::vector<multtriple> triples; 
    std::string verifier;
};
class SPDZXOR{
public:
    SPDZXOR(std::string verifier = "player0");
    ~SPDZXOR();
    std::vector<Xmaterial> share(uint8_t* data, std::vector<std::pair<mpz_class,mpz_class>> rs, const uint32_t lens, std::string owner);
    std::vector<uint8_t> reveal(const std::vector<Xmaterial> &data, std::string owner);
    std::vector<Xmaterial> add(const std::vector<Xmaterial> &a, const std::vector<Xmaterial>& b);
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
    std::vector<uint8_t> mult(const std::vector<uint8_t> &a, const std::vector<uint8_t> &b);
    void prepare(uint32_t lens);
private:
    XORCommitment<32> *COM;
    mpz_class prime;
    std::string verifier;
    std::vector<xortriple> triples; 
};
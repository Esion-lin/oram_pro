
#include <gmp.h>
#include <gmpxx.h>
#include <openssl/rand.h>
#include <iostream>
#include <string>
#include <utility>
using namespace std;


void randomm(mpz_class& target, mpz_class& pri){
    unsigned char intArray[32];
    RAND_bytes(intArray, 32);
    mpz_import(target.get_mpz_t(), 32, 1, sizeof(intArray[0]), 0, 0, intArray);
    target = target % pri;
}
void commit(mpz_class& g, mpz_class& h, mpz_class& m, mpz_class& r, mpz_class& target, mpz_class& p){
    mpz_class a, b;
    mpz_powm(a.get_mpz_t(), g.get_mpz_t(), m.get_mpz_t(), p.get_mpz_t());
    mpz_powm(b.get_mpz_t(), h.get_mpz_t(), r.get_mpz_t(), p.get_mpz_t());
    target = a * b % p;
}



void foo(std::string& param) {
  std::cout << "std::string& version" << std::endl;
}
void foo(std::string&& param) {
  std::cout << "std::string&& version" << std::endl;
}

template<typename T>
void wrapper(T&& param) {
  // foo(param); 
  foo(std::forward<T>(param));
}


int main()
{
    mpz_class p, g, h, r = 1024, m=2048, t1, t2, t3;
    mpz_ui_pow_ui(p.get_mpz_t(), 2, 32);
    mpz_nextprime(p.get_mpz_t(), p.get_mpz_t());
    randomm(g, p);randomm(h, p);
    commit(g, h, m, r, t1, p);
    commit(g, h, m, r, t2, p);
    m = m*2;r=r*2;
    commit(g, h, m, r, t3, p);
    std::cout<<t1*t2 %p<<std::endl;
    cout<<t3<<endl;

    foo("foo");

    std::string t = "foo";
    foo(t);


   
}
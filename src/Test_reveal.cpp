
#include "preliminaries.hpp"

#include "easylogging++.h"

#include "timer.hpp"
using namespace std;
INITIALIZE_EASYLOGGINGPP
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;


int test1(int a, int b, int n){
    if(n == 0) return a;
    return test1(b, a+b, n-1);
}
int test1(int n){
    return test1(0, 1, n);
}

int test2(int n){
    if(n == 1 || n==0) return n;
    return test2(n - 1) + test2(n - 2);
}


int main(int argc, char** argv){
    int r,x=7,y=1239,n=12389239, z;
    Timer::record("tes1t");
    // std::cout<<test1(30)<<std::endl;
    for(int i = 0; i < 1000000; i++) r = (x + y ) %n;
    Timer::stop("tes1t");
   
    Timer::record("tes2t");
    // std::cout<<test2(30)<<std::endl;
    for(int i = 0; i < 1000000; i++) {
        z = x + y;
        r = z - (n & -(z >=n));
    }
    Timer::stop("tes2t");
    Timer::test_print();
    // /*test full domain*/
    // Fss keys_full;
    // ServerKeyEq k0, k1;
    // mpz_class res[4], res2[4];
    // initializeClient(&keys_full, 3, 2);
    // generateTreeEq(&keys_full, &k0, &k1, 3, 1);
    // evaluateEq(&keys_full, &k0, res, 4);
    
    // evaluateEq(&keys_full, &k1, res2, 4);
    // for(int i = 0; i < 4; i++){
    //     std::cout<<res[i]<<" "<<res2[i]<<std::endl;
    // }
    
    // free(k0.cw[0]);free(k0.cw[1]);
    // free(k1.cw[0]);free(k1.cw[1]);
    // free(keys_full.aes_keys);
    return 0;
}
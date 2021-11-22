#include "preliminaries.hpp"
#include "easylogging++.h"
#include "risc.h"
#include <bitset>
INITIALIZE_EASYLOGGINGPP
int main(int argc, char** argv){
    Ins testins = {2,3,1,1,12,12312};
    uint64_t tmp = i2m(testins), tmp2;
    Env testenv;
    set_T<uint64_t>(testenv.mem, 0, tmp);
    tmp2 = load_T<uint64_t>(testenv.mem, 0);
    Ins ans = m2i(tmp2);
    std::cout<<ans;
}
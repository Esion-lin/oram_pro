#include <NTL/ZZ_pXFactoring.h>
#include <NTL/ZZ_pEX.h>
#include <NTL/ZZX.h>

using namespace std;
using namespace NTL;

int main()
{
    ZZX te;
    ZZX P;
    for(int i = 0; i < 19; i++){
        SetCoeff(te, i, i*2);
        
    }
    // SetCoeff(P, 20, 1);
    SetCoeff(P, 19, 1);
    // SetCoeff(P, 18, 1);
    ZZX ret;
    ret = (uint64_t)10177326765571957764;
    // MulMod(ret, te, te, P);
    ret = ret - ret;
    printf("%lu \n",deg(ret));
    for(int i = 0; i < deg(ret)+1; i++){
        long a= 2;
        ZZ c;
        GetCoeff(c, ret, i);
        conv(a, c);
        printf("%lu \n",a);
    }
}
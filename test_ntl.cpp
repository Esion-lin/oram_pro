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
        SetCoeff(te, i, i);
        SetCoeff(P, i, 1);
    }

    ZZX ret = te*te;
    // MulMod(ret, te, te, P);
    printf("%lu \n",deg(ret));
    for(int i = 0; i < 19; i++){
        long a= 2;
        ZZ c;
        GetCoeff(c, ret, i);
        conv(a, c);
        printf("%lu \n",a);
    }
}
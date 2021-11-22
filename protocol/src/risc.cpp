#include "risc.h"
#include "preliminaries.hpp"
Ins m2i(uint64_t data){
    Ins ans;
    ans.imme = (uint32_t)data;
    data>>=IMM_LEN;
    ans.pad = data % (1<<PAD_LEN);
    data>>=PAD_LEN;
    ans.j = data % (1<<REGIS_LEN);
    data>>=REGIS_LEN;
    ans.i = data % (1<<REGIS_LEN);
    data>>=REGIS_LEN;
    ans.idic = data % (1<<INDIC_LEN);
    data>>=INDIC_LEN;
    ans.optr = data % (1<<OPT_LEN);
    return ans;
}
uint64_t i2m(Ins data){
    uint64_t ans = 0;
    ans += data.optr;
    ans <<= INDIC_LEN;
    ans += data.idic;
    ans <<= REGIS_LEN;
    ans+= data.i;
    ans <<= REGIS_LEN;
    ans+= data.j;
    ans <<= PAD_LEN;
    ans+= data.pad;
    ans <<= IMM_LEN;
    ans+= data.imme;
    return ans;
}
//from aid
void Mechine::load_env(){
    replicated_share<WORD>(myenv.m, M_LEN, st, p2pchnl);
    replicated_share<WORD>(myenv.mem, MEM_LEN, st, p2pchnl);
    replicated_share<WORD>(myenv.tape1, TAPE_LEN, st, p2pchnl);
    replicated_share<WORD>(myenv.tape2, TAPE_LEN, st, p2pchnl);
    fourpc_share<WORD>(&(myenv.pc), 2, st, p2pchnl);
    twopc_share<WORD>(&(myenv.rc), 2, st, p2pchnl);
}            
void Mechine::load_env(std::string path){}//from file
#include "risc.h"
#include "preliminaries.hpp"
#include <iostream>
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
Ins rand_ins(){
    Ins ans;
    ans.optr = rand()%(1<<OPT_LEN);
    ans.idic = rand()%(1<<INDIC_LEN);
    ans.i = rand()%(1<<REGIS_LEN);
    ans.j = rand()%(1<<REGIS_LEN);
    ans.pad = rand()%(1<<PAD_LEN);
    ans.imme = rand();
    return ans;
}
uint64_t sub_ins(uint64_t a, uint64_t b){
    uint64_t res = i2m(m2i(a) - m2i(b));
    return res;
}
uint64_t mul_ins(uint64_t a, uint64_t b){
    uint64_t res = i2m(m2i(a) * m2i(b));
    return res;
}
uint64_t mul_ins(uint64_t a, uint32_t b){
    uint64_t tmp = b;
    tmp <<= 32;
    tmp += b;
    return mul_ins(a, tmp);
}
//from aid
void Mechine::load_env(){
    replicated_share<WORD>(myenv.m, M_LEN, st, p2pchnl);
    /*TODO: add a callback func*/
    replicated_share<INS_TYPE>(reinterpret_cast<INS_TYPE*>(myenv.mem), MEM_LEN/2, st, p2pchnl, sub_ins);
    
    replicated_share<WORD>(myenv.tape1, TAPE_LEN, st, p2pchnl);
    replicated_share<WORD>(myenv.tape2, TAPE_LEN, st, p2pchnl);
    
    fourpc_share<WORD>(&(myenv.pc), 2, st, p2pchnl);
    twopc_share<WORD>(&(myenv.rc), 2, st, p2pchnl);
    ins_ram = new Ram<uint64_t>(reinterpret_cast<uint64_t*>(myenv.mem), MEM_LEN/2, st, p2pchnl);
    ins_ram->init();
}            
void Mechine::load_env(std::string path){}//from file

Ins Mechine::load_ins(){
    ins_ram->prepare_read(1);
    return m2i(ins_ram->read(myenv.pc, false, mul_ins));
}
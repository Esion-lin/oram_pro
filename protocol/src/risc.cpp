#include "risc.h"
#include "preliminaries.h"
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
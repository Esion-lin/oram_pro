#include "risc.h"
#include "preliminaries.hpp"
#include <iostream>
#include "operation.h"
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
Ins rand_ins_t(){
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
uint64_t add_ins(uint64_t a, uint64_t b){
    uint64_t res = i2m(m2i(a) + m2i(b));
    return res;
}
uint64_t mul_ins(uint64_t a, uint64_t b){
    uint64_t res = i2m(m2i(a) * m2i(b));
    return res;
}
uint64_t mul_ins_single(uint64_t a, uint32_t b){
    Ins ans;
    ans.optr = b%(1<<OPT_LEN);
    ans.idic = b%(1<<INDIC_LEN);
    ans.i = b%(1<<REGIS_LEN);
    ans.j = b%(1<<REGIS_LEN);
    ans.pad = b%(1<<PAD_LEN);
    ans.imme = b;
    return i2m(m2i(a) * ans);
}
void rand_ins(uint64_t *data, uint16_t len){
    for(int i = 0; i < len; i++){
        data[i] = i2m(rand_ins_t());
    }
}
void rand_pc(uint32_t *data, uint16_t len){
    for(int i = 0; i < len; i++){
        data[i] = rand() % MEM_LEN;
    }
}
//from aid
void Mechine::load_env(){
    replicated_share<WORD>(myenv.m, M_LEN, st, p2pchnl);
    replicated_share<INS_TYPE>(reinterpret_cast<INS_TYPE*>(myenv.mem), MEM_LEN/2, st, p2pchnl, sub_ins, rand_ins);
    replicated_share<WORD>(myenv.tape1, TAPE_LEN, st, p2pchnl);
    replicated_share<WORD>(myenv.tape2, TAPE_LEN, st, p2pchnl);
    
    fourpc_share<WORD>(&(myenv.pc), 1, st, p2pchnl, [](WORD a, WORD b) -> WORD{return sub(a, b, MEM_LEN);}, rand_pc);
    fourpc_share<WORD>(&(myenv.flag), 1, st, p2pchnl);
    twopc_share<WORD>(&(myenv.rc), 2, st, p2pchnl);
    ins_ram = new Ram<uint64_t>(reinterpret_cast<uint64_t*>(myenv.mem), MEM_LEN/2, st, p2pchnl);
    m_ram = new Ram<WORD>(myenv.m, M_LEN, st, p2pchnl);
    dupliM<uint32_t> dup = {M_LEN, 0, myenv.m};
    dm_ram = new Ram<WORD>(dup, 2* M_LEN, st, p2pchnl);
    beta_ram = new Ram<WORD>(nullptr, 1<<OPT_LEN, st, p2pchnl);
    ins_ram->init();m_ram->init();dm_ram->init();beta_ram->init();
}            
void Mechine::load_env(std::string path){}//from file

Ins Mechine::load_ins(){
    /*offline*/
    ins_ram->prepare_read(1);
    m_ram->prepare_read(2);
    dm_ram->prepare_read(1);
    beta_ram->prepare_read(1);
    conv->fourpc_zeroshare<uint32_t>(2);

    
    now_ins = m2i(ins_ram->read(myenv.pc, false, mul_ins_single, add_ins));
    if(st == "player1" || st == "player3"){
        now_ins = !now_ins;
    }
    /*make flag and imme replicated*/
    WORD arr[2] = {myenv.flag, now_ins.imme};
    conv->fourpc_share_2_replicated_share<uint32_t>(arr, 2);
    myenv.flag = arr[0]; 
    dm_ram->data.A = arr[1]; 
    /*take Ri, Rj, A*/
    Ri = m_ram->read(now_ins.i, false);
    Rj = m_ram->read(now_ins.j, false);
    A = dm_ram->read(now_ins.idic, false);
    if(st == "player1" || st == "player3"){
        Ri = -Ri;Rj = -Rj;A = -A;
    }
    beta_ram->recover_list(now_ins.optr, betas, false);
    // for(int i = 0; i < OPT_SIZE; i++){
    //     std::cout<<betas[i]<<" ";
    // }
    return now_ins;
}
void Mechine::run_op(){
    run_op(now_ins, Ri, Rj, A);
}
void Mechine::run_op(Ins now_ins, uint32_t Ri, uint32_t Rj, uint32_t A){

	/*
	 * run all instruct
	 *get [res] and [flags]
	 *use op select res' and flag' and owrite to R_i and flag
	 *
	 */
    /*cmp*/
    
    res[COMPE] = Ri;
    Cmp_a_e_ae* eq_cmp = new Cmp_a_e_ae(&Rj, &A, &res[COMPE], &flags[COMPE], 1, 1, st, p2pchnl);
    Cmp_a_e_ae* ae_cmp = new Cmp_a_e_ae(&Rj, &A, &res[COMPAE], &flags[COMPAE], 1, 2, st, p2pchnl);

    /*add/sub*/
    Add_sub * add_op = new Add_sub(&Rj, &A, &res[ADD], &flags[ADD], 1, 1, st, p2pchnl);
    Add_sub * sub_op = new Add_sub(&Rj, &A, &res[SUB], &flags[SUB], 1, 2, st, p2pchnl);
    
    std::cout<<Rj<<" "<<A<<std::endl;/*offline*/
    eq_cmp->offline();
    ae_cmp->offline();
    add_op->offline();
    sub_op->offline();
    /*run*/
    /*round-1*/
    eq_cmp->round1();
    ae_cmp->round1();
    add_op->round1();
    sub_op->round1();
    /*round-2*/
    eq_cmp->round2();
    ae_cmp->round2();
    add_op->round2();
    sub_op->round2();
    /*round-3*/
    eq_cmp->round3();
    ae_cmp->round3();
    add_op->round3();
    sub_op->round3();
    /*round-end*/
    eq_cmp->roundend();
    ae_cmp->roundend();
    add_op->roundend();
    sub_op->roundend();
    /*regulate*/
	res[COMPAE] = res[COMPE];
    res[COMPA] = res[COMPE];
    flags[COMPA] = flags[COMPAE] - flags[COMPE];
    delete eq_cmp;
    delete ae_cmp;
    delete add_op;
    delete sub_op;
}

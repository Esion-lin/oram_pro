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
    replicated_share<INS_TYPE>(reinterpret_cast<INS_TYPE*>(myenv.mem), ins_count, st, p2pchnl, sub_ins, rand_ins);
    replicated_share<WORD>(&myenv.mem[ins_count*2], MEM_LEN - 2*ins_count, st, p2pchnl);
    replicated_share<WORD>(myenv.tape1, TAPE_LEN, st, p2pchnl);
    replicated_share<WORD>(myenv.tape2, TAPE_LEN, st, p2pchnl);
    /*due to fetch data, limit pc in [0, MEM_LEN]*/
    fourpc_share<WORD>(&(myenv.pc), 1, st, p2pchnl);
    
    fourpc_share<WORD>(&(myenv.flag), 1, st, p2pchnl);
    twopc_share<WORD>(&(myenv.rc), 2, st, p2pchnl);
    ins_ram = new Ram<uint64_t>(reinterpret_cast<uint64_t*>(myenv.mem), MEM_LEN/2, st, p2pchnl);
    mem_ram = new Ram<uint32_t>(myenv.mem, MEM_LEN, st, p2pchnl);
    m_ram = new Ram<WORD>(myenv.m, M_LEN, st, p2pchnl);
    dupliM<uint32_t> dup = {M_LEN, 0, myenv.m};
    dm_ram = new Ram<WORD>(dup, 2* M_LEN, st, p2pchnl);
    beta_ram = new Ram<WORD>(nullptr, 1<<OPT_LEN, st, p2pchnl);
    ins_ram->init();m_ram->init();dm_ram->init();beta_ram->init();mem_ram->init();
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
    if(st == "player1" || st == "player3" )
        for(int i = 0; i < OPT_SIZE; i++){
            
            betas[i] = -betas[i];
        }
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
    if(st == "player0") myenv.pc += 2;
    res[CMOV] = Ri;
    res[STORE] = Ri;
    
    flags[CMOV] = myenv.flag;
    flags[LOAD] = myenv.flag;
    flags[STORE] = myenv.flag;
    flags[JMP] = myenv.flag;
    flags[CJMP] = myenv.flag;
    flags[CNJMP] = myenv.flag;
    Cmp_a_e_ae* eq_cmp = new Cmp_a_e_ae(&Rj, &A, &res[COMPE], &flags[COMPE], 1, 1, st, p2pchnl);
    Cmp_a_e_ae* ae_cmp = new Cmp_a_e_ae(&Rj, &A, &res[COMPAE], &flags[COMPAE], 1, 2, st, p2pchnl);

    /*add/sub*/
    Add_sub * add_op = new Add_sub(&Rj, &A, &res[ADD], &flags[ADD], 1, 1, st, p2pchnl);
    Add_sub * sub_op = new Add_sub(&Rj, &A, &res[SUB], &flags[SUB], 1, 2, st, p2pchnl);
    
    /* mov & cmov*/
    Mov *mov_op = new Mov(&Rj, &A, &res[MOV], &flags[MOV], 1, 1, st, p2pchnl);
    Mov *cmov_op = new Mov(&Rj, &A, &res[CMOV], &flags[CMOV], 1, 2, st, p2pchnl);
    
    /* load & store*/
    Load * load_op = new Load(&Rj, &A, &res[LOAD], &flags[LOAD], 1, 1, st, p2pchnl, mem_ram);
    Store * store_op = new Store(&Ri, &A, &res[STORE], &flags[STORE], 1, 1, st, p2pchnl, mem_ram);
    Jump_op * jump_op = new Jump_op(Rj, A, &res[JMP], &flags[JMP], 1,st, p2pchnl);
    Jump_op * cjump_op = new Jump_op(Rj, A, &res[CJMP], &flags[CJMP], 2,st, p2pchnl);
    Jump_op * cnjump_op = new Jump_op(Rj, A, &res[CNJMP], &flags[CNJMP], 3,st, p2pchnl);
    /*offline*/
    eq_cmp->offline();
    ae_cmp->offline();
    add_op->offline();
    sub_op->offline();
    mov_op->offline();
    cmov_op->offline();
    load_op->offline();
    store_op->offline();
    /*run*/
    /*round-1*/
    eq_cmp->round1();
    ae_cmp->round1();
    add_op->round1();
    sub_op->round1();
    mov_op->round1();
    cmov_op->round1();
    load_op->round1();
    store_op->round1();
    jump_op->round1(&(myenv.pc));
    cjump_op->round1(&(myenv.pc));
    cnjump_op->round1(&(myenv.pc));
    /*round-2*/
    eq_cmp->round2();
    ae_cmp->round2();
    add_op->round2();
    sub_op->round2();
    mov_op->round2();
    cmov_op->round2();
    load_op->round2();
    store_op->round2();
    jump_op->round2(&(myenv.pc), betas[JMP]);
    cjump_op->round2(&(myenv.pc), betas[CJMP]);
    cnjump_op->round2(&(myenv.pc), betas[CNJMP]);
    /*round-3*/
    eq_cmp->round3();
    ae_cmp->round3();
    add_op->round3();
    sub_op->round3();
    mov_op->round3();
    cmov_op->round3();
    load_op->round3();
    store_op->round3(load_op->res, betas[STORE]);
    jump_op->round3(&(myenv.pc), betas[JMP]);
    cjump_op->round3(&(myenv.pc), betas[CJMP]);
    cnjump_op->round3(&(myenv.pc), betas[CNJMP]);
    myenv.pc = jump_op->j_pc + cjump_op->j_pc + cnjump_op->j_pc;
    /*round-end*/
    eq_cmp->roundend();
    ae_cmp->roundend();
    add_op->roundend();
    sub_op->roundend();
    store_op->roundend(load_op->res, betas[STORE]);
    /*regulate*/
    Ri_rep = res[STORE];
    res[COMPE] = res[STORE];
	res[COMPAE] = res[STORE];
    res[COMPA] = res[STORE];
    res[JMP] = res[STORE];
    res[CJMP] = res[STORE];
    res[CNJMP] = res[STORE];
    flags[COMPA] = flags[COMPAE] - flags[COMPE];
    delete eq_cmp;
    delete ae_cmp;
    delete add_op;
    delete sub_op;
    delete mov_op;
    delete cmov_op;
    delete load_op;
    delete store_op;
    delete jump_op;
    delete cjump_op;
    delete cnjump_op;
}
void Mechine::ret_res(){
    now_res = 0;
    now_flag = 0;
    for(int i = 0; i < OPT_SIZE; i++){
        now_res += res[i] * betas[i];
        now_flag += flags[i] * betas[i];
        
    }
    m_ram->prepare_write(1);
    m_ram->write_1(now_ins.i, now_res, Ri_rep, false, true);
    m_ram->write_2(now_ins.i, now_res, Ri_rep, false, true);
    myenv.flag = now_flag;

}
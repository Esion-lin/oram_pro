#include "risc.h"
#include "preliminaries.hpp"
#include <iostream>
#include "operation.h"
#include "gc_op.h"
#include "timer.hpp"
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
    // for(int i = 0; i < 4; i++){
    //     Ins test_ins = m2i(load_T<uint64_t>(myenv.mem, i));
    //     std::cout<<test_ins<<std::endl;
    // }
    replicated_share<WORD>(myenv.tape1, TAPE_LEN, st, p2pchnl);
    replicated_share<WORD>(myenv.tape2, TAPE_LEN, st, p2pchnl);
    /*due to fetch data, limit pc in [0, MEM_LEN]*/
    fourpc_share<WORD>(&(myenv.pc), 1, st, p2pchnl);
    
    fourpc_share<WORD>(&(myenv.flag), 1, st, p2pchnl);
    fourpc_share<WORD>(&(myenv.rc0), 4, st, p2pchnl);
    ins_ram = new Ram<uint64_t>(reinterpret_cast<uint64_t*>(myenv.mem), PRO_SIZE/2, st, p2pchnl);
    mem_ram = new Ram<uint32_t>(&myenv.mem[PRO_SIZE], MEM_LEN - PRO_SIZE, st, p2pchnl);
    tape1_ram = new Ram<WORD>(myenv.tape1, TAPE_LEN, st, p2pchnl);
    tape2_ram = new Ram<WORD>(myenv.tape2, TAPE_LEN, st, p2pchnl);
    m_ram = new Ram<WORD>(myenv.m, M_LEN, st, p2pchnl);
    dupliM<uint32_t> dup = {M_LEN, 0, myenv.m};
    dm_ram = new Ram<WORD>(dup, 2* M_LEN, st, p2pchnl);
    beta_ram = new Ram<WORD>(nullptr, 1<<OPT_LEN, st, p2pchnl);
    ins_ram->init();m_ram->init();dm_ram->init();beta_ram->init();mem_ram->init();tape1_ram->init();tape2_ram->init();
    // for(int i = 0; i < 4; i++){
    //     Ins test_ins = m2i(load_T<uint64_t>(myenv.mem, i));
    //     std::cout<<test_ins<<std::endl;
    // }
}            
void Mechine::load_env(std::string path){}//from file

Ins Mechine::load_ins(){
    /*offline*/
    ins_ram->prepare_read(1);
    m_ram->prepare_read(2);
    dm_ram->prepare_read(1);
    beta_ram->prepare_read(1);
    conv->fourpc_zeroshare<uint32_t>(2);
    // Ins test_ins = m2i(load_T<uint64_t>(myenv.mem, 3));
    count_num ++;
    
    now_ins = m2i(ins_ram->read(myenv.pc, false, mul_ins_single, add_ins));
    // std::cout<<test_ins<<std::endl;
    // std::cout<<now_ins<<std::endl;
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
    uint32_t stop;
    twopc_reveal<uint32_t>(&betas[0], &stop, 1, st, p2pchnl);
    if(stop == 1) done = true;

    return now_ins;
}
void Mechine::run_op(){
    run_op(now_ins, Ri, Rj, A);
}
void Mechine::run_op(Ins now_ins, uint32_t Ri, uint32_t Rj, uint32_t A){
    Timer::record("run_ins_offline");
	/*
	 * run all instruct
	 *get [res] and [flags]
	 *use op select res' and flag' and owrite to R_i and flag
	 *
	 */
    /*cmp*/
    Pratical_OT* ot = new Pratical_OT(p2pchnl, st);
    Bitwise * bitwise = new Bitwise(ot, st, p2pchnl);
#ifdef Total_bitwise
    std::vector<std::string> gcfiles = {"../cir/and_32.txt", "../cir/or_32.txt", "../cir/xor_32.txt", "../cir/not_32.txt", "../cir/left_shift.txt", "../cir/right_shift.txt"};
#else

    std::vector<std::string> gcfiles = {"../cir/right_shift.txt"};
#endif
    if(st == "player0") myenv.pc += 1;
    res[CMOV] = Ri;
    res[STORE] = Ri;
    
    flags[CMOV] = myenv.flag;
    flags[LOAD] = myenv.flag;
    flags[STORE] = myenv.flag;
    flags[JMP] = myenv.flag;
    flags[CJMP] = myenv.flag;
    flags[CNJMP] = myenv.flag;
    Cmp_a_e_ae* eq_cmp = new Cmp_a_e_ae(&Ri, &A, &res[COMPE], &flags[COMPE], 1, 1, st, p2pchnl);
    Cmp_a_e_ae* ae_cmp = new Cmp_a_e_ae(&Ri, &A, &res[COMPAE], &flags[COMPAE], 1, 2, st, p2pchnl);
    Read * read_op = new Read(A, &myenv.rc0, &res[READ], &flags[READ], st, p2pchnl, tape1_ram, tape2_ram);
    WORD read_eq[2];
    Cmp_a_e_ae*read_flag = new Cmp_a_e_ae(&myenv.rc0, &myenv.num0, read_eq, read_eq, 2, 1, st, p2pchnl);
    /*add/sub*/
    Add_sub * add_op = new Add_sub(&Rj, &A, &res[ADD], &flags[ADD], 1, 1, st, p2pchnl);
    Add_sub * sub_op = new Add_sub(&Rj, &A, &res[SUB], &flags[SUB], 1, 2, st, p2pchnl);
#ifdef NEED_MUL
    Mul *mul_op = new Mul(Rj, A, &res[MULL], &flags[MULL], st, p2pchnl);
#endif
    /* mov & cmov*/
    Mov *mov_op = new Mov(&Rj, &A, &res[MOV], &flags[MOV], 1, 1, st, p2pchnl);
#ifdef NEED_CMOV
    Mov *cmov_op = new Mov(&Rj, &A, &res[CMOV], &flags[CMOV], 1, 2, st, p2pchnl);
#endif
    /* load & store*/
    Load * load_op = new Load(&Rj, &A, &res[LOAD], &flags[LOAD], 1, 1, st, p2pchnl, mem_ram);
    Store * store_op = new Store(&Ri, &A, &res[STORE], &flags[STORE], 1, 1, st, p2pchnl, mem_ram);
    Jump_op * jump_op = new Jump_op(Rj, A, &res[JMP], &flags[JMP], 1,st, p2pchnl);
    Jump_op * cjump_op = new Jump_op(Rj, A, &res[CJMP], &flags[CJMP], 2,st, p2pchnl);
    Jump_op * cnjump_op = new Jump_op(Rj, A, &res[CNJMP], &flags[CNJMP], 3,st, p2pchnl);
    /*offline*/
    read_flag->offline();
    eq_cmp->offline();
    ae_cmp->offline();
    add_op->offline();
    sub_op->offline();
#ifdef NEED_MUL
    mul_op->offline();
#endif
    mov_op->offline();
#ifdef NEED_CMOV
    cmov_op->offline();
#endif
    load_op->offline();
    store_op->offline();
    read_op->offline(betas[READ]);
    Timer::stop("run_ins_offline");
    /*for test*/
    Timer::record("share trans");
    uint32_t new_value[2],old_value[2] = {Rj, A};
    if(st == "player1") p2pchnl->send_data_to("player0", old_value, 2*sizeof(uint32_t));
    else if(st == "player3") p2pchnl->send_data_to("player2", old_value, 2*sizeof(uint32_t));
    else if(st == "player0") p2pchnl->recv_data_from("player1", old_value, 2*sizeof(uint32_t));
    else if(st == "player2") p2pchnl->recv_data_from("player3", old_value, 2*sizeof(uint32_t));
    bitwise->to_Y("player0", "player2", Rj+old_value[0], A+old_value[1], 32);
    Timer::stop("share trans");
    /*run*/
    /*round-1*/
    Timer::record("round1");
    read_flag->round1();
    eq_cmp->round1();
    ae_cmp->round1();
    add_op->round1();
    sub_op->round1();
#ifdef NEED_MUL
    mul_op->round1();
#endif
    mov_op->round1();
#ifdef NEED_CMOV
    cmov_op->round1();
#endif
    load_op->round1();
    store_op->round1();
    jump_op->round1(&(myenv.pc));
    cjump_op->round1(&(myenv.pc));
    cnjump_op->round1(&(myenv.pc));
    if(st == "player0")
        bitwise->runs("player0", "player2", gcfiles, 32);
    
    read_op->round1();
    Timer::stop("round1");
    /*round-2*/
    Timer::record("round2");
    read_flag->round2();
    eq_cmp->round2();
    ae_cmp->round2();
    add_op->round2();
    sub_op->round2();
#ifdef NEED_MUL
    mul_op->round2();
#endif
    mov_op->round2();
#ifdef NEED_CMOV
    cmov_op->round2();
#endif
    load_op->round2();
    store_op->round2();
    jump_op->round2(&(myenv.pc), betas[JMP]);
    cjump_op->round2(&(myenv.pc), betas[CJMP]);
    cnjump_op->round2(&(myenv.pc), betas[CNJMP]);
    if(st == "player2")
        bitwise->runs("player0", "player2", gcfiles, 32);
    if(st == "player0"){
        bitwise->to_As("player0", "player2",32);
    }
    read_op->round2();
    Timer::stop("round2");
    /*round-3*/
    read_flag->round3();
    eq_cmp->round3();
    ae_cmp->round3();
    add_op->round3();
    sub_op->round3();
#ifdef NEED_MUL
    mul_op->round3();
#endif
    mov_op->round3();
#ifdef NEED_CMOV
    cmov_op->round3();
#endif
    load_op->round3();
    store_op->round3(load_op->res, betas[STORE]);
    jump_op->round3(&(myenv.pc), betas[JMP]);
    cjump_op->round3(&(myenv.pc), betas[CJMP]);
    cnjump_op->round3(&(myenv.pc), betas[CNJMP]);
   
    myenv.pc += jump_op->j_pc + cjump_op->j_pc + cnjump_op->j_pc;
    if(st == "player0"){
        p2pchnl->send_data_to("player1", &bitwise->r, sizeof(uint64_t));
        p2pchnl->send_data_to("player1", &bitwise->r_flag, sizeof(uint64_t));

        for(int i = AND; i <= SHR; i++){
            res[i] = bitwise->r;
            flags[i] = bitwise->r_flag;
        }    
    }
    if(st == "player2"){ 
        bitwise->to_As("player0", "player2",32);
        uint32_t r_arr[6], flag_arr[6];
        
        for(int i = 0; i < 6; i++){
            r_arr[i] = bitwise->r_set[i];
            flag_arr[i] = bitwise->rflag_set[i];
        } 
        p2pchnl->send_data_to("player3", r_arr, sizeof(uint32_t)* 6);
        p2pchnl->send_data_to("player3", flag_arr, sizeof(uint32_t)* 6);
#ifdef Total_bitwise
        for(int i = AND; i <= SHR; i++) {
            res[i] = r_arr[i - AND];
            flags[i] = flag_arr[i - AND];
        }
#else
        res[SHR] = r_arr[0];
        flags[SHR] = flag_arr[0];
#endif
    }
    read_op->round3(read_eq);
    
    /*round-end*/
    read_flag->roundend();
    eq_cmp->roundend();
    ae_cmp->roundend();
    add_op->roundend();
    sub_op->roundend();
#ifdef NEED_MUL
    mul_op->roundend();
#endif
    store_op->roundend(load_op->res, betas[STORE]);
    if(st == "player1"){
        uint64_t r,r_flag;
        p2pchnl->recv_data_from("player0", &r, sizeof(uint64_t));
        p2pchnl->recv_data_from("player0", &r_flag, sizeof(uint64_t));
        for(int i = AND; i <= SHR; i++){ 
            res[i] = r;
            flags[i] = r_flag;
        }
    }if(st == "player3"){
        uint32_t r_arr[6], flag_arr[6];
        p2pchnl->recv_data_from("player2", &r_arr, sizeof(uint32_t)* 6);
        p2pchnl->recv_data_from("player2", &flag_arr, sizeof(uint32_t)* 6);
#ifdef Total_bitwise
        for(int i = AND; i <= SHR; i++){
            res[i] = r_arr[i - AND];
            flags[i] = flag_arr[i - AND];
            
        } 
#else
        res[SHR] = r_arr[0];
        flags[SHR] = flag_arr[0];
#endif
        
    }
    read_op->roundend(read_eq);
    
    #ifdef THREE_ROUND_MSB
    mul_op->roundexp();
    P2Pchannel::mychnl->flush_all();
    #endif

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
#ifdef NEED_CMOV
    delete cmov_op;
#endif
    delete load_op;
    delete store_op;
    delete jump_op;
    delete cjump_op;
    delete cnjump_op;
    delete bitwise;
    delete ot;
    delete read_op;
    delete read_flag;
#ifdef NEED_MUL
    delete mul_op;
#endif
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
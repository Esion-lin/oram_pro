#ifndef _OPERATION_H__
#define _OPERATION_H__
#include "net.hpp"
#include "scmp.hpp"
class Operator_base{
    public: 

    virtual void round1();
    virtual void round2();
    virtual void round3();
};
class Operation{
private:

public:
    void b_and();
    void b_or();
    void b_not();
    void b_xor();
    void b_shl();
    void b_shr();
    void a_add();
    void a_sub();
    void a_mull();
    void a_mulh();
    void cmpa();
    void cmpe();
    void cmpae();
    void mov();
    void cmov();
    void jmp();
    void cjmp();
    void cnjmp();
    void load();
    void store();
    void read();
    
    
};

template <typename T>
T add_t(T a, T b){
    return a+b;
}

template <typename T>
T sub_t(T a, T b){
    return a-b;
} 

class Add_sub{
private:
    Compare *overflow;
    std::string st;
    P2Pchannel*p2pchnl;
    Convert *conv;
   /*
    * 1 -> add
    * 2 ->sub
    */
    int type_of = 1;
public:

    WORD* data_a;
    WORD* data_b;
    uint16_t len;
    WORD* flag;
    WORD* res;
    Add_sub(WORD*data_a, WORD*data_b, WORD*res, WORD*flag, uint16_t len, int type_of, std::string st,P2Pchannel* p2pchnl):
                                    data_a(data_a),data_b(data_b),res(res),flag(flag),len(len),type_of(type_of),st(st),p2pchnl(p2pchnl){
        overflow = new Compare(st, p2pchnl);
        conv = new Convert(st, p2pchnl);
    }
    ~Add_sub(){
        delete overflow;
        delete conv;
    }
    void offline(){
        overflow->scmp_off(len);
        
        conv->fourpc_zeroshare<uint32_t>(len);
    }
    void total(){
        for(int i = 0; i < len; i++){
            if(type_of == 1)
                res[i] = data_a[i] + data_b[i];
            else
                res[i] = data_a[i] - data_b[i];
        }
        conv->fourpc_share_2_replicated_share(res, len);
        if(type_of == 1)
            overflow->overflow( data_a, data_b, len,flag);
        else{
            overflow->overflow(data_a, data_b, len, flag, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        }
    }
    void round1(){
        for(int i = 0; i < len; i++){
            if(type_of == 1)
                res[i] = data_a[i] + data_b[i];
            else
                res[i] = data_a[i] - data_b[i];
        }
        conv->fourpc_share_2_replicated_share_1(res, len);
        if(type_of == 1)
            overflow->overflow_1( data_a, data_b, len,flag);
        else
            overflow->overflow_1(data_a, data_b, len, flag, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});

    }
    void round2(){

        conv->fourpc_share_2_replicated_share_2(res, len);
        if(type_of == 1)
            overflow->overflow_2(data_a, data_b, len,flag);
        else
            overflow->overflow_2(data_a, data_b, len, flag, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }
    void round3(){
        if(type_of == 1)
            overflow->overflow_3( data_a, data_b, len,flag);
        else
            overflow->overflow_3(data_a, data_b, len, flag, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }
    void roundend(){
        if(type_of == 1)
            overflow->overflow_end( data_a, data_b, len,flag);
        else
            overflow->overflow_end(data_a, data_b, len, flag, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
    }
    
};
class Mul{
    /*
    1. msb33 , msb34, add->replicated
    2. mul ->replicated
    3. msb, sub
    */
    std::string st;
    P2Pchannel*p2pchnl;
    Compare* eq_zero;
    Compare* msb_ans;
    uint64_t buf_ans;
    Compare* msb_33;
    uint64_t buf_33;
    Compare* msb_34;
    uint64_t buf_34;
    Compare* msb_33_diag;
    uint64_t buf_33_diag;
    Compare* msb_34_diag;
    uint64_t buf_34_diag;
    uint64_t total_mul;
public:
    WORD data_a;
    WORD data_b;
    uint16_t len;
    WORD* flag;
    WORD* res;
    Mul(WORD data_a, WORD data_b, WORD*res, WORD*flag, std::string st,P2Pchannel* p2pchnl):data_a(data_a),data_b(data_b),res(res), flag(flag), st(st), p2pchnl(p2pchnl){
        msb_ans = new Compare(st, p2pchnl);
        msb_33 = new Compare(st, p2pchnl);
        msb_34 = new Compare(st, p2pchnl);
        msb_33_diag = new Compare(st, p2pchnl);
        msb_34_diag = new Compare(st, p2pchnl);
        eq_zero = new Compare(st, p2pchnl);
    }
    Mul(std::string st,P2Pchannel* p2pchnl):st(st), p2pchnl(p2pchnl){
        msb_ans = new Compare(st, p2pchnl);
        msb_33 = new Compare(st, p2pchnl);
        msb_34 = new Compare(st, p2pchnl);
        msb_33_diag = new Compare(st, p2pchnl);
        msb_34_diag = new Compare(st, p2pchnl);
        eq_zero = new Compare(st, p2pchnl);
    }
    ~Mul(){
        delete eq_zero;
        delete msb_ans;
        delete msb_33;
        delete msb_34;
        delete msb_33_diag;
        delete msb_34_diag;
    }
    void offline(){
        msb_33->msb_off(1, 33, true);
        msb_34->msb_off(1, 34, true);
        msb_33_diag->msb_diag_off(1, 33, true);
        msb_34_diag->msb_diag_off(1, 34, true);
        msb_ans->msb_off(1, 33, true);
        eq_zero->eq_off(1, true);
    }
    void round1(){
        /*
        get the msb of R,A, transfer R/A to replicated
        */
        uint64_t R_64 = (uint64_t)data_a;
        uint64_t A_64 = (uint64_t)data_b;
        uint32_t mR33,mR34,mA33,mA34;
        twopc_reveal_1<uint64_t>(&R_64, &R_64, 1, st, p2pchnl);
        diag_reveal_1<uint64_t>(&A_64, &A_64, 1, st, p2pchnl);
        msb_33->msb_1<uint32_t>(&data_a, 1, &mR33, &buf_33, 33);
        msb_34->msb_1<uint32_t>(&data_a, 1, &mR34, &buf_34, 34);
        msb_33_diag->msb_1<uint32_t>(&data_b, 1, &mA33, &buf_33_diag, 33);
        msb_34_diag->msb_1<uint32_t>(&data_b, 1, &mA34, &buf_34_diag, 34);

    }
    void round2(){
        uint64_t R_64 = (uint64_t)data_a, R64;
        uint64_t A_64 = (uint64_t)data_b, A64;
        uint32_t mR33,mR34,mA33,mA34;
        twopc_reveal_2<uint64_t>(&R_64, &R64, 1, st, p2pchnl);
        diag_reveal_2<uint64_t>(&A_64, &A64, 1, st, p2pchnl);
        msb_33->msb_2<uint32_t>(&data_a, 1, &mR33, &buf_33, 33, 0, true);
        msb_34->msb_2<uint32_t>(&data_a, 1, &mR34, &buf_34, 34, 0, true);
        msb_33_diag->msb_diag_2<uint32_t>(&data_b, 1, &mA33, &buf_33_diag, 33, 0, true);
        msb_34_diag->msb_diag_2<uint32_t>(&data_b, 1, &mA34, &buf_34_diag, 34, 0, true);
        R64 = R64 - ((uint64_t)mR33<<32) - ((uint64_t)mR34<<33);
        A64 = A64 - ((uint64_t)mA33<<32) - ((uint64_t)mA34<<33);
        total_mul = R64* A64;
        twopc_reveal_1<uint64_t>(&total_mul, &total_mul, 1, st, p2pchnl);

    }
    void round3(){
        uint64_t total_rep;
        twopc_reveal_2<uint64_t>(&total_mul, &total_rep, 1, st, p2pchnl);
        res[0] = total_rep, res[1] = (total_rep >> 32);
        uint32_t ms_bit, zero = 0, zero_ans;
        msb_ans->msb_1<uint32_t>(&res[0], 1, &ms_bit, &buf_ans, 33, 0, true);

        eq_zero->equal_1(&res[1], &zero, 1, &zero_ans, true);
    }
    void roundend(){
        uint32_t ms_bit, zero = 0, zero_ans;
        msb_ans->msb_2<uint32_t>(&res[0], 1, &ms_bit, &buf_ans, 33, 0, true);
        eq_zero->equal_2(&res[1], &zero, 1, &zero_ans, true);
        if(st == "player0" || st == "player1") zero_ans = 1 - zero_ans;
        if(st == "player2" || st == "player3") zero_ans = - zero_ans;
        
        res[1] += ms_bit;
        flag[0] = ms_bit + zero_ans;
        flag[1] = ms_bit + zero_ans;
        
    }
};
class Cmp_a_e_ae{
private:
    Compare *cmp;
    std::string st;
    P2Pchannel*p2pchnl;
    Convert *conv;
   /*
    *1 -> cmpe
    *2 -> cmpae
    *3 -> cmpa
    */
    int type_of = 1;
public:

    WORD* data_a;
    WORD* data_b;
    uint16_t len;
    WORD* flag;
    WORD* res;
	Cmp_a_e_ae(WORD*data_a, WORD*data_b, WORD*res, WORD*flag, uint16_t len, int type_of, std::string st,P2Pchannel* p2pchnl):
		        data_a(data_a),data_b(data_b),res(res),flag(flag),len(len),type_of(type_of),st(st),p2pchnl(p2pchnl){
		cmp = new Compare(st, p2pchnl);
	}
    ~Cmp_a_e_ae(){
        delete cmp;
    }
    void offline(){
        if(type_of == 1) cmp->eq_off(len);
        else cmp->scmp_off(len);
    }
	void round1(){
        if(type_of == 1){
            cmp->equal_1(data_a, data_b, len, flag);
        }else if(type_of == 2){
            cmp->overflow_1(data_a, data_b, len, flag, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        }
		
	   
	}
	void round2(){
        if(type_of == 1){
            cmp->equal_2(data_a, data_b, len, flag);
        }else if(type_of == 2){
            cmp->overflow_2(data_a, data_b, len, flag, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        }
    }
	void round3(){
		if(type_of == 1){
            cmp->equal_3(data_a, data_b, len, flag);
        }else if(type_of == 2){
            cmp->overflow_3(data_a, data_b, len, flag, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        }
	}
    void roundend(){
        if(type_of == 2){
            cmp->overflow_end(data_a, data_b, len, flag, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
            for(int i = 0; i < len; i++){
                if(st == "player0" || st == "player1" )
                    flag[i] = 1 - flag[i];
                if(st == "player2" || st == "player3" )
                    flag[i] = - flag[i];
            }
        }
        
    }
};
class Mov{
    std::string st;
    P2Pchannel*p2pchnl;
    Convert *conv;
    /*  1 mov
        2 cmov
    */
    int type_of = 1;
public:
    WORD* data_a;
    WORD* data_b;
    uint16_t len;
    WORD* flag;
    WORD* res;
    Mov(WORD*data_a, WORD*data_b, WORD*res, WORD*flag, uint16_t len, int type_of, std::string st,P2Pchannel* p2pchnl):
		        data_a(data_a),data_b(data_b),res(res),flag(flag),len(len),type_of(type_of),st(st),p2pchnl(p2pchnl){
                    /*
                    res store Ri
                    flag store flag
                    
                    */
                    conv = new Convert(st, p2pchnl);
    }
    void offline(){
        conv->fourpc_zeroshare<uint32_t>(len);
    }
    void round1(){
        if(type_of == 1){
            for(int i = 0; i < len; i++) res[i] = data_b[i];
            conv->fourpc_share_2_replicated_share_1(res, len);
        }else{
            diag_reveal_1<uint32_t>(data_b,data_b,len, st, p2pchnl);
            diag_reveal_1<uint32_t>(res,res,len, st, p2pchnl);
            
            /*
            A_0,R_0    A_1,R_1   flag_0 (flag0+flag1) * (a0+a1)
            A_0,R_0    A_1,R_1   flag_1
            */
        }
    }
    void round2(){
        if(type_of == 1){
            conv->fourpc_share_2_replicated_share_2(res, len);
        }else{
            uint32_t tmpa[len], tmpb[len];
            diag_reveal_2<uint32_t>(data_b,tmpa,len, st, p2pchnl);
            diag_reveal_2<uint32_t>(res,tmpb,len, st, p2pchnl);
            /*
            a * flag + (1-flag)*res -*-:Ri -*-
            */
            for(int i = 0; i < len; i++){
                uint32_t tmpflag;
                if(st == "player0" || st == "player1") tmpflag = 1 - flag[i];
                else tmpflag = - flag[i];
                res[i] = tmpa[i] * flag[i] + tmpflag * tmpb[i];
            }
            conv->fourpc_share_2_replicated_share_1(res, len);
        }
    }
    void round3(){
        if(type_of == 2){
            conv->fourpc_share_2_replicated_share_2(res, len);
        }
    }
    ~Mov(){
        delete conv;
    }
};

class Load{
    std::string st;
    P2Pchannel*p2pchnl;
    Convert *conv;
    Ram<WORD>* mem_ram;
    
    int type_of = 1;
public:
    WORD* data_a;
    WORD* data_b;
    uint16_t len;
    WORD* flag;
    WORD* res;
    Load(WORD*data_a, WORD*data_b, WORD*res, WORD*flag, uint16_t len, int type_of, std::string st,P2Pchannel* p2pchnl, Ram<WORD>* mem_ram):
		        data_a(data_a),data_b(data_b),res(res),flag(flag),len(len),type_of(type_of),st(st),p2pchnl(p2pchnl),mem_ram(mem_ram){
                    conv = new Convert(st, p2pchnl);
    }
    void offline(){
        mem_ram->prepare_read(1);
        conv->fourpc_zeroshare<uint32_t>(len);
    }
    void round1(){
        for(int i = 0; i < len; i++)
            mem_ram->read_1(data_b[i], false);
    }
    void round2(){
        for(int i = 0; i < len; i++){
            res[i] = mem_ram->read_2(data_b[i], false);
            if(st == "player1" || st == "player3") res[i] = -res[i];
        }
        conv->fourpc_share_2_replicated_share_1(res, len);
    }
    void round3(){
        conv->fourpc_share_2_replicated_share_2(res, len);
    }
    ~Load(){
        delete conv;
    }
};
class Store{
    std::string st;
    P2Pchannel*p2pchnl;
    Convert *conv;
    Ram<WORD>* mem_ram;
    /*  1 mov
        2 cmov
    */
    WORD* Ri;
    int type_of = 1;
public:
    WORD* data_a;
    WORD* data_b;
    uint16_t len;
    WORD* flag;
    WORD* res;
    Store(WORD*data_a, WORD*data_b, WORD*res, WORD*flag, uint16_t len, int type_of, std::string st,P2Pchannel* p2pchnl, Ram<WORD>* mem_ram):
		        data_a(data_a),data_b(data_b),res(res),flag(flag),len(len),type_of(type_of),st(st),p2pchnl(p2pchnl),mem_ram(mem_ram){
                    /*
                    data_a : Ri
                    data_b : A
                    
                    */
                    Ri = (WORD *) malloc(sizeof(WORD)*len);
                    memcpy(Ri, data_a, sizeof(WORD)*len);
                    conv = new Convert(st, p2pchnl);
    }
    void offline(){
        mem_ram->prepare_write(1);
        conv->fourpc_zeroshare<uint32_t>(len);
    }
    void round1(){
        conv->fourpc_share_2_replicated_share_1(Ri, len);
    }
    void round2(){
        conv->fourpc_share_2_replicated_share_2(Ri, len);
        /*get target*/
        /*
            b0     b1       (b0 + b1)*(a0+a1) b0a1 a0b1
            b2     b3
        */
        for(int i = 0; i < len; i ++)
            res[i] = Ri[i];
    }
    void round3(WORD * buffer, WORD beta){
        WORD target[len];
        for(int i = 0; i < len; i++){
            if(st == "player0" || st == "player2")
                target[i] = Ri[i] * beta + (1 - beta) * buffer[i];
            else
                target[i] = Ri[i] * beta + (- beta) * buffer[i];
            mem_ram->write_1(data_b[i], target[i], buffer[i], false, true);
        }
        
    }
    void roundend(WORD * buffer, WORD beta){
        WORD target[len];
        for(int i = 0; i < len; i++){
            if(st == "player0" || st == "player2")
                target[i] = Ri[i] * beta + (1 - beta) * buffer[i];
            else
                target[i] = Ri[i] * beta + (- beta) * buffer[i];
            mem_ram->write_2(data_b[i], target[i], buffer[i], false, true);
        }
        
    }
    ~Store(){
        delete conv;
        free(Ri);
    }
};
class Read{
    std::string st;
    P2Pchannel*p2pchnl;
    Convert *conv;
    Ram<WORD>* tape1_ram,* tape2_ram;
    WORD x0,delta,A_tmp, rc_up[2];
public:
    WORD data_a;
    WORD *rc_ptr;
    WORD* flag;
    WORD* res;
    Read(WORD data_a, WORD*rc, WORD*res, WORD*flag,std::string st,P2Pchannel* p2pchnl, Ram<WORD>* tape1_ram, Ram<WORD>* tape2_ram):
		        data_a(data_a),rc_ptr(rc), res(res),flag(flag),st(st),p2pchnl(p2pchnl),tape1_ram(tape1_ram), tape2_ram(tape2_ram){
                    /*
                    res = x0 + A(x1 - x0)
                    */
                    conv = new Convert(st, p2pchnl);
    }
    ~Read(){
        delete conv;
    }
    void offline(){
        tape1_ram->prepare_read(1);
        tape2_ram->prepare_read(1);
        conv->fourpc_zeroshare<uint32_t>(1);
    }
    void round1(){
        tape1_ram->read_1(rc_ptr[0], false);
        tape2_ram->read_1(rc_ptr[1], false);
    }
    void round2(){
        x0 = tape1_ram->read_2(rc_ptr[0], false);
        WORD x1 = tape2_ram->read_2(rc_ptr[1], false);
        if(st == "player1" || st == "player3"){
            x0 = -x0;
            x1 = - x1;
        }
        delta = x1 - x0;
        twopc_reveal_1<uint32_t>(&delta, &delta, 1, st, p2pchnl);
        diag_reveal_1<uint32_t>(&data_a, &data_a, 1, st, p2pchnl);
    }
    void round3(uint32_t beta, WORD* read_flag){
        /*(1 - A)*read[0] + A*read[1]*/
        WORD tmp1;
        twopc_reveal_2<uint32_t>(&delta, &tmp1, 1, st, p2pchnl);
        diag_reveal_2<uint32_t>(&data_a, &A_tmp, 1, st, p2pchnl);
        if(st == "player0" || st == "player2"){
            rc_up[0] = (1 - A_tmp) * beta;
            rc_up[1] = A_tmp * beta;
            flag[0] = (1 - A_tmp) * read_flag[0] + A_tmp * read_flag[1];
            
        }else{
            rc_up[0] = (- A_tmp) * beta;
            rc_up[1] = A_tmp * beta;
            flag[0] = (- A_tmp) * read_flag[0] + A_tmp * read_flag[1];
        }
        res[0] = x0 + tmp1*A_tmp;
        conv->fourpc_share_2_replicated_share_1(res, 1);
        diag_reveal_1<uint32_t>(rc_up, rc_up, 2, st, p2pchnl);
        twopc_reveal_1<uint32_t>(flag, flag, 1, st, p2pchnl);
    }
    void roundend(WORD* read_flag){
        WORD new_flag,rc_rep[2];
        conv->fourpc_share_2_replicated_share_2(res, 1);
        diag_reveal_2<uint32_t>(rc_up, rc_rep, 2, st, p2pchnl);
        twopc_reveal_2<uint32_t>(flag, &new_flag, 1, st, p2pchnl);
        flag[0] = new_flag;
        if(st == "player0" || st == "player1"){
            /*
            rc0 = rc0 + (1 - A) * B[read] * (1 - flag)
            rc1 = rc1 + A * B[read] * (1 - flag)
            */
            rc_ptr[0] += (1 - read_flag[0]) * rc_rep[0];
            rc_ptr[1] += (1 - read_flag[1]) * rc_rep[1];
        }else{
            rc_ptr[0] += (- read_flag[0]) * rc_rep[0];
            rc_ptr[1] += (- read_flag[1]) * rc_rep[1];
        }

    }

};

/**/
class Jump_op{
    std::string st;
    P2Pchannel*p2pchnl;
    Convert *conv;
    /*  1 mov
        2 cmov
    */
    int type_of = 1;
    
public:
    uint32_t j_pc;
    WORD data_a;
    WORD data_b;
    uint16_t len = 1;
    WORD* flag;
    WORD* res;
    Jump_op(WORD data_a, WORD data_b, WORD*res, WORD*flag, int type_of, std::string st,P2Pchannel* p2pchnl):
		        data_a(data_a),data_b(data_b),res(res),flag(flag),type_of(type_of),st(st),p2pchnl(p2pchnl){
                    /*
                    1. beta*A+(1-beta)pc
                    2. beta(A*flag+pc(1-flag)) +(1- beta)pc
                    3. beta(A*(1-flag)+pc*flag) +(1- beta)pc
                    pc - b0 pc - b2 pc + b0 * A + b1 * A * flag + b2 * A *(1 - flag) + b1 * pc * (-flag) + b2* pc *flag
                    */
    }
    void round1(WORD*pc){
        if(type_of == 1){
            uint32_t tmp = data_b - pc[0];
            twopc_reveal_1<uint32_t>(&tmp, &tmp, len, st, p2pchnl);
        }
        else{ 
            diag_reveal_1<uint32_t>(&data_b, &data_b, len, st, p2pchnl);
            diag_reveal_1<uint32_t>(pc, pc, len, st, p2pchnl);
        }
        
    }
    void round2(WORD*pc, uint32_t beta){
        uint32_t tmp,tmp2, tmp3;
    
        if(type_of == 1){
            tmp2 = data_b - pc[0];
            twopc_reveal_2<uint32_t>(&tmp2, &tmp, len, st, p2pchnl);
            j_pc = beta*tmp;
        }else {
            diag_reveal_2<uint32_t>(&data_b, &tmp, len, st, p2pchnl);
            diag_reveal_2<uint32_t>(pc, &tmp2, len, st, p2pchnl);
            uint32_t tmp_flag;
            if(type_of == 2){
                if(st == "player0" || st == "player1") tmp_flag = 1 - flag[0];
                else tmp_flag = - flag[0];
                j_pc = tmp * flag[0] + tmp2 * tmp_flag - pc[0];
                /*(1 - b0 - b1 - b2)pc*/
                
            }else{
                if(st == "player0" || st == "player1") tmp_flag = 1 - flag[0];
                else tmp_flag = - flag[0];
                j_pc = tmp * tmp_flag + tmp2 * flag[0] - pc[0];
                
            }
            twopc_reveal_1<uint32_t>(&j_pc, &j_pc, len, st, p2pchnl);

        }
    }
    void round3(WORD*pc, uint32_t beta){
        if(type_of != 1){
            uint32_t tmpl;
            twopc_reveal_2<uint32_t>(&j_pc, &tmpl, len, st, p2pchnl);
            j_pc = tmpl * beta;
        }
    }
    ~Jump_op(){
    }
};
#endif
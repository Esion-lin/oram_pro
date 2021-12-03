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
template <typename T>
T mul_t(T a, T b, std::string st, P2Pchannel *p2pchnl, std::array<T, 3> triple = {1,1,4}){
    T org[2],des[2];
    org[0] = a - triple[0];
    org[1] = b - triple[1];
    T res;
    fourpc_reveal<T>(org, des, 2, {"player0","player1","player2","player3"}, st, p2pchnl);
    res = triple[2] + des[0]*triple[1] + triple[0]* des[1];
    if(st == "player0")
        res += des[0] * des[1];
    return res;
        
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
            std::cout<<data_a[0]<<" "<<data_b[0]<<std::endl;
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
    Load(WORD*data_a, WORD*data_b, WORD*res, WORD*flag, uint16_t len, int type_of, std::string st,P2Pchannel* p2pchnl, Ram<WORD>* mem_ram):
		        data_a(data_a),data_b(data_b),res(res),flag(flag),len(len),type_of(type_of),st(st),p2pchnl(p2pchnl),mem_ram(mem_ram){
                    /*
                    res store Ri
                    flag store flag
                    
                    */
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
            std::cout<<data_b[i]<<" "<<target[i]<<" "<<buffer[i]<<std::endl;
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
            std::cout<<data_b[i]<<" "<<target[i]<<" "<<buffer[i]<<std::endl;
            mem_ram->write_2(data_b[i], target[i], buffer[i], false, true);
        }
        
    }
    ~Store(){
        delete conv;
        free(Ri);
    }
};
class Read{

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
                    b0 * A + b1 * A * flag + b2 * A *(1 - flag) + b1 * pc * (1 - flag) + b2* pc *flag
                    */
    }
    void round1(WORD*pc){
        if(type_of == 1)
        twopc_reveal_1<uint32_t>(&data_b, &data_b, len, st, p2pchnl);
        else{ 
            diag_reveal_1<uint32_t>(&data_b, &data_b, len, st, p2pchnl);
            diag_reveal_1<uint32_t>(pc, pc, len, st, p2pchnl);
        }
        
    }
    void round2(WORD*pc, uint32_t beta){
        uint32_t tmp,tmp2, tmp3;
    
        if(type_of == 1){
            twopc_reveal_2<uint32_t>(&data_b, &tmp, len, st, p2pchnl);
            j_pc = beta*tmp;
            std::cout<<"pc "<<j_pc<<" "<<beta<<" "<<tmp<<std::endl;
        }else {
            diag_reveal_2<uint32_t>(&data_b, &tmp, len, st, p2pchnl);
            diag_reveal_2<uint32_t>(pc, &tmp2, len, st, p2pchnl);
            uint32_t tmp_flag;
            if(type_of == 2){
                if(st == "player0" || st == "player1") tmp_flag = 1 - flag[0];
                else tmp_flag = - flag[0];
                j_pc = tmp * flag[0] + tmp2 * tmp_flag;
                std::cout<<"pc "<<j_pc<<" "<<tmp_flag<<" "<<tmp2<<std::endl;
                
            }else{
                if(st == "player0" || st == "player1") tmp_flag = 1 - flag[0];
                else tmp_flag = - flag[0];
                j_pc = tmp * tmp_flag + tmp2 * flag[0];
                std::cout<<"pc "<<j_pc<<std::endl;
                
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
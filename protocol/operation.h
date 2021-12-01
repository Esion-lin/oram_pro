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
        conv = new Convert(st, p2pchnl);
	}
    ~Cmp_a_e_ae(){
        delete cmp;
        delete conv;
    }
    void offline(){
        if(type_of == 1) cmp->eq_off(len);
        else cmp->scmp_off(len);
    }
	void round1(){
        if(type_of == 1){
            conv->fourpc_share_2_replicated_share_1(res, len, false);
            cmp->equal_1(data_a, data_b, len, flag);
        }else if(type_of == 2){
            cmp->overflow_1(data_a, data_b, len, flag, 33, [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);});
        }
		
	   
	}
	void round2(){
        if(type_of == 1){
		    conv->fourpc_share_2_replicated_share_2(res, len, false);
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
class Mul:public Operator_base{

};
#endif
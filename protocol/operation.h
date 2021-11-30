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
    Compare *overflow = new Compare();
    std::string st;
    P2Pchannel*p2pchnl;
    Convert *conv;
public:

    WORD* data_a;
    WORD* data_b;
    uint16_t len;
    WORD* flag;
    WORD* res;
    Add_sub(WORD*data_a, WORD*data_b, WORD*res, WORD*flag, uint16_t len, std::string st,P2Pchannel* p2pchnl):
                                    data_a(data_a),data_b(data_b),res(res),flag(flag),len(len),st(st),p2pchnl(p2pchnl){}
    void offline(){
        overflow->scmp_off(st, p2pchnl, len);
        conv = new Convert(st, p2pchnl);
        conv->fourpc_zeroshare<uint32_t>(len);
    }
    void round1(){
        for(int i = 0; i < len; i++){
            res[i] = data_a[i] + data_b[i];
        }
        conv->fourpc_share_2_replicated_share_1(res, len);
        overflow->overflow_1(st, p2pchnl, data_a, data_b, len,flag);

    }
    void round2(){

        conv->fourpc_share_2_replicated_share_2(res, len);
        overflow->overflow_2(st, p2pchnl, data_a, data_b, len,flag);
    }
    void round3(){
        overflow->overflow_3(st, p2pchnl, data_a, data_b, len,flag);
    }
    void roundend(){
        overflow->overflow_end(st, p2pchnl, data_a, data_b, len,flag);
    }
    
};
class Mul:public Operator_base{

};
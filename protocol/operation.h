#include "net.hpp"
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
template <typename T>
class Add_sub{
    T* data_a;
    T* data_b;
    uint16_t len;
    WORD flag;
    WORD res;
    void rand1(){}
    void rand2(){}
    void rand3(){}
    
};
class Mul:public Operator_base{

};
#pragma once 
#include <vector>
#include "encode.hpp"
#include "config.hpp"
template<class T>
void random(T* target, uint32_t lens){
    AES_KEY aes_key;
    uint8_t seeds[16] = {0};
    AES_set_encrypt_key(seeds, 128, &(aes_key));
    for(int i = 0; i < lens * sizeof(T) / 16)
    AES_encrypt(seeds, (uint8_t*) (target + 128/sizeof(T)), &aes_key);

}
class Protocol{
    public:
    Protocol(){}
    //setp phase
    virtual void set_up();
    //set input/output address
    virtual void fill();
    //online phase
    virtual void online();
    //postprocessing phase
    virtual void verify();
    virtual void dump();
};
//<x>
bool ShareA();
bool RevealA();
bool RobustRevealA();
//[x]
bool ShareB();
bool RevealB();
//[[x]]
bool ShareC();
bool RevealC();

template<class T>
struct mult{
    T rz, gamma;
};
class Mult:public Protocol{
    public:
    Mult(){}
    void set_up();
    void online();
    void verify();
    private:
    std::vector<mult<uint32_t>> datas;
};
class Trunc:public Protocol{
    public:
    Trunc(){}
    void set_up();
    void online();
    void verify();
};
class Sign:public Protocol{
    public:
    Sign(){}
    void set_up();
    void online();
    void verify();
};
class Relu:public Protocol{
    public:
    Relu(){}
    void set_up();
    void online();
    void verify();
};
class Conv:public Protocol{
    public:
    Conv(){}
    void set_up();
    void online();
    void verify();
};
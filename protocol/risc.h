#ifndef _RISC_H__
#define _RISC_H__
#include <stdint.h>
#include "preliminaries.hpp"

#define M_LEN 32
#define MEM_LEN 1024
#define TAPE_LEN 32
//define instruction words lens
#define OPT_LEN 5
#define INDIC_LEN 6
#define REGIS_LEN 5
#define PAD_LEN 11
#define IMM_LEN 32
#define WORD uint32_t

//define ins

#define AND     0x01
#define OR      0x02
#define XOR     0x03
#define NOT     0x04
#define SHL     0x05
#define SHR     0x06

#define ADD     0x07
#define SUB     0x08
#define MULL    0x09
#define MULLH   0x0a

#define COMPE   0x0b
#define COMPA   0x0c
#define COMPAE  0x0d

#define MOV     0x0e
#define CMOV    0x0f

#define JMP     0x10
#define CJMP    0x11
#define CNJMP   0x12
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
#define STORE   0x13
#define LOAD    0x14
#define READ    0x15
#define ANSWER  0x16

struct Ins
{
    /* data */
    uint8_t optr, idic, i, j, pad;
    uint32_t imme;
    friend ostream & operator<<(ostream & out, Ins & A);
};
inline ostream & operator<<(ostream & out, Ins & A){
    out << (uint32_t)A.optr <<" "<< (uint32_t)A.idic <<" "<<(uint32_t)A.i <<" "<<(uint32_t)A.j <<" "<<(uint32_t)A.pad <<" "<<A.imme;
    return out;
}
struct Env{
    WORD pc,flag;
    WORD m[M_LEN];
    WORD mem[MEM_LEN];
    WORD tape1[TAPE_LEN];
    WORD tape2[TAPE_LEN];
    WORD rc,num;  
};
template <typename T>
T load_T(WORD *arr, uint16_t index){
    T* nptr = reinterpret_cast<T*> (arr);
    return nptr[index];
}
template <typename T>
void set_T(WORD *arr, uint16_t index, T tar){
    T* nptr = reinterpret_cast<T*> (arr);
    nptr[index] = tar;
}


Ins m2i(uint64_t data);
uint64_t i2m(Ins data);
class Mechine{
private:
    P2Pchannel *p2pchnl;
    std::string st;
    Env myenv;
    bool ismyenv_init = false;
public:
    Mechine(std::string st, P2Pchannel *p2pchnl):st(st),p2pchnl(p2pchnl){}
    Mechine(std::string st, P2Pchannel *p2pchnl, Env myenv):st(st),p2pchnl(p2pchnl),myenv(myenv){
        ismyenv_init = true;
    }
    void load_env();                //from aid
    void load_env(std::string path);//from file

};


#endif
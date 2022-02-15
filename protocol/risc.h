#ifndef _RISC_H__
#define _RISC_H__
#include <stdint.h>
#include "preliminaries.hpp"
#include "convert.h"
#include <memory>
#define M_LEN 32
#define MEM_LEN 4028
#define PRO_SIZE 512
#define TAPE_LEN 32
//define instruction words lens
#define OPT_LEN 5
#define INDIC_LEN 6
#define REGIS_LEN 5
#define PAD_LEN 11
#define IMM_LEN 32
#define WORD uint32_t
#define INS_TYPE uint64_t
#define OPT_SIZE 22
//define ins

#define AND     0x01
#define OR      0x02
#define EXOR    0x03
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
#define ANSWER  0x0
template <typename T>
T sub_mod(T a, T b, T mod){
    if(a < b){
        return a + mod - b;
    }
    return (a - b) % mod;
}

struct Ins
{
    /* data */
    uint8_t optr, idic, i, j;
    uint16_t pad;
    uint32_t imme;
    friend ostream & operator<<(ostream & out, Ins A){
        out << (uint32_t)A.optr <<" "<< (uint32_t)A.idic <<" "<<(uint32_t)A.i <<" "<<(uint32_t)A.j <<" "<<(uint32_t)A.pad <<" "<<A.imme;
        return out;
    }
    Ins operator - (const Ins other) const{
        return {
            sub_mod<uint8_t>(optr,other.optr, 1<<OPT_LEN),
            sub_mod<uint8_t>(idic,other.idic, 1<<INDIC_LEN),
            sub_mod<uint8_t>(i,other.i, 1<<REGIS_LEN),
            sub_mod<uint8_t>(j,other.j, 1<<REGIS_LEN),
            sub_mod<uint16_t>(pad,other.pad, 1<<PAD_LEN),
            imme - other.imme
        };
    }
    Ins operator + (const Ins other) const{
        return {
            (optr + other.optr) % (1<<OPT_LEN),
            (idic + other.idic) % (1<<INDIC_LEN),
            (i + other.i) % (1<<REGIS_LEN),
            (j + other.j) % (1<<REGIS_LEN),
            (pad + other.pad) % (1<<PAD_LEN),
            imme + other.imme
        };
    }
    Ins operator * (const Ins other) const{
        return {
            (optr * other.optr) % (1<<OPT_LEN),
            (idic * other.idic) % (1<<INDIC_LEN),
            (i * other.i) % (1<<REGIS_LEN),
            (j * other.j) % (1<<REGIS_LEN),
            (pad * other.pad) % (1<<PAD_LEN),
            imme * other.imme
        };
    }
    Ins operator!(){
        return {
            (1<<OPT_LEN) - optr,
            (1<<INDIC_LEN) - idic,
            (1<<REGIS_LEN) - i,
            (1<<REGIS_LEN) - j,
            (1<<PAD_LEN) - pad,
            -imme
        };
    }
};
struct Env{
    WORD pc,flag;
    WORD m[M_LEN];
    WORD mem[MEM_LEN];
    WORD tape1[TAPE_LEN];
    WORD tape2[TAPE_LEN];
    WORD rc0, rc1 ,num0, num1;  
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

Ins rand_ins_t();
void rand_ins(uint64_t *data, uint16_t len);
void rand_pc(uint32_t *data, uint16_t len);
Ins m2i(uint64_t data);
uint64_t i2m(Ins data);
uint64_t sub_ins(uint64_t a, uint64_t b);
uint64_t mul_ins(uint64_t a, uint64_t b);
uint64_t add_ins(uint64_t a, uint64_t b);
uint64_t mul_ins(uint64_t a, uint32_t b);

class Mechine{
private:
    P2Pchannel *p2pchnl;
    std::string st;
    Env myenv;
    
    Convert* conv;
    Ram<uint64_t>* ins_ram;
    Ram<uint32_t>* mem_ram;
    Ram<uint32_t>* m_ram;
    Ram<uint32_t>* dm_ram;
    Ram<uint32_t>* beta_ram;
    Ram<uint32_t>* tape1_ram;
    Ram<uint32_t>* tape2_ram;
    
    bool ismyenv_init = false;
    uint32_t res[OPT_SIZE];
    uint32_t flags[OPT_SIZE];
    uint16_t ins_count;
    uint16_t count_num = 0;
    uint32_t now_flag;
public:
    bool done = false;
    uint32_t now_res;
    uint32_t Ri,Rj,A,Ri_rep;
    Ins now_ins;
    uint32_t betas[1<<OPT_LEN];
    Mechine(std::string st, P2Pchannel *p2pchnl,uint16_t ins_count):st(st),p2pchnl(p2pchnl),ins_count(ins_count){
        conv = new Convert(st, p2pchnl);
    }
    Mechine(std::string st, P2Pchannel *p2pchnl, Env myenv, uint16_t ins_count):st(st),p2pchnl(p2pchnl),myenv(myenv),ins_count(ins_count){
        conv = new Convert(st, p2pchnl);
        ismyenv_init = true;
    }
    void load_env();                //from aid
    void load_env(std::string path);//from file
    Ins load_ins();
	void run_op(Ins now_ins, uint32_t Ri, uint32_t Rj, uint32_t A);
    void run_op();
    void ret_res();
    /*for debug*/
    void print_res_flag(uint indx = 0){
        uint32_t res_re[OPT_SIZE];
        uint32_t flags_re[OPT_SIZE];
        uint32_t betas_re[OPT_SIZE];
        uint32_t mem[MEM_LEN],m_rig[M_LEN];
        uint32_t pc, ress, flagss, rc[2];
        diag_reveal<uint32_t>(res, res_re, OPT_SIZE, st, p2pchnl);
        diag_reveal<uint32_t>(flags, flags_re, OPT_SIZE, st, p2pchnl);
        diag_reveal<uint32_t>(myenv.mem, mem, MEM_LEN, st, p2pchnl);
        diag_reveal<uint32_t>(myenv.m, m_rig, M_LEN, st, p2pchnl);
        twopc_reveal<uint32_t>(betas, betas_re, OPT_SIZE, st, p2pchnl);
        fourpc_reveal<uint32_t>(&myenv.pc, &pc, 1, st, p2pchnl);
        fourpc_reveal<uint32_t>(&myenv.flag, &flagss, 1, st, p2pchnl);
        fourpc_reveal<uint32_t>(&now_res, &ress, 1, st, p2pchnl);
        fourpc_reveal<uint32_t>(&myenv.rc0, rc, 2, st, p2pchnl);
        std::cout<<"--------------res list debug-----------\n";
        for(int i = 0; i < OPT_SIZE; i++){
            std::cout<<res_re[i]<<" ";
        }
        std::cout<<"\n--------------betas list debug-----------\n";
        for(int i = 0; i < OPT_SIZE; i++){
            std::cout<<betas_re[i]<<" ";
        }
        std::cout<<"\n--------------flag list debug-----------\n";
        for(int i = 0; i < OPT_SIZE; i++){
            std::cout<<flags_re[i]<<" ";
        }
        std::cout<<"\n--------------register list debug-----------\n";
        for(int i = 0; i < M_LEN; i++){
            std::cout<<m_rig[i]<<" ";
        }
        // std::cout<<"\n--------------mem debug-----------\n";
        // for(int i = 0; i < MEM_LEN; i++) std::cout<<"mem:"<<mem[i]<< " ";
        if(indx != 0){
            for(int i = PRO_SIZE; i <= PRO_SIZE + indx; i++) std::cout<<"mem:"<<mem[i]<< " ";
        }
        std::cout<<"\n pc: "<<pc<<" res: "<<ress<<" flag: "<<flagss<<" rc:"<<rc[0]<<" "<<rc[1];
        std::cout<<"\n--------------done-----------\n";
    }
    ~Mechine(){
        delete ins_ram;
        delete m_ram;
        delete dm_ram;
        delete mem_ram;
        delete conv;
        delete beta_ram;
        delete tape1_ram;
        delete tape2_ram;
    }
};


#endif

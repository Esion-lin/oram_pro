#ifndef _OT_BASED_H__
#define _OT_BASED_H__
#include "emp-tool/emp-tool.h"
#include "emp-ot/emp-ot.h"
#include "net.hpp"
#include <vector>
/**/

void send_pt(Point *A, P2Pchannel * p2pchnl, std::string st, int num_pts);
void recv_pt(emp::Group * g, emp::Point *A, P2Pchannel * p2pchnl, std::string st, int num_pts);
class ED_P2Pch:public P2Pchannel{
public:
    void send_block(const emp::block* data, int nblock) {
        if(st == "player0")
		    send_data_to("player2", data, nblock*sizeof(block));
        if(st == "player2")
		    send_data_to("player0", data, nblock*sizeof(block));
        if(st == "player1")
		    send_data_to("player3", data, nblock*sizeof(block));
        if(st == "player3")
		    send_data_to("player1", data, nblock*sizeof(block));
	}

	void recv_block(emp::block* data, int nblock) {
        if(st == "player0")
		    recv_data_from("player2", data, nblock*sizeof(block));
        if(st == "player2")
		    recv_data_from("player0", data, nblock*sizeof(block));
        if(st == "player1")
		    recv_data_from("player3", data, nblock*sizeof(block));
        if(st == "player3")
		    recv_data_from("player1", data, nblock*sizeof(block));
	}
};
class Pratical_OT{
    public:
    P2Pchannel * p2pchnl;
    std::string st;
    Group *G = nullptr;
	bool delete_G = true;
	Pratical_OT(P2Pchannel * p2pchnl, std::string st, Group * _G = nullptr):p2pchnl(p2pchnl),st(st) {
		if (_G == nullptr)
			G = new Group();
		else {
			G = _G;
			delete_G = false;
		}

	}
    void send(const block* data0, const block* data1, int64_t length, std::string myrole, std::string target);
    void recv(block* data, const bool* b, int64_t length, std::string myrole, std::string target);
    
	~Pratical_OT() {
		if (delete_G)
			delete G;
	}
};
class Bitwise{
    private:
    bool is_y =false;
    bool check_over = true;
    public:
    Pratical_OT* ot;
    std::string st;
    P2Pchannel*p2pchnl;
    PRG prg;
    block delta;
    uint64_t r = rand();
    uint64_t r_flag = rand();
    std::vector<uint64_t> r_set;
    std::vector<uint64_t> rflag_set;
    block R0[64],R1[64],A0[64],A1[64],R_plus[64],Flag_plus[64],res[64];
    std::vector<block*> res_set;
    std::vector<block*> flagres_set;
    Bitwise(Pratical_OT*ot, std::string st, P2Pchannel*p2pchnl):ot(ot),st(st), p2pchnl(p2pchnl){}
    void run_with_R_A(uint64_t R, uint64_t A, std::string file, std::string sender, std::string recver, uint16_t lens);
    void to_Y(std::string sender, std::string recver, uint64_t R, uint64_t A, int lens);
    void run(std::string sender, std::string recver, std::string file, uint16_t lens);
    void runs(std::string sender, std::string recver, std::vector<std::string> files, uint16_t lens);
    void to_A(std::string sender, std::string recver, uint16_t lens);
    void to_As(std::string sender, std::string recver, uint16_t lens);
    ~Bitwise(){
        delete emp::CircuitExecution::circ_exec;
    }
};



#endif
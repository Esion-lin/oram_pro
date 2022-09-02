#ifndef _GCM_BASED_H__
#define _GCM_BASED_H__
#include "gc_op.h"
#include "gmw.h"
class GC_CHANL:public P2Pchannel{
public:
    void send_block(const emp::block* data, int nblock) {
        if(st == "player0")
		    send_data_to("player1", data, nblock*sizeof(block));
        if(st == "player1")
		    send_data_to("player0", data, nblock*sizeof(block));
	}

	void recv_block(emp::block* data, int nblock) {
        if(st == "player0")
		    recv_data_from("player1", data, nblock*sizeof(block));
        if(st == "player1")
		    recv_data_from("player0", data, nblock*sizeof(block));
	}
};
class GCM{
    public:
    block iv;
    uint64_t counter;
    block key;
    block delta;
    Pratical_OT* ot;
    PRG prg;
    GCM(){

        ot = new Pratical_OT(P2Pchannel::mychnl, Config::myconfig->get_player());
    }
    //private:
    void do_aes(block key, block msg);
    void do_Ghash(block key, block msg);
    void runGC(block *msg, uint32_t lens, uint32_t lens2, std::string path);
    void sim_gcm_offline(block key, block msg);
    void sim_gcm_online(block key, block msg);
    ~GCM(){
        delete ot;
    }
    
};

#endif
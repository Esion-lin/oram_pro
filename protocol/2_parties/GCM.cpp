#include "GCM.h"
void GCM::do_aes(block key, block msg){
    /*
    key xor_shared, msg
    player0 gen
    player1 evl
    */
    /*key -> binary*/
    bool *select = new bool[128];
    block* c = new block[128];
    for(int i = 0; i < 64; i ++){
        select[i] = (((uint64_t)key[0] % 2) == 1);
        select[i+64] = (((uint64_t)key[1] % 2) == 1);
        key[0] /= 2;
        key[1] /= 2;
    }
    BristolFormat cf("../cir/AES128_E_32_KeyShared.txt");
    if (Config::myconfig->check("player0")) {
        /*msg -> binary*/
        bool *select_msg = new bool[128];
        for(int i = 0; i < 64; i ++){
            select_msg[i] = (((uint64_t)msg[0] % 2) == 1);
            select_msg[i+64] = (((uint64_t)msg[1] % 2) == 1);
            msg[0] /= 2;
            msg[1] /= 2;
            
        }
        block* a0 = new block[256];
        block* b0 = new block[128];
        HalfGateGen<GC_CHANL>::circ_exec = new HalfGateGen<GC_CHANL>(static_cast<GC_CHANL*>(P2Pchannel::mychnl));
        delta = dynamic_cast<HalfGateGen<GC_CHANL>*> (HalfGateGen<GC_CHANL>::circ_exec) -> delta;
        //dynamic_cast<HalfGateGen<GC_CHANL>*> (HalfGateGen<GC_CHANL>::circ_exec) -> set_delta(delta);
        prg.random_block(a0, 256);
        prg.random_block(b0, 128); 
        block a1[256],b1[128], as[256];
        for(int i = 0; i < 128; i ++) {
            a1[i] = a0[i] ^ delta;
            a1[i+128] = a0[i+128] ^ delta;
            b1[i] = b0[i] ^ delta;
            if(select[i]){
                as[i] = a1[i];
            }else{
                as[i] = a0[i];
            }
            if(select_msg[i]){
                as[i+128] = a1[i+128];
            }else{
                as[i+128] = a0[i+128];
            }
        }
        ot->send_with_rot(b0, b1, 128, "player0", "player1");
        static_cast<GC_CHANL*>(P2Pchannel::mychnl)->send_block(as, 256);
        P2Pchannel::mychnl->set_flush(false);
        cf.compute(c, a0, b0);
        P2Pchannel::mychnl->flush_all();
        P2Pchannel::mychnl->set_flush(true);
        delete HalfGateGen<GC_CHANL>::circ_exec;
        delete[] a0;
        delete[] b0;
        delete[] select_msg;

    }else{
        block* a0 = new block[256];
        block* b0 = new block[128];
        HalfGateEva<GC_CHANL>::circ_exec = new HalfGateEva<GC_CHANL>(static_cast<GC_CHANL*>(P2Pchannel::mychnl));
        ot->recv_with_rot(b0, select, 128, "player0", "player1");
        static_cast<GC_CHANL*>(P2Pchannel::mychnl)->recv_block(a0, 256);
        P2Pchannel::mychnl->set_flush(false);
        cf.compute(c, a0, b0);
        P2Pchannel::mychnl->flush_all();
        P2Pchannel::mychnl->set_flush(true);
        delete HalfGateEva<GC_CHANL>::circ_exec;
        delete[] a0;
        delete[] b0;
        
    }
    /*to binary share*/
    //for(int i = 0; i < 128; i++) std::cout<<(uint32_t)c[i][0]%2;
    delete[] c;
    delete[] select;
}

void GCM::do_Ghash(block key, block msg){
    /*
    key shared
    msg p0 local
    */
    BristolFormat cf("../cir/Ghash.txt");
    GMW<uint8_t>* gmw = new GMW<uint8_t>();
    vector<shareholder> wires;
    wires.resize(cf.num_wire);
    for(int i = 0; i < 64; i ++){
        /*key -> wires*/
        uint8_t tmp = ((uint64_t)key[0] % 2);
        uint8_t tmp2 = ((uint64_t)key[1] % 2);
        key[0] /= 2;
        key[1] /= 2;
        wires[i + 128].types = 1;wires[i + 128].data = tmp;
        wires[i + 192].types = 1;wires[i + 192].data = tmp2;
        
        
        tmp = ((uint64_t)msg[0] % 2);
        tmp2 = ((uint64_t)msg[1] % 2);
        msg[0] /= 2;
        msg[1] /= 2;
        wires[i + 0].types = 1;wires[i + 0].data = tmp;
        wires[i + 64].types = 1;wires[i + 64].data = tmp2;
        
    }
    
    for(int i = 0; i < cf.num_gate; ++i) {
        if(cf.gates[4*i+3] == AND_GATE) {
            if(wires[cf.gates[4*i]].types == 0){
                wires[cf.gates[4*i+2]].data = gmw->scale_c(wires[cf.gates[4*i+1]].data, wires[cf.gates[4*i]].data);
                if(wires[cf.gates[4*i+1]].types == 0){
                    wires[cf.gates[4*i+2]].types = 0;
                }else{
                    wires[cf.gates[4*i+2]].types = 1;
                }
            }else{
                if(wires[cf.gates[4*i+1]].types == 0){
                    wires[cf.gates[4*i+2]].data = gmw->scale_c(wires[cf.gates[4*i+1]].data, wires[cf.gates[4*i]].data);
                }else{
                    wires[cf.gates[4*i+2]].data = gmw->and_c(wires[cf.gates[4*i+1]].data, wires[cf.gates[4*i]].data, 0, 0, 0);
                }
                wires[cf.gates[4*i+2]].types = 1;
            }
        }
        else if (cf.gates[4*i+3] == XOR_GATE) {
            if(wires[cf.gates[4*i]].types == 0){
                
                if(wires[cf.gates[4*i+1]].types == 0){
                    wires[cf.gates[4*i+2]].data = gmw->xor_c(wires[cf.gates[4*i+1]].data, wires[cf.gates[4*i]].data);
                    wires[cf.gates[4*i+2]].types = 0;
                }else{
                    if(Config::myconfig->check("player0"))
                        wires[cf.gates[4*i+2]].data = gmw->xor_c(wires[cf.gates[4*i+1]].data, wires[cf.gates[4*i]].data);
                    else{
                        wires[cf.gates[4*i+2]].data = wires[cf.gates[4*i+1]].data;
                   
                    }
                    wires[cf.gates[4*i+2]].types = 1;
                }
            }else{
                if(wires[cf.gates[4*i+1]].types == 0){
                    if(Config::myconfig->check("player0"))
                        wires[cf.gates[4*i+2]].data = gmw->xor_c(wires[cf.gates[4*i+1]].data, wires[cf.gates[4*i]].data);
                    else{
                        wires[cf.gates[4*i+2]].data = wires[cf.gates[4*i]].data;
                    }
                }else{
                    wires[cf.gates[4*i+2]].data = gmw->xor_c(wires[cf.gates[4*i+1]].data, wires[cf.gates[4*i]].data);
                }
                wires[cf.gates[4*i+2]].types = 1;
            }
        }
        else{
            if(wires[cf.gates[4*i]].types = 0){
                wires[cf.gates[4*i+2]].data = wires[cf.gates[4*i]].data ^ 1;
                
            }else{
                if(Config::myconfig->check("player0"))
                    wires[cf.gates[4*i+2]].data = wires[cf.gates[4*i]].data ^ 1;
                else{
                    wires[cf.gates[4*i+2]].data = wires[cf.gates[4*i]].data;
                }
            }
            wires[cf.gates[4*i+2]].types = wires[cf.gates[4*i]].types;
        }
		//memcpy(out, wires.data()+(num_wire-n3), n3*sizeof(block));
    }
    // for(int i = cf.num_wire - cf.n3; i < cf.num_wire; i++){
        
    //     std::cout<<(uint32_t)wires[i].data;
    // }
    // for(int i = 0; i < 1000; i ++) {
    //     printf("%d",wires[i].types);
    // }
    // printf("\n");
    delete gmw;
}
void GCM::runGC(block *msg, uint32_t lens, uint32_t lens2, std::string path){
    bool *select = new bool[lens * 128];
    uint32_t lenss;
    if(Config::myconfig->check("player0")){
        lenss = lens;
    }else{
        lenss = lens2;
    }
    for(int i = 0; i < lenss; i ++){
        for(int j = 0; j < 64; j++){
            select[i*128 + j] = (((uint64_t)msg[i][0] % 2) == 1);
            select[i*128 + j+64] = (((uint64_t)msg[i][1] % 2) == 1);
            msg[i][0] >>= 2;
            msg[i][1] >>= 2;
        }
        
    }
    BristolFormat cf(path.c_str());
    block c[cf.n3];
    lens *= 128;
    lens2 *= 128;
    if (Config::myconfig->check("player0")) {
        block a0[lens],b0[lens2],a1[lens],b1[lens], as[lens];
        prg.random_block(a0, lens);
        prg.random_block(b0, lens2); 
        HalfGateGen<GC_CHANL>::circ_exec = new HalfGateGen<GC_CHANL>(static_cast<GC_CHANL*>(P2Pchannel::mychnl));
        delta = dynamic_cast<HalfGateGen<GC_CHANL>*> (HalfGateGen<GC_CHANL>::circ_exec) -> delta;
        for(int i = 0; i < lens; i ++) {
            a1[i] = a0[i] ^ delta;
            
            if(select[i]){
                as[i] = a1[i];
            }else{
                as[i] = a0[i];
            }
        }
        for(int i = 0; i < lens2; i ++){
            b1[i] = b0[i] ^ delta;
        }
        ot->send(b0, b1, lens2, "player0", "player1");
        
        static_cast<GC_CHANL*>(P2Pchannel::mychnl)->send_block(as, lens);
        P2Pchannel::mychnl->set_flush(false);
        cf.compute(c, a0, b0);
        P2Pchannel::mychnl->flush_all();
        P2Pchannel::mychnl->set_flush(true);
        delete HalfGateGen<GC_CHANL>::circ_exec;
    }
    else{
        block a0[lens],b0[lens2];
        HalfGateEva<GC_CHANL>::circ_exec = new HalfGateEva<GC_CHANL>(static_cast<GC_CHANL*>(P2Pchannel::mychnl));
        ot->recv(b0, select, lens2, "player1", "player0");
        static_cast<GC_CHANL*>(P2Pchannel::mychnl)->recv_block(a0, lens);
        P2Pchannel::mychnl->set_flush(false);
        cf.compute(c, a0, b0);
        P2Pchannel::mychnl->flush_all();
        P2Pchannel::mychnl->set_flush(true);
        delete HalfGateEva<GC_CHANL>::circ_exec;
        
    }
    delete[] select;
}
void GCM::sim_gcm_offline(block key, block msg){
    /**/
    do_aes(key, msg);
    do_aes(key, msg);
    do_aes(key, msg);
    do_aes(key, msg);
    do_aes(key, msg);
    do_aes(key, msg);
    do_aes(key, msg);
    do_Ghash(key, msg);
}
void GCM::sim_gcm_online(block key, block msg){
    do_Ghash(key, msg);do_Ghash(key, msg);do_Ghash(key, msg);do_Ghash(key, msg);do_Ghash(key, msg);do_Ghash(key, msg);
}
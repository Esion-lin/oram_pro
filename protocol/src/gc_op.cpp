#include "gc_op.h"

void send_pt(Point *A, P2Pchannel * p2pchnl, std::string st, int num_pts = 1) {
    for(int i = 0; i < num_pts; ++i) {
        size_t len = A[i].size();
        
        A[i].group->resize_scratch(len);
        
        unsigned char * tmp = A[i].group->scratch;
        p2pchnl->send_data_to(st, &len, sizeof(size_t));
        A[i].to_bin(tmp, len);
        p2pchnl->send_data_to(st, tmp, len*sizeof(unsigned char));
    }
}

void recv_pt(Group * g, Point *A, P2Pchannel * p2pchnl, std::string st, int num_pts = 1) {
    size_t len = 0;
    for(int i = 0; i < num_pts; ++i) {
        p2pchnl->recv_data_from(st, &len, sizeof(size_t));
        g->resize_scratch(len);
        unsigned char * tmp = g->scratch;
        p2pchnl->recv_data_from(st, tmp, len*sizeof(unsigned char));
        A[i].from_bin(g, tmp, len);
    }
}

void Pratical_OT::send(const block* data0, const block* data1, int64_t length, std::string myrole, std::string target) {
    if(st != myrole) return ;
    BigInt d;
    G->get_rand_bn(d);
    Point C = G->mul_gen(d);
    send_pt(&C, p2pchnl, target);

    BigInt * r = new BigInt[length];
    BigInt * rc = new BigInt[length];
    Point * pk0 = new Point[length], 
            pk1,
            *gr = new Point[length], 
            *Cr = new Point[length];
    for(int64_t i = 0; i < length; ++i) {
        G->get_rand_bn(r[i]);
        gr[i] = G->mul_gen(r[i]);
        rc[i] = r[i].mul(d, G->bn_ctx);
        rc[i] = rc[i].mod(G->order, G->bn_ctx);
        Cr[i] = G->mul_gen(rc[i]);
    }
    for(int64_t i = 0; i < length; ++i) {
        send_pt(&gr[i], p2pchnl, target);
    }
    for(int64_t i = 0; i < length; ++i) {
        recv_pt(G, &pk0[i], p2pchnl, target);
    }
    

    block m[2];
    for(int64_t i = 0 ; i < length; ++i) {
        pk0[i] = pk0[i].mul(r[i]);
        Point inv = pk0[i].inv();
        pk1 = Cr[i].add(inv);
        m[0] = Hash::KDF(pk0[i]) ^ data0[i];
        m[1] = Hash::KDF(pk1) ^ data1[i];
        p2pchnl->send_data_to(target, m, 2*sizeof(block));
    }

    delete[] r;
    delete[] gr;
    delete[] Cr;
    delete[] rc;
    delete[] pk0;
}

void Pratical_OT::recv(block* data, const bool* b, int64_t length, std::string myrole, std::string target) {
    
    if(st != myrole) return ;
    BigInt * k = new BigInt[length];
    Point * gr = new Point[length]; 
    Point pk[2];
    block m[2];
    Point C;
    for(int64_t i = 0; i < length; ++i) 
        G->get_rand_bn(k[i]);
    
    recv_pt(G, &C, p2pchnl, target);
    for(int64_t i = 0; i < length; ++i) {
        recv_pt(G, &gr[i], p2pchnl, target);
        gr[i] = gr[i].mul(k[i]);
    }
    for(int64_t i = 0; i< length; ++i) {
        if(b[i]) {
            pk[1] = G->mul_gen(k[i]);
            Point inv = pk[1].inv();
            pk[0] = C.add(inv);
        } else {
            pk[0] = G->mul_gen(k[i]);
        }
        send_pt(&pk[0], p2pchnl, target);
    }

    
    for(int64_t i = 0; i < length; ++i) {
        int ind = b[i];
        p2pchnl->recv_data_from(target, m, 2*sizeof(block));
        data[i] = m[ind] ^ Hash::KDF(gr[i]);
    }
    delete[] k;
    delete[] gr;
}
void Pratical_OT::gen_rot(int64_t length, std::string sender, std::string recver){
    a0 = new block[length];
    prg.random_block(a0, length);
    if(st == sender){
        a1 = new block[length];
        prg.random_block(a1, length);
        send(a0, a1, length, sender, recver);
    }
    if(st == recver){
        r = new bool[length];
        prg.random_bool(r, length);
        recv(a0, r, length, recver, sender);
    }


}
void Pratical_OT::send_with_rot(const block* data0, const block* data1, int64_t length, std::string sender, std::string recver){
    if(st != sender) return ;
    bool masked[length];
    block d[length*2];
    p2pchnl->recv_data_from(recver, masked, length);
    for(int i = 0; i < length; i ++){
        if(masked[i]){
            d[i] = a1[i] ^ data0[i];
            d[i + length] = a0[i] ^ data1[i];
        }else{
            d[i] = a0[i] ^ data0[i];
            d[i + length] = a1[i] ^ data1[i];
        }
    }
    p2pchnl->send_data_to(recver, d, length * 2 * sizeof(block));
}
void Pratical_OT::recv_with_rot(block* data, const bool* b, int64_t length, std::string sender, std::string recver){
    /*send masked select bit*/
    if(st != recver) return ;
    bool masked[length];
    block d[length*2];
    for(int i = 0; i < length; i++){
        masked[i] = r[i] ^ b[i];
    }
    p2pchnl->send_data_to(sender, masked, length);
    p2pchnl->recv_data_from(sender, d, length * 2 * sizeof(block));
    for(int i = 0; i < length; i++){
        if(b[i] == 0){
            data[i] = d[i] ^ a0[i];
        }else{
            data[i] = d[i + length] ^ a0[i];
        }
    }

}	
template<typename T>
void to_binary(T a, bool* bin_a, int lens){
    uint64_t d_data;
    memcpy(&d_data, &a, sizeof(T));
    for(int i = 0; i < lens; i ++){
        bin_a[i] = d_data % 2;
        d_data /= 2;
        
    }

}
void Bitwise::run_with_R_A(uint64_t R, uint64_t A, std::string file, std::string sender, std::string recver, uint16_t lens){
    if(st != sender && st != recver) return;
    block* a0 = new block[lens];
    block* b0 = new block[lens];
    bool *select_a = new bool[lens];
    block* c = new block[lens];
    /*convert R/A to select_a/select_b*/
    to_binary<uint64_t>(R, select_a, lens);
    BristolFormat cf(file.c_str());
    if (st == recver) {
        HalfGateEva<ED_P2Pch>::circ_exec = new HalfGateEva<ED_P2Pch>(static_cast<ED_P2Pch*>(p2pchnl));
        ot->recv(a0, select_a, lens, recver, sender);
        static_cast<ED_P2Pch*>(p2pchnl)->recv_block(b0, lens);
        cf.compute(c, b0, a0);
        
        delete HalfGateEva<ED_P2Pch>::circ_exec;
        std::cout<<c[0]<<std::endl;
    }else{
        HalfGateGen<ED_P2Pch>::circ_exec = new HalfGateGen<ED_P2Pch>(static_cast<ED_P2Pch*>(p2pchnl));
        delta = dynamic_cast<HalfGateGen<ED_P2Pch>*> (HalfGateGen<ED_P2Pch>::circ_exec) -> delta;
        prg.random_block(a0, lens);
        prg.random_block(b0, lens); 
        block a1[lens],b1[lens],bs[lens];
        for(int i = 0; i < lens; i ++) {
            a1[i] = a0[i] ^ delta;
            b1[i] = b0[i] ^ delta;
            if(select_a[i]){
                bs[i] = b1[i];
            }else{
                bs[i] = b0[i];
            }
        }
        ot->send(a0, a1, lens, sender, recver);
        static_cast<ED_P2Pch*>(p2pchnl)->send_block(bs, lens);
        cf.compute(c, b0, a0);
        delete HalfGateGen<ED_P2Pch>::circ_exec;
        std::cout<<c[0]<<std::endl;
    }
    delete[] a0;delete[] b0;
    delete[] select_a;delete[] c;
}
void Bitwise::prepare_ot(uint32_t lens, std::string sender, std::string recver){
    ot->gen_rot(lens, sender, recver);
}
void Bitwise::to_Y(std::string sender, std::string recver, uint64_t R, uint64_t A, int lens=64){
    bool select_a[lens],select_b[lens],select_r[lens], select_flag[lens];
    to_binary<uint64_t>(R, select_a, lens);
    to_binary<uint64_t>(A, select_b, lens);
    to_binary<uint64_t>(r, select_r, lens);
    to_binary<uint64_t>(r_flag, select_flag, lens);
    if (st == sender) {
        HalfGateGen<ED_P2Pch>::circ_exec = new HalfGateGen<ED_P2Pch>(static_cast<ED_P2Pch*>(p2pchnl));
        /**/
        prg.random_block(R0, lens);
        prg.random_block(R1, lens); 
        prg.random_block(A0, lens);
        prg.random_block(A1, lens); 
        prg.random_block(R_plus, lens);
        prg.random_block(Flag_plus, lens);
        block R0_1[lens],R1_1[lens],A0_1[lens],A1_1[lens],R_plus1[lens],Rs[lens], As[lens],R_pluss[lens], Flag_pluss[lens];
        delta = dynamic_cast<HalfGateGen<ED_P2Pch>*> (HalfGateGen<ED_P2Pch>::circ_exec) -> delta;
        for(int i = 0; i < lens; i ++) {
            R0_1[i] = R0[i] ^ delta;
            R1_1[i] = R1[i] ^ delta;
            A0_1[i] = A0[i] ^ delta;
            A1_1[i] = A1[i] ^ delta;
            R_plus1[i] = R_plus[i] ^ delta; 
            if(select_a[i]){
                Rs[i] = R1_1[i];
            }else{
                Rs[i] = R1[i];
            }
            if(select_b[i]){
                As[i] = A1_1[i];
            }else{
                As[i] = A1[i];
            }
            if(select_r[i]){
                R_pluss[i] = R_plus1[i];
            }else{
                R_pluss[i] = R_plus[i];
            }
            if(select_flag[i]){
                Flag_pluss[i] = Flag_plus[i] ^ delta; 
            }else{
                Flag_pluss[i] = Flag_plus[i];
            }
        }
        block tmps[lens * 2],tmps2[lens*2];
        memcpy(tmps, R0, lens*sizeof(block));
        memcpy(&tmps[lens], A0, lens*sizeof(block));
        memcpy(tmps2, R0_1, lens*sizeof(block));
        memcpy(&tmps2[lens], A0_1, lens*sizeof(block));
        ot->send(tmps, tmps2, lens*2, sender, recver);
        p2pchnl->set_flush(false);
        static_cast<ED_P2Pch*>(p2pchnl)->send_block(Rs, lens);
        static_cast<ED_P2Pch*>(p2pchnl)->send_block(As, lens);
        static_cast<ED_P2Pch*>(p2pchnl)->send_block(R_pluss, lens);
        static_cast<ED_P2Pch*>(p2pchnl)->send_block(Flag_pluss, lens);
        p2pchnl->flush_all();
    }
    else if(st == recver){
        HalfGateEva<ED_P2Pch>::circ_exec = new HalfGateEva<ED_P2Pch>(static_cast<ED_P2Pch*>(p2pchnl));
        bool selects[lens*2];
        block tmps[lens * 2];
        memcpy(selects, select_a, lens*sizeof(bool));
        memcpy(&selects[lens], select_b, lens*sizeof(bool));
        ot->recv(tmps, selects, 2*lens, recver, sender);
        memcpy(R0, tmps, lens*sizeof(block));
        memcpy(A0, &tmps[lens], lens*sizeof(block));
        p2pchnl->set_flush(false);
        static_cast<ED_P2Pch*>(p2pchnl)->recv_block(R1, lens);
        static_cast<ED_P2Pch*>(p2pchnl)->recv_block(A1, lens);
        static_cast<ED_P2Pch*>(p2pchnl)->recv_block(R_plus, lens);
        static_cast<ED_P2Pch*>(p2pchnl)->recv_block(Flag_plus, lens);
        p2pchnl->flush_all();
    }
    is_y = true;
}
void Bitwise::run(std::string sender, std::string recver, std::string file, uint16_t lens=64){
    if(st != sender && st != recver) return;
    block R[lens],A[lens],tmp[lens];
    BristolFormat cf("../cir/add_32.txt");
    cf.compute(R, R0, R1);
    cf.compute(A, A0, A1);
    BristolFormat cf2(file.c_str());
    cf2.compute(tmp, R, A);
    BristolFormat cf3("../cir/sub_32.txt");
    cf3.compute(res, tmp, R_plus);

}
void Bitwise::runs(std::string sender, std::string recver, std::vector<std::string> files, uint16_t lens=64){
    if(st != sender && st != recver) return;
    block R[lens],A[lens],tmp[lens] = {0},tmp_flag[lens] = {0};
    
    BristolFormat cf("../cir/add_32.txt");
    cf.compute(R, R0, R1);
    cf.compute(A, A0, A1);
    for(auto& file:files){
        BristolFormat cf2(file.c_str());
        cf2.compute(tmp, R, A);
        
        if(file == "../cir/left_shift.txt"){
            /*msb*/
            tmp_flag[0] = tmp[lens-1];
        }else if(file == "../cir/right_shift.txt"){
            /*msb*/
            tmp_flag[0] = tmp[0];
        }else{
            /*zero check*/
            BristolFormat zcheck("../cir/zero_check.txt");
            zcheck.compute(tmp_flag, tmp, tmp);
        }
        BristolFormat cf3("../cir/sub_32.txt");
        block *tmp2 = (block*) malloc(sizeof(block)*lens);
        block *tmp3 = (block*) malloc(sizeof(block)*lens);
        cf3.compute(tmp2, tmp, R_plus);
        cf3.compute(tmp3, tmp_flag, Flag_plus);
        res_set.push_back(tmp2);
        flagres_set.push_back(tmp3);
    }
    

}
void Bitwise::to_A(std::string sender, std::string recver, uint16_t lens=64){
    if(st == sender){
        static_cast<ED_P2Pch*>(p2pchnl)->send_block(res, lens);
    }
    if(st == recver){
        block tmpd[lens];
        static_cast<ED_P2Pch*>(p2pchnl)->recv_block(tmpd, lens);
        r = 0;
        for(int i = lens - 1; i >= 0; i--){
            if(tmpd[i][1] == res[i][1])
                r = r*2;
            else
                r = r*2 + 1; 
        }
    }
}
void Bitwise::to_As(std::string sender, std::string recver, uint16_t lens=64){
    if(st == sender){
        for(auto & ele : res_set){
            static_cast<ED_P2Pch*>(p2pchnl)->send_block(ele, lens);
            free(ele);
        }
        for(auto & ele : flagres_set){
            static_cast<ED_P2Pch*>(p2pchnl)->send_block(ele, lens);
            free(ele);
        }
        
    }
    if(st == recver){
        for(auto & ele : res_set){
            block tmpd[lens];
            static_cast<ED_P2Pch*>(p2pchnl)->recv_block(tmpd, lens);
            r = 0;
            for(int i = lens - 1; i >= 0; i--){
                if(tmpd[i][1] == ele[i][1])
                    r = r*2;
                else
                    r = r*2 + 1; 
            }
            free(ele);
            r_set.push_back(r);
        }
        for(auto & ele : flagres_set){
            block tmpd[lens];
            static_cast<ED_P2Pch*>(p2pchnl)->recv_block(tmpd, lens);
            r = 0;
            for(int i = lens - 1; i >= 0; i--){
                if(tmpd[i][1] == ele[i][1])
                    r = r*2;
                else
                    r = r*2 + 1; 
            }
            free(ele);
            rflag_set.push_back(r);
        }
    }
}
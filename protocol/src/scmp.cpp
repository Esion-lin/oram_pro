#include "scmp.hpp"
void Compare::scmp_off_ba(uint32_t len, int extend_len = 33){
        conv = new Convert(st, p2pchnl);
        isconv = true;
        uint32_t tmp32[2];
        uint64_t tmp33[2];
        
        if(st == "aid"){
            /*gen key and send*/
            
            initializeClient(&cmpkey_32, 32, 2);
            initializeClient(&cmpkey_33, extend_len, 2);
            cmpkey_32_init = true;
            cmpkey_33_init = true;
            send_fss_key(cmpkey_32, p2pchnl);
            send_fss_key(cmpkey_33, p2pchnl);
            uint32_t a, r;
            uint64_t s1, s2;
            ServerKeyLt lt_a0, lt_a1, lt_r0, lt_r1, lt_s10, lt_s11, lt_s20, lt_s21; 
            for(int i = 0; i < len; i++){
                a = rand();tmp32[0] = a;
                r = rand();tmp32[1] = r;
                s1 = rand();tmp33[0] = s1;
                s2 = s1 + ((uint64_t)1<<32);tmp33[1] = s2;
                generateTreeLt(&cmpkey_32, &lt_a0, &lt_a1, a, 1);
                generateTreeLt(&cmpkey_32, &lt_r0, &lt_r1, r, 1);
                generateTreeLt(&cmpkey_33, &lt_s10, &lt_s11, s1, 1);
                generateTreeLt(&cmpkey_33, &lt_s20, &lt_s21, s2, 1);
                send_lt_key(lt_a0, cmpkey_32, "player1", p2pchnl);
                send_lt_key(lt_a1, cmpkey_32, "player3", p2pchnl);
                send_lt_key(lt_s10, cmpkey_33, "player1", p2pchnl);
                send_lt_key(lt_s11, cmpkey_33, "player3", p2pchnl);
                send_lt_key(lt_r0, cmpkey_32, "player0", p2pchnl);
                send_lt_key(lt_r1, cmpkey_32, "player2", p2pchnl);
                send_lt_key(lt_s20, cmpkey_33, "player0", p2pchnl);
                send_lt_key(lt_s21, cmpkey_33, "player2", p2pchnl);
                fourpc_share<uint32_t>(tmp32, 2, st, p2pchnl);
                fourpc_share<uint64_t>(tmp33, 2, st, p2pchnl, 
                    [=](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<extend_len) - b) % ((uint64_t)1<<extend_len);},
                    [=](uint64_t* data, uint16_t lens)->void{for(int i = 0; i < lens; i++)data[i] = rand();});
                ServerKeyLt keyarr[8] = {lt_a0, lt_a1, lt_r0, lt_r1, lt_s10, lt_s11, lt_s20, lt_s21};
                free_keys<ServerKeyLt>(keyarr, 8);
            }
        }else{
            recv_fss_key(cmpkey_32, p2pchnl);
            recv_fss_key(cmpkey_33, p2pchnl);
            cmpkey_32_init = true;
            cmpkey_33_init = true;
            for(int i = 0; i < len; i++){
                ServerKeyLt lt_k0, lt_k1;
                recv_lt_key(lt_k0, cmpkey_32, "aid", p2pchnl);
                recv_lt_key(lt_k1, cmpkey_33, "aid", p2pchnl);
                //a,r
                fourpc_share<uint32_t>(tmp32, 2, st, p2pchnl);
                //s1, s2
                fourpc_share<uint64_t>(tmp33, 2, st, p2pchnl); 

                std::array<uint32_t, 2> arr32{tmp32[0], tmp32[1]};
                std::array<uint64_t, 2> arr33{tmp33[0], tmp33[1]};
                scmp_tuple tmp = {lt_k0, lt_k1, arr32, arr33};
                cks.push_back(tmp);
            }

        }
        conv->fourpc_zeroshare<uint32_t>(len);
        p2pchnl->flush_all();
    }
#ifndef _SCMP_H__
#define _SCMP_H__
#include "fss-client.h"
#include "net.hpp"
#include "fss-common.h"
#include "fss-server.h"
#include "preliminaries.hpp"
#include <tuple>
#include <array>
using scmp_tuple = tuple<ServerKeyLt, ServerKeyLt, array<uint32_t,2>, array<uint64_t,2>>;
class Compare{
    public:
    std::vector<scmp_tuple> cks;
    Fss cmpkey_32, cmpkey_33;
    Compare(){};
    void scmp_off(std::string st, P2Pchannel* p2pchnl, uint32_t len){
        uint32_t tmp32[2];
        uint64_t tmp33[2];
        if(st == "aid"){
            /*gen key and send*/
            
            initializeClient(&cmpkey_32, 32, 2);
            initializeClient(&cmpkey_33, 33, 2);
            send_fss_key(cmpkey_32, p2pchnl);
            send_fss_key(cmpkey_33, p2pchnl);
            uint32_t a, r;
            uint64_t s1, s2;
            ServerKeyLt lt_a0, lt_a1, lt_r0, lt_r1, lt_s10, lt_s11, lt_s20, lt_s21; 
            for(int i = 0; i < len; i++){
                a = rand();tmp32[0] = a;
                r = rand();tmp32[1] = r;
                s1 = rand()%(1<<33);tmp33[0] = s1;
                s2 = rand()%(1<<33);tmp33[1] = s2;
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
                    [](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<33) - b) % ((uint64_t)1<<33);},
                    [](uint64_t* data, uint16_t lens)->void{for(int i = 0; i < lens; i++)data[i] = rand();});
                ServerKeyLt keyarr[8] = {lt_a0, lt_a1, lt_r0, lt_r1, lt_s10, lt_s11, lt_s20, lt_s21};
                free_keys<ServerKeyLt>(keyarr, 8);
            }
        }else{
            recv_fss_key(cmpkey_32, p2pchnl);
            recv_fss_key(cmpkey_33, p2pchnl);
            for(int i = 0; i < len; i++){
                ServerKeyLt lt_k0, lt_k1;
                recv_lt_key(lt_k0, cmpkey_32, "aid", p2pchnl);
                recv_lt_key(lt_k1, cmpkey_33, "aid", p2pchnl);
                //a,r
                fourpc_share<uint32_t>(tmp32, 2, st, p2pchnl);
                //s1, s2
                fourpc_share<uint64_t>(tmp33, 2, st, p2pchnl); 
                uint32_t delta;
                p2pchnl->recv_data_from("aid",&delta,sizeof(uint32_t));
                std::array<uint32_t, 2> arr32{tmp32[0], tmp32[1]};
                std::array<uint64_t, 2> arr33{tmp33[0], tmp33[1]};
                scmp_tuple tmp = {lt_k0, lt_k1, arr32, arr33};
                cks.push_back(tmp);
            }

        }
        p2pchnl->flush_all();
    }
    void overflow(std::string st, P2Pchannel* p2pchnl, uint32_t* R, uint32_t* A, uint32_t len, 
                        uint64_t (*share_cb)(uint64_t a, uint64_t b) = [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);}){
        if(st == "aid"){
            return;
        }
        uint64_t* R_plus = (uint64_t*) malloc(sizeof(uint64_t) * len);
        uint64_t* A_plus = (uint64_t*) malloc(sizeof(uint64_t) * len);
        uint64_t* R_hat = (uint64_t*) malloc(sizeof(uint64_t) * len);
        uint64_t* A_hat = (uint64_t*) malloc(sizeof(uint64_t) * len);
        uint64_t* tmp = (uint64_t*) malloc(sizeof(uint64_t) * len);
        uint64_t* tmp2 = (uint64_t*) malloc(sizeof(uint64_t) * len);
        for(int i = 0; i < len; i ++){
            R_plus[i] = (uint64_t)R[i] + get<2>(cks[i])[1];
            A_plus[i] = (uint64_t)A[i] + get<2>(cks[i])[0];
        }
        
        fourpc_reveal<uint64_t>(R_plus, tmp, len, {"player0","player2"}, st, p2pchnl, 
                    [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);});
        memcpy(R_plus, tmp, len*sizeof(uint64_t));
        fourpc_reveal<uint64_t>(A_plus, tmp, len, {"player1","player3"}, st, p2pchnl, 
                    [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);});
        memcpy(A_plus, tmp, len*sizeof(uint64_t));
        for(int i = 0; i < len; i ++){
            uint32_t beta;
            if(st == "player0"|| st == "player2"){
                beta = (uint32_t)evaluateLt(&cmpkey_32, &get<0>(cks[i]), R_plus[i]);
                if(st == "player2") beta = -beta;
                R_hat[i] = ((uint64_t)R[i] - (uint64_t)beta<<32)%(1<<33);
                A_hat[i] = A[i];
                
            }else{
                beta = (uint32_t)evaluateLt(&cmpkey_32, &get<0>(cks[i]), A_plus[i]);
                if(st == "player3") beta = -beta;
                R_hat[i] = R[i];
                A_hat[i] = ((uint64_t)A[i] - (uint64_t)beta<<32)%(1<<33);

            }
            tmp[i] = share_cb(R_hat[i], A_hat[i]) + get<3>(cks[i])[0];
        }
        fourpc_reveal<uint64_t>(tmp, tmp2, len, {"player1","player3"}, st, p2pchnl, 
                    [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);});

        // for(int i = 0; i < len; i ++){
        //     del[i] = del[i] + del2[i];
        //     mpz_class ans_0 = evaluateLt(&cmpkey, &cks[2*i], del[i]);
        //     mpz_class ans_1 = evaluateLt(&cmpkey, &cks[2*i+1], del[i]);
        //     uint32_t ans0 = mpz_get_ui(ans_0.get_mpz_t());
        //     uint32_t ans1 = mpz_get_ui(ans_1.get_mpz_t());
        //     free_key<ServerKeyLt>(cks[2*i]);
        //     free_key<ServerKeyLt>(cks[2*i+1]);
        //     std::cout<<ans0<<" "<<ans1<<std::endl;
        // }
        // free(del); free(del2);
    }
    ~Compare(){
        free_fss_key(cmpkey_32);
        free_fss_key(cmpkey_33);
    }
};

#endif
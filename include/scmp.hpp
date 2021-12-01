#ifndef _SCMP_H__
#define _SCMP_H__
#include "fss-client.h"
#include "net.hpp"
#include "fss-common.h"
#include "fss-server.h"
#include "convert.h"
#include "preliminaries.hpp"
#include <tuple>
#include <array>
using scmp_tuple = tuple<ServerKeyLt, ServerKeyLt, array<uint32_t,2>, array<uint64_t,2>>;
using eq_tuple = tuple<ServerKeyEq, uint32_t>;
/*                                        

                                    原来使用uint32!!!!!
                                        |
                                        |
                                        V
*/
using msb_tuple = tuple<ServerKeyLt, uint64_t>;

class Compare{
    private:
    vector<uint64_t*> tmps_64;
    vector<uint32_t*> tmps_32;
    
    uint64_t* tmp_64;
    uint32_t* R_plus;
    uint32_t* A_plus;
    uint64_t* A_op_B;
    uint32_t* delta;
    std::string st;
    P2Pchannel* p2pchnl;
    bool isconv = false;
    bool cmpkey_32_init = false;
    bool cmpkey_33_init = false;
    public:
    std::vector<scmp_tuple> cks;
    std::vector<eq_tuple> eks;
    std::vector<msb_tuple> mks;
    Fss cmpkey_32, cmpkey_33;
    Convert *conv;
    Compare(std::string st, P2Pchannel* p2pchnl):st(st), p2pchnl(p2pchnl){};
    
    void eq_off(uint32_t len){
        if(st == "aid"){
            initializeClient(&cmpkey_32, 32, 2);
            cmpkey_32_init = true;
            send_fss_key(cmpkey_32, p2pchnl);
            ServerKeyEq k0, k1;
            uint32_t r_cmpe;
            for(int i = 0; i < len; i ++){
                r_cmpe = rand();
                generateTreeEq(&cmpkey_32, &k0, &k1, r_cmpe, 1);
                fourpc_share<uint32_t>(&r_cmpe, 1, st, p2pchnl);
                send_eq_key(k0, cmpkey_32, "player1", p2pchnl);
                send_eq_key(k1, cmpkey_32, "player3", p2pchnl);
                free_key<ServerKeyEq>(k0);free_key<ServerKeyEq>(k1);
            }
        }else{
            uint32_t r_cmpe;
            

            recv_fss_key(cmpkey_32, p2pchnl);
            cmpkey_32_init = true;
            for(int i = 0; i < len; i ++){
                ServerKeyEq k_w;
                fourpc_share<uint32_t>(&r_cmpe, 1, st, p2pchnl);
                if(st == "player1" || st == "player3"){
                    recv_eq_key(k_w, cmpkey_32, "aid", p2pchnl);
                }
                eq_tuple tmp = {k_w, r_cmpe};
                eks.push_back(tmp);
            }

        }
    }
    void msb_off(uint32_t len, int msb_len = 33){
        if(st == "aid"){
            initializeClient(&cmpkey_33, msb_len, 2);
            cmpkey_33_init = true;
            send_fss_key(cmpkey_33, p2pchnl);
            uint64_t msb_delta0,msb_delta1;
            ServerKeyLt lt_00, lt_01, lt_10, lt_11;
            for(int i = 0; i < len; i++){
                msb_delta0 = rand();
                msb_delta1 = msb_delta0+((uint64_t)1<<(msb_len-1));
                generateTreeLt(&cmpkey_33, &lt_00, &lt_01, msb_delta0, 1);
                generateTreeLt(&cmpkey_33, &lt_10, &lt_11, msb_delta1, 1);
                send_lt_key(lt_00, cmpkey_33, "player1", p2pchnl);
                send_lt_key(lt_01, cmpkey_33, "player3", p2pchnl);
                send_lt_key(lt_10, cmpkey_33, "player0", p2pchnl);
                send_lt_key(lt_11, cmpkey_33, "player2", p2pchnl);
                fourpc_share<uint64_t>(&msb_delta0, 1, st, p2pchnl, 
                    [=](uint64_t a, uint64_t b)->uint64_t{ return (a + ((uint64_t)1<<msb_len) - b) % ((uint64_t)1<<msb_len);},
                    [=](uint64_t* data, uint16_t lens)->void{for(int i = 0; i < lens; i++)data[i] = rand();});
                ServerKeyLt keyarr[4] = {lt_00, lt_01, lt_10, lt_11};
                free_keys<ServerKeyLt>(keyarr, 4);
            }
        }else{
            recv_fss_key(cmpkey_33, p2pchnl);
            cmpkey_33_init = true;

            for(int i = 0; i < len; i++){
                ServerKeyLt lt_k1;
                uint64_t delta;
                recv_lt_key(lt_k1, cmpkey_33, "aid", p2pchnl);
                
                fourpc_share<uint64_t>(&delta, 1, st, p2pchnl); 

                msb_tuple tmp = {lt_k1,  delta};
                mks.push_back(tmp);
            }
        }
    }
    void msb(uint32_t* R,  uint32_t len, uint32_t* flags, int msb_len = 33){
        if(st == "aid"){
            
            return;
        }
        uint64_t* tmp = (uint64_t*) malloc(sizeof(uint64_t) * len);
        uint64_t* R_plus = (uint64_t*) malloc(sizeof(uint64_t) * len);
        for(int i = 0; i < len; i++){
            R_plus[i] = ( (uint64_t)R[i] + get<1>(mks[i]) )% ((uint64_t)1<<msb_len);
            
        }
        fourpc_reveal<uint64_t>(R_plus, tmp, len, {"player0","player1","player2","player3"}, st, p2pchnl, 
                    [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<msb_len);});
        for(int i = 0; i < len; i ++){
            
            if(st == "player1"|| st == "player3"){
                flags[i] = (uint32_t)evaluateLt(&cmpkey_33, &get<0>(mks[i]), tmp[i]);
                if(st == "player3") flags[i] = -flags[i];
            }else{
                flags[i] = (uint32_t)evaluateLt(&cmpkey_33, &get<0>(mks[i]), tmp[i]);
                if(st == "player0") flags[i] = (uint32_t)1 - flags[i];
            }
            free_key<ServerKeyLt>(get<0>(mks[i]));
        }
        for(int i = 0; i < mks.size() - len; i++){
            mks[i] = mks[len + i];
        }
        for(int i = 0; i <  len; i++){
            mks.pop_back();
        }
        free(tmp);
        free(R_plus);
    }
    void msb(uint64_t* R,  uint32_t len, uint32_t* flags, int msb_len = 33){
        if(st == "aid"){
            
            return;
        }
        uint64_t* tmp = (uint64_t*) malloc(sizeof(uint64_t) * len);
        uint64_t* R_plus = (uint64_t*) malloc(sizeof(uint64_t) * len);
        for(int i = 0; i < len; i++){
            R_plus[i] = ( R[i] + get<1>(mks[i]) )% ((uint64_t)1<<msb_len);
            
        }
        fourpc_reveal<uint64_t>(R_plus, tmp, len, {"player0","player1","player2","player3"}, st, p2pchnl, 
                    [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<msb_len);});
        for(int i = 0; i < len; i ++){
            
            if(st == "player1"|| st == "player3"){
                flags[i] = (uint32_t)evaluateLt(&cmpkey_33, &get<0>(mks[i]), tmp[i]);
                if(st == "player3") flags[i] = -flags[i];
            }else{
                flags[i] = (uint32_t)evaluateLt(&cmpkey_33, &get<0>(mks[i]), tmp[i]);
                if(st == "player0") flags[i] = (uint32_t)1 - flags[i];
            }
            free_key<ServerKeyLt>(get<0>(mks[i]));
        }
        for(int i = 0; i < mks.size() - len; i++){
            mks[i] = mks[len + i];
        }
        for(int i = 0; i <  len; i++){
            cks.pop_back();
        }
        free(tmp);
        free(R_plus);
    }
    /*split version*/
    void msb_1(uint32_t* R,  uint32_t len, uint32_t* flags, uint64_t * tmp_ptr, int msb_len = 33, int bias = 0){
        if(st == "aid"){
            
            return;
        }
        for(int i = 0; i < len; i++){
            tmp_ptr[i] = ( (uint64_t)R[i] + get<1>(mks[bias + i]) )% ((uint64_t)1<<msb_len);
            
        }
        fourpc_reveal_1<uint64_t>(tmp_ptr, tmp_ptr, len, {"player0","player1","player2","player3"}, st, p2pchnl, 
                    [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<msb_len);});
        
    }
    void msb_1(uint64_t* R,  uint32_t len, uint32_t* flags, uint64_t * tmp_ptr, int msb_len = 33, int bias = 0){
        if(st == "aid"){
            
            return;
        }
        for(int i = 0; i < len; i++){
            tmp_ptr[i] = (R[i] + get<1>(mks[bias + i]) )% ((uint64_t)1<<msb_len);
            
        }
        fourpc_reveal_1<uint64_t>(tmp_ptr, tmp_ptr, len, {"player0","player1","player2","player3"}, st, p2pchnl, 
                    [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<msb_len);});
        
    }
    /*split version*/
    void msb_2(uint32_t* R,  uint32_t len, uint32_t* flags, uint64_t * tmp_ptr, int msb_len = 33, int bias = 0){
        if(st == "aid"){
            
            return;
        }
        uint64_t *tmp = (uint64_t*)malloc(len * sizeof(uint64_t)); 
        fourpc_reveal_2<uint64_t>(tmp_ptr, tmp, len, {"player0","player1","player2","player3"}, st, p2pchnl, 
                    [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<msb_len);});
        for(int i = 0; i < len; i ++){
            
            if(st == "player1"|| st == "player3"){
                flags[i] = (uint32_t)evaluateLt(&cmpkey_33, &get<0>(mks[bias + i]), tmp[i]);
                if(st == "player3") flags[i] = -flags[i];
            }else{
                flags[i] = (uint32_t)evaluateLt(&cmpkey_33, &get<0>(mks[bias + i]), tmp[i]);
                if(st == "player0") flags[i] = (uint32_t)1 - flags[i];
            }
            free_key<ServerKeyLt>(get<0>(mks[bias + i]));
        }
        free(tmp);

    }
    void msb_2(uint64_t* R,  uint32_t len, uint32_t* flags, uint64_t * tmp_ptr, int msb_len = 33, int bias = 0){
        if(st == "aid"){
            
            return;
        }
        uint64_t *tmp = (uint64_t*)malloc(len * sizeof(uint64_t)); 
        fourpc_reveal_2<uint64_t>(tmp_ptr, tmp, len, {"player0","player1","player2","player3"}, st, p2pchnl, 
                    [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<msb_len);});
        for(int i = 0; i < len; i ++){
            
            if(st == "player1"|| st == "player3"){
                flags[i] = (uint32_t)evaluateLt(&cmpkey_33, &get<0>(mks[bias + i]), tmp[i]);
                if(st == "player3") flags[i] = -flags[i];
            }else{
                flags[i] = (uint32_t)evaluateLt(&cmpkey_33, &get<0>(mks[bias + i]), tmp[i]);
                if(st == "player0") flags[i] = (uint32_t)1 - flags[i];
            }
            free_key<ServerKeyLt>(get<0>(mks[bias + i]));
        }
        free(tmp);
    }
    void scmp_off(uint32_t len, int extend_len = 33){
        msb_off(3*len, extend_len);
        conv = new Convert(st, p2pchnl);
        isconv = true;
        conv->fourpc_zeroshare<uint32_t>(len);
    }

    void equal(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags){
        if(st == "aid"){
            
            return;
        }
        uint32_t * delta = (uint32_t*) malloc(sizeof(uint32_t) * len);
        uint32_t * tmp = (uint32_t*) malloc(sizeof(uint32_t) * len);
        for(int i = 0; i < len; i++){
            delta[i] = R[i] - A[i] + get<1>(eks[i]);
        }
        fourpc_reveal<uint32_t>(delta, tmp, len, {"player1","player3"}, st, p2pchnl);
        if(st == "player1" || st == "player3"){
            for(int i = 0; i < len; i++){
                mpz_class ans = evaluateEq(&cmpkey_32, &get<0>(eks[i]), tmp[i]);
                flags[i] = mpz_get_ui(ans.get_mpz_t());
                free_key<ServerKeyEq>(get<0>(eks[i]));
            }
            if(st == "player1") p2pchnl->send_data_to("player0", flags, len*sizeof(uint32_t));
            else{
                for(int i = 0; i < len; i ++) flags[i] = - flags[i];
                p2pchnl->send_data_to("player2", flags, len*sizeof(uint32_t));
            } 
        }else{
            if(st == "player0") p2pchnl->recv_data_from("player1", flags, len*sizeof(uint32_t));
            else p2pchnl->recv_data_from("player3", flags, len*sizeof(uint32_t));
        }
        free(delta);
        free(tmp);
    }
    void equal_1(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags){
        if(st == "aid"){
                    
            return;
        }
        delta = (uint32_t*) malloc(sizeof(uint32_t) * len);
        for(int i = 0; i < len; i++){
            delta[i] = R[i] - A[i] + get<1>(eks[i]);
        }
        fourpc_reveal_1<uint32_t>(delta, delta, len, {"player1","player3"}, st, p2pchnl);
        
    }
    void equal_2(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags){
        if(st == "aid"){
            
            return;
        }
        uint32_t * tmp = (uint32_t*) malloc(sizeof(uint32_t) * len);
        fourpc_reveal_2<uint32_t>(delta, tmp, len, {"player1","player3"}, st, p2pchnl);
        if(st == "player1" || st == "player3"){
            for(int i = 0; i < len; i++){
                mpz_class ans = evaluateEq(&cmpkey_32, &get<0>(eks[i]), tmp[i]);
                flags[i] = mpz_get_ui(ans.get_mpz_t());
                free_key<ServerKeyEq>(get<0>(eks[i]));
            }
            if(st == "player1") p2pchnl->send_data_to("player0", flags, len*sizeof(uint32_t));
            else{
                for(int i = 0; i < len; i ++) flags[i] = - flags[i];
                p2pchnl->send_data_to("player2", flags, len*sizeof(uint32_t));
            } 
        }
        free(delta);
        free(tmp);
    }
    void equal_3(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags){
        if(st == "player0" || st == "player2"){
            if(st == "player0") p2pchnl->recv_data_from("player1", flags, len*sizeof(uint32_t));
            else p2pchnl->recv_data_from("player3", flags, len*sizeof(uint32_t));
        }
    }
    void overflow(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags, int extend_len = 33,
                        function<uint64_t (uint64_t, uint64_t)>share_cb = [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);}){
            
        if(st == "aid"){
            
            return;
        }
        uint64_t R_hat[len];
        uint64_t A_hat[len];
        uint32_t betar[len],betaa[len];
        msb(R, len, betar, extend_len);
        msb(A, len, betaa, extend_len);
        for(int i = 0; i < len; i ++){    
            R_hat[i] = ((uint64_t)R[i] + ((uint64_t)1<<extend_len) - (((uint64_t)betar[i]<<32) %((uint64_t)1<<extend_len)))%((uint64_t)1<<extend_len);
            A_hat[i] = ((uint64_t)A[i] + ((uint64_t)1<<extend_len) - (((uint64_t)betaa[i]<<32) %((uint64_t)1<<extend_len)))%((uint64_t)1<<extend_len);
            A_hat[i] = share_cb(R_hat[i],A_hat[i]);
        }
        
        msb(A_hat, len, flags, extend_len);
        conv->fourpc_share_2_replicated_share<uint32_t>(flags, len);

    }
    void overflow_1(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags, int extend_len = 33,
                        function<uint64_t (uint64_t, uint64_t)>share_cb = [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);}){
            
        if(st == "aid"){
            
            return;
        }
        R_plus = (uint32_t*) malloc(len*sizeof(uint32_t));
        A_plus = (uint32_t*) malloc(len*sizeof(uint32_t));
        uint64_t *tmp_ptr0 = (uint64_t*) malloc(len*sizeof(uint64_t));
        uint64_t *tmp_ptr1 = (uint64_t*) malloc(len*sizeof(uint64_t));
        tmps_64.push_back(tmp_ptr0);
        tmps_64.push_back(tmp_ptr1);
        
        msb_1(R, len, R_plus, tmp_ptr0, extend_len);
        msb_1(A, len, A_plus, tmp_ptr1, extend_len, len);


    }
    void overflow_2(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags, int extend_len = 33,
                        function<uint64_t (uint64_t, uint64_t)>share_cb = [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);}){
            
        if(st == "aid"){
            return;
        }
        uint64_t R_hat[len];
        uint64_t A_hat[len];
        msb_2(R, len, R_plus, tmps_64[0], extend_len);
        msb_2(A, len, A_plus, tmps_64[1], extend_len, len);
        
        for(int i = 0; i < len; i ++){    
            R_hat[i] = ((uint64_t)R[i] + ((uint64_t)1<<extend_len) - (((uint64_t)R_plus[i]<<32) %((uint64_t)1<<extend_len)))%((uint64_t)1<<extend_len);
            A_hat[i] = ((uint64_t)A[i] + ((uint64_t)1<<extend_len) - (((uint64_t)A_plus[i]<<32) %((uint64_t)1<<extend_len)))%((uint64_t)1<<extend_len);
            
            tmps_64[1][i] = share_cb(R_hat[i],A_hat[i]);
        }
        msb_1(tmps_64[1], len, flags, tmps_64[0], extend_len, 2*len);
        free(R_plus);free(A_plus);

    }
    void overflow_3(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags, int extend_len = 33,
                        function<uint64_t (uint64_t, uint64_t)>share_cb = [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);}){
        if(st == "aid"){
            return;
        }
        msb_2(tmps_64[1], len, flags, tmps_64[0], extend_len, 2*len);
        conv->fourpc_share_2_replicated_share_1<uint32_t>(flags, len);
        free(tmps_64[1]);free(tmps_64[0]);
    }
    void overflow_end(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags, int extend_len = 33,
                        function<uint64_t (uint64_t, uint64_t)>share_cb = [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);}){
        if(st == "aid"){
            return;
        }
        conv->fourpc_share_2_replicated_share_2<uint32_t>(flags, len);
    }
    /*split*/
    // void overflow_1(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags, int extend_len = 33,
    //                     function<uint64_t (uint64_t, uint64_t)>share_cb = [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);}){
        
        
    //     if(st == "aid"){
            
    //         return;
    //     }
    //     R_plus = (uint64_t*) malloc(sizeof(uint64_t) * len);
    //     A_plus = (uint64_t*) malloc(sizeof(uint64_t) * len);
    //     for(int i = 0; i < len; i ++){
    //         R_plus[i] = (uint64_t)R[i] + get<2>(cks[i])[1];
    //         A_plus[i] = (uint64_t)A[i] + get<2>(cks[i])[0];
    //     }
        
    //     fourpc_reveal_1<uint64_t>(R_plus, R_plus, len, {"player0","player2"}, st, p2pchnl, 
    //                 [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<extend_len);});      
    //     fourpc_reveal_1<uint64_t>(A_plus, A_plus, len, {"player1","player3"}, st, p2pchnl, 
    //                 [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<extend_len);});
    // }
    // void overflow_2(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags, int extend_len = 33,
    //                     function<uint64_t (uint64_t, uint64_t)>share_cb = [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);}){
        
        
    //     if(st == "aid"){
            
    //         return;
    //     }
    //     A_op_B = (uint64_t*) malloc(sizeof(uint64_t) * len);
    //     fourpc_reveal_2<uint64_t>(R_plus, A_op_B, len, {"player0","player2"}, st, p2pchnl, 
    //                 [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<extend_len);});
    //     memcpy(R_plus, A_op_B, len*sizeof(uint64_t));
    //     fourpc_reveal_2<uint64_t>(A_plus, A_op_B, len, {"player1","player3"}, st, p2pchnl, 
    //                 [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<extend_len);});
    //     memcpy(A_plus, A_op_B, len*sizeof(uint64_t));
    //     uint64_t*R_hat = (uint64_t*) malloc(sizeof(uint64_t) * len);
    //     uint64_t*A_hat = (uint64_t*) malloc(sizeof(uint64_t) * len);
        
    //     for(int i = 0; i < len; i ++){
    //         uint32_t beta;
    //         if(st == "player0"|| st == "player2"){
    //             beta = (uint32_t)evaluateLt(&cmpkey_32, &get<0>(cks[i]), R_plus[i]);
    //             if(st == "player2") beta = -beta;
    //             R_hat[i] = ((uint64_t)R[i] + ((uint64_t)1<<extend_len) - (((uint64_t)beta<<32) %((uint64_t)1<<extend_len)))%((uint64_t)1<<extend_len);
    //             A_hat[i] = A[i];
                
    //         }else{
    //             beta = (uint32_t)evaluateLt(&cmpkey_32, &get<0>(cks[i]), A_plus[i]);
    //             if(st == "player3") beta = -beta;
    //             R_hat[i] = R[i];
    //             A_hat[i] = ((uint64_t)A[i] + ((uint64_t)1<<extend_len) - (((uint64_t)beta<<32) %((uint64_t)1<<extend_len)))%((uint64_t)1<<extend_len);

    //         }
    //         A_op_B[i] = share_cb(R_hat[i], A_hat[i]) + get<3>(cks[i])[0] %((uint64_t)1<<extend_len);
    //         free_key<ServerKeyLt>(get<0>(cks[i]));
    //     }
    //     fourpc_reveal_1<uint64_t>(A_op_B, A_op_B, len, {"player0","player1","player2","player3"}, st, p2pchnl, 
    //                 [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<extend_len);});
    //     free(R_plus);
    //     free(A_plus);
    //     free(A_hat);
    //     free(R_hat);  
    // }
    // void overflow_3(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags, int extend_len = 33,
    //                     function<uint64_t (uint64_t, uint64_t)>share_cb = [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);}){
    //     if(st == "aid"){
            
    //         return;
    //     }
    //     uint64_t* tmp = (uint64_t*) malloc(sizeof(uint64_t) * len);
    //     fourpc_reveal_2<uint64_t>(A_op_B, tmp, len, {"player0","player1","player2","player3"}, st, p2pchnl, 
    //                 [=](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<extend_len);});
    //     for(int i = 0; i < len; i ++){
    //         if(st == "player1"|| st == "player3"){
    //             flags[i] = (uint32_t)evaluateLt(&cmpkey_33, &get<1>(cks[i]), tmp[i]);
    //             if(st == "player3") flags[i] = -flags[i];
                
                
    //         }else{
    //             flags[i] = (uint32_t)evaluateLt(&cmpkey_33, &get<1>(cks[i]), tmp[i]);
    //             if(st == "player0") flags[i] = (uint32_t)1 - flags[i];
    //         }
    //         free_key<ServerKeyLt>(get<1>(cks[i]));
    //     }
    //     for(int i = 0; i < cks.size() - len; i++){
    //         cks[i] = cks[len + i];
    //     }
    //     for(int i = 0; i <  len; i++){
    //         cks.pop_back();
    //     }
        
    //     conv->fourpc_share_2_replicated_share_1<uint32_t>(flags, len);
    //     free(tmp);
    //     free(A_op_B);
            
    // }
    // void overflow_end(uint32_t* R, uint32_t* A, uint32_t len, uint32_t* flags, int extend_len = 33,
    //                     function<uint64_t (uint64_t, uint64_t)>share_cb = [](uint64_t a, uint64_t b)->uint64_t{ return (a + b) % ((uint64_t)1<<33);}){

    //     conv->fourpc_share_2_replicated_share_2<uint32_t>(flags, len);
    // }
    ~Compare(){
        if(isconv) delete conv;
        if(cmpkey_32_init)free_fss_key(cmpkey_32);
        if(cmpkey_33_init)free_fss_key(cmpkey_33);
    }
};

#endif

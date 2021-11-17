#ifndef _PRELIMINARIES_H__
#define _PRELIMINARIES_H__
#include <string>
#include "fss-client.h"
#include "fss-common.h"
#include "fss_help.h"
#include <vector>
using namespace std;
using fss_triple = pair<ServerKeyLt, uint32_t>;
uint32_t logn(uint32_t x){
    for(uint32_t i = 0; i < 32; i++){
        if(pow(2, i) >= x)
            return i
    }
    return 32;
}
uint32_t sub(uint32_t a, uint32_t b, uint32_t c){
    if(a >= b){
        return a - b;
    }
    return a + c -b;
}
template <typename T>
class Ram{
    /*T type of element, L type of lens*/
private:
    Fss faccess;
    bool init_flag = false;
    std::vector<fss_triple> read_triples, write_triples;
    int read_times, write_times;
    void share(std::string from_p){

    }
    
public:
    uint32_t data_len;
    T* data_ptr;
    std::string st;
    P2Pchannel p2pchnl;
    /*load for data*/
    Ram(T* data_in, uint32_t lens, std::string st, P2Pchannel p2pchnl):data_ptr(data_in), data_len(lens), st(st), p2pchnl(p2pchnl){}
    /*from other's*/
    Ram(uint32_t lens, std::string st, P2Pchannel p2pchnl):data_len(lens), st(st), p2pchnl(p2pchnl){
        data_ptr = (T*)malloc(sizeof(T)*lens);
    }
    /*from files*/
    Ram( std::string path){}

    /*init fss key*/
    void init(){
        srand((unsigned)time(NULL));
        if(st == "aid"){
            initializeClient(&faccess, logn(data_len), 2);
            send_fss_key(faccess, p2pchnl);
        }
        else{
            recv_fss_key(faccess, p2pchnl);
        }
        init_flag = true;
    }
    void prepare_read(uint16_t ntimes){
        if(!init_flag){
            /*异常处理*/
            return;
        }
        for(int i = 0; i < ntimes; i++){
            if(st == "aid"){
                uint32_t delta0 = rand() % data_len, delta1 = rand() % data_len;
                ServerKeyEq k0, k1, k2, k3;
                generateTreeEq(&faccess, &k0, &k1, delta0, 1);
                generateTreeEq(&faccess, &k2, &k3, delta1, 1);
                send_eq_key(k0, faccess, "player0", p2pchnl);
                send_eq_key(k1, faccess, "player1", p2pchnl);
                send_eq_key(k2, faccess, "player2", p2pchnl);
                send_eq_key(k3, faccess, "player3", p2pchnl);
                uint32_t delta00 = rand() % data_len, delta01 = delta0 >= delta00 ? delta0 - delta00 : delta0 + data_len - delta00;
                p2pchnl->send_data_to("player0",&delta00,sizeof(uint32_t));
                p2pchnl->send_data_to("player1",&delta01,sizeof(uint32_t));
                uint32_t delta10 = rand() % data_len, delta11 = delta1 >= delta10 ? delta1 - delta10 : delta1 + data_len - delta10;
                p2pchnl->send_data_to("player2",&delta10,sizeof(uint32_t));
                p2pchnl->send_data_to("player3",&delta11,sizeof(uint32_t));
                free_key<ServerKeyEq>(k0);
                free_key<ServerKeyEq>(k1);
                free_key<ServerKeyEq>(k2);
                free_key<ServerKeyEq>(k3);
            }else{
                ServerKeyEq k_w;
                recv_eq_key(k_w, faccess, "aid", p2pchnl);
                uint32_t delta;
                p2pchnl->recv_data_from("aid",&delta,sizeof(uint32_t));
                read_triples.push_back({k_w, delta});
            }
        }
    }
    void prepare_write(uint16_t ntimes){

    }
    T read(uint32_t idex){
        if(read_times >= read_triples.size()){
            /*异常处理*/
            return;
        }
        uint32_t delta = sub(idex, read_triples[read_times].second, data_len);
        T res;
        for(uint32_t i = 0; i < data_len; i++){
            mpz_class ans = evaluateEq(&faccess, &read_triples[read_times].first, sub(i, delta, data_len));
            uint32_t tmp = mpz_get_ui(ans.get_mpz_t());
            
            res += data_ptr[i] * tmp;
        }

        read_times ++;
        return res;
    }
    void write(uint32_t idex, T target){
        
    }

};


#endif
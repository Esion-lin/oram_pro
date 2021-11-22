
#ifndef _PRELIMINARIES_H__
#define _PRELIMINARIES_H__
#include <string.h>
#include "fss-client.h"

#include "fss-server.h"

#include "fss-common.h"
#include "fss_help.h"
#include <vector>

using namespace std;
using fss_triple = pair<ServerKeyEq, uint32_t>;
using write_tuple = tuple<ServerKeyEq, ServerKeyEq, uint32_t, uint32_t, uint32_t>;
uint32_t logn(uint32_t x){
    for(uint32_t i = 0; i < 32; i++){
        if(pow(2, i) >= x)
            return i;
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
void rand_t(T* data, uint16_t data_len){
    /*
    TODO: use prf
    */
    int lens = sizeof(T);
    uint8_t tmp[lens*data_len];
    for(int i = 0; i < lens * data_len; i++){
        tmp[i] = rand();
    }
    memcpy(data, tmp, lens*data_len);
}
template <typename T>
void replicated_share(T* data, uint16_t lens, std::string st, P2Pchannel *p2pchnl){
    /*registers, memory and tapes*/
    if(st == "aid"){
        T *data0 = (T*) malloc(sizeof(T)*lens);
        T *data1 = (T*) malloc(sizeof(T)*lens);
        rand_t<T>(data0, lens);
        for(int i = 0; i < lens; i++){
            data1[i] = data[i] - data0[i];
        }
        p2pchnl->send_data_to("player0", data0, sizeof(T)*lens);
        p2pchnl->send_data_to("player1", data0, sizeof(T)*lens);
        p2pchnl->send_data_to("player2", data1, sizeof(T)*lens);
        p2pchnl->send_data_to("player3", data1, sizeof(T)*lens);
        free(data0);free(data1);
    }else{
        p2pchnl->recv_data_from("aid", data, sizeof(T)*lens);
    }

}
template <typename T>
void fourpc_share(T* data, uint16_t lens, std::string st, P2Pchannel *p2pchnl){
    if(st == "aid"){
        T *data0 = (T*) malloc(sizeof(T)*lens);
        T *data1 = (T*) malloc(sizeof(T)*lens);
        T *data2 = (T*) malloc(sizeof(T)*lens);
        rand_t<T>(data0, lens);
        rand_t<T>(data1, lens);
        rand_t<T>(data2, lens);
        for(int i = 0; i < lens; i++){
            data[i] = data[i] - data0[i] - data1[i] - data2[i];
        }
        p2pchnl->send_data_to("player0", data, sizeof(T)*lens);
        p2pchnl->send_data_to("player1", data0, sizeof(T)*lens);
        p2pchnl->send_data_to("player2", data1, sizeof(T)*lens);
        p2pchnl->send_data_to("player3", data2, sizeof(T)*lens);
        free(data0);free(data1);free(data2);
    }else{
        p2pchnl->recv_data_from("aid", data, sizeof(T)*lens);
    }
}
template <typename T>
void twopc_share(T* data, uint16_t lens, std::string st, P2Pchannel *p2pchnl){
    if(st == "aid"){
        T *data0 = (T*) malloc(sizeof(T)*lens);
        T *data1 = (T*) malloc(sizeof(T)*lens);
        rand_t<T>(data0, lens);
        for(int i = 0; i < lens; i++){
            data1[i] = data[i] - data0[i];
        }
        p2pchnl->send_data_to("player0", data0, sizeof(T)*lens);
        p2pchnl->send_data_to("player1", data1, sizeof(T)*lens);
        free(data0);free(data1);
    }else if(st == "player0" || st == "player1"){
        p2pchnl->recv_data_from("aid", data, sizeof(T)*lens);
    }
}
template <typename T>
class Ram{
    /*T type of element*/
    /*
    #### usage
    1. accept data and init
    '''
    Ram<uint32_t>* test_ram = new Ram<type_of_element>(type_of_element *data, len)
    test_ram->init();
    '''
    2. prepare read triples and write tuples
    '''
    test_ram->prepare_read(ntimes); # ntimes:  triples sizes
    test_ram->prepare_write(ntimes);
    '''
    3.read and write
    '''
    test_ram->read(index);
    test_ram->write(index, target, org);
    '''
    */
private:
    Fss faccess;
    bool init_flag = false;
    std::vector<fss_triple> read_triples;
    std::vector<write_tuple> write_triples;
    int read_times, write_times;
    void share(std::string from_p){

    }
    template <typename R>
    void twopc_reveal(R* data, R* data2, int len){
        if(st == "player0"){
            p2pchnl->send_data_to("player1", data, len*sizeof(T));
            p2pchnl->recv_data_from("player1", data2, len*sizeof(T));
            
        }else if(st == "player1"){
            p2pchnl->recv_data_from("player0", data2, len*sizeof(T));
            p2pchnl->send_data_to("player0", data, len*sizeof(T));
        }else if(st == "player2"){
            p2pchnl->send_data_to("player3", data, len*sizeof(T));
            p2pchnl->recv_data_from("player3", data2, len*sizeof(T));
        }else if(st == "player3"){
            p2pchnl->recv_data_from("player2", data2, len*sizeof(T));
            p2pchnl->send_data_to("player2", data, len*sizeof(T));
        }
        for(int i = 0 ; i < len; i ++){
            data2[i] = data[i] + data2[i];
        }
    }
    template <typename R>
    void diag_reveal(R* data, R* data2, int len){
        if(st == "player0"){
            p2pchnl->send_data_to("player2", data, len*sizeof(T));
            p2pchnl->recv_data_from("player2", data2, len*sizeof(T));
            
        }else if(st == "player2"){
            p2pchnl->recv_data_from("player0", data2, len*sizeof(T));
            p2pchnl->send_data_to("player0", data, len*sizeof(T));
        }else if(st == "player1"){
            p2pchnl->send_data_to("player3", data, len*sizeof(T));
            p2pchnl->recv_data_from("player3", data2, len*sizeof(T));
        }else if(st == "player3"){
            p2pchnl->recv_data_from("player1", data2, len*sizeof(T));
            p2pchnl->send_data_to("player1", data, len*sizeof(T));
        }
        for(int i = 0 ; i < len; i ++){
            data2[i] = data[i] + data2[i];
        }
    }
public:
    uint32_t data_len;
    T* data_ptr;
    std::string st;
    P2Pchannel* p2pchnl;
    /*load for data*/
    Ram(T* data_in, uint32_t lens, std::string st, P2Pchannel* p2pchnl):data_ptr(data_in), data_len(lens), st(st), p2pchnl(p2pchnl){}
    /*from other's*/
    Ram(uint32_t lens, std::string st, P2Pchannel* p2pchnl):data_len(lens), st(st), p2pchnl(p2pchnl){
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
        read_times = 0;
        write_times = 0;
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
        if(!init_flag){
            /*异常处理*/
            return;
        }
        for(int i = 0; i < ntimes; i++){
            if(st == "aid"){
                uint32_t r1 = rand() % data_len, r2 = rand() % data_len, v = rand();
                ServerKeyEq k0, k1, k2, k3;
                generateTreeEq(&faccess, &k0, &k1, r1, v);
                generateTreeEq(&faccess, &k2, &k3, r2, 1);
                send_eq_key(k0, faccess, "player0", p2pchnl);
                send_eq_key(k0, faccess, "player1", p2pchnl);
                send_eq_key(k2, faccess, "player0", p2pchnl);
                send_eq_key(k2, faccess, "player1", p2pchnl);
                send_eq_key(k1, faccess, "player2", p2pchnl);
                send_eq_key(k1, faccess, "player3", p2pchnl);
                send_eq_key(k3, faccess, "player2", p2pchnl);
                send_eq_key(k3, faccess, "player3", p2pchnl);
                uint32_t delta00 = rand() % data_len, delta01 = r1 >= delta00 ? r1 - delta00 : r1 + data_len - delta00;
                p2pchnl->send_data_to("player0",&delta00,sizeof(uint32_t));
                p2pchnl->send_data_to("player1",&delta00,sizeof(uint32_t));
                p2pchnl->send_data_to("player2",&delta01,sizeof(uint32_t));
                p2pchnl->send_data_to("player3",&delta01,sizeof(uint32_t));
                uint32_t delta10 = rand() % data_len, delta11 = r2 >= delta10 ? r2 - delta10 : r2 + data_len - delta10;
                p2pchnl->send_data_to("player0",&delta10,sizeof(uint32_t));
                p2pchnl->send_data_to("player1",&delta10,sizeof(uint32_t));
                p2pchnl->send_data_to("player2",&delta11,sizeof(uint32_t));
                p2pchnl->send_data_to("player3",&delta11,sizeof(uint32_t));
                uint32_t v0 = rand(), v1 = v - v0;
                p2pchnl->send_data_to("player0",&v0,sizeof(uint32_t));
                p2pchnl->send_data_to("player1",&v0,sizeof(uint32_t));
                p2pchnl->send_data_to("player2",&v1,sizeof(uint32_t));
                p2pchnl->send_data_to("player3",&v1,sizeof(uint32_t));
                free_key<ServerKeyEq>(k0);
                free_key<ServerKeyEq>(k1);
                free_key<ServerKeyEq>(k2);
                free_key<ServerKeyEq>(k3);
            }else{
                ServerKeyEq k_w, k_d;
                recv_eq_key(k_w, faccess, "aid", p2pchnl);
                recv_eq_key(k_d, faccess, "aid", p2pchnl);
                uint32_t delta0, delta1, v;
                p2pchnl->recv_data_from("aid",&delta0,sizeof(uint32_t));
                p2pchnl->recv_data_from("aid",&delta1,sizeof(uint32_t));
                p2pchnl->recv_data_from("aid",&v,sizeof(uint32_t));
                write_triples.push_back({k_w, k_d, delta0, delta1, v});
                
            }
        }
    }
    T read(uint32_t idex){
        if(st == "aid"){
            T a;
            return a;
        }
        if(read_times >= read_triples.size()){
            /*异常处理*/
            //return;
        }
        uint32_t delta = sub(idex, read_triples[read_times].second, data_len);
        uint32_t delta_r;
        twopc_reveal<uint32_t>(&delta, &delta_r, 1);
        while(delta_r > 2*data_len) delta_r+=data_len;
        delta_r = delta_r % data_len;
        T res;
        
        for(uint32_t i = 0; i < data_len; i++){
            mpz_class ans = evaluateEq(&faccess, &read_triples[read_times].first, sub(i, delta_r, data_len));
            uint32_t tmp = mpz_get_ui(ans.get_mpz_t());
            if(i == 0) res = data_ptr[i] * tmp;
            else res += data_ptr[i] * tmp;
        }
        free_key<ServerKeyEq>(read_triples[read_times].first);
        read_times ++;
        return res;
    }
    void write(uint32_t idex, T target, T org){
        if(st == "aid")
            return;
        if(write_times >= write_triples.size()){
            /*异常处理*/
            //return;
        }
        /*calculate and open deltaV*/
        T deltaV_plus = target - org - get<4>(write_triples[write_times]);
        T deltaV;
        diag_reveal<T>(&deltaV_plus, &deltaV, 1);
        std::cout<<"deltaV "<<deltaV<<std::endl;
        uint32_t delta[2] = {sub(idex, get<2>(write_triples[write_times]), data_len), sub(idex, get<3>(write_triples[write_times]), data_len)};
        uint32_t delta_r[2];
        diag_reveal<uint32_t>(delta, delta_r, 2);
        while(delta_r[0] > 2*data_len) delta_r[0]+=data_len;
        while(delta_r[1] > 2*data_len) delta_r[1]+=data_len;
        delta_r[0] = delta_r[0] % data_len;delta_r[1] = delta_r[1] % data_len;
        for(uint32_t i = 0; i < data_len; i++){
            mpz_class ans = evaluateEq(&faccess, &get<0>(write_triples[write_times]), sub(i, delta_r[0], data_len));
            uint32_t tmp = mpz_get_ui(ans.get_mpz_t());
            if(st == "player0" || st == "player1") data_ptr[i] += tmp;
            else data_ptr[i] -= tmp;
            ans = evaluateEq(&faccess, &get<1>(write_triples[write_times]), sub(i, delta_r[1], data_len));
            tmp = mpz_get_ui(ans.get_mpz_t());
            if(st == "player0" || st == "player1") data_ptr[i] += deltaV*tmp;
            else data_ptr[i] -= deltaV*tmp;
        }
        free_key<ServerKeyEq>(get<1>(write_triples[write_times]));
        free_key<ServerKeyEq>(get<0>(write_triples[write_times]));
        write_times ++;
    }
    ~Ram(){
        free_fss_key(faccess);
    }

};


#endif
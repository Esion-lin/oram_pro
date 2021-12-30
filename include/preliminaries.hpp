
#ifndef _PRELIMINARIES_H__
#define _PRELIMINARIES_H__
#include <string.h>
#include "fss-client.h"

#include "fss-server.h"

#include "fss-common.h"
#include "fss_help.hpp"
#include <vector>
#include <set>
#include <functional>
using namespace std;
using fss_triple = pair<ServerKeyEq, uint32_t>;
using write_tuple = tuple<ServerKeyEq, ServerKeyEq, uint32_t, uint32_t, uint32_t>;
inline uint32_t logn(uint32_t x){
    for(uint32_t i = 0; i < 32; i++){
        if(pow(2, i) >= x)
            return i;
    }
    return 32;
}
inline uint32_t sub(uint32_t a, uint32_t b, uint32_t c){
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
void replicated_share(T* data, uint16_t lens, std::string st, P2Pchannel *p2pchnl, 
                        function<T (T, T)>share_cb = [](T a, T b) -> T{return a - b;}, 
                        function<void (T*, uint16_t)>rand_data = rand_t<T>
                        ){
    /*registers, memory and tapes*/
    if(st == "aid"){
        
        T *data0 = (T*) malloc(sizeof(T)*lens);
        T *data1 = (T*) malloc(sizeof(T)*lens);
        rand_data(data0, lens);
        for(int i = 0; i < lens; i++){
            data1[i] = share_cb(data[i], data0[i]);
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
void fourpc_share(T* data, uint16_t lens, std::string st, P2Pchannel *p2pchnl, 
                            function<T (T, T)>share_cb = [](T a, T b) -> T{return a - b;},
                            function<void (T*, uint16_t)>rand_data = rand_t<T>){
    if(st == "aid"){
        T *data0 = (T*) malloc(sizeof(T)*lens);
        T *data1 = (T*) malloc(sizeof(T)*lens);
        T *data2 = (T*) malloc(sizeof(T)*lens);
        rand_data(data0, lens);
        rand_data(data1, lens);
        rand_data(data2, lens);
        for(int i = 0; i < lens; i++){
            data[i] = share_cb(share_cb(share_cb(data[i], data0[i]), data1[i]), data2[i]);
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
template <typename R>
void twopc_reveal(R* data, R* data2, int len, std::string st, P2Pchannel *p2pchnl){
    if(st == "player0"){
        p2pchnl->send_data_to("player1", data, len*sizeof(R));
        p2pchnl->recv_data_from("player1", data2, len*sizeof(R));
        
    }else if(st == "player1"){
        p2pchnl->send_data_to("player0", data, len*sizeof(R));
        p2pchnl->recv_data_from("player0", data2, len*sizeof(R));

    }else if(st == "player2"){
        p2pchnl->send_data_to("player3", data, len*sizeof(R));
        p2pchnl->recv_data_from("player3", data2, len*sizeof(R));
    }else if(st == "player3"){
        p2pchnl->send_data_to("player2", data, len*sizeof(R));
        p2pchnl->recv_data_from("player2", data2, len*sizeof(R));

    }
    for(int i = 0 ; i < len; i ++){
        data2[i] = data[i] + data2[i];
    }
}
/*split twopc_reveal*/
template <typename R>
void twopc_reveal_1(R* data, R* data2, int len, std::string st, P2Pchannel *p2pchnl){
    if(st == "player0"){
        p2pchnl->send_data_to("player1", data, len*sizeof(R));
    }else if(st == "player1"){
        p2pchnl->send_data_to("player0", data, len*sizeof(R));
    }else if(st == "player2"){
        p2pchnl->send_data_to("player3", data, len*sizeof(R));
    }else if(st == "player3"){
        p2pchnl->send_data_to("player2", data, len*sizeof(R));
    }
}
template <typename R>
void twopc_reveal_2(R* data, R* data2, int len, std::string st, P2Pchannel *p2pchnl){
    if(st == "player0"){
        p2pchnl->recv_data_from("player1", data2, len*sizeof(R));
    }else if(st == "player1"){
        p2pchnl->recv_data_from("player0", data2, len*sizeof(R));
    }else if(st == "player2"){
        p2pchnl->recv_data_from("player3", data2, len*sizeof(R));
    }else if(st == "player3"){
        p2pchnl->recv_data_from("player2", data2, len*sizeof(R));
    }
    for(int i = 0 ; i < len; i ++){
        data2[i] = data[i] + data2[i];
    }
}
template <typename R>
void diag_reveal(R* data, R* data2, int len, std::string st, P2Pchannel *p2pchnl){
    if(st == "player0"){
        p2pchnl->send_data_to("player2", data, len*sizeof(R));
        p2pchnl->recv_data_from("player2", data2, len*sizeof(R));
        
    }else if(st == "player2"){
        p2pchnl->send_data_to("player0", data, len*sizeof(R));
        p2pchnl->recv_data_from("player0", data2, len*sizeof(R));    
    }else if(st == "player1"){
        p2pchnl->send_data_to("player3", data, len*sizeof(R));
        p2pchnl->recv_data_from("player3", data2, len*sizeof(R));
    }else if(st == "player3"){
        p2pchnl->send_data_to("player1", data, len*sizeof(R));
        p2pchnl->recv_data_from("player1", data2, len*sizeof(R));  
    }
    for(int i = 0 ; i < len; i ++){
        data2[i] = data[i] + data2[i];
    }
}
/*split diag_reveal*/
template <typename R>
void diag_reveal_1(R* data, R* data2, int len, std::string st, P2Pchannel *p2pchnl){
    if(st == "player0"){
        p2pchnl->send_data_to("player2", data, len*sizeof(R));     
    }else if(st == "player2"){
        p2pchnl->send_data_to("player0", data, len*sizeof(R));  
    }else if(st == "player1"){
        p2pchnl->send_data_to("player3", data, len*sizeof(R));
    }else if(st == "player3"){
        p2pchnl->send_data_to("player1", data, len*sizeof(R));
    }
}
template <typename R>
void diag_reveal_2(R* data, R* data2, int len, std::string st, P2Pchannel *p2pchnl){
    if(st == "player0"){
        p2pchnl->recv_data_from("player2", data2, len*sizeof(R));
    }else if(st == "player2"){
        p2pchnl->recv_data_from("player0", data2, len*sizeof(R));    
    }else if(st == "player1"){
        p2pchnl->recv_data_from("player3", data2, len*sizeof(R));
    }else if(st == "player3"){
        p2pchnl->recv_data_from("player1", data2, len*sizeof(R));  
    }
    for(int i = 0 ; i < len; i ++){
        data2[i] = data[i] + data2[i];
    }
}
template <typename R>
void latt_reveal_1(R* data, R* data2, int len, std::string st, P2Pchannel *p2pchnl){
    if(st == "player0"){
        p2pchnl->send_data_to("player3", data, len*sizeof(R));     
    }else if(st == "player3"){
        p2pchnl->send_data_to("player0", data, len*sizeof(R));  
    }else if(st == "player1"){
        p2pchnl->send_data_to("player2", data, len*sizeof(R));
    }else if(st == "player2"){
        p2pchnl->send_data_to("player1", data, len*sizeof(R));
    }
}
template <typename R>
void latt_reveal_2(R* data, R* data2, int len, std::string st, P2Pchannel *p2pchnl){
    if(st == "player0"){
        p2pchnl->recv_data_from("player3", data2, len*sizeof(R));
    }else if(st == "player3"){
        p2pchnl->recv_data_from("player0", data2, len*sizeof(R));    
    }else if(st == "player1"){
        p2pchnl->recv_data_from("player2", data2, len*sizeof(R));
    }else if(st == "player2"){
        p2pchnl->recv_data_from("player1", data2, len*sizeof(R));  
    }
    for(int i = 0 ; i < len; i ++){
        data2[i] = data[i] + data2[i];
    }
}
template <typename R>
void fourpc_reveal(R* data, R* data2, int len, std::set<std::string> sts, std::string st, P2Pchannel *p2pchnl, 
                                    function<R (R, R)>share_cb = [](R a, R b) -> R{return a + b;}){
    /*reveal to target parties*/
    std::string ll[4] = {"player0","player1","player2","player3"};
    if(st == "aid") return;
    /*send phase*/
    for(auto &ele:sts){
        if(ele != st)
            p2pchnl->send_data_to(ele, data, len*sizeof(R));
    }
    /*recv phase*/
    R* tmp = (R*) malloc(len*sizeof(R));
    memcpy(data2, data, len*sizeof(R));
    if(sts.find(st) != sts.end()){
        for(auto &ele:ll){
            if(ele != st){
                p2pchnl->recv_data_from(ele, tmp, len*sizeof(R));
                for(int i = 0 ; i < len; i ++){
                    data2[i] = share_cb(data2[i], tmp[i]);
                }
            }
        }
    }
    free(tmp);
}
template <typename R>
void fourpc_reveal(R* data, int len, std::set<std::string> sts, std::string st, P2Pchannel *p2pchnl, 
                                    function<R (R, R)>share_cb = [](R a, R b) -> R{return a + b;}){
    R* data2 = (R*)malloc(len*sizeof(R));
    memcpy(data2, data, len*sizeof(R));
    fourpc_reveal<R>(data2, data, len, sts, st, p2pchnl, share_cb);
    free(data2);
}
/*split fourpc_reveal*/

template <typename R>
void fourpc_reveal_1(R* data, R* data2, int len, std::set<std::string> sts, std::string st, P2Pchannel *p2pchnl, 
                                    function<R (R, R)>share_cb = [](R a, R b) -> R{return a + b;}){
    /*reveal to target parties*/
    std::string ll[4] = {"player0","player1","player2","player3"};
    if(st == "aid") return;
    /*send phase*/
    for(auto &ele:sts){
        if(ele != st)
            p2pchnl->send_data_to(ele, data, len*sizeof(R));
    }
}

template <typename R>
void fourpc_reveal_2(R* data, R* data2, int len, std::set<std::string> sts, std::string st, P2Pchannel *p2pchnl, 
                                    function<R (R, R)>share_cb = [](R a, R b) -> R{return a + b;}){
    /*reveal to target parties*/
    std::string ll[4] = {"player0","player1","player2","player3"};
    if(st == "aid") return;
    /*recv phase*/
    R* tmp = (R*) malloc(len*sizeof(R));
    memcpy(data2, data, len*sizeof(R));
    if(sts.find(st) != sts.end()){
        for(auto &ele:ll){
            if(ele != st){
                p2pchnl->recv_data_from(ele, tmp, len*sizeof(R));
                for(int i = 0 ; i < len; i ++){
                    data2[i] = share_cb(data2[i], tmp[i]);
                }
            }
        }
    }
    free(tmp);
}

template <typename R>
void fourpc_reveal(R* data, R* data2, int len, std::string st, P2Pchannel *p2pchnl){
    /*reveal to all parties*/
    if(st == "aid") return;
    if(st != "player0")
        p2pchnl->send_data_to("player0", data, len*sizeof(R));
    if(st != "player1")
        p2pchnl->send_data_to("player1", data, len*sizeof(R));
    if(st != "player2")
        p2pchnl->send_data_to("player2", data, len*sizeof(R));
    if(st != "player3")
        p2pchnl->send_data_to("player3", data, len*sizeof(R));
    R* tmp = (R*) malloc(len*sizeof(R));
    memcpy(data2, data, len*sizeof(R));
    if(st != "player0"){
        p2pchnl->recv_data_from("player0", tmp, len*sizeof(R));
        for(int i = 0 ; i < len; i ++){
            data2[i] = data2[i] + tmp[i];
        }
    }  
    if(st != "player1"){
        p2pchnl->recv_data_from("player1", tmp, len*sizeof(R));
        for(int i = 0 ; i < len; i ++){
            data2[i] = data2[i] + tmp[i];
        }
    } 
    if(st != "player2"){
        p2pchnl->recv_data_from("player2", tmp, len*sizeof(R));
        for(int i = 0 ; i < len; i ++){
            data2[i] = data2[i] + tmp[i];
        }
    }  
    if(st != "player3"){
        p2pchnl->recv_data_from("player3", tmp, len*sizeof(R));
        for(int i = 0 ; i < len; i ++){
            data2[i] = data2[i] + tmp[i];
        }
    }
    free(tmp);
}
template <typename T>
struct dupliM
{
    uint16_t lens;
    T A;
    T *m;
    T operator[](int i){
        if(i<lens) return A;
        if(i >= 2*lens) return A;
        return m[i-lens];
    }
};
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
    bool is_contain = false;
    int read_times, write_times;
    void share(std::string from_p){

    }
    
public:
    uint32_t data_len;
    T* data_ptr;
    dupliM<T> data;
    std::string st;
    P2Pchannel* p2pchnl;
    /*load for data*/
    Ram(T* data_in, uint32_t lens, std::string st, P2Pchannel* p2pchnl):data_ptr(data_in), data_len(lens), st(st), p2pchnl(p2pchnl){}
    Ram(dupliM<T> data_in, uint32_t lens, std::string st, P2Pchannel* p2pchnl):data(data_in), data_len(lens), st(st), p2pchnl(p2pchnl){
        is_contain = true;
    }
    
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
    void prepare_write(uint16_t ntimes, bool is_additive = false){
        if(!init_flag){
            /*异常处理*/
            return;
        }
        for(int i = 0; i < ntimes; i++){
            if(st == "aid"){
                uint32_t r1 = rand() % data_len, r2 = rand() % data_len, v = rand()%(1<<8);
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
                if(is_additive){
                    /*nonimpelement*/
                }else{
                    uint32_t v0 = rand(), v1 = v - v0;
                    p2pchnl->send_data_to("player0",&v0,sizeof(uint32_t));
                    p2pchnl->send_data_to("player1",&v0,sizeof(uint32_t));
                    p2pchnl->send_data_to("player2",&v1,sizeof(uint32_t));
                    p2pchnl->send_data_to("player3",&v1,sizeof(uint32_t));
                }
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
    void recover_list(uint32_t idex, uint32_t * list, bool is_rep = true){
        if(st == "aid"){
            return;
        }
        if(read_times >= read_triples.size()){
            /*异常处理*/
            //return;
        }
        uint32_t delta_r;
        if(is_rep){
            uint32_t delta = sub(idex, read_triples[read_times].second, data_len);
            twopc_reveal<uint32_t>(&delta, &delta_r, 1, st, p2pchnl);
        }else{
            /*TODO: 添加0-shared*/
            uint32_t delta1[2],delta2[2];
            if(st == "player0"||st == "player1")
                delta1[0] = sub(idex, read_triples[read_times].second, data_len);
            else delta1[0] = idex;
            if(st == "player2"||st == "player3")
                delta1[1] = sub(idex, read_triples[read_times].second, data_len);
            else delta1[1] = idex;
            fourpc_reveal<uint32_t>(delta1, delta2, 2, st, p2pchnl);
            if(st == "player0"||st == "player1") delta_r = delta2[0];
            else delta_r = delta2[1];
        }
       while(delta_r > (1<<31)) delta_r+=data_len;
        delta_r = delta_r % data_len;
        for(uint32_t i = 0; i < data_len; i++){
            mpz_class ans = evaluateEq(&faccess, &read_triples[read_times].first, sub(i, delta_r, data_len));
            list[i] = mpz_get_ui(ans.get_mpz_t());
        }
        free_key<ServerKeyEq>(read_triples[read_times].first);
        read_times ++;
    }
    T read(uint32_t idex, bool is_rep = true, 
            T (*mul_cb)(T a, uint32_t b) = [](T a, uint32_t b) -> T {return a * b;},
            T (*add_cb)(T a, T b) = [](T a, T b) -> T {return a + b;}
            ){
        /*对于ins 直接读block 会出错， 需要分字段读取, 使用回调函数进行分字段处理*/
        /*idex i -> i0 + i1 = i2 + i3*/
        if(st == "aid"){
            T a;
            return a;
        }
        if(read_times >= read_triples.size()){
            /*异常处理*/
            //return;
        }
        uint32_t delta_r;
        if(is_rep){
            uint32_t delta = sub(idex, read_triples[read_times].second, data_len);
            twopc_reveal<uint32_t>(&delta, &delta_r, 1, st, p2pchnl);
        }else{
            /*TODO: 添加0-shared*/
            uint32_t delta1[2],delta2[2];
            if(st == "player0"||st == "player1")
                delta1[0] = sub(idex, read_triples[read_times].second, data_len);
            else delta1[0] = idex;
            if(st == "player2"||st == "player3")
                delta1[1] = sub(idex, read_triples[read_times].second, data_len);
            else delta1[1] = idex;
            fourpc_reveal<uint32_t>(delta1, delta2, 2, st, p2pchnl);
            if(st == "player0"||st == "player1") delta_r = delta2[0];
            else delta_r = delta2[1];
        }
        /*这个while我也不知道为啥要这样写:)*/
        while(delta_r > (1<<31)) delta_r+=data_len;
        delta_r = delta_r % data_len;
        T res;
        
        for(uint32_t i = 0; i < data_len; i++){
            mpz_class ans = evaluateEq(&faccess, &read_triples[read_times].first, sub(i, delta_r, data_len));
            uint32_t tmp = mpz_get_ui(ans.get_mpz_t());
            if(i == 0) res = mul_cb(is_contain?data[i]:data_ptr[i], tmp);
            else res = add_cb(res, mul_cb(is_contain?data[i]:data_ptr[i], tmp));
        }
        free_key<ServerKeyEq>(read_triples[read_times].first);
        read_times ++;
        return res;
    }
    void read_1(uint32_t idex, bool is_rep = true, 
            T (*mul_cb)(T a, uint32_t b) = [](T a, uint32_t b) -> T {return a * b;},
            T (*add_cb)(T a, T b) = [](T a, T b) -> T {return a + b;}
            ){
        /*对于ins 直接读block 会出错， 需要分字段读取, 使用回调函数进行分字段处理*/
        /*idex i -> i0 + i1 = i2 + i3*/
        if(st == "aid"){
            return;
        }
        if(read_times >= read_triples.size()){
            /*异常处理*/
            //return;
        }
        if(is_rep){
            uint32_t delta = sub(idex, read_triples[read_times].second, data_len);
            twopc_reveal_1<uint32_t>(&delta, &delta, 1, st, p2pchnl);
        }else{
            /*TODO: 添加0-shared*/
            uint32_t delta1[2];
            if(st == "player0"||st == "player1")
                delta1[0] = sub(idex, read_triples[read_times].second, data_len);
            else delta1[0] = idex;
            if(st == "player2"||st == "player3")
                delta1[1] = sub(idex, read_triples[read_times].second, data_len);
            else delta1[1] = idex;
            fourpc_reveal_1<uint32_t>(delta1, delta1, 2,{"player0","player1","player2","player3"}, st, p2pchnl);
            
        }
    }
    T read_2(uint32_t idex, bool is_rep = true, 
            T (*mul_cb)(T a, uint32_t b) = [](T a, uint32_t b) -> T {return a * b;},
            T (*add_cb)(T a, T b) = [](T a, T b) -> T {return a + b;}
            ){
        /*对于ins 直接读block 会出错， 需要分字段读取, 使用回调函数进行分字段处理*/
        /*idex i -> i0 + i1 = i2 + i3*/
        if(st == "aid"){
            T a;
            return a;
        }
        if(read_times >= read_triples.size()){
            /*异常处理*/
            //return;
        }
        uint32_t delta_r;
        if(is_rep){
            uint32_t delta = sub(idex, read_triples[read_times].second, data_len);
            twopc_reveal_2<uint32_t>(&delta, &delta_r, 1, st, p2pchnl);
        }else{
            /*TODO: 添加0-shared*/
            uint32_t delta1[2],delta2[2];
            if(st == "player0"||st == "player1")
                delta1[0] = sub(idex, read_triples[read_times].second, data_len);
            else delta1[0] = idex;
            if(st == "player2"||st == "player3")
                delta1[1] = sub(idex, read_triples[read_times].second, data_len);
            else delta1[1] = idex;
            fourpc_reveal_2<uint32_t>(delta1, delta2, 2,{"player0","player1","player2","player3"}, st, p2pchnl);
            if(st == "player0"||st == "player1") delta_r = delta2[0];
            else delta_r = delta2[1];
        }
        
        while(delta_r >  (1<<31)) delta_r+=data_len;
        delta_r = delta_r % data_len;
        T res;
        
        for(uint32_t i = 0; i < data_len; i++){
            mpz_class ans = evaluateEq(&faccess, &read_triples[read_times].first, sub(i, delta_r, data_len));
            uint32_t tmp = mpz_get_ui(ans.get_mpz_t());
            if(i == 0) res = mul_cb(is_contain?data[i]:data_ptr[i], tmp);
            else res = add_cb(res, mul_cb(is_contain?data[i]:data_ptr[i], tmp));
        }
        free_key<ServerKeyEq>(read_triples[read_times].first);
        read_times ++;
        return res;
    }
    void write(uint32_t idex, T target, T org, bool is_rep = true){
        /*target & org -> rep*/
        /*idex -> is_rep*/
        if(st == "aid")
            return;
        if(write_times >= write_triples.size()){
            /*异常处理*/
            //return;
        }
        /*calculate and open deltaV*/
        T deltaV_plus = target - org - get<4>(write_triples[write_times]);
        T deltaV;
        diag_reveal<T>(&deltaV_plus, &deltaV, 1, st, p2pchnl);
        uint32_t delta_r[2];
        if(is_rep){
            uint32_t delta[2] = {sub(idex, get<2>(write_triples[write_times]), data_len), sub(idex, get<3>(write_triples[write_times]), data_len)};
            diag_reveal<uint32_t>(delta, delta_r, 2, st, p2pchnl);
        }else{
            uint32_t delta[4],delta2[4];
            if(st == "player0"||st == "player2"){
                delta[0] = sub(idex, get<2>(write_triples[write_times]), data_len);
                delta[1] = sub(idex, get<3>(write_triples[write_times]), data_len);
                delta[2] = idex;
                delta[3] = idex;
            }
            else{
                delta[0] = idex;
                delta[1] = idex;
                delta[2] = sub(idex, get<2>(write_triples[write_times]), data_len);
                delta[3] = sub(idex, get<3>(write_triples[write_times]), data_len);
            }
            fourpc_reveal<uint32_t>(delta, delta2, 4, st, p2pchnl);
            if(st == "player0"||st == "player2"){
                delta_r[0] = delta2[0];delta_r[1] = delta2[1];
            }else{
                delta_r[0] = delta2[2];delta_r[1] = delta2[3];
            }
        }
        while(delta_r[0] > (1<<31)) delta_r[0]+=data_len;
        while(delta_r[1] > (1<<31)) delta_r[1]+=data_len;
        delta_r[0] = delta_r[0] % data_len;delta_r[1] = delta_r[1] % data_len;
        for(uint32_t i = 0; i < data_len; i++){
            mpz_class ans = evaluateEq(&faccess, &get<0>(write_triples[write_times]), sub(i, delta_r[0], data_len));
            uint32_t tmp = mpz_get_ui(ans.get_mpz_t());
            /*------*--*-不合法的域转换，TODO使用truncation协议-*--*--*------*/
            if(st == "player0" || st == "player1") data_ptr[i] = data_ptr[i] + tmp;
            else data_ptr[i] = data_ptr[i] - tmp;
            ans = evaluateEq(&faccess, &get<1>(write_triples[write_times]), sub(i, delta_r[1], data_len));
            tmp = mpz_get_ui(ans.get_mpz_t());
            if(st == "player0" || st == "player1") data_ptr[i] = data_ptr[i] + deltaV*tmp;
            else data_ptr[i] = data_ptr[i] - deltaV*tmp;
        }
        free_key<ServerKeyEq>(get<1>(write_triples[write_times]));
        free_key<ServerKeyEq>(get<0>(write_triples[write_times]));
        write_times ++;
    }
    void write_1(uint32_t idex, T target, T org, bool is_rep = true, bool add_target = false){
        if(st == "aid")
            return;
        if(write_times >= write_triples.size()){
            /*异常处理*/
            //return;
        }
        /*calculate and open deltaV*/
        T deltaV_plus = target - org - get<4>(write_triples[write_times]);
        diag_reveal_1<T>(&deltaV_plus, &deltaV_plus, 1, st, p2pchnl);
        if(add_target){
            twopc_reveal_1<T>(&target, &target, 1, st, p2pchnl);
            latt_reveal_1<T>(&target, &target, 1, st, p2pchnl);
        }
        if(is_rep){
            uint32_t delta[2] = {sub(idex, get<2>(write_triples[write_times]), data_len), sub(idex, get<3>(write_triples[write_times]), data_len)};
            diag_reveal_1<uint32_t>(delta, delta, 2, st, p2pchnl);
            
        }else{
            uint32_t delta[4];
            if(st == "player0"||st == "player2"){
                delta[0] = sub(idex, get<2>(write_triples[write_times]), data_len);
                delta[1] = sub(idex, get<3>(write_triples[write_times]), data_len);
                delta[2] = idex;
                delta[3] = idex;
            }
            else{
                delta[0] = idex;
                delta[1] = idex;
                delta[2] = sub(idex, get<2>(write_triples[write_times]), data_len);
                delta[3] = sub(idex, get<3>(write_triples[write_times]), data_len);
            }
            fourpc_reveal_1<uint32_t>(delta, delta, 4, {"player0","player1","player2","player3"}, st, p2pchnl);
        }
        
    }
    void write_2(uint32_t idex, T target, T org, bool is_rep = true, bool add_target = false){
        if(st == "aid")
            return;
        if(write_times >= write_triples.size()){
            /*异常处理*/
            //return;
        }
        /*calculate and open deltaV*/
        T deltaV_plus = target - org - get<4>(write_triples[write_times]);
        T deltaV;
        diag_reveal_2<T>(&deltaV_plus, &deltaV, 1, st, p2pchnl);
        if(add_target){
            T new_deltaV;
            twopc_reveal_2<T>(&deltaV, &new_deltaV, 1, st, p2pchnl);
            deltaV = new_deltaV;
            latt_reveal_2<T>(&deltaV, &new_deltaV, 1, st, p2pchnl);
            deltaV = new_deltaV;
        }
        uint32_t delta_r[2];
        if(is_rep){
            uint32_t delta[2] = {sub(idex, get<2>(write_triples[write_times]), data_len), sub(idex, get<3>(write_triples[write_times]), data_len)};
            diag_reveal_2<uint32_t>(delta, delta_r, 2, st, p2pchnl);
        }else{
            uint32_t delta[4],delta2[4];
            if(st == "player0"||st == "player2"){
                delta[0] = sub(idex, get<2>(write_triples[write_times]), data_len);
                delta[1] = sub(idex, get<3>(write_triples[write_times]), data_len);
                delta[2] = idex;
                delta[3] = idex;
            }
            else{
                delta[0] = idex;
                delta[1] = idex;
                delta[2] = sub(idex, get<2>(write_triples[write_times]), data_len);
                delta[3] = sub(idex, get<3>(write_triples[write_times]), data_len);
            }
            fourpc_reveal_2<uint32_t>(delta, delta2, 4, {"player0","player1","player2","player3"}, st, p2pchnl);
            if(st == "player0"||st == "player2"){
                delta_r[0] = delta2[0];delta_r[1] = delta2[1];
            }else{
                delta_r[0] = delta2[2];delta_r[1] = delta2[3];
            }
        }
        while(delta_r[0] > (1<<31)) delta_r[0]+=data_len;
        while(delta_r[1] > (1<<31)) delta_r[1]+=data_len;
        delta_r[0] = delta_r[0] % data_len;delta_r[1] = delta_r[1] % data_len;
        for(uint32_t i = 0; i < data_len; i++){
            mpz_class ans = evaluateEq(&faccess, &get<0>(write_triples[write_times]), sub(i, delta_r[0], data_len));
            uint32_t tmp = mpz_get_ui(ans.get_mpz_t());
            if(st == "player0" || st == "player1") data_ptr[i] = data_ptr[i] + tmp;
            else data_ptr[i] = data_ptr[i] - tmp;
            ans = evaluateEq(&faccess, &get<1>(write_triples[write_times]), sub(i, delta_r[1], data_len));
            tmp = mpz_get_ui(ans.get_mpz_t());
            if(st == "player0" || st == "player1") data_ptr[i] = data_ptr[i] + deltaV*tmp;
            else data_ptr[i] = data_ptr[i] - deltaV*tmp;
        }
        free_key<ServerKeyEq>(get<1>(write_triples[write_times]));
        free_key<ServerKeyEq>(get<0>(write_triples[write_times]));
        write_times ++;
    }
    ~Ram(){
        if(init_flag)
            free_fss_key(faccess);
    }

};


#endif
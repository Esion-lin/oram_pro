#ifndef _FSS_HELP_H__
#define _FSS_HELP_H__
#include "net.hpp"
#include <string>
#include "fss-client.h"
#include "fss-common.h"
#include <set>
inline void send_all(void *data, int len, P2Pchannel *p2pchnl = P2Pchannel::mychnl){
    for( auto & name: Config::myconfig->Pmap){
        if( !Config::myconfig->check(name.first)) 
            p2pchnl->send_data_to(name.first,data,len);
    }
}
inline void send_all(std::set<std::string> roles, void *data, int len, P2Pchannel *p2pchnl = P2Pchannel::mychnl){
    for(auto &rl:roles)
        p2pchnl->send_data_to(rl,data,len);
}
inline void send_mpz(mpz_class data, std::string player, P2Pchannel *p2pchnl = P2Pchannel::mychnl){
    size_t b=0;
    char* datt = (char*) malloc(40);
    mpz_export(datt, &b, -1, sizeof(char), 0, 0, data.get_mpz_t());
    if (b >= 40){
        LOG(ERROR) << "size out of array! ";
        free(datt);
        return ;
    }
    p2pchnl->send_data_to(player, &b,sizeof(size_t));
    p2pchnl->send_data_to(player, datt, b*sizeof(char));
    free(datt);
}
inline mpz_class recv_mpz(P2Pchannel *p2pchnl = P2Pchannel::mychnl, std::string fplayer = "aid"){
    size_t b=0;
    char* datt = (char*) malloc(40);
    mpz_class z;
    
    p2pchnl->recv_data_from(fplayer, &b, sizeof(size_t));
    p2pchnl->recv_data_from(fplayer, datt, b*sizeof(char));
    mpz_import(z.get_mpz_t(), b, -1, sizeof(char), 0, 0, datt);
    free(datt);
    return z;
}
inline void send_mpz(mpz_class data, P2Pchannel *p2pchnl = P2Pchannel::mychnl){
    size_t b=0;
    char* datt = (char*) malloc(40);
    mpz_export(datt, &b, -1, sizeof(char), 0, 0, data.get_mpz_t());
    if (b >= 40){
        LOG(ERROR) << "size out of array! ";
        free(datt);
        return ;
    }
    send_all(&b, sizeof(size_t), p2pchnl);
    send_all(datt, b*sizeof(char), p2pchnl);
    free(datt);
}
inline void send_role_mpz(mpz_class data, P2Pchannel *p2pchnl = P2Pchannel::mychnl, std::set<std::string> roles = {"player0", "player1", "player2", "player3"}){
    size_t b=0;
    char* datt = (char*) malloc(40);
    mpz_export(datt, &b, -1, sizeof(char), 0, 0, data.get_mpz_t());
    if (b >= 40){
        LOG(ERROR) << "size out of array! ";
        free(datt);
        return ;
    }
    send_all(roles, &b, sizeof(size_t), p2pchnl);
    send_all(roles, datt, b*sizeof(char), p2pchnl);
    free(datt);
}
inline void send_fss_key(Fss &key, P2Pchannel *p2pchnl = P2Pchannel::mychnl){
    send_mpz(key.prime, p2pchnl);
    send_all(&key,sizeof(Fss)-16, p2pchnl);
    send_all(key.aes_keys,sizeof(AES_KEY_FSS)*key.numKeys, p2pchnl);
}
inline void recv_fss_key(Fss& key, P2Pchannel *p2pchnl = P2Pchannel::mychnl, std::string st = "aid"){
    mpz_class z = recv_mpz(p2pchnl, st);
    p2pchnl->recv_data_from(st,&key,sizeof(Fss)-16);
    key.prime = z;
    key.aes_keys = (AES_KEY_FSS*) malloc(sizeof(AES_KEY_FSS)*key.numKeys);
    p2pchnl->recv_data_from(st,key.aes_keys,sizeof(AES_KEY_FSS)*key.numKeys);
}
inline void send_eq_key(ServerKeyEq& key, Fss &fkey, std::string player, P2Pchannel *p2pchnl = P2Pchannel::mychnl){
    send_mpz(key.w,player, p2pchnl);
    p2pchnl->send_data_to(player,&key,sizeof(ServerKeyEq)-16);
    p2pchnl->send_data_to(player,key.cw[0],(fkey.numBits - 1)*sizeof(CWEq));
    p2pchnl->send_data_to(player,key.cw[1],(fkey.numBits - 1)*sizeof(CWEq));
}
inline void recv_eq_key(ServerKeyEq& key, Fss &fkey, std::string player, P2Pchannel *p2pchnl = P2Pchannel::mychnl){
    mpz_class tmp = recv_mpz(p2pchnl, player);
    p2pchnl->recv_data_from(player,&key,sizeof(ServerKeyEq)-16);
    key.w = tmp;
    key.cw[0] = (CWEq*) malloc(sizeof(CWEq)*(fkey.numBits - 1));
    key.cw[1] = (CWEq*) malloc(sizeof(CWEq)*(fkey.numBits - 1));
    p2pchnl->recv_data_from(player,key.cw[0],sizeof(CWEq)*(fkey.numBits - 1));
    p2pchnl->recv_data_from(player,key.cw[1],sizeof(CWEq)*(fkey.numBits - 1));
}

inline void send_lt_key(ServerKeyLt& key, Fss &fkey, std::string player, P2Pchannel *p2pchnl = P2Pchannel::mychnl){
    p2pchnl->send_data_to(player,&key,sizeof(ServerKeyLt));
    p2pchnl->send_data_to(player,key.cw[0],(fkey.numBits - 1)*sizeof(CWLt));
    p2pchnl->send_data_to(player,key.cw[1],(fkey.numBits - 1)*sizeof(CWLt));
}
inline void recv_lt_key(ServerKeyLt& key, Fss &fkey, std::string player, P2Pchannel *p2pchnl = P2Pchannel::mychnl){
    p2pchnl->recv_data_from(player,&key,sizeof(ServerKeyLt));
    key.cw[0] = (CWLt*) malloc(sizeof(CWLt)*(fkey.numBits - 1));
    key.cw[1] = (CWLt*) malloc(sizeof(CWLt)*(fkey.numBits - 1));
    p2pchnl->recv_data_from(player,key.cw[0],sizeof(CWLt)*(fkey.numBits - 1));
    p2pchnl->recv_data_from(player,key.cw[1],sizeof(CWLt)*(fkey.numBits - 1));
}
inline void free_fss_key(Fss& key){
    free(key.aes_keys);
}
template <typename T>
void free_key(T& key){
    free(key.cw[0]);free(key.cw[1]);
}
template <typename T>
void free_keys(T* keys, uint16_t len){
    for(int i = 0; i < len; i++){
        free(keys[i].cw[0]);free(keys[i].cw[1]);
    }
    
}
#endif
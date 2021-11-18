#ifndef _FSS_HELP_H__
#define _FSS_HELP_H__
#include "net.h"
#include <string>
#include "fss-client.h"
#include "fss-common.h"
void send_all(void *data, int len, P2Pchannel *p2pchnl){
    p2pchnl->send_data_to("player0",data,len);
    p2pchnl->send_data_to("player1",data,len);
    p2pchnl->send_data_to("player2",data,len);
    p2pchnl->send_data_to("player3",data,len);
}
void send_mpz(mpz_class data, std::string player, P2Pchannel *p2pchnl){
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
mpz_class recv_mpz(P2Pchannel *p2pchnl){
    size_t b=0;
    char* datt = (char*) malloc(40);
    mpz_class z;
    p2pchnl->recv_data_from("aid", &b, sizeof(size_t));
    p2pchnl->recv_data_from("aid", datt, b*sizeof(char));
    mpz_import(z.get_mpz_t(), b, -1, sizeof(char), 0, 0, datt);
    free(datt);
    return z;
}
void send_mpz(mpz_class data, P2Pchannel *p2pchnl){
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
void send_fss_key(Fss &key, P2Pchannel *p2pchnl){
    send_mpz(key.prime, p2pchnl);
    send_all(&key,sizeof(Fss)-16, p2pchnl);
    send_all(key.aes_keys,sizeof(AES_KEY)*key.numKeys, p2pchnl);
}
void recv_fss_key(Fss& key, P2Pchannel *p2pchnl){
    mpz_class z = recv_mpz(p2pchnl);
    p2pchnl->recv_data_from("aid",&key,sizeof(Fss)-16);
    key.prime = z;
    key.aes_keys = (AES_KEY*) malloc(sizeof(AES_KEY)*key.numKeys);
    p2pchnl->recv_data_from("aid",key.aes_keys,sizeof(AES_KEY)*key.numKeys);
}
void send_eq_key(ServerKeyEq& key, Fss &fkey, std::string player, P2Pchannel *p2pchnl){
    send_mpz(key.w,player, p2pchnl);
    p2pchnl->send_data_to(player,&key,sizeof(ServerKeyEq)-16);
    p2pchnl->send_data_to(player,key.cw[0],(fkey.numBits - 1)*sizeof(CWEq));
    p2pchnl->send_data_to(player,key.cw[1],(fkey.numBits - 1)*sizeof(CWEq));
}
void recv_eq_key(ServerKeyEq& key, Fss &fkey, std::string player, P2Pchannel *p2pchnl){
    mpz_class tmp = recv_mpz(p2pchnl);
    p2pchnl->recv_data_from(player,&key,sizeof(ServerKeyEq)-16);
    key.w = tmp;
    key.cw[0] = (CWEq*) malloc(sizeof(CWEq)*(fkey.numBits - 1));
    key.cw[1] = (CWEq*) malloc(sizeof(CWEq)*(fkey.numBits - 1));
    p2pchnl->recv_data_from(player,key.cw[0],sizeof(CWEq)*(fkey.numBits - 1));
    p2pchnl->recv_data_from(player,key.cw[1],sizeof(CWEq)*(fkey.numBits - 1));
}

void send_lt_key(ServerKeyLt& key, Fss &fkey, std::string player, P2Pchannel *p2pchnl){
    p2pchnl->send_data_to(player,&key,sizeof(ServerKeyLt));
    p2pchnl->send_data_to(player,key.cw[0],(fkey.numBits - 1)*sizeof(CWLt));
    p2pchnl->send_data_to(player,key.cw[1],(fkey.numBits - 1)*sizeof(CWLt));
}
void recv_lt_key(ServerKeyLt& key, Fss &fkey, std::string player, P2Pchannel *p2pchnl){
    p2pchnl->recv_data_from(player,&key,sizeof(ServerKeyLt));
    key.cw[0] = (CWLt*) malloc(sizeof(CWLt)*(fkey.numBits - 1));
    key.cw[1] = (CWLt*) malloc(sizeof(CWLt)*(fkey.numBits - 1));
    p2pchnl->recv_data_from(player,key.cw[0],sizeof(CWLt)*(fkey.numBits - 1));
    p2pchnl->recv_data_from(player,key.cw[1],sizeof(CWLt)*(fkey.numBits - 1));
}
void free_fss_key(Fss& key){
    free(key.aes_keys);
}
template <typename T>
void free_key(T& key){
    free(key.cw[0]);free(key.cw[1]);
}
#endif
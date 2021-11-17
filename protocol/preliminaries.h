#ifndef _PRELIMINARIES_H__
#define _PRELIMINARIES_H__
#include <string>
#include "fss-client.h"
#include "fss-common.h"
using namespace std;
template <typename T, typename L>
class Ram{
    /*T type of element, L type of lens*/
    public:
    L data_len;
    T* data_ptr;
    /*load for data*/
    Ram(T* data_in, L lens):data_ptr(data_in), data_len(lens){}
    /*from other's*/
    Ram(L lens):data_len(lens){}
    /*from files*/
    Ram( std::string path){}



    T read(L idex){}
    void write(L idex, T target){}
    private:
    void share(std::string from_p){

    }
    void prepare_read(uint16_t ntimes){

    }
    void prepare_write(uint16_t ntimes){

    }
};


#endif
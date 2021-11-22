
#ifndef _ENCODE_H__
#define _ENCODE_H__
#include <math.h>
template <typename T>
class Encode{
    public:
    T threshold;
    Encode(){
        threshold = (T)pow(2, sizeof(T)*8 - 1);
    }
    int to_int(T value){
        return (int)value;
    }


};

#endif
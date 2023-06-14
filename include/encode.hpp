
#ifndef _ENCODE_H__
#define _ENCODE_H__
#include <math.h>
#include <stdio.h>
template <typename T>
class Encode{
    public:
    T threshold;
    Encode(){
        threshold = (T)(1 << (sizeof(T)*8 - 1));
    }
    int to_int(T value){
        return (int)value;
    }


};

class Bitmap{
    public:
    char* data;
    int lens;
    /*little endian*/
    Bitmap(bool* array, int lens):lens(lens){
        data = new char[lens/8 + 1];
        for(int i = 0; i < lens; i ++){

        }
    }
    Bitmap(char* array, int char_lens){
        data = array;
        lens = char_lens * 8;
    }
    bool get_idex(int idex){
        if(idex >= lens || idex < 0){
            printf("!!!!!!!!!!!!error index!!!!!!!!!!!!!\n");
            return false;
        }
        return ( data[idex / 8] >> (idex % 8) ) & 1;
    }

};
#endif
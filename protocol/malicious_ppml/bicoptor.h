#pragma once 

#include "connect.h"
#include "config.hpp"
#include <thread>
#include "timer.hpp"
template<class T>
class Bicoptor{
    private:
    int batch_len;

    public:
    Bicoptor(int batch):batch_len(batch){
    }
    void online(const std::vector<T>& x, std::vector<T>& output){
        if(!Config::myconfig->check("player0")){

            //trunc
            std::vector<T> temp(x.size() * sizeof(T) * 8); //divide 2 for l_x
            for(int i = 0; i < x.size(); i++){
                for( int j = 0; j < sizeof(T) * 8; j++)
                    temp[i*sizeof(T) * 8 + j] = x[i] >> (sizeof(T) * 8 - j - 1);
            }
            //open to $P_0$
            sendVector<T>(temp.data(), 0, temp.size());
            receiveVector<T>(output.data(), 0, x.size());
        }
        else{
            std::vector<T> temp1(x.size()* sizeof(T) * 8), temp2(x.size()* sizeof(T) * 8);
            thread *threads = new thread[2];

            threads[0] = thread(static_cast<void(*)(T*, size_t, size_t)>(receiveVector<T>), (T*)temp1.data(), 1, x.size()* sizeof(T) * 8);
            threads[1] = thread(static_cast<void(*)(T*, size_t, size_t)>(receiveVector<T>), (T*)temp2.data(), 2, x.size()* sizeof(T) * 8);

            for (int i = 0; i < 2; i++)
                threads[i].join();

            delete[] threads;
            for(int i = 0; i < x.size(); i++){
                for(int j = 0; j < sizeof(T) * 8; j++){
                    temp1[i * sizeof(T) * 8 + j] += temp2[i * sizeof(T) * 8 + j];
                    if(j != 0 && temp1[i * sizeof(T) * 8 + j - 1] == 0 &&temp1[i * sizeof(T) * 8 + j] == 1){
                        output[i] = 1;
                        break;
                    }
                }
                
            }

            thread *threads2 = new thread[2];
            threads2[0] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)output.data(), 1, x.size());
            threads2[1] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)output.data(), 2, x.size());
            for (int i = 0; i < 2; i++)
                threads2[i].join();

            delete[] threads2;
        }
        
    }
};
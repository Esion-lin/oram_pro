#pragma once 

#include "connect.h"
#include "config.hpp"
#include <thread>
#include "timer.hpp"
#include "unit.h"
#define ERROR_SIZE 64
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
            std::vector<T> temp(x.size() * sizeof(T) * 8 / 2); //divide 2 for l_x
            std::vector<T> v(x.size() * sizeof(T) * 8 / 2);
            T * R = (T*) malloc(x.size() * sizeof(T) * 8 * sizeof(T) / 2);
            random_T<T>(R, x.size() * sizeof(T) * 8 * sizeof(T) / 2);
            for(int i = 0; i < x.size(); i++){
                for( int j = 0; j < sizeof(T) * 8/2; j++){
                    temp[i*sizeof(T) * 4 + j] = x[i] * -1 >> (sizeof(T) * 4 - j - 1);
                    v[i*sizeof(T) * 4 + j] = temp[i*sizeof(T) * 4 + j] + 3 * temp[i*sizeof(T) * 4 + j] - 1;
                    v[i*sizeof(T) * 4 + j] *= R[i*sizeof(T) * 4 + j];
                }
                //permute
            }
            //open to $P_0$
            sendVector<T>(temp.data(), 0, temp.size());
            receiveVector<T>(output.data(), 0, x.size());
        }
        else{
            std::vector<T> temp1(x.size()* sizeof(T) * 4), temp2(x.size()* sizeof(T) * 4);
            thread *threads = new thread[2];

            threads[0] = thread(static_cast<void(*)(T*, size_t, size_t)>(receiveVector<T>), (T*)temp1.data(), 1, output.size()* sizeof(T) * 4);
            threads[1] = thread(static_cast<void(*)(T*, size_t, size_t)>(receiveVector<T>), (T*)temp2.data(), 2, output.size()* sizeof(T) * 4);

            for (int i = 0; i < 2; i++)
                threads[i].join();

            delete[] threads;
            for(int i = 0; i < x.size(); i++){
                for(int j = 0; j < sizeof(T) * 4; j++){
                    temp1[i * sizeof(T) * 4 + j] += temp2[i * sizeof(T) * 4 + j];
                    if(j != 0 && temp1[i * sizeof(T) * 4 + j - 1] == 0 &&temp1[i * sizeof(T) * 4 + j] == 1){
                        output[i] = 1;
                        break;
                    }
                }
                
            }

            thread *threads2 = new thread[2];
            threads2[0] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)output.data(), 1, output.size());
            threads2[1] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), (T*)output.data(), 2, output.size());
            for (int i = 0; i < 2; i++)
                threads2[i].join();

            delete[] threads2;
        }
        
    }
};
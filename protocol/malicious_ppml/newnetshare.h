#pragma once
#include "connect.h"
#include <thread>

template<class T>
bool ShareCt(const T* org, T* target, size_t lens, T p){
    
    if(Config::myconfig->check("player0")){
        //random_T<T>(target, lens);
        printf("%d\n", lens);
        #pragma omp parallel for
        for(int i = 0; i < lens; i++){
            // printf("%d\n", i);
            target[i] %= p;

        }
        T* temp = (T*)malloc(lens * sizeof(T));
        sub_T_mod<T>(temp, org, target, lens, p);
        printf("start send %d\n", lens* sizeof(T));
        sendVector<T>(temp, 1, lens);
        printf("send done\n");
        // P2Pchannel::mychnl->send_data_to("player1", temp, lens*sizeof(T));;
        free(temp);
    }else if(Config::myconfig->check("player1")){
        printf("start recejve \n");
        receiveVector<T>(target, 0, lens);
        printf(" recejved \n");
        // P2Pchannel::mychnl->recv_data_from("player0", target, lens*sizeof(T));
    }else{
        random_T<T>(target, lens);
        #pragma omp parallel for
        for(int i = 0; i < lens; i++){
            target[i] %= p;
        }
    }
    return true;
}

template<class T>
bool RevealCt(T* org, size_t lens, T p){

    


    if(Config::myconfig->check("player2") || Config::myconfig->check("player1")){
        
        sendVector<T>(org, 0, lens);
        // P2Pchannel::mychnl->send_data_to("player0", org, lens*sizeof(T));
        
    }else if(Config::myconfig->check("player0")){
        T* temp = (T*) malloc(lens * sizeof(T));
        // receiveVector<T>(temp, 1, lens);
        // receiveVector<T>(org, 2, lens);
        thread *threads = new thread[2];

		threads[0] = thread(static_cast<void(*)(T*, size_t, size_t)>(receiveVector<T>), temp, 1, lens);
		threads[1] = thread(static_cast<void(*)(T*, size_t, size_t)>(receiveVector<T>), org, 2, lens);

		for (int i = 0; i < 2; i++)
			threads[i].join();

		delete[] threads;

        // receiveVector(temp, 1, lens);
        // receiveVector(org, 2, lens);
        
        
        add_T_mod(org, org, temp, lens, p);
        free(temp);
    }
    return true;
}

template<class T>
bool RevealBt(T* org, size_t lens){
    T* temp = (T*) malloc(lens*sizeof(T));
    if(Config::myconfig->get_idex() == 1){
        // sendVector<T>(org, 2, lens);
        //  receiveVector<T>(temp, 2, lens);
        thread *threads = new thread[2];

		threads[0] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), org, 2, lens);
        threads[1] = thread(static_cast<void(*)(T*, size_t, size_t)>(receiveVector<T>), temp, 2, lens);


		for (int i = 0; i < 2; i++)
			threads[i].join();

		delete[] threads;

        add_T<T>(org, org, temp, lens);
    }else if(Config::myconfig->get_idex() == 2){
        // sendVector<T>(org, 1, lens);
        // receiveVector<T>(temp, 1, lens);
        thread *threads = new thread[2];

		threads[0] = thread(static_cast<void(*)(T*, size_t, size_t)>(sendVector<T>), org, 1, lens);
threads[1] = thread(static_cast<void(*)(T*, size_t, size_t)>(receiveVector<T>), temp, 1, lens);


		for (int i = 0; i < 2; i++)
			threads[i].join();

		delete[] threads;

        add_T<T>(org, org, temp, lens);
    }
    free(temp);
    return true;
}
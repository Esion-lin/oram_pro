#ifndef _TIMERS_H__
#define _TIMERS_H__
#include <sys/time.h>
#include <string.h>
#include <iostream>
#include <fstream>  
#include <map>
class Timer{
    public:
    Timer(){}
    static std::map<std::string, double> times;
    static std::map<std::string, struct timeval> ptrs;
    static std::string now_name;
    static void record(std::string name){
        struct timeval t1;
        gettimeofday(&t1,NULL);
        ptrs[name] = t1;
        now_name = name;
    }
    static void stop(std::string name){
        struct timeval t1;
        gettimeofday(&t1,NULL);
        if(times.find(name) == times.end()){
            times[name] = (t1.tv_sec - ptrs[name].tv_sec) + (double)(t1.tv_usec - ptrs[name].tv_usec)/1000000.0;
        }else{
            times[name] = times[name] + (t1.tv_sec - ptrs[name].tv_sec) + (double)(t1.tv_usec - ptrs[name].tv_usec)/1000000.0;
        }
    }
    static void get_time(){
        struct timeval t1;
        gettimeofday(&t1,NULL);
        std::cout<<"------------"<<t1.tv_usec<<"-----------------"<<std::endl;
    }
    static void tag(std::string name){
        struct timeval t1;
        gettimeofday(&t1,NULL);
        std::string tmp = now_name + ":" + name;
        ptrs[tmp] = t1;
    }
    static void untag(std::string name){
        struct timeval t1;
        gettimeofday(&t1,NULL);
        std::string tmp = now_name + ":" + name;
        if(times.find(tmp) == times.end()){
            times[tmp] = (t1.tv_sec - ptrs[tmp].tv_sec) + (double)(t1.tv_usec - ptrs[tmp].tv_usec)/1000000.0;
        }else{
            times[tmp] = times[tmp] + (t1.tv_sec - ptrs[tmp].tv_sec) + (double)(t1.tv_usec - ptrs[tmp].tv_usec)/1000000.0;
        }
    }
    static void test_print(std::string path = "data.csv"){
        //ofstream outFile;  
        //outFile.open(path, ios::app);
        for(auto ele:times){
            
            printf("%s: %lf ms\n", ele.first.c_str(), ele.second * 1000);
            //if(ele.first == "mpc_write_total" || ele.first == "mpc_write_online"){outFile << ele.second * 1000 << ',';}
            // if(ele.first == "init") outFile << ele.second << ',';

            // else outFile << ele.second * 1000 << ',';
        }
        //outFile<<std::endl;
    }

};


#endif
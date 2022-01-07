#ifndef _FACTORY_H__
#define _FACTORY_H__
#include <set>
#include <stdint.h>
#include <iostream>
#include <string.h>
template <typename T>
void get_rand(std::set<std::string> roles, T* data, uint32_t lens){
    /*TODO: 对roles中的角色使用共同持有的seed生成随机数*/
    memset(data, 2, lens*sizeof(T));
}
#endif
#ifndef _DT_HELPER_
#define _DT_HELPER_
#include <fstream>
#include <vector>
#include <iostream>
#include <map>
#include <queue>
#include <algorithm>
struct node{
    uint32_t weight,idx;
    uint32_t left;
    uint32_t right;
    node(uint32_t w, uint32_t idx, uint32_t l, uint32_t r):
        weight(w), idx(idx), left(l), right(r){}
};
void free_tree(std::map<int, node*> tmp){
    for(int i = 0; i < tmp.size(); i++)
        delete tmp[i];
}
void tokenize(const std::string& str, std::vector<std::string>& tokens) {
    tokens.clear();
    std::size_t prev = 0, pos;
    while ((pos = str.find_first_of(" [] \\ ", prev)) != std::string::npos)
    {
        if (pos > prev)
            tokens.push_back(str.substr(prev, pos-prev));
        prev = pos+1;
    }
    if (prev < str.length())
        tokens.push_back(str.substr(prev, std::string::npos));
}
std::map<int, node*> read_from_file(std::string string_file){
    std::map<int,node*> nodes;
    const char* filename = string_file.c_str();
    std::ifstream file;
    file.open(filename);
    

    std::string line;
    std::vector<std::string> tokens;
    while (getline(file, line)){
        
        tokenize(line, tokens);
        if(tokens.size() <= 1) continue;
        if(tokens[1] == "label=\"gini"){
            nodes[atoi(tokens[0].c_str())] = new node(atoi(tokens[6].c_str())* 1000, 0, 0, 0);
            //leaf
        }
        else if(tokens[1] == "label=\"X"){
            //node
            nodes[atoi(tokens[0].c_str())] = new node(atof(tokens[4].c_str())*1000, atoi(tokens[2].c_str()), 0, 0);
        }
        else if(tokens[1] == "->"){
            if(nodes[atoi(tokens[0].c_str())]->left == 0){
                nodes[atoi(tokens[0].c_str())]->left = atoi(tokens[2].c_str());
            }
            else{
                nodes[atoi(tokens[0].c_str())]->right = atoi(tokens[2].c_str());
            }
            //edge
        }
    }
    file.close();
    return nodes;
}


#endif
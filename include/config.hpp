#ifndef _CONFIG_H__
#define _CONFIG_H__
#include <map>
#include <fstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;
struct Player{
    std::string address;
    int port;
};
inline json load_json(std::string path){
    json res;
    std::ifstream in(path);
    in>>res;
    in.close();
    return res;
}
class Config{
    public:
    std::map<std::string, Player> Pmap;
    Player aid;
    static Config * myconfig;
    Config(std::string path){
        config_json = load_json(path);
        json sub_conf = config_json["config"];
        current_player = config_json["myplayer"].get<std::string>();
        for (auto& el : sub_conf.items()) {
            Pmap[el.key()] = {el.value()["host"].get<std::string>(), el.value()["port"].get<int>()};
        }
        //aid = {config_json["myplayer"]["host"].get<std::string>(), config_json["myplayer"]["port"].get<int>()};
    }
    void set_player(std::string st){
        current_player = st;
    }
    std::string get_player(){
        return current_player;
    }
    bool check(std::string st){
        return st == current_player;
    }
    private:
    json config_json;
    std::string current_player;
    

};


#endif



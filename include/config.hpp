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
    std::string get_suc(){
        /*for 3-parties*/
        if(current_player == "player0") return "player1";
        if(current_player == "player1") return "player2";
        if(current_player == "player2") return "player0";
    }
    std::string get_suc(std::string st){
        /*for 3-parties*/
        if(st == "player0") return "player1";
        if(st == "player1") return "player2";
        if(st == "player2") return "player0";
    }
    std::string get_pre(){
        /*for 3-parties*/
        if(current_player == "player1") return "player0";
        if(current_player == "player2") return "player1";
        if(current_player == "player0") return "player2";
    }
    std::string get_pre(std::string st){
        /*for 3-parties*/
        if(st == "player1") return "player0";
        if(st == "player2") return "player1";
        if(st == "player0") return "player2";
    }
    uint32_t get_idex(){
        return current_player[current_player.length() - 1] - '0';
    }
    uint32_t get_suc_idex(){
        return (get_idex() + 1) % 3;
    }
    uint32_t get_pre_idex(){
        return (get_idex() + 2) % 3;
    }
    bool check(std::string st){
        return st == current_player;
    }

    private:
    json config_json;
    std::string current_player;
    

};


#endif



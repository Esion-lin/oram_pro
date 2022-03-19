#include "argparse/argparse.hpp"
#include "outsource_dt.h"
#include "easylogging++.h"
#include "timer.hpp"
#include "io.hpp"
using namespace argparse;
INITIALIZE_EASYLOGGINGPP
#define max_ele_size 100
uint16_t data_len = 100;
uint32_t deep = 0, node_size;
bool is_cons = false;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
int main(int argc, const char** argv) {
    /*
    args check
    */
    ArgumentParser parser("ORAM_TEST", "Argument parser example");

    parser.add_argument()
        .names({"-p", "--role"})
        .description("party's role")
        .required(true);
    parser.add_argument()
        .names({"-d", "--dataset"})
        .description("dataset of test")
        .required(false);
    parser.add_argument()
        .names({"-e", "--data_lens"})
        .description("data elements lens")
        .required(false);
    parser.add_argument()
        .names({"-c", "--is_cons"})
        .description("use constant dt")
        .required(true);
    parser.add_argument()
        .names({"-n", "--size_of_nodes"})
        .description("nodes size")
        .required(false);
    parser.add_argument()
        .names({"-a", "--deeps"})
        .description("nodes deep")
        .required(false);
    parser.enable_help();
    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return -1;
    }
    if (parser.exists("help")) {
        parser.print_help();
        return 0;
    }
    if (parser.exists("data_lens")){
        data_len = parser.get<int>("data_lens");
    }
    if (parser.exists("size_of_nodes")){
        node_size = parser.get<int>("size_of_nodes");
    }
    if (parser.exists("deeps")){
        deep = parser.get<int>("deeps");
    }
    if (parser.exists("is_cons")){
        is_cons = parser.get<bool>("is_cons");
    }
    std::string st = parser.get<std::string>("role");
    std::string data_set = "";
    if(parser.exists("dataset"))
        data_set = parser.get<std::string>("dataset");
    // std::cout<<"ele lens "<<ele_lens;
    /*
    start oram
    */
    Config::myconfig = new Config("./3_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, st);
    Config::myconfig->set_player(st);

    uint64_t data[data_len], rep[data_len];
    for(int i = 0; i < data_len ;i ++){
        data[i] = i + 2;
    }
    replicated_share<uint64_t>("player2", {"player0", "player1", "player2"}, data, rep, data_len);
    uint64_t res;
    std::vector<std::string> total_set = {"iris", "wine", "linnerud", "breast", "digits", "diabetes", "boston"};
    uint32_t data_lesns[7] = {4, 7, 3, 12, 47, 10, 13};
    if(data_set != ""){
        if(data_set == "all" && !is_cons){
            
            for(auto & str:total_set){
                std::cout<<"dataset "<<str<<std::endl;
                Dt<uint64_t, Node_el<uint64_t>>*dt = new Dt<uint64_t, Node_el<uint64_t>>("../dt/" + str, data_len, "player2", "player0", "player1");
                PolyDtEval<uint64_t> *dt_evl = new PolyDtEval<uint64_t>(dt, rep, data, data_len);
                dt_evl->offline();
                Timer::record(str);
                res = dt_evl->online();
                Timer::stop(str);
                delete dt;
                delete dt_evl;
            }
        }else if(data_set == "all" && is_cons){
            
            uint32_t nodes[7] = {8, 11, 19, 21, 168, 58, 525};
            uint32_t deeps[7] = {5, 5, 6, 7, 14, 17, 30};
            for(int i = 0; i < 5; i++){
                std::cout<<"dataset "<<total_set[i]<<std::endl;
                Dt<uint64_t, Node<uint64_t>> *dt = new Dt<uint64_t, Node<uint64_t>>(nodes[i], deeps[i], data_lesns[i], is_cons, "player2", "player0", "player1");
                ConstDtEval<uint64_t, 64> *dt_evl = new ConstDtEval<uint64_t, 64>(dt, rep, data, data_lesns[i]);
                std::cout<<"offline start\n";
                dt_evl->offline();
                std::cout<<"offline done\n";
                Timer::record(total_set[i]);
                res = dt_evl->online();
                Timer::stop(total_set[i]);
                delete dt;
                delete dt_evl;
            }
            

        }
        else{
            std::cout<<"dataset "<<data_set<<std::endl;
            Dt<uint64_t, Node_el<uint64_t>>*dt = new Dt<uint64_t, Node_el<uint64_t>>("../dt/" + data_set, data_len, "player2", "player0", "player1");
            PolyDtEval<uint64_t> *dt_evl = new PolyDtEval<uint64_t>(dt, rep, data, data_len);
            dt_evl->offline();
            Timer::record("poly evl");
            res = dt_evl->online();
        
            Timer::stop("poly evl");
            delete dt;
            delete dt_evl;
        }
    }else{
        if(is_cons){
            std::cout<<"eval "<<deep<<" const dt"<<std::endl;
            Dt<uint64_t, Node<uint64_t>> *dt = new Dt<uint64_t, Node<uint64_t>>(node_size, deep, data_len, is_cons, "player2", "player0", "player1");
            ConstDtEval<uint64_t> *dt_evl = new ConstDtEval<uint64_t>(dt, rep, data, data_len);
            
            dt_evl->offline();
            Timer::record("const evl");
            res = dt_evl->online();
            Timer::stop("const evl");
            delete dt;
            delete dt_evl;
        }else{
            std::cout<<"eval "<<deep<<" poly dt"<<std::endl;
            Dt<uint64_t, Node_el<uint64_t>>*dt = new Dt<uint64_t, Node_el<uint64_t>>(node_size, deep, data_len, "player2", "player0", "player1");   
            PolyDtEval<uint64_t> *dt_evl = new PolyDtEval<uint64_t>(dt, rep, data, data_len);
            Timer::record("poly evl");
            res = dt_evl->online();
            Timer::stop("poly evl");
            delete dt;
            delete dt_evl;
        }
    }
    // add_reveal<uint32_t>({"player0", "player1", "player2"}, {"player0", "player1", "player2"}, &res, 1);
    // std::cout<<res<<std::endl;
    Timer::test_print();
    delete Config::myconfig;
    delete P2Pchannel::mychnl;

}
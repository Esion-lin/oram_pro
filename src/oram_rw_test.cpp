#include "argparse/argparse.hpp"
#include "easylogging++.h"
#include "preliminaries.hpp"
#include "timer.hpp"
#include "convert.h"
#include "fss_mpc.h"
using namespace argparse;
INITIALIZE_EASYLOGGINGPP
#define max_ele_size 100
uint16_t ele_size = 1;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
struct block_t
{
    uint32_t ptr[max_ele_size];
    uint16_t lens = ele_size;
    block_t operator *(uint32_t other){
        block_t res;
        res.lens = lens;
        for(int i = 0; i < lens; i++ ){
            res.ptr[i] = ptr[i] * other;
        }
        return res;
    }
    block_t operator +(uint32_t other){
        block_t res;
        res.lens = lens;
        for(int i = 0; i < lens; i++ ){
            res.ptr[i] = ptr[i] + other;
        }
        return res;
    }
    block_t operator -(uint32_t other){
        block_t res;
        res.lens = lens;
        for(int i = 0; i < lens; i++ ){
            res.ptr[i] = ptr[i] - other;
        }
        return res;
    }
    block_t operator +(block_t other){
        block_t res;
        res.lens = lens;
        for(int i = 0; i < lens; i++ ){
            res.ptr[i] = ptr[i] + other.ptr[i];
        }
        return res;
    }
    block_t operator -(block_t other){
        block_t res;
        res.lens = lens;
        for(int i = 0; i < lens; i++ ){
            res.ptr[i] = ptr[i] - other.ptr[i];
        }
        return res;
    }
    block_t operator () ( int other){
        block_t res;
        return res;
    }
};

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
        .names({"-i", "--iteration"})
        .description("benchmark iteration times")
        .required(false);
    parser.add_argument()
        .names({"-e", "--elements_lens"})
        .description("oram elements lens")
        .required(false);
    parser.add_argument()
        .names({"-s", "--element_size"})
        .description("oram element size(x 32 bits)")
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
    int itr = 1;
    if (parser.exists("iteration")){
        itr = parser.get<int>("iteration");
    }
    if (parser.exists("element_size")){
        ele_size = parser.get<int>("element_size");
    }
    int ele_lens = 1000;
    if (parser.exists("elements_lens")){
        ele_lens = parser.get<int>("elements_lens");
    }
    std::string st = parser.get<std::string>("role");
    
    // std::cout<<"ele lens "<<ele_lens;
    /*
    start oram
    */
    Config::myconfig = new Config("./config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, st);
    Config::myconfig->set_player(st);
    if(ele_size == 1){
        
            Convert* conv = new Convert(st, P2Pchannel::mychnl);
            conv->fourpc_zeroshare<uint32_t>(itr);

            ele_lens/=128;
            uint32_t* ram_data = (uint32_t*)malloc(sizeof(uint32_t)*ele_lens);
            
            for(int i = 0; i < ele_lens; i++) ram_data[i] = i;
            Timer::record("total time");
            Timer::record("init");
            replicated_share<uint32_t>(ram_data, ele_lens, st, P2Pchannel::mychnl);
            Ram<uint32_t>* test_ram = new Ram<uint32_t>(ram_data, ele_lens, st, P2Pchannel::mychnl);
            
            Oram *test_ram2 = new Oram(ele_lens, ram_data);
            test_ram->init();
            P2Pchannel::mychnl->flush_all();
            Timer::stop("init");
            Timer::record("mpc_write_total");
            Timer::record("read_offline");
            test_ram->prepare_read(itr);
            P2Pchannel::mychnl->flush_all();
            Timer::stop("read_offline");
            Timer::stop("mpc_write_total");
            Timer::record("write_online");
            test_ram->prepare_write(itr);
            P2Pchannel::mychnl->flush_all();
            Timer::stop("write_online");

            while(itr -- ){
                uint32_t res;
                Timer::record("mpc_write_total");
                Timer::record("read_online");
                Timer::record("mpc_write_online");
                uint32_t tmp = test_ram->read(2, false);
                Timer::stop("mpc_write_online");
                Timer::stop("read_online");
                Timer::stop("mpc_write_total");
                if(st == "player1" || st == "player3") tmp = - tmp;
                fourpc_reveal<uint32_t>(&tmp, &res, 1, st, P2Pchannel::mychnl);
                //if(res > 1000)
                // printf("-----------%u-----------\n", res);
                conv->fourpc_share_2_replicated_share<uint32_t>(&tmp, 1);
                Timer::record("write_online");
                test_ram->write(1, 8, tmp, false);
                Timer::stop("write_online");
                Timer::record("mpc_write_total");
                test_ram2->prepare_write(1);
                Timer::record("mpc_write_online");
                test_ram2->write(1,8,2);
                Timer::stop("mpc_write_online");
                Timer::stop("mpc_write_total");
                P2Pchannel::mychnl->flush_all();
            }
            Timer::stop("total time");
            
            free(ram_data);
            delete test_ram;
            delete test_ram2;
            delete conv;
            
        
    }else{
            
            Convert* conv = new Convert(st, P2Pchannel::mychnl);
            conv->fourpc_zeroshare<block_t>(itr);


            block_t ram_data[ele_lens];
            Ram<block_t>* test_ram = new Ram<block_t>(ram_data, ele_lens, st, P2Pchannel::mychnl);
            test_ram->init();
            Timer::record("read_offline");
            test_ram->prepare_read(itr);
            P2Pchannel::mychnl->flush_all();
            Timer::stop("read_offline");
            Timer::record("write_offline");
            test_ram->prepare_write(itr);
            P2Pchannel::mychnl->flush_all();
            Timer::stop("write_offline");

            while(itr -- ){
                Timer::record("read_online");
                block_t tmp = test_ram->read(2, false);
                block_t target;
                Timer::stop("read_online");
                conv->fourpc_share_2_replicated_share<block_t>(&tmp, 1);
                
                Timer::record("write_online");
                test_ram->write(2, target, tmp, false);
                Timer::stop("write_online");
            }
            delete conv;
            delete test_ram;
        
    }
    Timer::test_print(st+".csv");
    delete Config::myconfig;
    delete P2Pchannel::mychnl;

}
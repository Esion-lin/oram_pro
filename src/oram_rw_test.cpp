#include "argparse/argparse.hpp"
#include "easylogging++.h"
#include "preliminaries.hpp"
#include "timer.hpp"
#include "convert.h"
using namespace argparse;
INITIALIZE_EASYLOGGINGPP
#define max_ele_size 100
uint16_t ele_size = 1;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
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

    /*
    start oram
    */
    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, st);
    if(ele_size == 1){
        
            Convert* conv = new Convert(st, p2pchnl);
            conv->fourpc_zeroshare<uint32_t>(1);


            uint32_t* ram_data = (uint32_t*)malloc(sizeof(uint32_t)*ele_lens);
            Ram<uint32_t>* test_ram = new Ram<uint32_t>(ram_data, ele_lens, st, p2pchnl);
            test_ram->init();
            Timer::record("read_offline");
            test_ram->prepare_read(itr);
            p2pchnl->flush_all();
            Timer::stop("read_offline");
            Timer::record("write_offline");
            test_ram->prepare_write(itr);
            p2pchnl->flush_all();
            Timer::stop("write_offline");

            while(itr -- ){
                Timer::record("read_online");
                uint32_t tmp = test_ram->read(2, false);
                Timer::stop("read_online");
                conv->fourpc_share_2_replicated_share<uint32_t>(&tmp, 1);
                Timer::record("write_online");
                test_ram->write(2, 6, tmp, false);
                Timer::stop("write_online");
            }
            free(ram_data);
            delete conv;
            delete test_ram;
        
    }else{
        
            Convert* conv = new Convert(st, p2pchnl);
            conv->fourpc_zeroshare<block_t>(1);


            block_t ram_data[ele_lens];
            Ram<block_t>* test_ram = new Ram<block_t>(ram_data, ele_lens, st, p2pchnl);
            test_ram->init();
            Timer::record("read_offline");
            test_ram->prepare_read(itr);
            p2pchnl->flush_all();
            Timer::stop("read_offline");
            Timer::record("write_offline");
            test_ram->prepare_write(itr);
            p2pchnl->flush_all();
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
    Timer::test_print();
    delete cfg;
    delete p2pchnl;

}
#include <openssl/rand.h>
#include <iostream>
#include <string>
#include <utility>
#include "io.hpp"
#include "timer.hpp"
#include <omp.h>
#include "nlohmann/json.hpp"
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
using namespace std;
#define table_bit 4



//void decode(vector<uint8_t>& table,)
void result_decode(vector<uint8_t>& result, uint32_t table_len){
  uint32_t other_dim = result.size()/table_len;
  printf("%d \n",result.size());
  for(int i = 0; i < table_len; i++){
    uint8_t sum;
    for(int j = 0; i < other_dim; i++){
      sum += result[i + j*table_len];
    }
  }
}
void table_gen(uint32_t table_len, uint32_t table_size, vector<uint8_t>& table, vector<uint8_t>& r){
  uint32_t table_item_size = 1<<table_bit;
  uint32_t table_overall = (table_bit) * (1<<table_bit)/8 * table_size * table_len;
  table.resize(table_overall);
  r.resize(table_size);
}
void table_evl(uint32_t table_size, uint32_t query_len,vector<uint8_t>& table, vector<uint8_t>& x, vector<uint8_t>& result){
  uint32_t table_item_size = table.size()/query_len/(1<<table_bit);
  uint32_t table_item = table.size()/x.size();
  result.resize(x.size()*query_len);
  for(int i = 0; i < x.size(); i++){
    memcpy(result.data() + i*query_len, table.data() + x[i]+i*table_item, query_len * table_bit / 8);
    //result[i] = table[x[i]+i*table_item_size*table_len];
  }
}
template<class T>
void reveal(vector<T>& x, vector<T>& r){
  for(int i = 0; i < x.size(); i++){
    x[i] = x[i] - r[i];
  }
  if(Config::myconfig->check("player0")){
    P2Pchannel::mychnl->send_data_to("player1", x.data(), x.size()*sizeof(T));
    P2Pchannel::mychnl->recv_data_from("player1", r.data(), x.size()*sizeof(T));
  }else if(Config::myconfig->check("player1")){
    P2Pchannel::mychnl->send_data_to("player0", x.data(), x.size()*sizeof(T));
    P2Pchannel::mychnl->recv_data_from("player0", r.data(), x.size()*sizeof(T));
  }
  for(int i = 0; i < x.size(); i++){
    x[i] = x[i] + r[i];
  }
}
template<class T>
void bench_lookup(int table_len, int query_len, int table_size){
    //vector<uint8_t> table(table_len * query_len * table_size, 0);
    vector<T> x(table_len);
    vector<T> bias(table_len);
    
    table_len /= 100;
    #pragma omp for
    for(int i=0; i < 100; i++){
    T * table = (T *)malloc(table_len * query_len * table_size);
    T * result = (T *)malloc(table_len * query_len);
    //uint8_t * table = mal
    
    //
    free(table);
    free(result);
    }
    
    reveal<T>(x, bias);
    // if(Config::myconfig->check("player1"))
    // table_evl(1<<table_bit, query_len, table, x, result);
}
void bench_conv(int batchsize, int height, int width, int din, int dout, int filter, int stride){
    int table_len = batchsize * height * width * din ;
    int query_len = filter * filter * dout / (stride*stride);
    int table_size = 1<<table_bit;
    bench_lookup<uint8_t>(table_len, query_len, table_size);

}
void bench_res(int batchsize, int height, int width, int din, int dout){
    bench_conv(batchsize, height, width, din,dout,3,2);
    bench_conv(batchsize, height, width, din,dout,1,2);
    bench_conv(batchsize, height, width, din,dout,3,2);
    

}
void bench_fc(int batchsize,  int din, int dout){
    int table_len = batchsize * din ;
    int query_len = dout;
    int table_size = 1<<table_bit;
    bench_lookup<uint8_t>(table_len, query_len, table_size);

}
void bench_activate(int batchsize,  int din){
  //printf("activate\n");
    int table_len = batchsize * din ;
    int query_len = 1;
    int table_size = 1<<table_bit;
    bench_lookup<uint8_t>(table_len, query_len, table_size);
}
void bench_attention(int batchsize,  int din){
  //printf("attention  %d\n", batch_size);
    int table_len = batchsize * din /3;
    int query_len = batchsize;
    int table_size = 1<<table_bit;
    bench_lookup<uint8_t>(table_len, query_len, table_size);
    bench_lookup<uint8_t>(table_len/64*batchsize, 1, table_size);
    for(int i = 1; i < batchsize; i*=2)
      bench_lookup<uint8_t>(table_len/64*batchsize*2, 1, table_size);
    bench_lookup<uint8_t>(table_len/64*batchsize, 64, table_size);
}
void bench_ffn(int batchsize,  int din){
  printf("ffn\n");
  int table_size = 1<<table_bit;
  bench_fc(batchsize, din, 4*din);
   bench_lookup<uint8_t>(batchsize*4*din, 1, table_size);
   bench_fc(batchsize, 4*din, din);
   
}
void bench_model( std::string network_filename) {

    std::cout << "network filename: " << network_filename << std::endl;

    nlohmann::json model_doc;

    std::ifstream model_file(network_filename);
    model_file >> model_doc;

    int INPUT_SIZE = model_doc["input_size"];
    int NUM_CLASSES = model_doc["num_classes"];
    int MINI_BATCH_SIZE = model_doc["batch_size"];
    int LOG_MINI_BATCH = log2(MINI_BATCH_SIZE); 
    auto layers = model_doc["model"];
    Timer::record("model");
    for(int i = 0; i < layers.size(); i++) {

        auto l = layers[i];
    
        if (l["layer"] == "fc") {
            bench_fc(MINI_BATCH_SIZE, l["input_dim"], l["output_dim"]);
        } else if (l["layer"] == "relu") {
        } else if (l["layer"] == "cnn") {
            bench_conv(MINI_BATCH_SIZE, l["input_hw"][0], l["input_hw"][1], l["in_channels"], l["out_channels"], l["filter_hw"][0], l["stride"]);
        } else if (l["layer"] == "maxpool") {
        } else if (l["layer"] == "averagepool") {
        } else if (l["layer"] == "ln") {
          bench_activate(MINI_BATCH_SIZE, l["input_dim"]);
        } else if (l["layer"] == "attention") {
          bench_attention(MINI_BATCH_SIZE, l["input_dim"]);
        }else if (l["layer"] == "FFN") {
          bench_ffn(MINI_BATCH_SIZE, l["input_dim"]);
        }else if (l["layer"] == "res") {
          bench_res(MINI_BATCH_SIZE, l["input_hw"][0], l["input_hw"][1], l["in_planes"], l["out_planes"]);
        } else {
            assert(false && "layer type not supported");
        }
    }
    Timer::stop("model");
}
int main(int argc, char** argv)
{
    Config::myconfig = new Config("./2_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    bench_model(argv[2]);


  //  vector<uint8_t> table, r, x, result;
  //  x.resize(table_size);
  //  Timer::record("gen");
  //  table_gen(tablelen, table_size, table, r);
  //  Timer::record("gen");
  //  table_gen(tablelen, table_size, table, r);
  //  table_gen(tablelen, table_size, table, r);
  //  table_gen(tablelen, table_size, table, r);
  //  table_gen(tablelen, table_size, table, r);
  //  table_gen(tablelen, table_size, table, r);
  //  table_gen(tablelen, table_size, table, r);
  //  table_gen(tablelen, table_size, table, r);
  //  table_gen(tablelen, table_size, table, r);
  //  table_gen(tablelen, table_size, table, r);
  //  table_gen(tablelen, table_size, table, r);
  //  Timer::stop("gen");
  //  Timer::record("evl");
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);
  //  reveal(x, r);

  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  //  table_evl(tablelen, table, x, result);
  // for( int j = 0; j < 20; j ++){
  //  result_decode(result, tablelen);
  //  }
  
   Timer::test_print();

}
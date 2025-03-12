
#include <gmp.h>
#include <gmpxx.h>
#include <openssl/rand.h>
#include <iostream>
#include <string>
#include <utility>
#include "io.hpp"
#include "easylogging++.h"
#include "timer.hpp"
#include <fstream>
#include <sstream>
#include <chrono>
INITIALIZE_EASYLOGGINGPP
Config* Config::myconfig;
P2Pchannel* P2Pchannel::mychnl;
std::map<std::string, double> Timer::times;
std::map<std::string, struct timeval> Timer::ptrs;
std::string Timer::now_name;
using namespace std;
#define table_bit 16
//void decode(vector<uint8_t>& table,)
void result_decode(vector<uint8_t>& result, uint64_t table_len){
  uint64_t other_dim = result.size()/table_len;
  //printf("%d \n",result.size());
  for(uint64_t i = 0; i < table_len; i++){
    uint8_t sum;
    for(uint64_t j = 0; j < other_dim; j++){
      sum += result[i + j*table_len];
    }
  }
}
void table_gen(uint64_t table_len, uint64_t table_size, vector<uint8_t>& table, vector<uint8_t>& r){
  uint64_t table_item_size = 1<<table_bit;
  table_item_size *= table_bit;
  table_item_size /= 8;
  uint64_t table_overall = (table_bit) * (1<<table_bit)/8 * table_size * table_len;
  table.resize(table_overall+table_size);
  r.resize(table_size);
 
  if (Config::myconfig->check("player2")) {
	  vector<uint8_t> t1(table_overall+table_size), r1(table_size);

//    RAND_bytes(t1.data(), table_overall);
    RAND_bytes(r1.data(), table_size);
    RAND_bytes(r.data(), table_size);
    for (uint64_t i = 0; i < table_size; ++i) {
      rotate(table.begin()+i*table_item_size*table_len, table.begin()+i*table_item_size*table_len+((r1[i]+r[i])&0x03), table.begin()+(i+1)*table_item_size*table_len);
    }
    //
    for (uint64_t i = 0; i < table.size(); ++i) {
	    table[i] -= t1[i];
    }
    

		P2Pchannel::mychnl->send_data_to("player1", t1.data(), table_overall+table_size);
	//	P2Pchannel::mychnl->send_data_to("player0", table.data(), table.size());

  } else  if (Config::myconfig->check("player1")){
	  P2Pchannel::mychnl->recv_data_from("player2", table.data(), table_overall+table_size);
  }
  
}
void table_evl(uint64_t table_len,vector<uint8_t>& table, vector<uint8_t>& x, vector<uint8_t>& result){
  uint64_t table_item_size = 1<<table_bit;
  table_item_size *= table_bit;
  table_item_size /= 8;
  result.resize(x.size()*table_len);
  for(uint64_t i = 0; i < x.size(); i++){
    memcpy(result.data() + i*table_len, table.data() +i*table_item_size*table_len, table_len * table_bit / 8);
    //result[i] = table[x[i]+i*table_item_size*table_len];
  }
}
void reveal(vector<uint8_t>& x, vector<uint8_t>& r){
  for(uint64_t i = 0; i < x.size(); i++){
    x[i] = x[i] - r[i];
  }
  if(Config::myconfig->check("player0")){
    P2Pchannel::mychnl->send_data_to("player1", x.data(), x.size());
    P2Pchannel::mychnl->recv_data_from("player1", r.data(), x.size());
  }else if(Config::myconfig->check("player1")){
    P2Pchannel::mychnl->send_data_to("player0", x.data(), x.size());
    P2Pchannel::mychnl->recv_data_from("player0", r.data(), x.size());
  }
  for(uint64_t i = 0; i < x.size(); i++){
    x[i] = x[i] + r[i];
  }
}


int main(int argc, char** argv)
{

    //cout << "123" << endl;
    Config::myconfig = new Config("./2_p_config.json");
    P2Pchannel::mychnl = new P2Pchannel(Config::myconfig->Pmap, argv[1]);
    Config::myconfig->set_player(argv[1]);
    //table_len : multiple table with same r
    uint64_t tablelen;
    uint64_t table_size;

    ifstream file(argv[2]);
    string line;
    vector<uint8_t> table, r, x, result;
    
            auto offlinebegin = chrono::steady_clock::now();

            auto offlineend = chrono::steady_clock::now();
	    int tmp = 0;
    while(getline(file, line)) {
	string ope;
	istringstream iss(line);
	iss >> ope;
	cout << ope << endl;

	if (ope == "matmul") {
		iss >> table_size >> tablelen;
		x.resize(table_size);
		Timer::record("gen");
 if (Config::myconfig->check("player0")){
                                P2Pchannel::mychnl->send_data_to("player1", &tmp, 1);
                                P2Pchannel::mychnl->recv_data_from("player1", &tmp, 1);
                                } else if (Config::myconfig->check("player1")) {
                                P2Pchannel::mychnl->recv_data_from("player0", &tmp, 1);
                                P2Pchannel::mychnl->send_data_to("player0", &tmp, 1);
                                }
            offlinebegin = chrono::steady_clock::now();
		table_gen(tablelen, table_size, table, r);

            offlineend = chrono::steady_clock::now();
		Timer::stop("gen");
		//P2Pchannel::mychnl->sync();
		if (!Config::myconfig->check("player2")) {
		Timer::record("evl");
		reveal(x, r);
		table_evl(tablelen, table, x, result);
		result_decode(result, tablelen);
		Timer::stop("evl");
		}
		//P2Pchannel::mychnl->sync();
	} else if (ope == "lookup" || ope == "add") {
		iss >> table_size;
		tablelen = 1;
		x.resize(table_size);

                Timer::record("gen");
		if (Config::myconfig->check("player0")){
				P2Pchannel::mychnl->send_data_to("player1", &tmp, 1);
				P2Pchannel::mychnl->recv_data_from("player1", &tmp, 1);
				} else if (Config::myconfig->check("player1")) {
				P2Pchannel::mychnl->recv_data_from("player0", &tmp, 1);
				P2Pchannel::mychnl->send_data_to("player0", &tmp, 1);
				}
            offlinebegin = chrono::steady_clock::now();
                table_gen(tablelen, table_size, table, r);
            offlineend = chrono::steady_clock::now();
                Timer::stop("gen");
		//P2Pchannel::mychnl->sync();
		if (!Config::myconfig->check("player2")) {
                Timer::record("evl");
                reveal(x, r);
		Timer::stop("evl");
//		cout << "reveal end" << endl;Timer::stop("evl");
		Timer::record("evl");
                table_evl(tablelen, table, x, result);
//result_decode(result, tablelen);
                Timer::stop("evl");
		}
		//P2Pchannel::mychnl->sync();
	}
	auto offlinemilli = chrono::duration<double,std::milli>(offlineend-offlinebegin).count();
            cout << offlinemilli << endl;
    }

    delete P2Pchannel::mychnl;
    //cout << "123" << endl;
/**
   vector<uint8_t> table, r, x, result;
   x.resize(table_size);
   Timer::record("gen");
   table_gen(tablelen, table_size, table, r);
   Timer::record("gen");
   table_gen(tablelen, table_size, table, r);
   table_gen(tablelen, table_size, table, r);
   table_gen(tablelen, table_size, table, r);
   table_gen(tablelen, table_size, table, r);
   table_gen(tablelen, table_size, table, r);
   table_gen(tablelen, table_size, table, r);
   table_gen(tablelen, table_size, table, r);
   table_gen(tablelen, table_size, table, r);
   table_gen(tablelen, table_size, table, r);
   table_gen(tablelen, table_size, table, r);
   Timer::stop("gen");
   Timer::record("evl");
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);
   reveal(x, r);

   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
   table_evl(tablelen, table, x, result);
  for( int j = 0; j < 20; j ++){
   result_decode(result, tablelen);
   }
  
   Timer::stop("evl");

   */
   Timer::test_print();
   

}

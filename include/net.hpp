#ifndef _HIGHSPEED_NETWORK_IO_CHANNEL_H__
#define _HIGHSPEED_NETWORK_IO_CHANNEL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include "config.hpp"


#include <iostream>
#include <string>

using std::string;

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

//#include "easylogging++.h"
namespace net {
	const static int NETWORK_BUFFER_SIZE2 = 1024 * 1;  // 减小到 64 KB
	const static int NETWORK_BUFFER_SIZE = 5 * 1024 * 1024 * 1024;  // 增加到 256 MB


class SubChannel { public:
	int sock;
	FILE *stream = nullptr;
	char *buf = nullptr;
	int ptr;
	char *stream_buf = nullptr;
	uint64_t counter = 0;
	uint64_t flushes = 0;
	SubChannel(int sock) : sock(sock) {
		stream_buf = new char[NETWORK_BUFFER_SIZE];
		buf = new char[NETWORK_BUFFER_SIZE2];
		stream = fdopen(sock, "wb+");
		memset(stream_buf, 0, NETWORK_BUFFER_SIZE);
		setvbuf(stream, stream_buf, _IOFBF, NETWORK_BUFFER_SIZE);
	}
	~SubChannel() {
		fclose(stream);
		delete[] stream_buf;
		delete[] buf;
	}
};

class SenderSubChannel : public SubChannel { public:
	SenderSubChannel(int sock) : SubChannel(sock) { ptr = 0; }

	void flush() {
		flushes++;
		send_data_raw(buf, ptr);
		if (counter % NETWORK_BUFFER_SIZE2 != 0)
			send_data_raw(buf + ptr, NETWORK_BUFFER_SIZE2 - counter % NETWORK_BUFFER_SIZE2);
		fflush(stream);
		ptr = 0;
	}
	
	void send_data(const void *data, uint64_t len) {
		if (len <= NETWORK_BUFFER_SIZE2 - ptr) {
			memcpy(buf + ptr, data, len);
			ptr += len;
		} else {
			send_data_raw(buf, ptr);
			send_data_raw(data, len);
			ptr = 0;
		}
	}

	void send_data_raw(const void *data, uint64_t len) {
		counter += len;
		uint64_t sent = 0;
		while (sent < len) {
			fd_set write_fds;
			FD_ZERO(&write_fds);
			FD_SET(sock, &write_fds);
	
			// 使用 select 等待套接字可写
			struct timeval timeout = {0, 200000}; // 100ms 超时
			int ready = select(sock + 1, nullptr, &write_fds, nullptr, &timeout);
			if (ready > 0 && FD_ISSET(sock, &write_fds)) {
				int res = write(sock, (char *)data + sent, len - sent);
				if (res > 0) {
					sent += res;
				} else if (res == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
					fprintf(stderr, "error: net_send_data %d\n", res);
					break;
				}
			}
		}
	}
};

class RecverSubChannel : public SubChannel { public:
	RecverSubChannel(int sock) : SubChannel(sock) { ptr = NETWORK_BUFFER_SIZE2; }
	void flush() {
		flushes++;
		ptr = NETWORK_BUFFER_SIZE2;
	}

	void recv_data(void *data, uint64_t len) {
		if (len <= NETWORK_BUFFER_SIZE2 - ptr) {
			memcpy(data, buf + ptr, len);
			ptr += len;
		} else {
			uint64_t remain = len;
			memcpy(data, buf + ptr, NETWORK_BUFFER_SIZE2 - ptr);
			remain -= NETWORK_BUFFER_SIZE2 - ptr;

			while (true) {
				recv_data_raw(buf, NETWORK_BUFFER_SIZE2);
				if (remain <= NETWORK_BUFFER_SIZE2) {
					memcpy(len - remain + (char *)data, buf, remain);
					ptr = remain;
					break;
				} else {
					memcpy(len - remain + (char *)data, buf, NETWORK_BUFFER_SIZE2);
					remain -= NETWORK_BUFFER_SIZE2;
				}
			}
		}
	}

	void recv_data_raw(void *data, uint64_t len) {
		counter += len;
		uint64_t received = 0;
		while (received < len) {
			fd_set read_fds;
			FD_ZERO(&read_fds);
			FD_SET(sock, &read_fds);
	
			// 使用 select 等待套接字可读
			struct timeval timeout = {0, 200000}; // 1000ms 超时
			int ready = select(sock + 1, &read_fds, nullptr, nullptr, &timeout);
			if (ready > 0 && FD_ISSET(sock, &read_fds)) {
				int res = read(sock, (char *)data + received, len - received);
				if (res > 0) {
					received += res;
				} else if (res == -1 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
					fprintf(stderr, "error: net_recv_data %d\n", res);
					break;
				}
			}
		}
	}
};

class HighSpeedNetIO { public:
	bool is_server, quiet;
	int send_sock = 0;
	int recv_sock = 0;
	int FSM = 0;
	SenderSubChannel *schannel;
	RecverSubChannel *rchannel;

	HighSpeedNetIO(const char *address, int send_port, int recv_port, bool quiet = true) : quiet(quiet) {
		is_server = (address == nullptr);
		if (is_server) {
			recv_sock = server_listen(send_port);
			usleep(2000);
			send_sock = server_listen(recv_port & 0xFFFF);
		} else {
			send_sock = client_connect(address, send_port);
			recv_sock = client_connect(address, recv_port & 0xFFFF);
		}
		FSM = 0;
		set_delay_opt(send_sock, true);
		set_delay_opt(recv_sock, true);
		schannel = new SenderSubChannel(send_sock);
		rchannel = new RecverSubChannel(recv_sock);
		if (not quiet) std::cout << "connected\n";
	}

	int server_listen(int port) {
		int mysocket;
		struct sockaddr_in dest;
		struct sockaddr_in serv;
		socklen_t socksize = sizeof(struct sockaddr_in);
		memset(&serv, 0, sizeof(serv));
		serv.sin_family = AF_INET;
		serv.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
		serv.sin_port = htons(port);              /* set the server port number */
		mysocket = socket(AF_INET, SOCK_STREAM, 0);
		int reuse = 1;
		setsockopt(mysocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));
		if (bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr)) < 0) {
			perror("error: bind");
			exit(1);
		}
		if (listen(mysocket, 1) < 0) {
			perror("error: listen");
			exit(1);
		}
		int sock = accept(mysocket, (struct sockaddr *)&dest, &socksize);
		close(mysocket);
		return sock;
	}
	int client_connect(const char *address, int port) {
		int sock;
		struct sockaddr_in dest;
		memset(&dest, 0, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_addr.s_addr = inet_addr(address);
		dest.sin_port = htons(port);

		while (1) {
			sock = socket(AF_INET, SOCK_STREAM, 0);
			if (connect(sock, (struct sockaddr *)&dest, sizeof(struct sockaddr)) == 0) break;

			close(sock);
			usleep(1000);
		}
		return sock;
	}

	~HighSpeedNetIO() {
		flush();
		if (not quiet) {
			std::cout << "Data Sent: \t" << schannel->counter << "\n";
			std::cout << "Data Received: \t" << rchannel->counter << "\n";
			std::cout << "Flushes:\t" << schannel->flushes << "\t" << rchannel->flushes << "\n";
		}
		delete schannel;
		delete rchannel;
		close(send_sock);
		close(recv_sock);
	}

	void sync() {
		int tmp = 0;
		if (is_server) {
			send_data_internal(&tmp, 1);
			recv_data_internal(&tmp, 1);
		} else {
			recv_data_internal(&tmp, 1);
			send_data_internal(&tmp, 1);
			flush();
		}
	}

	void set_delay_opt(int sock, bool enable_nodelay) {
		if (enable_nodelay) {
			const int one = 1;
			setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
		} else {
			const int zero = 0;
			setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &zero, sizeof(zero));
		}
	}

	void flush() {
		if (is_server) {
			schannel->flush();
			rchannel->flush();
		} else {
			rchannel->flush();
			schannel->flush();
		}
		FSM = 0;
	}

	void send_data_internal(const void *data, uint64_t len) {
		// if (FSM == 1) {
		// 	rchannel->flush();
		// }
		schannel->send_data(data, len);
		FSM = 2;
	}

	void recv_data_internal(void *data, uint64_t len) {
		// if (FSM == 2) {
		// 	schannel->flush();
		// }
		rchannel->recv_data(data, len);
		FSM = 1;
	}
};
}
class P2Pchannel{
	private:
	std::map<std::string, net::HighSpeedNetIO*> subio;
	double send_len = 0;
	double recv_len = 0;
	std::map<std::string, uint32_t> bytes_send_lens;
	std::map<std::string, uint32_t> bytes_recv_lens;
	std::map<std::string, uint32_t> ptrs_send;
	std::map<std::string, uint32_t> ptrs_recv;
	int FSM = 0;
	protected:
	
	std::string st;
    public:
	bool is_flush = true;
	static P2Pchannel* mychnl;
	void record(std::string name){
		ptrs_send[name] = send_len;
		ptrs_recv[name] = recv_len;
	}
	void stop(std::string name){
		if(bytes_send_lens.find(name) == bytes_send_lens.end()){
			bytes_send_lens[name] = send_len - ptrs_send[name];
		}else{
			bytes_send_lens[name] += send_len - ptrs_send[name];
		}
		if(bytes_recv_lens.find(name) == bytes_recv_lens.end()){
			bytes_recv_lens[name] = recv_len - ptrs_recv[name];
		}else{
			bytes_recv_lens[name] += recv_len - ptrs_recv[name];
		}
		
	}
	void test_print(){
		for(auto ele:bytes_send_lens){
            
            printf("%s send: %lu bytes\n", ele.first.c_str(), ele.second);
		}
		for(auto ele:bytes_recv_lens){
            
            printf("%s recv: %lu bytes\n", ele.first.c_str(), ele.second);
		}
	}
    P2Pchannel(std::map<std::string, Player> pmap, std::string user){
		
		st = user;
		/*根据当前用户构建通信通道*/
		for(auto &v : pmap){
			if(v.first == user) continue;
			int port1 = 9000 + pmap[user].port*10 + v.second.port;
			int port2 = 9000 + v.second.port*10 + pmap[user].port;
			if(port1 > port2) subio[v.first] = new net::HighSpeedNetIO(nullptr, port1, port2);
			else subio[v.first] = new net::HighSpeedNetIO(v.second.address.c_str(), port2, port1);
			#ifdef DISK_NET
			#endif
		}
		// LOG(INFO) << "P2Pchannel construction done"<<std::endl;
	}
	void set_flush(bool flush_t){
		is_flush = flush_t;

	}
	void bloadcast(const void* data, int len){
		for(auto & ele : subio){
			ele.second->send_data_internal(data, len);
		}
		if(is_flush) flush_all();
	}
	std::map<std::string, std::vector<uint8_t>> 
	recv_all(int len){
		std::map<std::string, std::vector<uint8_t>>  ret;
		for(auto & ele : subio){
			std::vector<uint8_t> tmp(len);
			ele.second->recv_data_internal(tmp.data(), len);
			ret[ele.first] = tmp;
		}
		return ret;
	}
	void send_data_to(std::string player, const void* data, uint64_t len){
		//if (FSM == 1) {
			
		//}
		#ifdef DISK_NET

		#else
		send_len += len;
		subio[player]->send_data_internal(data, len);
		if(is_flush) flush_all();
		#endif
	}
	void recv_data_from(std::string player, void* data, uint64_t len){
		#ifdef DISK_NET
		
		#else
		recv_len += len;
		subio[player]->recv_data_internal(data, len);
		if(is_flush) flush_all();
		#endif		
	}
	void flush_player(std::string player) {
		subio[player]->flush();
	}
	void flush_all(){
		for(auto &v : subio){
			v.second->flush();
		}
	}
	void sync() {
		for(auto &v : subio){
			v.second->sync();
		}
	}
	~P2Pchannel(){
		printf("send data %lf\n",send_len);
		printf("recv data %lf\n",recv_len);
		//清理节点
		for(auto &v : subio) delete v.second;
	}

	
};
#endif

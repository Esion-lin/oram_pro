#include <memory>
#include "salticidae/event.h"
#include "salticidae/network.h"

using Net = salticidae::PeerNetwork<uint8_t>;

int main() {
    std::vector<std::pair<salticidae::NetAddr, std::unique_ptr<Net>>> nodes;
    Net::Config config;
    salticidae::EventContext ec;
    config.ping_period(2);
    nodes.resize(4);
    for (size_t i = 0; i < nodes.size(); i++)
    {
        salticidae::NetAddr addr("127.0.0.1:" + std::to_string(10000 + i));
        auto &net = (nodes[i] = std::make_pair(addr, std::make_unique<Net>(ec, config))).second;
        net->start();
        net->listen(addr);
    }
    for (size_t i = 0; i < nodes.size(); i++)
        for (size_t j = 0; j < nodes.size(); j++)
            if (i != j)
            {
                auto &node = nodes[i].second;
                auto &peer_addr = nodes[j].first;
                salticidae::PeerId pid{peer_addr};
                node->add_peer(pid);
                node->set_peer_addr(pid, peer_addr);
                node->conn_peer(pid);
            }
    ec.dispatch();
    return 0;
}
#include "preliminaries.h"
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP
int main(argc, char** argv){
    Config* cfg = new Config("./config.json");
    P2Pchannel* p2pchnl = new P2Pchannel(cfg->Pmap, argv[1]);

}
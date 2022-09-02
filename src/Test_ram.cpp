#include "fss-server.h"
#include "fss-common.h"
#include "fss-client.h"
int main(int argc, char** argv){
    Fss server_key;
    ServerKeyEq k0, k1;
    
    uint32_t datalens = 2048 ;
    uint32_t res[datalens],res2[datalens];

    initializeClient(&server_key, 11, 2);
    uint32_t b = 200;
    generateTreeEq_32(&server_key, &k0, &k1, 3, b);
    
    evaluateEq(&server_key, &k0, res, datalens);
    //evaluateEq_full(&server_key, &k0, res2, 16);
    evaluateEq(&server_key, &k1, res2, datalens);

    for(int i = datalens; i > datalens - 100; i--){
        std::cout<<res[i]<<" "<<res2[i]<<std::endl;
    }    
}
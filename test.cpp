#include <openssl/obj_mac.h>
#include <openssl/ec.h>
#include <sys/time.h>
#include <omp.h>
#include <iostream>
struct commitment{

};
int main(){

    EC_GROUP *curve;
    if(NULL == (curve = EC_GROUP_new_by_curve_name(NID_secp224r1)))
        printf("create error !\n");
    const EC_POINT * G  = EC_GROUP_get0_generator(curve);
    EC_POINT *H, *res = EC_POINT_new(curve), *res_2 = EC_POINT_new(curve), *res_3 =  EC_POINT_new(curve), *temp = EC_POINT_new(curve);;
    BIGNUM *x = BN_new(), *r = BN_new();
    BIGNUM *x_2 = BN_new(), *r_2 = BN_new();
    uint8_t xx[4] = {1,2,3,4}, xx2[4] = {89,12,3,14}, rr[32] = {1,2,3,4}, xx_1[4], rr_1[32];
    H = EC_POINT_new(curve);
    BN_bin2bn(rr, 32, r);
    BN_bin2bn(xx, 4, x);
    // BN_bin2bn(rr_1, 4, r_1);
    // BN_bin2bn(xx_1, 4, x_1);
    struct timeval t1;
    gettimeofday(&t1,NULL);
omp_set_num_threads(4);
#pragma omp parallel
    {
    #pragma omp for
    for(int i = 0; i < 1000; i++)
    EC_POINT_mul(curve, res, x,
                H, r, NULL);
    }
    // BN_bin2bn(xx2, 4, x);
    EC_POINT_mul(curve, res_3, x,
                H, r, NULL);
    struct timeval t2;
    gettimeofday(&t2,NULL);
    std::cout<<(t2.tv_sec - t1.tv_sec) + (double)(t2.tv_usec - t1.tv_usec)/1000000.0<<std::endl;
    // EC_POINT_mul(curve, res_1, x_1,
    //             H, r_1, NULL);
    gettimeofday(&t1,NULL);
    omp_set_num_threads(3);
#pragma omp parallel
    {
    #pragma omp for
        for(int i = 0; i < 1000; i++)
        EC_POINT_add(curve, res_2, res, res_3, NULL);
    }
    gettimeofday(&t2,NULL);
    std::cout<<(t2.tv_sec - t1.tv_sec) + (double)(t2.tv_usec - t1.tv_usec)/1000000.0<<std::endl;
    BN_add(r_2, r, r); BN_add(x_2, x, x);
    EC_POINT_mul(curve, temp, x_2,
                H, r_2, NULL);
    printf("%d\n", EC_POINT_cmp(curve, temp, res_2, NULL));
    BN_clear(x);BN_clear(r);
    BN_clear(x_2);BN_clear(r_2);
    EC_POINT_clear_free(res);
    EC_POINT_clear_free(res_2);
    EC_POINT_clear_free(temp);
    EC_POINT_clear_free(H);
    EC_GROUP_free(curve);
    gettimeofday(&t1,NULL);
    int a = 13124, b = 3234;
omp_set_num_threads(3);
#pragma omp parallel
    {
    #pragma omp for
        for(int i = 0; i < 100000000; i++)
        a = a * b;
    }
    gettimeofday(&t2,NULL);
    std::cout<<(t2.tv_sec - t1.tv_sec) + (double)(t2.tv_usec - t1.tv_usec)/1000000.0<<std::endl;
    
}
// #include "secp256k1_rangeproof.h"

// int main(){
//     unsigned char blind[32];
//     for (int i = 0; i < 32; i++) blind[i] = i + 1;
//     secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
//     secp256k1_pedersen_commitment c1, c2, c3;
//     uint64_t value1 = 100, value2 = 103;
//     secp256k1_pedersen_commit(ctx, &c1, blind, value1, secp256k1_generator_h);
//     secp256k1_pedersen_commit(ctx, &c2, blind, value2, secp256k1_generator_h);
//     secp256k1_scalar_add(&c3, &c1, &c2);
// }
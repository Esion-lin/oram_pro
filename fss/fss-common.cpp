#include "fss-common.h"

AES_KEY* prf(unsigned char* out, unsigned char* key, uint64_t in_size, AES_KEY* aes_keys, uint32_t numKeys) {
#ifndef AESNI
    // check if there is aes-ni instruction
    uint32_t eax, ebx, ecx, edx;

    eax = ebx = ecx = edx = 0;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
#endif
    
    AES_KEY* temp_keys = aes_keys;
    // Do Matyas–Meyer–Oseas one-way compression function using different AES keys to get desired
    // output length
    uint32_t num_keys_required = in_size/16;
    if (num_keys_required > numKeys) {
        free(temp_keys);
        temp_keys = (AES_KEY*) malloc(sizeof(AES_KEY)*num_keys_required); 
        //#pragma omp parallel for
        for (int i = 0; i < num_keys_required; i++) {
            unsigned char rand_bytes[16];
            if (!RAND_bytes(rand_bytes, 16)) {
                printf("Random bytes failed.\n");
            }
#ifndef AESNI
            AES_set_encrypt_key(rand_bytes, 128, &(temp_keys[i]));
#else
            aesni_set_encrypt_key(rand_bytes, 128, &(temp_keys[i]));
#endif
        }
        free(temp_keys);
    }
    //#pragma omp parallel for
    for (int i = 0; i < num_keys_required; i++) {
#ifndef AESNI
        //AES_encrypt(key, out + (i*16), &temp_keys[i]);
        offline_prg(out + (i*16), key, &temp_keys[i]);
#else
        //aesni_encrypt(key, out + (i*16), &temp_keys[i]);
        offline_prg(out + (i*16), key, &temp_keys[i]);
#endif
    }
    // for (int i = 0; i < in_size; i++) {
    //     out[i] = out[i] ^ key[i%16];
    // }
    return temp_keys;
}

void offline_prg(uint8_t * dest, uint8_t * src, void * ri) {
	__m128i xr;
    __m128i mr;
	__m128i * r = (__m128i *)ri;

    xr = _mm_load_si128((__m128i *) src);
    mr = xr;
    
    mr = _mm_xor_si128(mr, r[0]);

    mr = _mm_aesenc_si128(mr, r[1]);
    mr = _mm_aesenc_si128(mr, r[2]);
    mr = _mm_aesenc_si128(mr, r[3]);
    mr = _mm_aesenc_si128(mr, r[4]);
    mr = _mm_aesenc_si128(mr, r[5]);
    mr = _mm_aesenc_si128(mr, r[6]);
    mr = _mm_aesenc_si128(mr, r[7]);
    mr = _mm_aesenc_si128(mr, r[8]);
    mr = _mm_aesenc_si128(mr, r[9]);
    mr = _mm_aesenclast_si128(mr, r[10]);
    mr = _mm_xor_si128(mr, xr);
    _mm_storeu_si128((__m128i*) dest, mr);

}

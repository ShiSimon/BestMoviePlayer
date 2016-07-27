#ifndef RSA_H
#define RSA_H

#ifdef __cplusplus
extern "C"
{
#endif


    int rsa_decrypt( const char *key_fn, unsigned char in[256], unsigned char out[256]);
    int rsa_decrypt_from_buffer( const char *buf, unsigned int len, unsigned char in[256], unsigned char out[256]);


#ifdef __cplusplus
}
#endif

#endif // RSA_H

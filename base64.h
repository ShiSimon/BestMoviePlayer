#ifndef INCLUDED_BASE64_H
#define INCLUDED_BASE64_H

#ifdef __cplusplus
extern "C"
{
#endif

unsigned int base64_get_linesize(void);
void base64_set_linesize(unsigned int linesize);


/* 从字节的长度计算 base64 编码所占用的长度，计入尾部padding */
unsigned int base64_size(unsigned int byte_size);

/* byte->base64 string */
char *base64_enc(const void *buf, unsigned int sz);
char *base64_enc_str(const char *buf);
int base64_enc_buf( const void *input_buf, unsigned int in_sz, void *output_buf, unsigned int out_sz);

void *base64_dec(const char *buf, unsigned int *p_out_size, unsigned int *p_alloc_size);

#ifdef __cplusplus
}
#endif

#endif

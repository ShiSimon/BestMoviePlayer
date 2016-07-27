#include "base64.h"
#include <stdio.h>
#include <string.h>
#include <malloc.h>

static unsigned int g_base64_linesize = 76;
static const char *g_base64_chartbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static unsigned char g_base64_valtbl[256];

unsigned int base64_get_linesize(void)
{
    return g_base64_linesize;
}

void base64_set_linesize(unsigned int linesize)
{
    if( linesize)
    {
        g_base64_linesize = linesize;
    }
}


unsigned int base64_size(unsigned int byte_size)
{
    unsigned int token = byte_size / 3;
    unsigned int q = byte_size % 3;
    unsigned int ret = token * 4;

    ret += (q ? 4: 0);
    /* 计算回车的数目量 */
    ret += ret / base64_get_linesize();
    return ret;
}

char *base64_enc(const void *input_buf, unsigned int sz)
{
    unsigned int out_size = base64_size(sz);
    void *output_buf = malloc(out_size+1); /* 计入结尾0 */
    if( !output_buf)
    {
        return 0;
    }
    base64_enc_buf(input_buf, sz, output_buf, out_size);

    ((char*)output_buf)[out_size] = 0;
    return (char*)output_buf;
}

char *base64_enc_str(const char *buf)
{
    return base64_enc(buf, strlen(buf));
}

int base64_enc_buf( const void *input_buf, unsigned int in_sz, void *output_buf, unsigned int out_sz)
{
    unsigned long data_buf = 0;
    const unsigned char *pin = (const unsigned char *)(input_buf);
    unsigned char *pout = (unsigned char *)output_buf;
    unsigned int i;
    unsigned int line_count = 0;

#define CHECK_NEWLINE(pout, line_count) \
    if( ++line_count == g_base64_linesize)\
    {\
        *pout ++ = '\n'; line_count = 0;\
    }\


    if( base64_size(in_sz) > out_sz) return -1;
    for( i = 0; i < in_sz; i+= 3)
    {
        switch(in_sz - i)
        {
        case 1:
            data_buf = (unsigned long)((pin[i] << 16));
            *pout = g_base64_chartbl[(data_buf >> 18) & 0x3F]; pout++;
            CHECK_NEWLINE(pout, line_count);

            *pout = g_base64_chartbl[(data_buf >> 12) & 0x3F]; pout++;
            CHECK_NEWLINE(pout, line_count);

            *pout ='='; pout++;
            CHECK_NEWLINE(pout, line_count);

            *pout ='='; pout++;
            CHECK_NEWLINE(pout, line_count);
            break;
        case 2:
            data_buf = (unsigned long)((pin[i] << 16) | (pin[i+1] << 8));

            *pout = g_base64_chartbl[(data_buf >> 18) & 0x3F]; pout++;
            CHECK_NEWLINE(pout, line_count);

            *pout = g_base64_chartbl[(data_buf >> 12) & 0x3F]; pout++;
            CHECK_NEWLINE(pout, line_count);

            *pout = g_base64_chartbl[(data_buf >> 6) & 0x3F]; pout++;
            CHECK_NEWLINE(pout, line_count);

            *pout ='='; pout ++;
            CHECK_NEWLINE(pout, line_count);
            break;
        default:
            data_buf = (unsigned long)((pin[i] << 16) | (pin[i+1] << 8) | pin[i+2]);

            *pout = g_base64_chartbl[(data_buf >> 18) & 0x3F]; pout++;
            CHECK_NEWLINE(pout, line_count);

            *pout = g_base64_chartbl[(data_buf >> 12) & 0x3F]; pout++;
            CHECK_NEWLINE(pout, line_count);

            *pout = g_base64_chartbl[(data_buf >> 6) & 0x3F]; pout++;
            CHECK_NEWLINE(pout, line_count);

            *pout = g_base64_chartbl[(data_buf) & 0x3F]; pout++;
            CHECK_NEWLINE(pout, line_count);

            break;
        }
    }
#undef CHECK_NEWLINE
    return 0;
}
static void base64_init_valtbl(void)
{
    if( g_base64_valtbl[0] == 0)
    {
        const char *p = g_base64_chartbl;
        memset(g_base64_valtbl, 0xFF, sizeof(g_base64_valtbl));
        while(*p)
        {
            g_base64_valtbl[(unsigned char)(*p)] = p - g_base64_chartbl;
            p++;
        }
    }
}


void *base64_dec(const char *buf, unsigned int *p_out_size, unsigned int *p_alloc_size)
{
    const char *p = buf;
    unsigned char *pout = 0;
    unsigned int out_size = strlen(buf);
    unsigned long data = 0;
    int data_token = 3;
    unsigned char *pret;
    unsigned char val;

    base64_init_valtbl();


    out_size *= 3;
    out_size /= 4;
    out_size += 2; // + null terminated.

    if( (pout = (unsigned char *)malloc(out_size)) == 0)
    {
        return 0;
    }
    pret = pout;
    ((char*)pout)[out_size-1] = 0;

    //fprintf( stderr, "out_size=%u\n", out_size);

    while(*p && *p!='=')
    {
        if(data_token < 0)
        {
            *pout = (unsigned char)((data >> 16) & 0x0FF);
            pout++;
            *pout = (unsigned char)((data >> 8) & 0x0FF);
            pout++;
            *pout = (unsigned char)((data) & 0x0FF);
            pout++;
            data_token = 3;
            data = 0;
        }
        val = g_base64_valtbl[(unsigned char)(*p)];
        if( val != 0xFF)
        {
            data |= (val << (data_token * 6));
            //fprintf( stderr, "%c: %d:val=%02X, data=%08lX\n", *p, data_token, val, data);
            data_token --;
        }
        p++;
    }
    switch(data_token)
    {
    case 3:                     /*  */
        break;
    case 2:                     /* 6bit */
        *pout = (unsigned char)((data>>16) & 0x0FF);
        pout++;
        break;
    case 1:                     /* 12bit */
        *pout = (unsigned char)((data>>16) & 0x0FF);
        pout++;
        *pout = (unsigned char)((data>>8) & 0x0FF);
        pout++;
        break;
    case 0:                     /* 18bit */
    case -1:                    /* 24bit */
        *pout = (unsigned char)((data>>16) & 0x0FF);
        pout++;
        *pout = (unsigned char)((data>>8) & 0x0FF);
        pout++;
        *pout = (unsigned char)((data) & 0x0FF);
        pout++;
        break;
    }
    if( p_out_size) *p_out_size = pout - pret;
    if( p_alloc_size) *p_alloc_size = out_size;
    return pret;
}

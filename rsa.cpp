#include "rsa.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/ui.h>
#include <openssl/rand.h>
#include <openssl/bn.h>


#define FORMAT_UNDEF    0
#define FORMAT_ASN1     1
#define FORMAT_TEXT     2
#define FORMAT_PEM      3
#define FORMAT_NETSCAPE 4
#define FORMAT_PKCS12   5
#define FORMAT_SMIME    6
#define FORMAT_ENGINE   7
#define FORMAT_IISSGC	8	/* XXX this stupid macro helps us to avoid
                 * adding yet another param to load_*key() */


#define PW_MIN_LENGTH 4
typedef struct pw_cb_data
    {
    const void *password;
    const char *prompt_info;
    } PW_CB_DATA;

static BIO *bio_err = NULL;
static UI_METHOD *ui_method = NULL;
static const char rnd_seed[] = "string to make the random number generator think it has entropy";


static int password_callback(char *buf, int bufsiz, int verify,
                      PW_CB_DATA *cb_tmp)
{
    UI *ui = NULL;
    int res = 0;
    const char *prompt_info = NULL;
    const char *password = NULL;
    PW_CB_DATA *cb_data = (PW_CB_DATA *)cb_tmp;

    if (cb_data)
    {
        if (cb_data->password)
            password = (const char *)cb_data->password;
        if (cb_data->prompt_info)
            prompt_info = cb_data->prompt_info;
    }

    if (password)
    {
        res = strlen(password);
        if (res > bufsiz)
            res = bufsiz;
        memcpy(buf, password, res);
        return res;
    }

    ui = UI_new_method(ui_method);
    if (ui)
    {
        int ok = 0;
        char *buff = NULL;
        int ui_flags = 0;
        char *prompt = NULL;

        prompt = UI_construct_prompt(ui, "pass phrase",
                                     prompt_info);

        ui_flags |= UI_INPUT_FLAG_DEFAULT_PWD;
        UI_ctrl(ui, UI_CTRL_PRINT_ERRORS, 1, 0, 0);

        if (ok >= 0)
            ok = UI_add_input_string(ui,prompt,ui_flags,buf,
                                     PW_MIN_LENGTH,BUFSIZ-1);
        if (ok >= 0 && verify)
        {
            buff = (char *)OPENSSL_malloc(bufsiz);
            ok = UI_add_verify_string(ui,prompt,ui_flags,buff,
                                      PW_MIN_LENGTH,BUFSIZ-1, buf);
        }
        if (ok >= 0)
            do
            {
                ok = UI_process(ui);
            }
            while (ok < 0 && UI_ctrl(ui, UI_CTRL_IS_REDOABLE, 0, 0, 0));

        if (buff)
        {
            OPENSSL_cleanse(buff,(unsigned int)bufsiz);
            OPENSSL_free(buff);
        }

        if (ok >= 0)
            res = strlen(buf);
        if (ok == -1)
        {
            BIO_printf(bio_err, "User interface error\n");
            ERR_print_errors(bio_err);
            OPENSSL_cleanse(buf,(unsigned int)bufsiz);
            res = 0;
        }
        if (ok == -2)
        {
            BIO_printf(bio_err,"aborted!\n");
            OPENSSL_cleanse(buf,(unsigned int)bufsiz);
            res = 0;
        }
        UI_free(ui);
        OPENSSL_free(prompt);
    }
    return res;
}

static EVP_PKEY *load_key(BIO *err, const char *file, int format, int maybe_stdin,
                          const char *pass, ENGINE *e, const char *key_descrip)
{
    BIO *key=NULL;
    EVP_PKEY *pkey=NULL;
    PW_CB_DATA cb_data;

    cb_data.password = pass;
    cb_data.prompt_info = file;

    if (file == NULL && (!maybe_stdin || format == FORMAT_ENGINE))
    {
        BIO_printf(err,"no keyfile specified\n");
        goto end;
    }
    key=BIO_new(BIO_s_file());
    if (key == NULL)
    {
        ERR_print_errors(err);
        goto end;
    }
    if (file == NULL && maybe_stdin)
    {
        setvbuf(stdin, NULL, _IONBF, 0);
        BIO_set_fp(key,stdin,BIO_NOCLOSE);
    }
    else
        if (BIO_read_filename(key,file) <= 0)
        {
            BIO_printf(err, "Error opening %s %s\n",
                       key_descrip, file);
            ERR_print_errors(err);
            goto end;
        }
    if (format == FORMAT_ASN1)
    {
        pkey=d2i_PrivateKey_bio(key, NULL);
    }
    else if (format == FORMAT_PEM)
    {
        pkey=PEM_read_bio_PrivateKey(key,NULL,
                                     (pem_password_cb *)password_callback, &cb_data);
    }
    else
    {
        BIO_printf(err,"bad input format specified for key file\n");
        goto end;
    }
end:
    if (key != NULL) BIO_free(key);
    if (pkey == NULL)
        BIO_printf(err,"unable to load %s\n", key_descrip);
    return(pkey);
}

static EVP_PKEY *load_key_from_buffer(BIO *err, const char *buf, unsigned len, int format, int maybe_stdin,
                                      const char *pass, ENGINE *e, const char *key_descrip)
{
    BIO *key=NULL;
    EVP_PKEY *pkey=NULL;
    PW_CB_DATA cb_data;

    cb_data.password = pass;
    cb_data.prompt_info = NULL;

    if (buf == NULL && (!maybe_stdin || format == FORMAT_ENGINE))
    {
        BIO_printf(err,"no key buffer specified\n");
        goto end;
    }
    key=BIO_new(BIO_s_mem());
    if (key == NULL)
    {
        ERR_print_errors(err);
        goto end;
    }

    if (BIO_write(key,buf,len) <= 0)
    {
        BIO_printf(err, "Error write from buf %s\n",
                   key_descrip);
        ERR_print_errors(err);
        goto end;
    }

    if (format == FORMAT_ASN1)
    {
        pkey=d2i_PrivateKey_bio(key, NULL);
    }
    else if (format == FORMAT_PEM)
    {
        pkey=PEM_read_bio_PrivateKey(key,NULL,
                                     (pem_password_cb *)password_callback, &cb_data);
    }
    else
    {
        BIO_printf(err,"bad input format specified for key file\n");
        goto end;
    }
end:
    if (key != NULL) BIO_free(key);
    if (pkey == NULL)
        BIO_printf(err,"unable to load %s\n", key_descrip);
    return(pkey);
}


//int main(int argc, char* argv[])
int rsa_decrypt( const char *key_fn, unsigned char in[256], unsigned char out[256])
{
    RSA *rsa=NULL;
    BIO *output=NULL;
    int informat = FORMAT_PEM;
    ENGINE *e = NULL;
    char *passin = NULL;
    int sgckey=0;

    int ret;

    if(bio_err == NULL)
    {
        if((bio_err=BIO_new(BIO_s_file())) != NULL)
            BIO_set_fp(bio_err,stderr,BIO_NOCLOSE|BIO_FP_TEXT);
    }

    output=BIO_new(BIO_s_file());
    BIO_set_fp(output,stdout,BIO_NOCLOSE);

    {
        EVP_PKEY	*pkey;

        pkey = load_key(bio_err, key_fn,
                        (informat == FORMAT_NETSCAPE && sgckey ?
                         FORMAT_IISSGC : informat), 1,
                        passin, e, "Private Key");

        if (pkey != NULL)
            rsa = pkey == NULL ? NULL : EVP_PKEY_get1_RSA(pkey);
        EVP_PKEY_free(pkey);
    }

   /*  if (!RSA_print(output,rsa,0)) */
/*     { */
/*         ERR_print_errors(bio_err); */
/*         goto cleanup; */
/*     } */

    CRYPTO_malloc_debug_init();
    CRYPTO_dbg_set_options(V_CRYPTO_MDEBUG_ALL);
    CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);

    RAND_seed(rnd_seed, sizeof rnd_seed); /* or OAEP may fail */

    /* dec */
    {
        //fprintf(stderr, "Begin decrypt...\n");

        ret = RSA_private_decrypt(256, in, out, rsa, RSA_PKCS1_OAEP_PADDING);
        if( ret < 0)
        {
            unsigned long err;
            const char *file;
            int line;

            err = ERR_get_error_line(&file, &line);
            if(err == 0)
            {
                fprintf(stderr, "no err at all\n");
            }
            else
            {
                fprintf(stderr, "RSA_private_decrypt:%s\n", ERR_error_string(err, NULL));
                fprintf(stderr, "err file:%s, line:%d\n", file, line);
            }
            //fprintf(stderr, "Decrypt failed!\n");
        }
        else
        {
            //fprintf(stderr, "OAEP decrypted size=%d\n", ret);
        }

    }

    if(output != NULL) BIO_free_all(output);
    if(rsa != NULL) RSA_free(rsa);

    CRYPTO_cleanup_all_ex_data();
    ERR_remove_state(0);

    CRYPTO_mem_leaks_fp(stderr);
    return ret;
}

int rsa_decrypt_from_buffer( const char *buf, unsigned int len, unsigned char in[256], unsigned char out[256])
{
    RSA *rsa=NULL;
    BIO *output=NULL;
    int informat = FORMAT_PEM;
    ENGINE *e = NULL;
    char *passin = NULL;
    int sgckey=0;

    int ret;

    if(bio_err == NULL)
    {
        if((bio_err=BIO_new(BIO_s_file())) != NULL)
            BIO_set_fp(bio_err,stderr,BIO_NOCLOSE|BIO_FP_TEXT);
    }

    output=BIO_new(BIO_s_file());
    BIO_set_fp(output,stdout,BIO_NOCLOSE);

    {
        EVP_PKEY	*pkey;

        pkey = load_key_from_buffer(bio_err, buf, len,
                                    (informat == FORMAT_NETSCAPE && sgckey ?
                                     FORMAT_IISSGC : informat), 1,
                                    passin, e, "Private Key");

        if (pkey != NULL)
            rsa = pkey == NULL ? NULL : EVP_PKEY_get1_RSA(pkey);
        EVP_PKEY_free(pkey);
    }

   /*  if (!RSA_print(output,rsa,0)) */
/*     { */
/*         ERR_print_errors(bio_err); */
/*         goto cleanup; */
/*     } */

    CRYPTO_malloc_debug_init();
    CRYPTO_dbg_set_options(V_CRYPTO_MDEBUG_ALL);
    CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);

    RAND_seed(rnd_seed, sizeof rnd_seed); /* or OAEP may fail */

    /* dec */
    {
        //fprintf(stderr, "Begin decrypt...\n");

        ret = RSA_private_decrypt(256, in, out, rsa, RSA_PKCS1_OAEP_PADDING);
        if( ret < 0)
        {
            unsigned long err;
            const char *file;
            int line;

            err = ERR_get_error_line(&file, &line);
            if(err == 0)
            {
                fprintf(stderr, "no err at all\n");
            }
            else
            {
                fprintf(stderr, "RSA_private_decrypt:%s\n", ERR_error_string(err, NULL));
                fprintf(stderr, "err file:%s, line:%d\n", file, line);
            }
            //fprintf(stderr, "Decrypt failed!\n");
        }
        else
        {
            //fprintf(stderr, "OAEP decrypted size=%d\n", ret);
        }

    }

    if(output != NULL) BIO_free_all(output);
    if(rsa != NULL) RSA_free(rsa);

    CRYPTO_cleanup_all_ex_data();
    ERR_remove_state(0);

    CRYPTO_mem_leaks_fp(stderr);
    return ret;
}

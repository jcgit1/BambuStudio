// OpenSSL 4 removed the ENGINE API. The deps-built curl-7.75 openssl backend
// still references it. These stubs make every ENGINE_* path a no-op failure,
// which curl handles gracefully (engine support simply unavailable).
// Built without OpenSSL headers; signatures are ABI-compatible.
#include <stddef.h>
#include <dlfcn.h>

typedef struct engine_st ENGINE;
typedef struct evp_pkey_st EVP_PKEY;
typedef struct x509_st X509;
typedef struct ssl_st SSL;

int ENGINE_set_default(ENGINE *e, unsigned flags) { (void)e; (void)flags; return 0; }
void ENGINE_free(ENGINE *e) { (void)e; }
void ENGINE_finish(ENGINE *e) { (void)e; }
ENGINE *ENGINE_by_id(const char *id) { (void)id; return NULL; }
ENGINE *ENGINE_get_first(void) { return NULL; }
ENGINE *ENGINE_get_next(ENGINE *e) { (void)e; return NULL; }
const char *ENGINE_get_id(const ENGINE *e) { (void)e; return NULL; }
int ENGINE_ctrl(ENGINE *e, int cmd, long i, void *p, void (*f)(void)) { (void)e;(void)cmd;(void)i;(void)p;(void)f; return 0; }
int ENGINE_ctrl_cmd(ENGINE *e, const char *cmd_name, long i, void *p, void (*f)(void), int cmd_optional) { (void)e;(void)cmd_name;(void)i;(void)p;(void)f;(void)cmd_optional; return 0; }
EVP_PKEY *ENGINE_load_private_key(ENGINE *e, const char *key_id, void *ui_data) { (void)e;(void)key_id;(void)ui_data; return NULL; }

X509 *SSL_get_peer_certificate(const SSL *s) {
    static X509 *(*real)(const SSL *);
    if (!real) real = (X509 *(*)(const SSL *))dlsym(RTLD_DEFAULT, "SSL_get1_peer_certificate");
    return real ? real(s) : NULL;
}
int EVP_PKEY_id(const EVP_PKEY *pkey) {
    static int (*real)(const EVP_PKEY *);
    if (!real) real = (int (*)(const EVP_PKEY *))dlsym(RTLD_DEFAULT, "EVP_PKEY_get_id");
    return real ? real(pkey) : -1;
}

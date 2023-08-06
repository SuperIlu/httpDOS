#ifndef __MICRO_HTTP_SERVER_H__
#define __MICRO_HTTP_SERVER_H__

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "ini.h"

#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"
#include "mbedtls/platform.h"
#include "mbedtls/entropy_poll.h"

#ifndef HTTP_SERVER
#define HTTP_SERVER "Micro CHTTP Server"
#endif

#ifndef MAX_HEADER_FIELDS
#define MAX_HEADER_FIELDS 20
#endif

typedef struct _HTTPHeaderField {
    char *key;
    char *value;
} HTTPHeaderField;

typedef enum { HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_DELETE, HTTP_NUM_METHOD } HTTPMethod;

typedef struct _HTTPReqHeader {
    HTTPMethod Method;
    char *URI;
    char *Version;
    HTTPHeaderField Fields[MAX_HEADER_FIELDS];
    unsigned int Amount;
} HTTPReqHeader;

typedef struct _HTTPReqMessage {
    HTTPReqHeader Header;
    uint8_t *Body;
    uint8_t *_buf;
    uint16_t _index;
} HTTPReqMessage;

typedef struct _HTTPResHeader {
    char *Version;
    char *StatusCode;
    char *Description;
    HTTPHeaderField Fields[MAX_HEADER_FIELDS];
    unsigned int Amount;
} HTTPResHeader;

typedef struct _HTTPResMessage {
    HTTPResHeader Header;
    uint8_t *Body;
    uint8_t *_buf;
    uint16_t _index;
} HTTPResMessage;

typedef void (*SOCKET_CALLBACK)(void *);

typedef struct _HTTPReq {
    mbedtls_net_context client_fd;
    HTTPReqMessage req;
    HTTPResMessage res;
    SOCKET_CALLBACK OnRead;
    SOCKET_CALLBACK EndRead;
    SOCKET_CALLBACK OnWrite;
    SOCKET_CALLBACK EndWrite;
    size_t rindex;
    size_t windex;
    uint8_t work_state;
} HTTPReq;

typedef struct _HTTPServer {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt srvcert;
    mbedtls_pk_context pkey;
    mbedtls_net_context listen_fd;
    HTTPReq http_req;
    int maxheader;
    int maxbody;
    size_t maxsize;
    ini_t *ini;
    char *static_folder;
} HTTPServer;

typedef void (*HTTPREQ_CALLBACK)(HTTPServer *, HTTPReqMessage *, HTTPResMessage *);

bool HTTPServerInit(HTTPServer *, ini_t *inifile);
void HTTPServerRun(HTTPServer *, HTTPREQ_CALLBACK);
#define HTTPServerRunLoop(srv, callback)  \
    {                                     \
        while (1) {                       \
            HTTPServerRun(srv, callback); \
        }                                 \
    }
void HTTPServerClose(HTTPServer *);

#ifdef DEBUG_MSG
#include <stdio.h>
#define DebugMsg(...) (printf(__VA_ARGS__))
#else
#define DebugMsg(...)
#endif

#endif

#include <string.h>
#include <errno.h>
#include "server.h"
#include "main.h"

#include <arpa/inet.h>
#include <sys/socket.h>

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

#define NOTWORK_SOCKET 0
#define READING_SOCKET 1
#define READEND_SOCKET 2
#define WRITING_SOCKET 4
#define WRITEEND_SOCKET 8
#define CLOSE_SOCKET 128
#define IsReqReading(s) (s == READING_SOCKET)
#define IsReqWriting(s) (s == WRITING_SOCKET)
#define IsReqReadEnd(s) (s == READEND_SOCKET)
#define IsReqWriteEnd(s) (s == WRITEEND_SOCKET)
#define IsReqClose(s) (s == CLOSE_SOCKET)

static void my_debug(void *ctx, int level, const char *file, int line, const char *str) {
    ((void)level);
    LOGF("%s:%04d: %s", file, line, str);
}

bool HTTPServerInit(HTTPServer *srv, ini_t *inifile) {
    int ret;
    const char *pers = "ssl_server";

    mbedtls_net_init(&srv->listen_fd);
    mbedtls_net_init(&srv->http_req.client_fd);
    mbedtls_ssl_init(&srv->ssl);
    mbedtls_ssl_config_init(&srv->conf);
    mbedtls_x509_crt_init(&srv->srvcert);
    mbedtls_pk_init(&srv->pkey);
    mbedtls_entropy_init(&srv->entropy);
    mbedtls_ctr_drbg_init(&srv->ctr_drbg);

    srv->static_folder = (char *)ini_get(inifile, "dirs", "static");
    if (!srv->static_folder) {
        LOGF("No static folder configured!\n");
        return false;
    }

    int debug = 0;
    ini_sget(inifile, "logging", "level", "%d", &debug);
    mbedtls_debug_set_threshold(debug);
    LOGF("Loglevel is %d\n", debug);

    // 1. Load the certificates and private RSA key
    LOGF("Loading the server cert. and key...\n");
#if CERT_FILES == 1
    const char *val;

    val = ini_get(inifile, "httpd", "cert");
    if (!val) {
        LOGF("server certificate missing in INI");
        return false;
    }
    ret = mbedtls_x509_crt_parse_file(&srv->srvcert, val);
    if (ret != 0) {
        LOGF(" failed: mbedtls_x509_crt_parse returned %d\n", ret);
        return false;
    }

    val = ini_get(inifile, "httpd", "ca");
    if (!val) {
        LOGF("CA certificate missing in INI");
        return false;
    }
    ret = mbedtls_x509_crt_parse_file(&srv->srvcert, val);
    if (ret != 0) {
        LOGF(" failed: mbedtls_x509_crt_parse returned %d\n", ret);
        return false;
    }

    val = ini_get(inifile, "httpd", "key");
    if (!val) {
        LOGF("certificate key missing in INI");
        return false;
    }
    ret = mbedtls_pk_parse_keyfile(&srv->pkey, val, NULL);
    if (ret != 0) {
        LOGF(" failed: mbedtls_pk_parse_key returned %d\n", ret);
        return false;
    }
#else
    ret = mbedtls_x509_crt_parse(&srv->srvcert, (const unsigned char *)mbedtls_test_srv_crt, mbedtls_test_srv_crt_len);
    if (ret != 0) {
        LOGF(" failed: mbedtls_x509_crt_parse returned %d\n", ret);
        return false;
    }

    ret = mbedtls_x509_crt_parse(&srv->srvcert, (const unsigned char *)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
    if (ret != 0) {
        LOGF(" failed: mbedtls_x509_crt_parse returned %d\n", ret);
        return false;
    }

    ret = mbedtls_pk_parse_key(&srv->pkey, (const unsigned char *)mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0);
    if (ret != 0) {
        LOGF(" failed: mbedtls_pk_parse_key returned %d\n", ret);
        return false;
    }
#endif
    LOGF(" ok\n");

    // 2. Setup the listening TCP socket
    const char *port = ini_get(inifile, "httpd", "port");
    if (!port) {
        port = "443";
    }
    LOGF("Bind on port %s ...", port);
    if ((ret = mbedtls_net_bind(&srv->listen_fd, NULL, port, MBEDTLS_NET_PROTO_TCP)) != 0) {
        LOGF(" failed: mbedtls_net_bind returned %d\n", ret);
        return false;
    }
    LOGF(" ok\n");

    // 3. Seed the RNG
    LOGF("Seeding the random number generator...");
    if ((ret = mbedtls_ctr_drbg_seed(&srv->ctr_drbg, mbedtls_entropy_func, &srv->entropy, (const unsigned char *)pers, strlen(pers))) != 0) {
        LOGF(" failed: mbedtls_ctr_drbg_seed returned %d\n", ret);
        return false;
    }
    LOGF(" ok\n");

    // 4. Setup stuff
    LOGF("Setting up the SSL data....");
    if ((ret = mbedtls_ssl_config_defaults(&srv->conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        LOGF(" failed: mbedtls_ssl_config_defaults returned %d\n", ret);
        return false;
    }

    mbedtls_ssl_conf_rng(&srv->conf, mbedtls_ctr_drbg_random, &srv->ctr_drbg);
    mbedtls_ssl_conf_dbg(&srv->conf, my_debug, stdout);

    mbedtls_ssl_conf_ca_chain(&srv->conf, srv->srvcert.next, NULL);
    if ((ret = mbedtls_ssl_conf_own_cert(&srv->conf, &srv->srvcert, &srv->pkey)) != 0) {
        LOGF(" failed: mbedtls_ssl_conf_own_cert returned %d\n", ret);
        return false;
    }

    if ((ret = mbedtls_ssl_setup(&srv->ssl, &srv->conf)) != 0) {
        LOGF(" failed: mbedtls_ssl_setup returned %d\n", ret);
        return false;
    }
    LOGF(" ok\n");

    /* Prepare the HTTP client requests. */
    srv->maxheader = 2048;
    ini_sget(inifile, "memory", "maxheader", "%d", &srv->maxheader);
    LOGF("MAX header size %d\n", srv->maxheader);

    srv->maxbody = 1024 * 1024;
    ini_sget(inifile, "memory", "maxbody", "%d", &srv->maxbody);
    LOGF("MAX body size %d\n", srv->maxbody);

    srv->maxsize = srv->maxheader + srv->maxbody;
    srv->http_req.req._buf = malloc(srv->maxsize);
    srv->http_req.res._buf = malloc(srv->maxsize);
    srv->http_req.work_state = NOTWORK_SOCKET;
    srv->ini = inifile;

    if (!srv->http_req.req._buf || !srv->http_req.res._buf) {
        LOGF("Not enough memory for buffers");
        return false;
    }

    return true;
}

static bool _HTTPServerAccept(HTTPServer *srv) {
    int ret;
    struct sockaddr_in cli_addr;
    socklen_t sockaddr_inlen = sizeof(cli_addr);
    size_t sockaddr_outlen;

reset:
    // cleanup
    mbedtls_net_free(&srv->http_req.client_fd);
    mbedtls_ssl_session_reset(&srv->ssl);

    // 3. Wait until a client connects
    LOGF("Waiting for a remote connection ...");
    if ((ret = mbedtls_net_accept(&srv->listen_fd, &srv->http_req.client_fd, &cli_addr, sockaddr_inlen, &sockaddr_outlen)) != 0) {
        LOGF(" failed: mbedtls_net_accept returned %d\n", ret);
        return false;
    }
    mbedtls_ssl_set_bio(&srv->ssl, &srv->http_req.client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);
    LOGF(" ok\n");

    // 5. Handshake
    mbedtls_printf("Performing the SSL/TLS handshake...");
    while ((ret = mbedtls_ssl_handshake(&srv->ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            LOGF(" failed: mbedtls_ssl_handshake returned %d\n", ret);
            goto reset;
        }
    }
    LOGF(" ok\n");

    LOGF("Accept 1 client %s:%d with %s\n", inet_ntoa(cli_addr.sin_addr), (int)ntohs(cli_addr.sin_port), mbedtls_ssl_get_ciphersuite(&srv->ssl));
    srv->http_req.req.Header.Amount = 0;
    srv->http_req.res.Header.Amount = 0;
    srv->http_req.rindex = 0;
    srv->http_req.windex = 0;

    return true;
}

static int _CheckLine(char *buf) {
    int i = 0;

    if (buf[i] == '\n') {
        if (buf[i - 1] == '\r')
            i = 2;
        else
            i = 1;
    }

    return i;
}

static int _CheckFieldSep(char *buf) {
    int i = 0;

    if ((buf[i - 1] == ':') && (buf[i] == ' ')) {
        i = 2;
    }

    return i;
}

static HTTPMethod HaveMethod(char *method) {
    HTTPMethod m;

    if (memcmp(method, "GET", 3) == 0)
        m = HTTP_GET;
    else if (memcmp(method, "POST", 4) == 0)
        m = HTTP_POST;
    else if (memcmp(method, "PUT", 3) == 0)
        m = HTTP_PUT;
    else if (memcmp(method, "DELETE", 6) == 0)
        m = HTTP_DELETE;
    else
        m = HTTP_GET;

    return m;
}

static void _WriteSock(HTTPServer *srv) {
    HTTPReq *hr = &srv->http_req;
    ssize_t n;

    // 7. Write to client
    while (true) {
        const unsigned char *buf = hr->res._buf + hr->windex;
        size_t len = hr->res._index - hr->windex;
        n = mbedtls_ssl_write(&srv->ssl, buf, len);
        LOGF("ssl_write(%ld) := %ld\n", len, n);
        if (n > 0) {
            /* Send some bytes and send left next loop. */
            hr->windex += n;
            if (hr->res._index > hr->windex) {
                hr->work_state = WRITING_SOCKET;
            } else {
                hr->work_state = WRITEEND_SOCKET;
            }
            continue;
        } else if (n == 0) {
            /* Writing is finished. */
            hr->work_state = WRITEEND_SOCKET;
        } else if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
            /* Send with non-blocking socket. */
            hr->windex += hr->res._index - hr->windex;
            hr->work_state = WRITING_SOCKET;
        } else {
            /* Send with error. */
            if (n == MBEDTLS_ERR_NET_CONN_RESET) {
                LOGF(" failed: peer closed the connection\n");
            } else if (n != MBEDTLS_ERR_SSL_WANT_READ && n != MBEDTLS_ERR_SSL_WANT_WRITE) {
                continue;  // call SSL again
            }
            hr->work_state = CLOSE_SOCKET;
        }
        break;
    }
}

static int mbed_recv(HTTPServer *srv, void *buf, int len) {
    int ret;
    do {
        ret = mbedtls_ssl_read(&srv->ssl, buf, len);

        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            continue;
        }

        if (ret <= 0) {
            switch (ret) {
                case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
                    LOGF("connection was closed gracefully\n");
                    break;

                case MBEDTLS_ERR_NET_CONN_RESET:
                    LOGF("connection was reset by peer\n");
                    break;

                default:
                    LOGF("mbedtls_ssl_read returned -0x%x\n", (unsigned int)-ret);
                    break;
            }

            break;
        }

        len = ret;

        if (ret > 0) {
            break;
        }
    } while (1);
    return ret;
}

static int _ParseHeader(HTTPServer *srv) {
    HTTPReq *hr = &srv->http_req;
    HTTPReqMessage *req = &(hr->req);
    int n;
    int l, end;
    int i = 0;
    char *p;

    p = (char *)req->_buf;
    /* GET, PUT ... and a white space are 3 charaters. */
    n = mbed_recv(srv, p, 3);
    if (n == 3) {
        /* Parse method. */
        for (i = 3; n > 0; i++) {
            n = mbed_recv(srv, p + i, 1);
            if (p[i] == ' ') {
                p[i] = '\0';
                break;
            }
        }
        req->Header.Method = HaveMethod(p);

        /* Parse URI. */
        if (n > 0) i += 1;
        req->Header.URI = p + i;
        for (; n > 0; i++) {
            n = mbed_recv(srv, p + i, 1);
            if (p[i] == ' ') {
                p[i] = '\0';
                break;
            }
        }

        /* Parse HTTP version. */
        if (n > 0) i += 1;
        req->Header.Version = p + i;
        /* HTTP/1.1 has 8 charaters. */
        n = mbed_recv(srv, p + i, 8);
        for (i += 8; (n > 0) && (i < srv->maxheader); i++) {
            n = mbed_recv(srv, p + i, 1);
            if ((l = _CheckLine(p + i))) {
                if (l == 2) p[i - 1] = '\0';
                p[i] = '\0';
                break;
            }
        }

        /* Parse other fields. */
        if (n > 0) i += 1;
        req->Header.Fields[req->Header.Amount].key = p + i;
        end = 0;
        for (; (n > 0) && (i < srv->maxheader) && (req->Header.Amount < MAX_HEADER_FIELDS); i++) {
            n = mbed_recv(srv, p + i, 1);
            /* Check field key name end. */
            if ((l = _CheckFieldSep(p + i))) {
                p[i - 1] = '\0';
                req->Header.Fields[req->Header.Amount].value = p + i + 1;
            }

            /* Check header end. */
            if ((l = _CheckLine(p + i))) {
                if (end == 0) {
                    if (l == 2) p[i - 1] = '\0';
                    p[i] = '\0';

                    /* CRLF have 2 characters, so check 2 times new line. */
                    end = 2;

                    /* Go to parse next header field. */
                    req->Header.Amount += 1;
                    req->Header.Fields[req->Header.Amount].key = p + i + 1;
                } else {
                    /* Requset message header finished. */
                    break;
                }
            } else {
                if (end > 0) end -= 1;
            }
        }
    }
    if (n < 0) {
        hr->work_state = CLOSE_SOCKET;
    }

    req->_index = (n > 0) ? i + 1 : i;
    return i;
}

static int _IsLengthHeader(char *key) {
    const char *len_header = "content-length";

    return !strncasecmp(key, len_header, strlen(len_header));
}

static int _GetBody(HTTPServer *srv) {
    HTTPReq *hr = &srv->http_req;
    HTTPReqMessage *req = &(hr->req);
    int n = 1;
    unsigned int i = 0;
    int c = 0, len = 0;
    uint8_t *p;

    req->Body = req->_buf + req->_index;

    if (req->Header.Method == HTTP_POST) {
        for (i = 0; i < req->Header.Amount; i++) {
            if (_IsLengthHeader(req->Header.Fields[i].key)) {
                len = atoi(req->Header.Fields[i].value);
                break;
            }
        }
        p = req->Body;
        if (len >= srv->maxbody) {
            len = srv->maxbody - 1;
        }
        for (c = 0; (n > 0) && (c < len); c += n) {
            n = mbed_recv(srv, p + c, (len - c));
        }
    }

    req->Body[c] = '\0';

    return (n < 0) ? -1 : c;
}

static void _HTTPServerRequest(HTTPServer *srv, HTTPREQ_CALLBACK callback) {
    HTTPReq *hr = &srv->http_req;
    int n;

    hr->work_state = READING_SOCKET;
    n = _ParseHeader(srv);
    if (n > 0) {
        n = _GetBody(srv);
        if (n >= 0) {
            callback(srv, &(hr->req), &(hr->res));
            /* Write all response. */
            hr->work_state = WRITING_SOCKET;
        } else {
            hr->work_state = CLOSE_SOCKET;
        }
    } else {
        hr->work_state = CLOSE_SOCKET;
    }
}

void HTTPServerRun(HTTPServer *srv, HTTPREQ_CALLBACK callback) {
    int ret;

    if (!_HTTPServerAccept(srv)) {
        return;
    }
    _HTTPServerRequest(srv, callback);
    _WriteSock(srv);

    if (IsReqWriteEnd(srv->http_req.work_state)) {
        srv->http_req.work_state = CLOSE_SOCKET;
    }
    if (IsReqClose(srv->http_req.work_state)) {
        LOGF("Closing the connection...");

        while ((ret = mbedtls_ssl_close_notify(&srv->ssl)) < 0) {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
                LOGF("failed: mbedtls_ssl_close_notify returned %d\n", ret);
                break;
            }
        }
        LOGF("ok\n");

        srv->http_req.work_state = NOTWORK_SOCKET;
    }
}

void HTTPServerClose(HTTPServer *srv) {
    mbedtls_net_free(&srv->http_req.client_fd);
    mbedtls_net_free(&srv->listen_fd);
    mbedtls_x509_crt_free(&srv->srvcert);
    mbedtls_pk_free(&srv->pkey);
    mbedtls_ssl_free(&srv->ssl);
    mbedtls_ssl_config_free(&srv->conf);
    mbedtls_ctr_drbg_free(&srv->ctr_drbg);
    mbedtls_entropy_free(&srv->entropy);
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if LINUX == 0
#include <dos.h>
#include <pc.h>
#include <tcp.h>
#endif

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

#include "main.h"
#include "server.h"
#include "middleware.h"

/* The HTTP server of this process. */
HTTPServer srv;

FILE *logfile;  //!< file for log output.

// add random n data to buffer b and icrement size counter p
#define CURL_ADD_RANDOM(n, p, b)                        \
    {                                                   \
        if (p + sizeof(n) < sizeof(b)) {                \
            memcpy(&b[p], (const void *)&n, sizeof(n)); \
            p += sizeof(n);                             \
        }                                               \
    }

#if LINUX == 0
static int poll_noise_sys(void *data, unsigned char *output, size_t len, size_t *olen) {
    FILE *file;
    size_t ret, left = len;
    unsigned char *p = output;
    ((void)data);

    *olen = 0;

    file = fopen("/dev/urandom$", "rb");
    if (file == NULL) {
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }

    while (left > 0) {
        /* /dev/random can return much less than requested. If so, try again */
        ret = fread(p, 1, left, file);
        if (ret == 0 && ferror(file)) {
            fclose(file);
            return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
        }

        p += ret;
        left -= ret;
        sleep(1);
    }
    fclose(file);
    *olen = len;

    return 0;
}
#endif

int mbedtls_hardware_poll(void *data, unsigned char *output, size_t len, size_t *olen) {
    LOGF("%ld bytes random data requested\n", len);

#if LINUX == 0
    int noise_ret = poll_noise_sys(data, output, len, olen);
    if (noise_ret != 0) {
        LOGF("using fallback pseudo RNG\n");
        uint8_t rnd_buff[5 * 4];
        unsigned int pos = 0;
        rnd_buff[pos++] = inportb(0x40);  // PIT timer 0 at ports 40h-43h
        rnd_buff[pos++] = inportb(0x41);
        rnd_buff[pos++] = inportb(0x42);
        rnd_buff[pos++] = inportb(0x43);
        time_t t = time(NULL);
        CURL_ADD_RANDOM(t, pos, rnd_buff);

        // find smaller of the two
        *olen = pos < len ? pos : len;

        // copy to output buffer
        memcpy(output, rnd_buff, *olen);
    } else {
        LOGF("using NOISE.SYS\n");
    }
#else
    FILE *file;
    size_t ret, left = len;
    unsigned char *p = output;
    ((void)data);

    *olen = 0;

    file = fopen("/dev/random", "rb");
    if (file == NULL) {
        return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }

    while (left > 0) {
        /* /dev/random can return much less than requested. If so, try again */
        ret = fread(p, 1, left, file);
        if (ret == 0 && ferror(file)) {
            fclose(file);
            return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
        }

        p += ret;
        left -= ret;
        sleep(1);
    }
    fclose(file);
    *olen = len;

    return 0;
#endif
    LOGF("%ld bytes random data delivered\n", *olen);

    // always success
    return 0;
}

int main(void) {
    LOG("This is httpDOS 0.0.3 (https://github.com/SuperIlu/httpDOS)\n");
    LOG("(c) 2024 by Andre Seidelt <superilu@yahoo.com> and others.\n");
    LOG("See README.md for detailed licensing information.\n");

#if LINUX == 0
    char buffer[1024];
    _watt_do_exit = 0;
    int err = sock_init();
    if (!err) {
        LOGF("WATTCP init         : %s\n", sock_init_err(err));
        LOGF("WATTCP Address      : %s\n", _inet_ntoa(NULL, _gethostid()));
        LOGF("WATTCP Network Mask : %s\n", _inet_ntoa(NULL, sin_mask));
        gethostname(buffer, sizeof(buffer));
        LOGF("WATTCP hostname     : %s\n", buffer);
        getdomainname(buffer, sizeof(buffer));
        LOGF("WATTCP domainname   : %s\n", buffer);
        LOGF("WATTCP              : %s / %s\n", wattcpVersion(), wattcpCapabilities());
    } else {
        LOGF("WATTCP init: %s\n", sock_init_err(err));
    }
#endif

    ini_t *ini = ini_load("HTTPDOS.INI");
    if (!ini) {
        LOGF("Could not load HTTPDOS.INI!\n");
        exit(1);
    }

    const char *logname = ini_get(ini, "logging", "logfile");
    // create logfile
    if (logname) {
        logfile = fopen(logname, "a");
        if (!logfile) {
            fprintf(stderr, "Could not open/create logfile %s.\n", logname);
            exit(1);
        }
        setbuf(logfile, 0);
    } else {
        logfile = NULL;
    }

    /* Initial the HTTP server and make it listening on MHS_PORT. */
    if (!HTTPServerInit(&srv, ini)) {
        exit(1);
    }
    /* Run the HTTP server forever. */
    /* Run the dispatch callback if there is a new request */
    HTTPServerRunLoop(&srv, Dispatch);
    HTTPServerClose(&srv);
    return 0;
}

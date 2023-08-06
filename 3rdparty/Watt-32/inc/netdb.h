/*!\file netdb.h
 * BSD netdb functions.
 */

/*-
 * Copyright (c) 1980, 1983, 1988, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * -
 * Portions Copyright (c) 1993 by Digital Equipment Corporation.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies, and that
 * the name of Digital Equipment Corporation not be used in advertising or
 * publicity pertaining to distribution of the document or software without
 * specific, written prior permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND DIGITAL EQUIPMENT CORP. DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL DIGITAL EQUIPMENT
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 * -
 * --Copyright--
 */

#ifndef __NETDB_H
#define __NETDB_H

#ifndef _PATH_HEQUIV
#define _PATH_HEQUIV    "/etc/hosts.equiv"
#endif

#ifndef _PATH_HOSTS
#define _PATH_HOSTS     "/etc/hosts"
#endif

#ifndef _PATH_NETWORKS
#define _PATH_NETWORKS  "/etc/networks"
#endif

#ifndef _PATH_PROTOCOLS
#define _PATH_PROTOCOLS "/etc/protocols"
#endif

#ifndef _PATH_SERVICES
#define _PATH_SERVICES  "/etc/services"
#endif

#ifndef __SYS_W32API_H
#include <sys/w32api.h>
#endif

#ifndef __SYS_SOCKET_H
#include <sys/socket.h> /* struct sockaddr */
#endif

#ifndef __SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

#ifndef __SYS_CDEFS_H
#include <sys/w32api.h>
#endif


/*
 * Structures returned by network data base library.  All addresses are
 * supplied in host order, and returned in network order (suitable for
 * use in system calls).
 */
struct hostent {
        char    *h_name;        /* official name of host */
        char    **h_aliases;    /* alias list */
        int     h_addrtype;     /* host address type */
        int     h_length;       /* length of address */
        char    **h_addr_list;  /* list of addresses from name server */
#define h_addr  h_addr_list[0]  /* address, for backward compatiblity */
};

/*
 * Assumption here is that a network number
 * fits in 32 bits -- probably a poor one.
 */
struct netent {
       char            *n_name;        /* official name of net */
       char            **n_aliases;    /* alias list */
       int             n_addrtype;     /* net address type */
       unsigned long   n_net;          /* network # */
     };

struct servent {
       char    *s_name;        /* official service name */
       char    **s_aliases;    /* alias list */
       int     s_port;         /* port # */
       char    *s_proto;       /* protocol to use */
     };

struct protoent {
       char    *p_name;        /* official protocol name */
       char    **p_aliases;    /* alias list */
       int     p_proto;        /* protocol # */
     };

struct addrinfo {
       int     ai_flags;       /* AI_PASSIVE, AI_CANONNAME */
       int     ai_family;      /* PF_xxx */
       int     ai_socktype;    /* SOCK_xxx */
       int     ai_protocol;    /* 0 or IPPROTO_xxx for IPv4 and IPv6 */
       int     ai_addrlen;     /* length of ai_addr */
       char   *ai_canonname;   /* canonical name for hostname */
       struct sockaddr  *ai_addr; /* binary address */
       struct addrinfo  *ai_next; /* next structure in linked list */
     };

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */
W32_DATA int h_errno;

#define NETDB_INTERNAL  -1      /* see errno */
#define NETDB_SUCCESS   0       /* no problem */
#define HOST_NOT_FOUND  1       /* Authoritative Answer Host not found */
#define TRY_AGAIN       2       /* Non-Authoritive Host not found, or SERVERFAIL */
#define NO_RECOVERY     3       /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define NO_DATA         4       /* Valid name, no data record of requested type */
#define NO_ADDRESS      NO_DATA /* no address, look for MX record */


/*
 * Error return codes from getaddrinfo()
 */
#define EAI_ADDRFAMILY   1  /* address family for hostname not supported */
#define EAI_AGAIN        2  /* temporary failure in name resolution */
#define EAI_BADFLAGS     3  /* invalid value for ai_flags */
#define EAI_FAIL         4  /* non-recoverable failure in name resolution */
#define EAI_FAMILY       5  /* ai_family not supported */
#define EAI_MEMORY       6  /* memory allocation failure */
#define EAI_NODATA       7  /* no address associated with hostname */
#define EAI_NONAME       8  /* hostname nor servname provided, or not known */
#define EAI_SERVICE      9  /* servname not supported for ai_socktype */
#define EAI_SOCKTYPE    10  /* ai_socktype not supported */
#define EAI_SYSTEM      11  /* system error returned in errno */
#define EAI_BADHINTS    12
#define EAI_PROTOCOL    13
#define EAI_MAX         14

/*
 * Flag values for getaddrinfo()
 */
#define AI_PASSIVE      0x00000001 /* get address to use bind() */
#define AI_CANONNAME    0x00000002 /* fill ai_canonname */
#define AI_NUMERICHOST  0x00000004 /* prevent name resolution */
#define AI_NUMERICSERV  0x00000008 /* has no effect */
#define AI_IDN          0x00000010 /* convert ti IDN form as needed */

/* valid flags for addrinfo
 */
#define AI_MASK         (AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST | AI_NUMERICSERV | AI_IDN)

#define AI_ALL          0x00000100 /* IPv6 and IPv4-mapped (with AI_V4MAPPED) */
#define AI_V4MAPPED_CFG 0x00000200 /* accept IPv4-mapped if kernel supports */
#define AI_ADDRCONFIG   0x00000400 /* only if any address is assigned */
#define AI_V4MAPPED     0x00000800 /* accept IPv4-mapped IPv6 address */

/* special recommended flags for getipnodebyname
 */
#define AI_DEFAULT      (AI_V4MAPPED_CFG | AI_ADDRCONFIG)

/*
 * Constants for getnameinfo()
 */
#define NI_MAXHOST      1025
#define NI_MAXSERV      32

/*
 * Flag values for getnameinfo()
 */
#define NI_NOFQDN       0x00000001
#define NI_NUMERICHOST  0x00000002
#define NI_NAMEREQD     0x00000004
#define NI_NUMERICSERV  0x00000008
#define NI_DGRAM        0x00000010
#define NI_WITHSCOPEID  0x00000020

/*
 * Scope delimit character
 */
#define SCOPE_DELIMITER '%'

__BEGIN_DECLS

W32_FUNC void              W32_CALL endhostent     (void);
W32_FUNC void              W32_CALL endhostent6    (void);
W32_FUNC void              W32_CALL endnetent      (void);
W32_FUNC void              W32_CALL endprotoent    (void);
W32_FUNC void              W32_CALL endservent     (void);
W32_FUNC struct hostent  * W32_CALL gethostbyaddr  (const char *, socklen_t, int);
W32_FUNC struct hostent  * W32_CALL gethostbyaddr6 (const void *);
W32_FUNC struct hostent  * W32_CALL gethostbyname  (const char *name);
W32_FUNC struct hostent  * W32_CALL gethostbyname2 (const char *name, int af);
W32_FUNC struct hostent  * W32_CALL gethostbyname6 (const char *name);
W32_FUNC struct hostent  * W32_CALL gethostent     (void);
W32_FUNC struct hostent  * W32_CALL gethostent6    (void);
W32_FUNC struct hostent  * W32_CALL getipnodebyaddr(const void *, size_t, int, int *);
W32_FUNC struct hostent  * W32_CALL getipnodebyname(const char *, int, int, int *);
W32_FUNC struct netent   * W32_CALL getnetbyaddr   (long, int);
W32_FUNC struct netent   * W32_CALL getnetbyname   (const char *);
W32_FUNC struct netent   * W32_CALL getnetent      (void);
W32_FUNC struct protoent * W32_CALL getprotobyname (const char *);
W32_FUNC struct protoent * W32_CALL getprotobynumber (int);
W32_FUNC struct protoent * W32_CALL getprotoent    (void);
W32_FUNC struct servent  * W32_CALL getservbyname  (const char *, const char *);
W32_FUNC struct servent  * W32_CALL getservbyport  (int, const char *);
W32_FUNC struct servent  * W32_CALL getservent     (void);
W32_FUNC void              W32_CALL herror         (const char *);
W32_FUNC const char      * W32_CALL hstrerror      (int);
W32_FUNC void              W32_CALL sethostent     (int stayopen);
W32_FUNC void              W32_CALL sethostent6    (int stayopen);
W32_FUNC void              W32_CALL setnetent      (int);
W32_FUNC void              W32_CALL setprotoent    (int);
W32_FUNC void              W32_CALL setservent     (int);

W32_FUNC int    W32_CALL getnameinfo (const struct sockaddr *sa, socklen_t salen,
                                      char *host, socklen_t hostlen,
                                      char *serv, socklen_t servlen, int flags);

W32_FUNC int    W32_CALL getaddrinfo (const char *hostname, const char *servname,
                                      const struct addrinfo *hints,
                                      struct addrinfo **res);

W32_FUNC void   W32_CALL freeaddrinfo (struct addrinfo *ai);
W32_FUNC void   W32_CALL freehostent  (struct hostent *);
W32_FUNC char * W32_CALL gai_strerror (int ecode);

W32_FUNC int    W32_CALL if_nametoindex (const char *);
W32_FUNC char * W32_CALL if_indextoname (int, char *);

W32_FUNC int  * W32_CALL __h_errno_location (void);

/*
 * The implementation of these reentrant 'getXbyY_r()' functions is not
 * finished (ref. ./src/get_xbyr.c). And since MinGW-w64 has '_REENTRANT'
 * as a built-in, we do not provide any prototypes for these yet.
 */
#if defined(_REENTRANT) && !defined(__MINGW64_VERSION_MAJOR)
  W32_FUNC int W32_CALL gethostbyaddr_r (
           const void      *addr, socklen_t len, int type,
           struct hostent  *ret, char *buf, size_t buflen,
           struct hostent **result, int *h_errnop);

  W32_FUNC int W32_CALL gethostbyname_r (
           const char      *name,
           struct hostent  *ret, char *buf, size_t buflen,
           struct hostent **result, int *h_errnop);

  W32_FUNC int W32_CALL gethostent_r (
           struct hostent  *ret, char *buf, size_t buflen,
           struct hostent **result, int *h_errnop);

  W32_FUNC int W32_CALL getnetbyaddr_r (
           u_int32_t net, int type, struct netent *result_buf, char *buf,
           size_t buflen, struct netent **result, int *h_errnop);

  W32_FUNC int W32_CALL getnetbyname_r (
           const char *name, struct netent *result_buf, char *buf,
           size_t buflen, struct netent **result, int *h_errnop);

  W32_FUNC int W32_CALL getnetent_r (
           struct netent *result_buf, char *buf,
           size_t buflen, struct netent **result, int *h_errnop);

  W32_FUNC int W32_CALL getprotobyname_r (
           const char *name, struct protoent *result_buf, char *buf,
           size_t buflen, struct protoent **result);

  W32_FUNC int W32_CALL getprotobynumber_r (
           int proto, struct protoent *result_buf, char *buf,
           size_t buflen, struct protoent **result);

  W32_FUNC int W32_CALL getprotoent_r (
           struct protoent *result_buf, char *buf,
           size_t buflen, struct protoent **result);

  W32_FUNC int W32_CALL getservbyname_r (
           const char *name, const char *proto,
           struct servent *result_buf, char *buf,
           size_t buflen, struct servent **result);

  W32_FUNC int W32_CALL getservbyport_r (
           int port, const char *proto,
           struct servent *result_buf, char *buf,
           size_t buflen, struct servent **result);

  W32_FUNC int W32_CALL getservent_r (
           struct servent *result_buf, char *buf,
           size_t buflen, struct servent **result);

  W32_FUNC int W32_CALL sethostent_r (
                        int stay_open, struct hostent *buffer);

  W32_FUNC int W32_CALL endhostent_r (
                        struct hostent *buffer);

  W32_FUNC int W32_CALL setprotoent_r (
                        int stay_open, struct protoent *buffer);

  W32_FUNC int W32_CALL endprotoent_r (struct protoent *buffer);

#endif /* _REENTRANT && !__MINGW64_VERSION_MAJOR */

__END_DECLS

#endif

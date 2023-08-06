/*!\file sys/socket.h
 *
 * BSD socket API.
 */

/* Modified for emx by hv and em 1994-1995
 *
 * Copyright (c) 1982,1985,1986,1988 Regents of the University of California.
 * All rights reserved.
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
 */

#ifndef __SYS_SOCKET_H
#define __SYS_SOCKET_H

#ifndef __SYS_W32API_H
#include <sys/w32api.h>
#endif

#ifndef __SYS_WERRNO_H
#include <sys/werrno.h>
#endif

#ifndef __SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

#ifndef __SYS_WTYPES_H
#include <sys/wtypes.h>
#endif

#ifndef __SYS_WTIME_H
#include <sys/wtime.h>
#endif

#ifndef __NETINET_IN_H
#include <netinet/in.h>
#endif

/*
 * Definitions related to sockets: types, address families, options.
 */

/* The following constants should be used for the second parameter,
   the "how" field, of `shutdown'.  */
enum
{
  SHUT_RD = 0,          /* Further receives are disallowed */
#define SHUT_RD         SHUT_RD
  SHUT_WR,              /* Further sends are disallowed */
#define SHUT_WR         SHUT_WR
  SHUT_RDWR             /* Further sends and receives are disallowed */
#define SHUT_RDWR       SHUT_RDWR
};

/*
 * This is used instead of -1, since the socket type is signed.
 */
#define INVALID_SOCKET  (int)(~0)
#define SOCKET_ERROR    (-1)

/*
 * Types
 */
#define SOCK_STREAM     1               /* stream socket */
#define SOCK_DGRAM      2               /* datagram socket */
#define SOCK_RAW        3               /* raw-protocol interface */
#define SOCK_RDM        4               /* reliably-delivered message */
#define SOCK_SEQPACKET  5               /* sequenced packet stream */
#define SOCK_PACKET     10              /* linux specific way of        */
                                        /* getting packets at the dev   */
                                        /* level.  For writing rarp and */
                                        /* other similar things on the  */
                                        /* user level.                  */

/*
 * Option flags per-socket.
 */
#define SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SO_LINGER       0x0080          /* linger on close if data present */
#define SO_OOBINLINE    0x0100          /* leave received OOB data in line */

#define SO_DONTLINGER   (int)(~SO_LINGER)

/*
 * Additional options, not kept in so_options.
 */
#define SO_SNDBUF       0x1001          /* send buffer size */
#define SO_RCVBUF       0x1002          /* receive buffer size */
#define SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SO_SNDTIMEO     0x1005          /* send timeout */
#define SO_RCVTIMEO     0x1006          /* receive timeout */
#define SO_ERROR        0x1007          /* get error status and clear */
#define SO_TYPE         0x1008          /* get socket type */

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

/*
 * Structure used for manipulating linger option.
 */
struct linger {
       int l_onoff;                /* option on/off */
       int l_linger;               /* linger time */
     };

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define SOL_SOCKET      0xffff          /* options for socket level */

/*
 * Address families.
 */
#define AF_UNSPEC       0               /* unspecified */
#define AF_UNIX         1               /* local to host (pipes, portals) */
#define AF_LOCAL        1               /* POSIX name for AF_UNIX */
#define AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define AF_IMPLINK      3               /* arpanet imp addresses */
#define AF_PUP          4               /* pup protocols: e.g. BSP */
#define AF_CHAOS        5               /* mit CHAOS protocols */
#define AF_NS           6               /* XEROX NS protocols */
#define AF_ISO          7               /* ISO protocols */
#define AF_OSI          AF_ISO
#define AF_ECMA         8               /* european computer manufacturers */
#define AF_DATAKIT      9               /* datakit protocols */
#define AF_CCITT        10              /* CCITT protocols, X.25 etc */
#define AF_SNA          11              /* IBM SNA */
#define AF_DECnet       12              /* DECnet */
#define AF_DLI          13              /* DEC Direct data link interface */
#define AF_LAT          14              /* LAT */
#define AF_HYLINK       15              /* NSC Hyperchannel */
#define AF_APPLETALK    16              /* Apple Talk */
#define AF_ROUTE        17              /* Internal Routing Protocol */
#define AF_LINK         18              /* Link layer interface */
#define pseudo_AF_XTP   19              /* eXpress Transfer Protocol (no AF) */
#define AF_INET6        24              /* IPv6 address family */
#define AF_PACKET       25              /* raw packets */
#define AF_MAX          26

/*
 * Structure used by kernel to store most
 * addresses.
 * is called struct osockaddr in 4.4BSD
 */
struct sockaddr {
       sa_family_t sa_family;              /* address family */
       char        sa_data[14];            /* up to 14 bytes of direct address */
     };

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
       u_short sp_family;              /* address family */
       u_short sp_protocol;            /* protocol */
     };

/*
 * Protocol families, same as address families for now.
 */
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_INET         AF_INET
#define PF_INET6        AF_INET6
#define PF_IMPLINK      AF_IMPLINK
#define PF_PUP          AF_PUP
#define PF_CHAOS        AF_CHAOS
#define PF_NS           AF_NS
#define PF_ISO          AF_ISO
#define PF_OSI          AF_ISO
#define PF_ECMA         AF_ECMA
#define PF_DATAKIT      AF_DATAKIT
#define PF_CCITT        AF_CCITT
#define PF_SNA          AF_SNA
#define PF_DECnet       AF_DECnet
#define PF_DLI          AF_DLI
#define PF_LAT          AF_LAT
#define PF_HYLINK       AF_HYLINK
#define PF_APPLETALK    AF_APPLETALK
#define PF_ROUTE        AF_ROUTE
#define PF_LINK         AF_LINK
#define PF_PACKET       AF_PACKET
#define PF_XTP          pseudo_AF_XTP   /* really just proto family, no AF */

#define PF_MAX          AF_MAX

/*
 * Maximum queue length specifiable by listen.
 */
#define SOMAXCONN       32

/*
 * Flags for send(), sendto(), recv() and recvfrom():
 */
#define MSG_OOB         0x1             /* process out-of-band data */
#define MSG_PEEK        0x2             /* peek at incoming message */
#define MSG_DONTROUTE   0x4             /* send without using routing tables */
#define MSG_EOR         0x8             /* data completes record */
#define MSG_TRUNC       0x10            /* data discarded before delivery */
#define MSG_CTRUNC      0x20            /* control data lost before delivery */
#define MSG_WAITALL     0x40            /* wait for full request or error */
#define MSG_NOSIGNAL    0x80            /* don't raise SIGPIPE on reset */

#define MSG_MAXIOVLEN   16


/*
 * Header for ancillary data objects in msg_control buffer.
 * Used for additional information with/about a datagram
 * not expressible by flags.  The format is a sequence
 * of message elements headed by cmsghdr structures.
 */
struct cmsghdr {
       u_int   cmsg_len;               /* data byte count, including hdr */
       int     cmsg_level;             /* originating protocol */
       int     cmsg_type;              /* protocol-specific type */
       /* followed by  u_char  cmsg_data[]; */
     };

struct msghdr {
       char         *msg_name;         /* Contains an optional address. */
       socklen_t     msg_namelen;      /* len of optional address */
       struct iovec *msg_iov;          /* scatter/gather array. */
       int           msg_iovlen;       /* number of elements in msg_iov */
       char         *msg_accrights;    /* does not apply to IP - not changed */
       int           msg_accrightslen; /* does not apply to IP */
       void *        msg_control;      /* Ancillary data               */
       socklen_t     msg_controllen;   /* Ancillary data buffer length */
       int           msg_flags;        /* Received flags on recvmsg    */
     };

/* CMSG_DATA clashes with <wincrypt.h>
 */
#if (defined(WIN32) || defined(_WIN32)) && !defined(__CYGWIN__) && \
    !defined(_WINDOWS_H) && !defined(_INC_WINDOWS) && !defined(__windows_h__) && \
    !defined(_GNU_H_WINDOWS_H)
  #error Include <windows.h> before this point.
#endif

/* given pointer to 'struct cmsghdr', return pointer to data
 */
#undef  CMSG_DATA
#define CMSG_DATA(cmsg)   ((u_char *)((cmsg) + 1))

/* given pointer to 'struct msghdr', return pointer to next 'msghdr'
 */
#define CMSG_NXTHDR(mhdr, cmsg) \
        (((caddr_t)(cmsg) + (cmsg)->cmsg_len + sizeof(struct cmsghdr) > \
            (mhdr)->msg_control + (mhdr)->msg_controllen) ? \
            (struct cmsghdr *)NULL : \
            (struct cmsghdr *)((caddr_t)(cmsg) + (cmsg)->cmsg_len))

#define CMSG_FIRSTHDR(mhdr)     ((struct cmsghdr *)(mhdr)->msg_control)

/* "Socket"-level control message types: */
#define SCM_RIGHTS      0x01            /* access rights (array of int) */

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

__BEGIN_DECLS

W32_FUNC int W32_CALL accept (int, struct sockaddr *, socklen_t *);
W32_FUNC int W32_CALL bind (int, const struct sockaddr *, socklen_t);
W32_FUNC int W32_CALL closesocket (int s);
W32_FUNC int W32_CALL connect (int, const struct sockaddr *, socklen_t);
W32_FUNC int W32_CALL ioctlsocket (int s, long cmd, char *argp);
W32_FUNC int MS_CDECL fcntlsocket (int s, int cmd, ...);

W32_FUNC int W32_CALL getdomainname (char *name, size_t len);
W32_FUNC int W32_CALL setdomainname (const char *name, size_t len);

W32_FUNC int W32_CALL getpeername (int, struct sockaddr *, socklen_t *);
W32_FUNC int W32_CALL getsockname (int, struct sockaddr *, socklen_t *);
W32_FUNC int W32_CALL getsockopt (int, int, int, void *, socklen_t *);
W32_FUNC int W32_CALL listen (int, int);
W32_FUNC int W32_CALL recv (int, void *, int, int);
W32_FUNC int W32_CALL recvfrom (int, void *, int, int, struct sockaddr *, socklen_t *);
W32_FUNC int W32_CALL recvmsg (int, struct msghdr *, int);
W32_FUNC int W32_CALL send (int, const void *, int, int);
W32_FUNC int W32_CALL sendto (int, const void *, int, int, const struct sockaddr *, socklen_t);
W32_FUNC int W32_CALL sendmsg (int, const struct msghdr *, int);

W32_FUNC int W32_CALL setsockopt (int, int, int, const void *, socklen_t);
W32_FUNC int W32_CALL shutdown (int, int);
W32_FUNC int W32_CALL socket (int, int, int);

#ifdef __DJGPP__
  W32_FUNC int W32_CALL gethostname (char *name, int len);
#else
  W32_FUNC int W32_CALL gethostname (char *name, size_t len);
#endif

#if defined(__CYGWIN__)
  /*
   * CygWin's <unistd.h> doesn't agree with other vendors here.
   */
  W32_FUNC long   W32_CALL gethostid (void);
  W32_FUNC long   W32_CALL sethostid (u_long ip);
  W32_FUNC int    W32_CALL sethostname (const char *name, size_t len);
#else
  W32_FUNC u_long W32_CALL gethostid (void);
  W32_FUNC u_long W32_CALL sethostid (u_long ip);
  W32_FUNC int    W32_CALL sethostname (const char *name, int len);
#endif

__END_DECLS

#endif

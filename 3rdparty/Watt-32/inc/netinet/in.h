/*!\file netinet/in.h
 * IP address, options and definitions.
 */

/*
 * Copyright (c) 1982, 1986, 1990, 1993
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
 */

#ifndef __NETINET_IN_H
#define __NETINET_IN_H

#ifndef __SYS_SOCKET_H
#include <sys/socket.h>
#endif

/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981, and numerous additions.
 */

/*
 * Protocols
 */
#define IPPROTO_IP              0               /* dummy for IP */
#define IPPROTO_ICMP            1               /* control message protocol */
#define IPPROTO_IGMP            2               /* group mgmt protocol */
#define IPPROTO_GGP             3               /* gateway^2 (deprecated) */
#define IPPROTO_IPIP            4               /* IP encapsulation in IP */
#define IPPROTO_TCP             6               /* tcp */
#define IPPROTO_EGP             8               /* exterior gateway protocol */
#define IPPROTO_IGRP            9               /* interior gw routing protocol */
#define IPPROTO_PUP             12              /* pup */
#define IPPROTO_UDP             17              /* user datagram protocol */
#define IPPROTO_IDP             22              /* xns idp */
#define IPPROTO_TP              29              /* tp-4 w/ class negotiation */
#define IPPROTO_XTP             36              /* eXpress Transfer Protocol */
#define IPPROTO_RSVP            46              /* resource reservation */
#define IPPROTO_ESP             50
#define IPPROTO_AH              51              /* authenticate header */
#define IPPROTO_ICMPV6          58              /* ICMP v6 */
#define IPPROTO_NONE            59
#define IPPROTO_EON             80              /* ISO cnlp */
#define IPPROTO_ENCAP           98              /* encapsulation header */
#define IPPROTO_PIM             103
#define IPPROTO_VRRP            112
#define IPPROTO_SCTP            132

#define IPPROTO_DIVERT          254             /* divert pseudo-protocol */
#define IPPROTO_RAW             255             /* raw IP packet */
#define IPPROTO_MAX             256


/*
 * Local port number conventions:
 *
 * When a user does a bind(2) or connect(2) with a port number of zero,
 * a non-conflicting local port address is chosen.
 * The default range is IPPORT_RESERVED through
 * IPPORT_USERRESERVED, although that is settable by sysctl.
 *
 * A user may set the IPPROTO_IP option IP_PORTRANGE to change this
 * default assignment range.
 *
 * The value IP_PORTRANGE_DEFAULT causes the default behavior.
 *
 * The value IP_PORTRANGE_HIGH changes the range of candidate port numbers
 * into the "high" range.  These are reserved for client outbound connections
 * which do not want to be filtered by any firewalls.
 *
 * The value IP_PORTRANGE_LOW changes the range to the "low" are
 * that is (by convention) restricted to privileged processes.  This
 * convention is based on "vouchsafe" principles only.  It is only secure
 * if you trust the remote host to restrict these ports.
 *
 * The default range of ports and the high range can be changed by
 * sysctl(3).  (net.inet.ip.port{hi,low}{first,last}_auto)
 *
 * Changing those values has bad security implications if you are
 * using a a stateless firewall that is allowing packets outside of that
 * range in order to allow transparent outgoing connections.
 *
 * Such a firewall configuration will generally depend on the use of these
 * default values.  If you change them, you may find your Security
 * Administrator looking for you with a heavy object.
 */

/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).         (IP_PORTRANGE_LOW)
 * Ports > IPPORT_USERRESERVED are reserved
 * for servers, not necessarily privileged.  (IP_PORTRANGE_DEFAULT)
 */
#define IPPORT_RESERVED         1024
#define IPPORT_USERRESERVED     5000

/*
 * Default local port range to use by setting IP_PORTRANGE_HIGH
 */
#define IPPORT_HIFIRSTAUTO      40000
#define IPPORT_HILASTAUTO       44999

/*
 * Scanning for a free reserved port return a value below IPPORT_RESERVED,
 * but higher than IPPORT_RESERVEDSTART.  Traditionally the start value was
 * 512, but that conflicts with some well-known-services that firewalls may
 * have a fit if we use.
 */
#define IPPORT_RESERVEDSTART    600


/*
 * Internet address (a structure for historical reasons)
 */
struct in_addr {
       u_int32_t s_addr;
     };

/*
 *  For IPv6 from RFC2133
 */
#if 0
  struct in6_addr {
         u_int8_t  s6_addr[16];
       };
#else
  struct in6_addr {
         union {
           u_char  _S6_u8[16];
           u_short _S6_u16[8];
           u_long  _S6_u32[4];
         } _S6_un;
       };
  /* s6_addr is the standard name */
  #define s6_addr         _S6_un._S6_u8
  #define s6_bytes        _S6_un._S6_u8
  #define s6_words        _S6_un._S6_u16
#endif

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define IN_CLASSA(i)            (((long)(i) & 0x80000000) == 0)
#define IN_CLASSA_NET           0xff000000
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          0x00ffffff
#define IN_CLASSA_MAX           128

#define IN_CLASSB(i)            (((long)(i) & 0xC0000000) == 0x80000000)
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          0x0000ffff
#define IN_CLASSB_MAX           65536

#define IN_CLASSC(i)            (((long)(i) & 0xE0000000) == 0xC0000000)
#define IN_CLASSC_NET           0xffffff00
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          0x000000ff

#define IN_CLASSD(i)            (((long)(i) & 0xF0000000) == 0xE0000000)
#define IN_CLASSD_NET           0xf0000000      /* These ones aren't really */
#define IN_CLASSD_NSHIFT        28              /* net and host fields, but */
#define IN_CLASSD_HOST          0x0fffffff      /* routing needn't know.    */
#define IN_MULTICAST(i)         IN_CLASSD(i)

#define IN_EXPERIMENTAL(i)      (((long)(i) & 0xF0000000) == 0xF0000000)
#define IN_BADCLASS(i)          (((long)(i) & 0xF0000000) == 0xF0000000)

#define INADDR_ANY              (u_int32_t)0x00000000
#define INADDR_BROADCAST        (u_int32_t)0xFFFFFFFF   /* must be masked */
#define INADDR_LOOPBACK         (u_int32_t)0x7F000001   /* 127.0.0.1 */
#define INADDR_NONE             0xFFFFFFFF              /* -1 return */

#define INADDR_UNSPEC_GROUP     (u_int32_t)0xE0000000   /* 224.0.0.0 */
#define INADDR_ALLHOSTS_GROUP   (u_int32_t)0xE0000001   /* 224.0.0.1 */
#define INADDR_ALLRTRS_GROUP    (u_int32_t)0xE0000002   /* 224.0.0.2 */
#define INADDR_MAX_LOCAL_GROUP  (u_int32_t)0xE00000FF   /* 224.0.0.255 */

#define IN_LOOPBACKNET          127                     /* official! */

W32_DATA const struct in6_addr in6addr_any;             /* :: */
W32_DATA const struct in6_addr in6addr_loopback;        /* ::1 */

#define IN6ADDR_ANY_INIT      { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } } }
#define IN6ADDR_LOOPBACK_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } } }

#define INET_ADDRSTRLEN       16
#define INET6_ADDRSTRLEN      46


/*
 * Socket address, internet style.
 */
struct sockaddr_in {
       sa_family_t    sin_family;
       u_short        sin_port;
       struct in_addr sin_addr;
       char           sin_zero[8];
     };

struct sockaddr_in6 {
       u_int16_t       sin6_family;
       u_int16_t       sin6_port;
       u_int32_t       sin6_flowinfo;
       struct in6_addr sin6_addr;
       u_int32_t       sin6_scope_id; /* IPv6 scope-id */
     };

/*
 * Stolen from winsock2.h (hope this is portable)
 * Portable IPv6/IPv4 version of sockaddr.
 * Uses padding to force 8 byte alignment
 * and maximum size of 128 bytes
 */
struct sockaddr_storage {
       sa_family_t ss_family;
       u_short     ss_len;        /* !! added for BSD progs, gv Nov-2003 */
       char      __ss_pad1[6];              /* pad to 8 */
       long      __ss_align1, __ssalign2;   /* force alignment */
       char      __ss_pad2[110];            /* pad to 128 */
     };


/* IPv6 multicast request. */
struct ipv6_mreq {
       /* IPv6 multicast address of group */
       struct in6_addr ipv6mr_multiaddr;

       /* local interface */
       unsigned long ipv6mr_interface;
     };

/*
 * Structure used to describe IP options.
 * Used to store options internally, to pass them to a process,
 * or to restore options retrieved earlier.
 * The ip_dst is used for the first-hop gateway when using a source route
 * (this gets put into the header proper).
 */
struct ip_opts {
       struct in_addr ip_dst;     /* first hop, 0 w/o src rt */
       char   IP_opts[40];        /* actually variable in size */
     };

/*
 * Options for use with [gs]etsockopt at the IP level.
 * First word of comment is data type; bool is stored in int.
 */
#define IP_OPTIONS              1    /* buf/ip_opts; set/get IP options */
#define IP_HDRINCL              2    /* int; header is included with data */
#define IP_TOS                  3    /* int; IP type of service and preced. */
#define IP_TTL                  4    /* int; IP time to live */
#define IP_RECVOPTS             5    /* bool; receive all IP opts w/dgram */
#define IP_RECVRETOPTS          6    /* bool; receive IP opts for response */
#define IP_RECVDSTADDR          7    /* bool; receive IP dst addr w/dgram */
#define IP_RETOPTS              8    /* ip_opts; set/get IP options */
#define IP_MULTICAST_IF         9    /* u_char; set/get IP multicast i/f  */
#define IP_MULTICAST_TTL        10   /* u_char; set/get IP multicast ttl */
#define IP_MULTICAST_LOOP       11   /* u_char; set/get IP multicast loopback */
#define IP_ADD_MEMBERSHIP       12   /* ip_mreq; add an IP group membership */
#define IP_DROP_MEMBERSHIP      13   /* ip_mreq; drop an IP group membership */
#define IP_MULTICAST_VIF        14   /* set/get IP mcast virt. iface */
#define IP_RSVP_ON              15   /* enable RSVP in kernel */
#define IP_RSVP_OFF             16   /* disable RSVP in kernel */
#define IP_RSVP_VIF_ON          17   /* set RSVP per-vif socket */
#define IP_RSVP_VIF_OFF         18   /* unset RSVP per-vif socket */
#define IP_PORTRANGE            19   /* int; range to choose for unspec port */
#define IP_RECVIF               20   /* bool; receive reception if w/dgram */

#define IP_FW_ADD               50   /* add a firewall rule to chain */
#define IP_FW_DEL               51   /* delete a firewall rule from chain */
#define IP_FW_FLUSH             52   /* flush firewall rule chain */
#define IP_FW_ZERO              53   /* clear single/all firewall counter(s) */
#define IP_FW_GET               54   /* get entire firewall rule chain */
#define IP_NAT                  55   /* set/get NAT opts */

/*
 * Defaults and limits for options
 */
#define IP_DEFAULT_MULTICAST_TTL  1     /* normally limit m'casts to 1 hop  */
#define IP_DEFAULT_MULTICAST_LOOP 1     /* normally hear sends if a member  */
#define IP_MAX_MEMBERSHIPS        20    /* per socket */

/*
 * Argument structure for IP_ADD_MEMBERSHIP and IP_DROP_MEMBERSHIP.
 */
struct ip_mreq {
        struct  in_addr imr_multiaddr;  /* IP multicast address of group */
        struct  in_addr imr_interface;  /* local IP address of interface */
};

/*
 * Argument for IP_PORTRANGE:
 * - which range to search when port is unspecified at bind() or connect()
 */
#define IP_PORTRANGE_DEFAULT    0       /* default range */
#define IP_PORTRANGE_HIGH       1       /* "high" - request firewall bypass */
#define IP_PORTRANGE_LOW        2       /* "low" - vouchsafe security */

/*
 * Definitions for inet sysctl operations.
 *
 * Third level is protocol number.
 * Fourth level is desired variable within that protocol.
 */
#define IPPROTO_MAXID   (IPPROTO_IDP + 1)       /* don't list to IPPROTO_MAX */

#define CTL_IPPROTO_NAMES { \
        { "ip", CTLTYPE_NODE }, \
        { "icmp", CTLTYPE_NODE }, \
        { "igmp", CTLTYPE_NODE }, \
        { "ggp", CTLTYPE_NODE }, \
        { 0, 0 }, \
        { 0, 0 }, \
        { "tcp", CTLTYPE_NODE }, \
        { 0, 0 }, \
        { "egp", CTLTYPE_NODE }, \
        { 0, 0 }, \
        { 0, 0 }, \
        { 0, 0 }, \
        { "pup", CTLTYPE_NODE }, \
        { 0, 0 }, \
        { 0, 0 }, \
        { 0, 0 }, \
        { 0, 0 }, \
        { "udp", CTLTYPE_NODE }, \
        { 0, 0 }, \
        { 0, 0 }, \
        { 0, 0 }, \
        { 0, 0 }, \
        { "idp", CTLTYPE_NODE }, \
}

/*
 * Names for IP sysctl objects
 */
#define IPCTL_FORWARDING        1       /* act as router */
#define IPCTL_SENDREDIRECTS     2       /* may send redirects when forwarding */
#define IPCTL_DEFTTL            3       /* default TTL */
#define IPCTL_DEFMTU            4       /* default MTU */
#define IPCTL_RTEXPIRE          5       /* cloned route expiration time */
#define IPCTL_RTMINEXPIRE       6       /* min value for expiration time */
#define IPCTL_RTMAXCACHE        7       /* trigger level for dynamic expire */
#define IPCTL_SOURCEROUTE       8       /* may perform source routes */
#define IPCTL_DIRECTEDBROADCAST 9       /* may re-broadcast received packets */
#define IPCTL_INTRQMAXLEN       10      /* max length of netisr queue */
#define IPCTL_INTRQDROPS        11      /* number of netisr q drops */
#define IPCTL_MAXID             12

#define IPCTL_NAMES { \
        { 0, 0 }, \
        { "forwarding", CTLTYPE_INT }, \
        { "redirect", CTLTYPE_INT }, \
        { "ttl", CTLTYPE_INT }, \
        { "mtu", CTLTYPE_INT }, \
        { "rtexpire", CTLTYPE_INT }, \
        { "rtminexpire", CTLTYPE_INT }, \
        { "rtmaxcache", CTLTYPE_INT }, \
        { "sourceroute", CTLTYPE_INT }, \
        { "directed-broadcast", CTLTYPE_INT }, \
        { "intr-queue-maxlen", CTLTYPE_INT }, \
        { "intr-queue-drops", CTLTYPE_INT }, \
}


/*
 * IPv6 address macros
 */
#ifndef IN6_IS_ADDR_UNSPECIFIED
#define IN6_IS_ADDR_UNSPECIFIED(a)      \
           (((u_int32_t*)(a))[0] == 0   \
         && ((u_int32_t*)(a))[1] == 0   \
         && ((u_int32_t*)(a))[2] == 0   \
         && ((u_int32_t*)(a))[3] == 0)
#endif

#ifndef IN6_IS_ADDR_LOOPBACK
#define IN6_IS_ADDR_LOOPBACK(a)         \
           (((u_int32_t*)(a))[0] == 0   \
         && ((u_int32_t*)(a))[1] == 0   \
         && ((u_int32_t*)(a))[2] == 0   \
         && ((u_int32_t*)(a))[3] == htonl (1))
#endif

#ifndef IN6_IS_ADDR_MULTICAST
#define IN6_IS_ADDR_MULTICAST(a) (((u_int8_t*) (a))[0] == 0xff)
#endif

#define IN6_IS_ADDR_LINKLOCAL(a) \
        ((((u_int32_t*)(a))[0] & htonl(0xffc00000)) == htonl(0xfe800000))

#ifndef IN6_IS_ADDR_SITELOCAL
#define IN6_IS_ADDR_SITELOCAL(a) \
        ((((u_int32_t*)(a))[0] & htonl(0xffc00000)) == htonl(0xfec00000))
#endif

#ifndef IN6_IS_ADDR_V4MAPPED
#define IN6_IS_ADDR_V4MAPPED(a)         \
           ((((u_int32_t*)(a))[0] == 0) \
         && (((u_int32_t*)(a))[1] == 0) \
         && (((u_int32_t*)(a))[2] == htonl(0xffff)))
#endif

#ifndef IN6_IS_ADDR_V4COMPAT
#define IN6_IS_ADDR_V4COMPAT(a)         \
           ((((u_int32_t*)(a))[0] == 0) \
         && (((u_int32_t*)(a))[1] == 0) \
         && (((u_int32_t*)(a))[2] == 0) \
         && (ntohl(((u_int32_t*)(a))[3]) > 1))
#endif

#ifndef IN6_ARE_ADDR_EQUAL
#define IN6_ARE_ADDR_EQUAL(a, b) \
        (memcmp ((void*)(a), (void*)(b), sizeof(struct in6_addr)) == 0)
#endif

#ifndef IN6_ADDR_EQUAL
#define IN6_ADDR_EQUAL(a, b) IN6_ARE_ADDR_EQUAL(a, b)
#endif

#ifndef IN6_IS_ADDR_MC_NODELOCAL
#define IN6_IS_ADDR_MC_NODELOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a) && ((((u_int8_t*)(a))[1] & 0xf) == 0x1))
#endif

#ifndef IN6_IS_ADDR_MC_LINKLOCAL
#define IN6_IS_ADDR_MC_LINKLOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a) && ((((u_int8_t*)(a))[1] & 0xf) == 0x2))
#endif

#ifndef IN6_IS_ADDR_MC_SITELOCAL
#define IN6_IS_ADDR_MC_SITELOCAL(a) \
        (IN6_IS_ADDR_MULTICAST(a) && ((((u_int8_t*)(a))[1] & 0xf) == 0x5))
#endif

#ifndef IN6_IS_ADDR_MC_ORGLOCAL
#define IN6_IS_ADDR_MC_ORGLOCAL(a)  \
        (IN6_IS_ADDR_MULTICAST(a) && ((((u_int8_t*)(a))[1] & 0xf) == 0x8))
#endif

#ifndef IN6_IS_ADDR_MC_GLOBAL
#define IN6_IS_ADDR_MC_GLOBAL(a)    \
        (IN6_IS_ADDR_MULTICAST(a) && ((((u_int8_t*)(a))[1] & 0xf) == 0xe))
#endif

#endif /* __NETINET_IN_H */


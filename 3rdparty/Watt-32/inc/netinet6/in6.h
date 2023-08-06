/*!\file netinet6/in6.h
 */

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Copyright (c) 1982, 1986, 1990, 1993
 *  The Regents of the University of California.  All rights reserved.
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
 *  This product includes software developed by the University of
 *  California, Berkeley and its contributors.
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

#ifndef _NETINET6_IN6_H_
#define _NETINET6_IN6_H_

/*
 * Identification of the network protocol stack
 * for *BSD-current/release: http://www.kame.net/dev/cvsweb.cgi/kame/COVERAGE
 * has the table of implementation/integration differences.
 */
#define __KAME__
#define __KAME_VERSION  "20010528/FreeBSD"

#ifndef __SYS_SOCKET_H
#include <sys/socket.h>
#endif

/*
 * Local port number conventions:
 *
 * Ports < IPPORT_RESERVED are reserved for privileged processes (e.g. root),
 * unless a kernel is compiled with IPNOPRIVPORTS defined.
 *
 * When a user does a bind(2) or connect(2) with a port number of zero,
 * a non-conflicting local port address is chosen.
 *
 * The default range is IPPORT_ANONMIN to IPPORT_ANONMAX, although
 * that is settable by sysctl(3); net.inet.ip.anonportmin and
 * net.inet.ip.anonportmax respectively.
 *
 * A user may set the IPPROTO_IP option IP_PORTRANGE to change this
 * default assignment range.
 *
 * The value IP_PORTRANGE_DEFAULT causes the default behavior.
 *
 * The value IP_PORTRANGE_HIGH is the same as IP_PORTRANGE_DEFAULT,
 * and exists only for FreeBSD compatibility purposes.
 *
 * The value IP_PORTRANGE_LOW changes the range to the "low" are
 * that is (by convention) restricted to privileged processes.
 * This convention is based on "vouchsafe" principles only.
 * It is only secure if you trust the remote host to restrict these ports.
 * The range is IPPORT_RESERVEDMIN to IPPORT_RESERVEDMAX.
 */
#if defined(__BSD_VISIBLE) && (__BSD_VISIBLE != 0)
#define IPV6PORT_RESERVED      1024
#define IPV6PORT_ANONMIN      49152
#define IPV6PORT_ANONMAX      65535
#define IPV6PORT_RESERVEDMIN    600
#define IPV6PORT_RESERVEDMAX  (IPV6PORT_RESERVED-1)
#define SIN6_LEN
#endif

#define INET6_ADDRSTRLEN  46

#ifndef __NETINET_IN_H
  /*
   * IPv6 address
   */
  struct in6_addr {
         union {
           uint8_t  __u6_addr8[16];
           uint16_t __u6_addr16[8];
           uint32_t __u6_addr32[4];
         } __u6_addr;  /* 128-bit IP6 address */
      };

  #define s6_addr   __u6_addr.__u6_addr8

  /*
   * Socket address for IPv6
   */
  struct sockaddr_in6 {
         uint8_t         sin6_len;      /* length of this struct */
         sa_family_t     sin6_family;   /* AF_INET6 */
         uint16_t        sin6_port;     /* Transport layer port # */
         uint32_t        sin6_flowinfo; /* IP6 flow information */
         struct in6_addr sin6_addr;     /* IP6 address */
         uint32_t        sin6_scope_id; /* scope zone index */
  };

  /*
   * Definition of some useful macros to handle IP6 addresses
   */
  #if defined(__BSD_VISIBLE) && (__BSD_VISIBLE != 0)
    #define IN6ADDR_ANY_INIT \
             {{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }}}
    #define IN6ADDR_LOOPBACK_INIT \
            {{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
    #define IN6ADDR_NODELOCAL_ALLNODES_INIT \
            {{{ 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
    #define IN6ADDR_LINKLOCAL_ALLNODES_INIT \
            {{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 }}}
    #define IN6ADDR_LINKLOCAL_ALLROUTERS_INIT \
            {{{ 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02 }}}
  #endif

  W32_DATA const struct in6_addr in6addr_any;
  W32_DATA const struct in6_addr in6addr_loopback;

  W32_DATA const struct in6_addr in6addr_nodelocal_allnodes;
  W32_DATA const struct in6_addr in6addr_linklocal_allnodes;
  W32_DATA const struct in6_addr in6addr_linklocal_allrouters;
#endif  /* __NETINET_IN_H */

/*
 * Equality
 * NOTE: Some of kernel programming environment (for example, openbsd/sparc)
 * does not supply memcmp().  For userland memcmp() is preferred as it is
 * in ANSI standard.
 */
#ifndef IN6_ARE_ADDR_EQUAL
#define IN6_ARE_ADDR_EQUAL(a, b) \
        (memcmp(&(a)->s6_addr[0], &(b)->s6_addr[0], sizeof(struct in6_addr)) == 0)
#endif

#ifndef SA6_ARE_ADDR_EQUAL
  /*
   * see if two addresses are equal in a scope-conscious manner.
   */
  #define SA6_ARE_ADDR_EQUAL(a, b) \
          (((a)->sin6_scope_id == 0 || (b)->sin6_scope_id == 0 || \
          ((a)->sin6_scope_id == (b)->sin6_scope_id)) && \
          (memcmp(&(a)->sin6_addr, &(b)->sin6_addr, sizeof(struct in6_addr)) == 0))
#endif

#ifndef __NETINET_IN_H
/*
 * Unspecified
 */
#define IN6_IS_ADDR_UNSPECIFIED(a) \
         ((*(const u_int32_t *)(const void *)(&(a)->s6_addr[0]) == 0) && \
          (*(const u_int32_t *)(const void *)(&(a)->s6_addr[4]) == 0) && \
          (*(const u_int32_t *)(const void *)(&(a)->s6_addr[8]) == 0) && \
          (*(const u_int32_t *)(const void *)(&(a)->s6_addr[12]) == 0))

/*
 * Loopback
 */
#define IN6_IS_ADDR_LOOPBACK(a) \
        ((*(const u_int32_t *)(const void *)(&(a)->s6_addr[0]) == 0) && \
         (*(const u_int32_t *)(const void *)(&(a)->s6_addr[4]) == 0) && \
         (*(const u_int32_t *)(const void *)(&(a)->s6_addr[8]) == 0) && \
         (*(const u_int32_t *)(const void *)(&(a)->s6_addr[12]) == ntohl(1)))

/*
 * IPv4 compatible
 */
#define IN6_IS_ADDR_V4COMPAT(a) \
        ((*(const u_int32_t *)(const void *)(&(a)->s6_addr[0]) == 0) &&  \
         (*(const u_int32_t *)(const void *)(&(a)->s6_addr[4]) == 0) &&  \
         (*(const u_int32_t *)(const void *)(&(a)->s6_addr[8]) == 0) &&  \
         (*(const u_int32_t *)(const void *)(&(a)->s6_addr[12]) != 0) && \
         (*(const u_int32_t *)(const void *)(&(a)->s6_addr[12]) != ntohl(1)))

/*
 * Mapped
 */
#define IN6_IS_ADDR_V4MAPPED(a) \
        ((*(const u_int32_t *)(const void *)(&(a)->s6_addr[0]) == 0) && \
         (*(const u_int32_t *)(const void *)(&(a)->s6_addr[4]) == 0) && \
         (*(const u_int32_t *)(const void *)(&(a)->s6_addr[8]) == ntohl(0x0000ffff)))

/*
 * KAME Scope Values
 */
#define __IPV6_ADDR_SCOPE_NODELOCAL  0x01
#define __IPV6_ADDR_SCOPE_LINKLOCAL  0x02
#define __IPV6_ADDR_SCOPE_SITELOCAL  0x05
#define __IPV6_ADDR_SCOPE_ORGLOCAL   0x08 /* just used in this file */
#define __IPV6_ADDR_SCOPE_GLOBAL     0x0e

/*
 * Unicast Scope
 * Note that we must check topmost 10 bits only, not 16 bits (see RFC2373).
 */
#define IN6_IS_ADDR_LINKLOCAL(a) \
        (((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0x80))

#define IN6_IS_ADDR_SITELOCAL(a) \
        (((a)->s6_addr[0] == 0xfe) && (((a)->s6_addr[1] & 0xc0) == 0xc0))

/*
 * Multicast
 */
#define IN6_IS_ADDR_MULTICAST(a)   ((a)->s6_addr[0] == 0xff)

#define __IPV6_ADDR_MC_SCOPE(a)    ((a)->s6_addr[1] & 0x0f)

/*
 * Multicast Scope
 */
#define IN6_IS_ADDR_MC_NODELOCAL(a) \
    (IN6_IS_ADDR_MULTICAST(a) &&    \
     (__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_NODELOCAL))

#define IN6_IS_ADDR_MC_LINKLOCAL(a) \
    (IN6_IS_ADDR_MULTICAST(a) &&    \
     (__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_LINKLOCAL))

#define IN6_IS_ADDR_MC_SITELOCAL(a) \
    (IN6_IS_ADDR_MULTICAST(a) &&    \
     (__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_SITELOCAL))

#define IN6_IS_ADDR_MC_ORGLOCAL(a)  \
    (IN6_IS_ADDR_MULTICAST(a) &&    \
     (__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_ORGLOCAL))

#define IN6_IS_ADDR_MC_GLOBAL(a)    \
    (IN6_IS_ADDR_MULTICAST(a) &&    \
     (__IPV6_ADDR_MC_SCOPE(a) == __IPV6_ADDR_SCOPE_GLOBAL))

#endif /* __NETINET_IN_H */

#ifndef _NETINET6_IP6_VAR_H_
  /*
   * IP6 route structure
   */
  struct route_in6 {
        struct  rtentry *ro_rt;
        struct  sockaddr_in6 ro_dst;
      };
#endif

/*
 * Options for use with [gs]etsockopt at the IPV6 level.
 * First word of comment is data type; bool is stored in int.
 */
#define IPV6_JOIN_GROUP     12 /* ip6_mreq; join a group membership */
#define IPV6_LEAVE_GROUP    13 /* ip6_mreq; leave a group membership */
#define IPV6_MULTICAST_HOPS 10 /* int; set/get IP6 multicast hops */
#define IPV6_MULTICAST_IF    9  /* u_int; set/get IP6 multicast i/f  */
#define IPV6_MULTICAST_LOOP 11 /* u_int; set/get IP6 multicast loopback */
#define IPV6_UNICAST_HOPS    4  /* int; IP6 hops */
#define IPV6_V6ONLY         27 /* bool; only bind INET6 at wildcard bind */

#if defined(__BSD_VISIBLE) && (__BSD_VISIBLE != 0)
#define IPV6_SOCKOPT_RESERVED1  3  /* reserved for future use */
#define IPV6_PORTRANGE         14  /* int; range to choose for unspec port */
#define ICMP6_FILTER           18  /* icmp6_filter; icmp6 filter */

/* RFC2292 options */
#define IPV6_PKTINFO        19 /* bool; send/recv if, src/dst addr */
#define IPV6_HOPLIMIT       20 /* bool; hop limit */
#define IPV6_NEXTHOP        21 /* bool; next hop addr */
#define IPV6_HOPOPTS        22 /* bool; hop-by-hop option */
#define IPV6_DSTOPTS        23 /* bool; destination option */
#define IPV6_RTHDR          24 /* bool; routing header */
#define IPV6_PKTOPTIONS     25 /* buf/cmsghdr; set/get IPv6 options */

#define IPV6_CHECKSUM       26 /* int; checksum offset for raw socket */

#define IPV6_BINDV6ONLY     IPV6_V6ONLY

#define IPV6_IPSEC_POLICY   28 /* struct; get/set security policy */
#define IPV6_FAITH          29 /* bool; accept FAITH'ed connections */

#define IPV6_FW_ADD         30 /* add a firewall rule to chain */
#define IPV6_FW_DEL         31 /* delete a firewall rule from chain */
#define IPV6_FW_FLUSH       32 /* flush firewall rule chain */
#define IPV6_FW_ZERO        33 /* clear single/all firewall counter(s) */
#define IPV6_FW_GET         34 /* get entire firewall rule chain */

/* to define items, should talk with KAME guys first, for *BSD compatibility */

#define IPV6_RTHDR_LOOSE     0 /* this hop need not be a neighbor. XXX old spec */
#define IPV6_RTHDR_STRICT    1 /* this hop must be a neighbor. XXX old spec */
#define IPV6_RTHDR_TYPE_0    0 /* IPv6 routing header type 0 */

/*
 * Defaults and limits for options
 */
#define IPV6_DEFAULT_MULTICAST_HOPS 1   /* normally limit m'casts to 1 hop */
#define IPV6_DEFAULT_MULTICAST_LOOP 1   /* normally hear sends if a member */

/*
 * Argument structure for IPV6_JOIN_GROUP and IPV6_LEAVE_GROUP.
 */
#ifndef __NETINET_IN_H
  struct ipv6_mreq {
      struct in6_addr ipv6mr_multiaddr;
      unsigned int    ipv6mr_interface;
  };
#endif

/*
 * IPV6_PKTINFO: Packet information(RFC2292 sec 5)
 */
struct in6_pktinfo {
    struct in6_addr ipi6_addr;  /* src/dst IPv6 address */
    unsigned int    ipi6_ifindex;   /* send/recv interface index */
};

/*
 * Argument for IPV6_PORTRANGE:
 * - which range to search when port is unspecified at bind() or connect()
 */
#define IPV6_PORTRANGE_DEFAULT  0   /* default range */
#define IPV6_PORTRANGE_HIGH     1   /* "high" - request firewall bypass */
#define IPV6_PORTRANGE_LOW      2   /* "low" - vouchsafe security */

/*
 * Definitions for inet6 sysctl operations.
 *
 * Third level is protocol number.
 * Fourth level is desired variable within that protocol.
 */
#define IPV6PROTO_MAXID (IPPROTO_PIM + 1)   /* don't list to IPV6PROTO_MAX */

#define CTL_IPV6PROTO_NAMES { \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, \
    { "tcp6", CTLTYPE_NODE }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { "udp6", CTLTYPE_NODE }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, \
    { "ip6", CTLTYPE_NODE }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, \
    { "ipsec6", CTLTYPE_NODE }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { "icmp6", CTLTYPE_NODE }, \
    { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { 0, 0 }, \
    { "pim6", CTLTYPE_NODE }, \
}

/*
 * Names for IP sysctl objects
 */
#define IPV6CTL_FORWARDING          1   /* act as router */
#define IPV6CTL_SENDREDIRECTS       2   /* may send redirects when forwarding*/
#define IPV6CTL_DEFHLIM             3   /* default Hop-Limit */
#define IPV6CTL_FORWSRCRT           5   /* forward source-routed dgrams */
#define IPV6CTL_STATS               6   /* stats */
#define IPV6CTL_MRTSTATS            7   /* multicast forwarding stats */
#define IPV6CTL_MRTPROTO            8   /* multicast routing protocol */
#define IPV6CTL_MAXFRAGPACKETS      9   /* max packets reassembly queue */
#define IPV6CTL_SOURCECHECK        10  /* verify source route and intf */
#define IPV6CTL_SOURCECHECK_LOGINT 11   /* minimume logging interval */
#define IPV6CTL_ACCEPT_RTADV       12
#define IPV6CTL_KEEPFAITH          13
#define IPV6CTL_LOG_INTERVAL       14
#define IPV6CTL_HDRNESTLIMIT       15
#define IPV6CTL_DAD_COUNT          16
#define IPV6CTL_AUTO_FLOWLABEL     17
#define IPV6CTL_DEFMCASTHLIM       18
#define IPV6CTL_GIF_HLIM           19  /* default HLIM for gif encap packet */
#define IPV6CTL_KAME_VERSION       20
#define IPV6CTL_USE_DEPRECATED     21  /* use deprecated addr (RFC2462 5.5.4) */
#define IPV6CTL_RR_PRUNE           22  /* walk timer for router renumbering */
#define IPV6CTL_V6ONLY             24
#define IPV6CTL_RTEXPIRE           25  /* cloned route expiration time */
#define IPV6CTL_RTMINEXPIRE        26  /* min value for expiration time */
#define IPV6CTL_RTMAXCACHE         27  /* trigger level for dynamic expire */

#define IPV6CTL_USETEMPADDR        32  /* use temporary addresses (RFC3041) */
#define IPV6CTL_TEMPPLTIME         33  /* preferred lifetime for tmpaddrs */
#define IPV6CTL_TEMPVLTIME         34  /* valid lifetime for tmpaddrs */
#define IPV6CTL_AUTO_LINKLOCAL     35  /* automatic link-local addr assign */
#define IPV6CTL_RIP6STATS          36  /* raw_ip6 stats */

/* New entries should be added here from current IPV6CTL_MAXID value. */
/* to define items, should talk with KAME guys first, for *BSD compatibility
 */
#define IPV6CTL_MAXID       37

/*
 * Redefinition of mbuf flags
 */
#define M_AUTHIPHDR M_PROTO2
#define M_DECRYPTED M_PROTO3
#define M_LOOP      M_PROTO4
#define M_AUTHIPDGM M_PROTO5

__BEGIN_DECLS

struct cmsghdr;

extern int              inet6_option_space (int);
extern int              inet6_option_init (void *, struct cmsghdr **, int);
extern int              inet6_option_append (struct cmsghdr *, const uint8_t *, int, int);
extern uint8_t         *inet6_option_alloc (struct cmsghdr *, int, int, int);
extern int              inet6_option_next (const struct cmsghdr *, uint8_t **);
extern int              inet6_option_find (const struct cmsghdr *, uint8_t **, int);

extern size_t           inet6_rthdr_space (int, int);
extern struct cmsghdr  *inet6_rthdr_init (void *, int);
extern int              inet6_rthdr_add (struct cmsghdr *, const struct in6_addr *, unsigned int);
extern int              inet6_rthdr_lasthop (struct cmsghdr *, unsigned int);

extern int              inet6_rthdr_segments (const struct cmsghdr *);
extern struct in6_addr *inet6_rthdr_getaddr (struct cmsghdr *, int);
extern int              inet6_rthdr_getflags (const struct cmsghdr *, int);

extern int              inet6_opt_init (void *, size_t);
extern int              inet6_opt_append (void *, size_t, int, uint8_t, size_t, uint8_t, void **);
extern int              inet6_opt_finish (void *, size_t, int);
extern int              inet6_opt_set_val (void *, size_t, void *, int);

extern int              inet6_opt_next (void *, size_t, int, uint8_t *, size_t *, void **);
extern int              inet6_opt_find (void *, size_t, int, uint8_t, size_t *, void **);
extern int              inet6_opt_get_val (void *, size_t, void *, int);
extern size_t           inet6_rth_space (int, int);
extern void            *inet6_rth_init (void *, int, int, int);
extern int              inet6_rth_add (void *, const struct in6_addr *);
extern int              inet6_rth_reverse (const void *, void *);
extern int              inet6_rth_segments (const void *);
extern struct in6_addr *inet6_rth_getaddr (const void *, int);

__END_DECLS

#endif /* __BSD_VISIBLE */

#endif /* !_NETINET6_IN6_H_ */

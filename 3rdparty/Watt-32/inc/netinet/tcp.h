/*!\file netinet/tcp.h
 * TCP header and options.
 */

/*
 * Copyright (c) 1982, 1986, 1993
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

#ifndef __NETINET_TCP_H
#define __NETINET_TCP_H

#include <sys/cdefs.h>
#include <sys/wtypes.h>

typedef u_long  tcp_seq;
typedef u_long  tcp_cc;                 /* connection count per rfc1644 */

W32_CLANG_PACK_WARN_OFF()

#include <sys/pack_on.h>

/*
 * TCP header.
 * Per RFC 793, September, 1981.
 */
struct tcphdr {
        u_short th_sport;               /* source port */
        u_short th_dport;               /* destination port */
        tcp_seq th_seq;                 /* sequence number */
        tcp_seq th_ack;                 /* acknowledgement number */
        u_char  th_x2:4;                /* (unused) */
        u_char  th_off:4;               /* data offset */
        u_char  th_flags;
#define TH_FIN  0x01
#define TH_SYN  0x02
#define TH_RST  0x04
#define TH_PUSH 0x08
#define TH_ACK  0x10
#define TH_URG  0x20
#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG)

        u_short th_win;                 /* window */
        u_short th_sum;                 /* checksum */
        u_short th_urp;                 /* urgent pointer */
};

#include <sys/pack_off.h>

W32_CLANG_PACK_WARN_DEF()

#define TCPOPT_EOL              0
#define TCPOPT_NOP              1
#define TCPOPT_MAXSEG           2
#define    TCPOLEN_MAXSEG               4
#define TCPOPT_WINDOW           3
#define    TCPOLEN_WINDOW               3
#define TCPOPT_SACK_PERMITTED   4
#define    TCPOLEN_SACK_PERMITTED       2
#define TCPOPT_SACK             5
#define TCPOPT_TIMESTAMP        8
#define    TCPOLEN_TIMESTAMP            10
#define    TCPOLEN_TSTAMP_APPA          (TCPOLEN_TIMESTAMP+2) /* appendix A */

#define TCPOPT_TSTAMP_HDR       \
    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_TIMESTAMP<<8|TCPOLEN_TIMESTAMP)

#define TCPOPT_CC               11              /* CC options: RFC-1644 */
#define TCPOPT_CCNEW            12
#define TCPOPT_CCECHO           13
#define    TCPOLEN_CC                   6
#define    TCPOLEN_CC_APPA              (TCPOLEN_CC+2)
#define    TCPOPT_CC_HDR(ccopt)         \
    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|(ccopt)<<8|TCPOLEN_CC)

#define TCPOPT_SIGNATURE  19
#define    TCP_SIGLEN 16

/*
 * Default maximum segment size for TCP.
 * With an IP MSS of 576, this is 536,
 * but 512 is probably more convenient.
 * This should be defined as MIN(512, IP_MSS - sizeof (struct tcpiphdr)).
 */
#define TCP_MSS         512

#define TCP_MAXWIN      65535   /* largest value for (unscaled) window */
#define TTCP_CLIENT_SND_WND     4096    /* dflt send window for T/TCP client */

#define TCP_MAX_WINSHIFT        14      /* maximum window shift */

#define TCP_MAXHLEN     (0xf<<2)        /* max length of header in bytes */
#define TCP_MAXOLEN     (TCP_MAXHLEN - sizeof(struct tcphdr))
                                        /* max space left for options */

/*
 * User-settable options (used with setsockopt).
 */
#define TCP_NODELAY     0x01    /* don't delay send to coalesce packets */
#define TCP_MAXSEG      0x02    /* set maximum segment size */
#ifdef TCP_AUTO
#define TCP_AUTO_OP     0x03    /* 0 to disable autotuning, 1 to reenable */
#endif
#define TCP_NOPUSH      0x04    /* don't push last block of write */
#define TCP_NOOPT       0x08    /* don't use TCP options */

/* For PSC modifications */
#if defined(TCP_AUTO) || defined(TCP_SACK) || defined(TCP_FACK)
#define PSC_VERSION "0.8"
#endif

#endif

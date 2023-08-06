/*!\file netinet/udp_var.h
 * Internal UDP statistics.
 */

/*
 * Copyright (c) 1982, 1986, 1989, 1993
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

#ifndef __NETINET_UDP_VAR_H
#define __NETINET_UDP_VAR_H

#include <sys/wtypes.h>

/*
 * UDP kernel structures and variables.
 */
#ifdef NOT_NEEDED
  W32_CLANG_PACK_WARN_OFF()

  #include <sys/pack_on.h>

  struct  udpiphdr {
          struct  ipovly ui_i;            /* overlaid ip structure */
          struct  udphdr ui_u;            /* udp header */
        };

  #include <sys/pack_off.h>

  W32_CLANG_PACK_WARN_DEF()

  #define ui_next         ui_i.ih_next
  #define ui_prev         ui_i.ih_prev
  #define ui_x1           ui_i.ih_x1
  #define ui_pr           ui_i.ih_pr
  #define ui_len          ui_i.ih_len
  #define ui_src          ui_i.ih_src
  #define ui_dst          ui_i.ih_dst
  #define ui_sport        ui_u.uh_sport
  #define ui_dport        ui_u.uh_dport
  #define ui_ulen         ui_u.uh_ulen
  #define ui_sum          ui_u.uh_sum
#endif

struct  udpstat {
        /* input statistics: */
        u_long  udps_ipackets;          /* total input packets */
        u_long  udps_hdrops;            /* packet shorter than header */
        u_long  udps_badsum;            /* checksum error */
        u_long  udps_badlen;            /* data length larger than packet */
        u_long  udps_noport;            /* no socket on port */
        u_long  udps_noportbcast;       /* of above, arrived as broadcast */
        u_long  udps_fullsock;          /* not delivered, input socket full */
        u_long  udpps_pcbcachemiss;     /* input packets missing pcb cache */
        u_long  udpps_pcbhashmiss;      /* input packets not for hashed pcb */
                                /* output statistics: */
        u_long  udps_opackets;          /* total output packets */
};

/*
 * Names for UDP sysctl objects
 */
#define UDPCTL_CHECKSUM         1       /* checksum UDP packets */
#define UDPCTL_STATS            2       /* statistics (read-only) */
#define UDPCTL_MAXDGRAM         3       /* max datagram size */
#define UDPCTL_RECVSPACE        4       /* default receive buffer space */
#define UDPCTL_MAXID            5

#define UDPCTL_NAMES { \
        { 0, 0 }, \
        { "checksum", CTLTYPE_INT }, \
        { "stats", CTLTYPE_STRUCT }, \
        { "maxdgram", CTLTYPE_INT }, \
        { "recvspace", CTLTYPE_INT }, \
}

#endif

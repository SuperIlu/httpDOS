/*!\file net/radix.h
 * Radix search trees.
 */

/*
 * Copyright (c) 1988, 1989, 1993
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

#ifndef _NET_RADIX_H_
#define _NET_RADIX_H_

#ifndef __SYS_W32API_H
#include <sys/w32api.h>
#endif

#ifndef __SYS_TYPES_H
#include <sys/wtypes.h>
#endif

#ifndef __SYS_CDEFS_H
#include <sys/cdefs.h>
#endif

__BEGIN_DECLS

/*
 * Radix search tree node layout.
 */

struct radix_node {
       struct  radix_mask     *rn_mklist;  /* list of masks contained in subtree */
       struct  radix_node     *rn_p;       /* parent */
       short                   rn_b;       /* bit offset; -1-index(netmask) */
       char                    rn_bmask;   /* node: mask for bit test*/
       u_char                  rn_flags;   /* enumerated next */
#define RNF_NORMAL  1                      /* leaf contains normal route */
#define RNF_ROOT    2                      /* leaf is root leaf for tree */
#define RNF_ACTIVE  4                      /* This node is alive (for rtfree) */
       union {
         struct {                          /* leaf only data: */
           caddr_t             rn_Key;     /* object of search */
           caddr_t             rn_Mask;    /* netmask, if present */
           struct  radix_node *rn_Dupedkey;
         } rn_leaf;
         struct {                          /* node only data: */
           int                  rn_Off;    /* where to start compare */
           struct  radix_node  *rn_L;      /* progeny */
           struct  radix_node  *rn_R;      /* progeny */
         } rn_node;
       } rn_u;
#ifdef RN_DEBUG
       int                      rn_info;
       struct radix_node       *rn_twin;
       struct radix_node       *rn_ybro;
#endif
     };

#define rn_dupedkey   rn_u.rn_leaf.rn_Dupedkey
#define rn_key        rn_u.rn_leaf.rn_Key
#define rn_mask       rn_u.rn_leaf.rn_Mask
#define rn_off        rn_u.rn_node.rn_Off
#define rn_l          rn_u.rn_node.rn_L
#define rn_r          rn_u.rn_node.rn_R

/*
 * Annotations to tree concerning potential routes applying to subtrees.
 */
struct radix_mask {
       short                 rm_b;         /* bit offset; -1-index(netmask) */
       char                  rm_unused;    /* cf. rn_bmask */
       u_char                rm_flags;     /* cf. rn_flags */
       struct  radix_mask   *rm_mklist;    /* more masks to try */
       union {
         caddr_t             rmu_mask;     /* the mask */
         struct  radix_node *rmu_leaf;     /* for normal routes */
       }                     rm_rmu;
       int                   rm_refs;      /* # of references to this struct */
     };

W32_DATA struct radix_mask *rn_mkfreelist; /* not present */

#define rm_mask rm_rmu.rmu_mask
#define rm_leaf rm_rmu.rmu_leaf     /* extra field would make 32 bytes */

#define MKGet(m)         do {                          \
    if (rn_mkfreelist) {                               \
       m = rn_mkfreelist;                              \
       rn_mkfreelist = (m)->rm_mklist;                 \
    }                                                  \
    else                                               \
      R_Malloc (m, struct radix_mask *, sizeof(*(m))); \
    } while (0)

#define MKFree(m) { (m)->rm_mklist = rn_mkfreelist; rn_mkfreelist = (m);}

struct radix_node_head {
    struct  radix_node *rnh_treetop;
    int rnh_addrsize;       /* permit, but not require fixed keys */
    int rnh_pktsize;        /* permit, but not require fixed keys */
    struct radix_node *(*rnh_addaddr)  /* add based on sockaddr */
            (void *v, void *mask,
             struct radix_node_head *head, struct radix_node nodes[]);
    struct radix_node *(*rnh_addpkt)   /* add based on packet hdr */
            (void *v, void *mask,
             struct radix_node_head *head, struct radix_node nodes[]);
    struct radix_node *(*rnh_deladdr)  /* remove based on sockaddr */
            (void *v, void *mask, struct radix_node_head *head);
    struct radix_node *(*rnh_delpkt)   /* remove based on packet hdr */
            (void *v, void *mask, struct radix_node_head *head);
    struct radix_node *(*rnh_matchaddr)    /* locate based on sockaddr */
            (void *v, struct radix_node_head *head);
    struct radix_node *(*rnh_lookup)   /* locate based on sockaddr */
            (void *v, void *mask, struct radix_node_head *head);
    struct radix_node *(*rnh_matchpkt) /* locate based on packet hdr */
             (void *v, struct radix_node_head *head);
    int (*rnh_walktree)         /* traverse tree */
          (struct radix_node_head *,
           int (*)(struct radix_node *, void *), void *);
    struct radix_node rnh_nodes[3];    /* empty tree for common case */
};


#define Bcmp(a,b,n)     memcmp (a,b,n)
#define Bcopy(a,b,n)    memcpy (b,a,n)
#define Bzero(p,n)      memset ((void*)(p),0,(int)(n));
#define R_Malloc(p,t,n) (p = (t) malloc((unsigned int)(n)))
#define Free(p)         free ((void*)p);

W32_FUNC void W32_CALL rn_init     (void);
W32_FUNC int  W32_CALL rn_inithead (void **, int);
W32_FUNC int  W32_CALL rn_refines  (void *, void *);
W32_FUNC int  W32_CALL rn_walktree (struct radix_node_head *,
                                    int (W32_CALL *)(struct radix_node *, void *),
                                    void *);

W32_FUNC struct radix_node * W32_CALL rn_addroute (void *, void *, struct radix_node_head *, struct radix_node [2]);
W32_FUNC struct radix_node * W32_CALL rn_insert (void *, struct radix_node_head *, int *, struct radix_node [2]);
W32_FUNC struct radix_node * W32_CALL rn_newpair (void *, int, struct radix_node[2]);

W32_FUNC struct radix_node * W32_CALL rn_addmask (void *, int, int);
W32_FUNC struct radix_node * W32_CALL rn_delete (void *, void *, struct radix_node_head *);
W32_FUNC struct radix_node * W32_CALL rn_lookup (void *, void *, struct radix_node_head *);
W32_FUNC struct radix_node * W32_CALL rn_match (void *, struct radix_node_head *);
W32_FUNC struct radix_node * W32_CALL rn_search (void *, struct radix_node *);
W32_FUNC struct radix_node * W32_CALL rn_search_m (void *, struct radix_node *, void *);

__END_DECLS

#endif /* _NET_RADIX_H_ */

/*!\file res_quer.c
 * Resolver query function.
 */

/*
 * ++Copyright++ 1988, 1993
 * -
 * Copyright (c) 1988, 1993
 *    The Regents of the University of California.  All rights reserved.
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
 *   This product includes software developed by the University of
 *   California, Berkeley and its contributors.
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

#include "resolver.h"

#if defined(USE_BIND)

#if (PACKETSZ > 1024)
  #define MAX_PACKET  PACKETSZ
#else
  #define MAX_PACKET  1024
#endif

/*
 * Formulate a normal query, send, and await answer.
 * Returned answer is placed in supplied buffer "answer".
 * Perform preliminary check of answer, returning success only
 * if no error is indicated and the answer count is nonzero.
 * Return the size of the response on success, -1 on error.
 * Error number is left in h_errno.
 *
 * Caller must parse answer and determine whether it answers the question.
 */
int W32_CALL res_query (const char *name, int Class, int type,
                        u_char *answer, int anslen)
{
  u_char  buf[MAX_PACKET];
  HEADER *hp = (HEADER*) answer;
  int     n;

  hp->rcode = NOERROR;  /* default */

  if ((_res.options & RES_INIT) == 0 && res_init() == -1)
  {
    h_errno = NETDB_INTERNAL;
    return (-1);
  }

  if (_res.options & RES_DEBUG)
    (*_printf) (";; res_query(%s, %d, %d)\n", name, Class, type);

  n = res_mkquery (QUERY, name, Class, type, NULL, 0, NULL,
                   buf, sizeof(buf));
  if (n <= 0)
  {
    if (_res.options & RES_DEBUG)
      (*_printf) (";; res_query: mkquery failed\n");

    h_errno = NO_RECOVERY;
    return (n);
  }
  n = res_send (buf, n, answer, anslen);
  if (n < 0)
  {
    if (_res.options & RES_DEBUG)
       (*_printf) (";; res_query: send error\n");

    h_errno = TRY_AGAIN;
    return (n);
  }

  if (hp->rcode != NOERROR || ntohs(hp->ancount) == 0)
  {
    if (_res.options & RES_DEBUG)
       (*_printf) (";; rcode = %d, ancount=%d\n",
                   hp->rcode, ntohs(hp->ancount));

    switch (hp->rcode)
    {
      case NXDOMAIN:
           h_errno = HOST_NOT_FOUND;
           break;
      case SERVFAIL:
           h_errno = TRY_AGAIN;
           break;
      case NOERROR:
           h_errno = NO_DATA;
           break;
      case FORMERR:
      case NOTIMP:
      case REFUSED:
      default:
           h_errno = NO_RECOVERY;
           break;
    }
    return (-1);
  }
  return (n);
}

/*
 * Formulate a normal query, send, and retrieve answer in supplied buffer.
 * Return the size of the response on success, -1 on error.
 * If enabled, implement search rules until answer or unrecoverable failure
 * is detected.  Error code, if any, is left in h_errno.
 */
int W32_CALL res_search (const char *name, int Class, int type,
                         u_char *answer, int anslen)
{
  #define RDBG(x)  (*_printf) ("res_search() = %d at line %u\n", x, __LINE__)

  const char *cp;
  const char *const *domain;
  HEADER     *hp = (HEADER *) answer;
  u_int       dots;
  int         trailing_dot, ret = -1;
  int         saved_herrno;
  int         got_nodata   = 0;
  int         got_servfail = 0;
  int         tried_as_is  = 0;
  int         done         = 0;

  if ((_res.options & RES_INIT) == 0 && res_init() == -1)
  {
    h_errno = NETDB_INTERNAL;
    RDBG (-1);
    return (-1);
  }

  SOCK_ERRNO (0);
  h_errno = 0;       /* default, if we never query */
  dots    = 0;

  for (cp = name; *cp; cp++)
      dots += (*cp == '.');

  trailing_dot = 0;
  if (cp > name && *--cp == '.')
     trailing_dot++;

  /* if there aren't any dots, it could be a user-level alias
   */
  if (!dots && (cp = __hostalias(name)) != NULL)
  {
    int rc = res_query (cp, Class, type, answer, anslen);
    RDBG (rc);
    return (rc);
  }

  /* If there are dots in the name already, let's just give it a try
   * 'as is'.  The threshold can be set with the "ndots" option.
   */
  saved_herrno = -1;
  if (dots >= _res.ndots)
  {
    ret = res_querydomain (name, NULL, Class, type, answer, anslen);
    if (ret > 0)
    {
      RDBG (ret);
      return (ret);
    }
    saved_herrno = h_errno;
    tried_as_is++;
  }

  /*
   * We do at least one level of search if
   *  - there is no dot and RES_DEFNAME is set, or
   *  - there is at least one dot, there is no trailing dot,
   *    and RES_DNSRCH is set.
   */
  if ((!dots && (_res.options & RES_DEFNAMES)) ||
      (dots && !trailing_dot && (_res.options & RES_DNSRCH)))
  {
    for (domain = (const char * const*)_res.dnsrch; *domain && !done; domain++)
    {
      ret = res_querydomain (name, *domain, Class, type, answer, anslen);
      if (ret > 0)
      {
        RDBG (ret);
        return (ret);
      }

      /*
       * If no server present, give up.
       * If name isn't found in this domain,
       * keep trying higher domains in the search list
       * (if that's enabled).
       * On a NO_DATA error, keep trying, otherwise
       * a wildcard entry of another type could keep us
       * from finding this entry higher in the domain.
       * If we get some other error (negative answer or
       * server failure), then stop searching up,
       * but try the input name below in case it's
       * fully-qualified.
       */
      if (errno == ECONNREFUSED)
      {
        h_errno = TRY_AGAIN;
        RDBG (-1);
        return (-1);
      }

      switch (h_errno)
      {
        case NO_DATA:
             got_nodata++;
             /* fallthrough */
        case HOST_NOT_FOUND:
             /* keep trying */
             break;
        case TRY_AGAIN:
             if (hp->rcode == SERVFAIL)
             {
               /* try next search element, if any */
               got_servfail++;
               break;
             }
             /* fallthrough */
        default:
             /* anything else implies that we're done */
             done++;
      }

      /* if we got here for some reason other than DNSRCH,
       * we only wanted one iteration of the loop, so stop.
       */
      if (!(_res.options & RES_DNSRCH))
         done++;
    }
  }

  /* if we have not already tried the name "as is", do that now.
   * note that we do this regardless of how many dots were in the
   * name or whether it ends with a dot.
   */
  if (!tried_as_is)
  {
    ret = res_querydomain (name, NULL, Class, type, answer, anslen);
    if (ret > 0)
    {
      RDBG (ret);
      return (ret);
    }
  }

  /* if we got here, we didn't satisfy the search.
   * if we did an initial full query, return that query's h_errno
   * (note that we wouldn't be here if that query had succeeded).
   * else if we ever got a nodata, send that back as the reason.
   * else send back meaningless h_errno, that being the one from
   * the last DNSRCH we did.
   */
  if (saved_herrno != -1)
     h_errno = saved_herrno;

  else if (got_nodata)
     h_errno = NO_DATA;

  else if (got_servfail)
     h_errno = TRY_AGAIN;

  if (done || h_errno == 0)
     return (1);

  TRACE_CONSOLE (1, "res_search() / h_errno = %d\n", h_errno);
  RDBG (-1);
  return (-1);
}


/*
 * Perform a call on res_query on the concatenation of name and domain,
 * removing a trailing dot from name if domain is NULL.
 */
int W32_CALL res_querydomain (const char *name, const char *domain, int Class,
                              int type, u_char *answer, int anslen)
{
  char  nbuf [2*MAXDNAME+2];
  const char *longname = nbuf;

  if ((_res.options & RES_INIT) == 0 && res_init() == -1)
  {
    h_errno = NETDB_INTERNAL;
    return (-1);
  }

  if (_res.options & RES_DEBUG)
     (*_printf) (";; res_querydomain(%s, %s, %d, %d)\n",
                 name, domain ? domain : "<Nil>", Class, type);

  if (!domain)
  {
    /* Check for trailing '.'; copy without '.' if present.
     */
    size_t len = strlen (name) - 1;

    if (len < sizeof(nbuf)-1 && name[len] == '.')
    {
      memcpy (nbuf, name, len);
      nbuf [len] = '\0';
    }
    else
      longname = name;
  }
  else
  {
    /* won't overflow
     */
    sprintf (nbuf, "%.*s.%.*s", MAXDNAME, name, MAXDNAME, domain);
  }

  return res_query (longname, Class, type, answer, anslen);
}

/*
 * Open and parse hostalias file. Lines may have comments ('#' or ';').
 * Line format is:
 *   [ ].host[ ]*alias{#}.[\r\n]
 */
char * W32_CALL __hostalias (const char *name)
{
  const char *file;
  char   buf[BUFSIZ], *tok_buf = NULL;
  static char abuf[MAXDNAME];
  FILE  *fp;

  if (_res.options & RES_NOALIASES)
     return (NULL);

  file = getenv ("HOSTALIASES");
  if (!file)
     file = res_cfg_aliases;

  if (!file || (fp = fopen(file, "r")) == NULL)
     return (NULL);

  abuf[0] = '\0';

  while (fgets(buf, sizeof(buf), fp))
  {
    char *p, *host, *alias;

    p = strltrim (buf);
    if (!*p || *p == '\n' || *p == '#' || *p == ';')
       continue;

    host  = strtok_r (p, " \t", &tok_buf);
    alias = strtok_r (NULL, " \t\n", &tok_buf);
    if (!host || !alias)
       continue;

    if (!stricmp(host, name))
    {
      _strlcpy (abuf, alias, sizeof(abuf)-1);
      break;
    }
  }
  fclose (fp);
  return (abuf[0] ? abuf : NULL);
}
#endif /* USE_BIND */


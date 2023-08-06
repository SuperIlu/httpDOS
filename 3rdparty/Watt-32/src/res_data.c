/*!\file res_data.c
 * Resolver library.
 */

/* ++Copyright++ 1995
 * -
 * Copyright (c) 1995
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

#if defined(USE_BSD_API) || defined(USE_BIND)

const char *_res_opcodes[] = {
           "QUERY",
           "IQUERY",
           "CQUERYM",
           "CQUERYU",  /* experimental */
           "NOTIFY",   /* experimental */
           "5",
           "6",
           "7",
           "8",
           "UPDATEA",
           "UPDATED",
           "UPDATEDA",
           "UPDATEM",
           "UPDATEMA",
           "ZONEINIT",
           "ZONEREF"
         };

const char *_res_resultcodes[] = {
           "NOERROR",
           "FORMERR",
           "SERVFAIL",
           "NXDOMAIN",
           "NOTIMP",
           "REFUSED",
           "6",
           "7",
           "8",
           "9",
           "10",
           "11",
           "12",
           "13",
           "14",
           "NOCHANGE"
         };

const char *h_errlist[] = {
           "Error 0",                          /* 0 NETDB_SUCCESS */
           "Unknown host",                     /* 1 HOST_NOT_FOUND */
           "Host name lookup failure",         /* 2 TRY_AGAIN */
           "Unknown server error",             /* 3 NO_RECOVERY */
           "No address associated with name",  /* 4 NO_ADDRESS / NO_DATA */
         };

int h_nerr = DIM (h_errlist);

/*
 * herror --
 *  print the error indicated by the h_errno value.
 */
void W32_CALL herror (const char *prefix)
{
  if (prefix && *prefix)
     fprintf (stderr, "%s :", prefix);

  fputs (hstrerror(h_errno), stderr);

#ifdef __DJGPP__
  fputc ('\n', stderr); /* only djgpp(?) use newline, ref. perror() */
#endif
}

const char * W32_CALL hstrerror (int err)
{
  if (err < 0)
     return ("Resolver internal error");
  if (err < h_nerr)
     return (h_errlist[err]);
  return ("Unknown resolver error");
}
#endif /* USE_BSD_API || USE_BIND */


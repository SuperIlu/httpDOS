/*!\file nsapaddr.c
 * inet_nsap_addr(), inet_nsap_ntoa().
 */

/* Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include "socket.h"

#if defined(USE_BSD_API) || defined(USE_BIND)

static __inline char xtob (int c)
{
  return (c - (((c >= '0') && (c <= '9')) ? '0' : '7'));
}

u_int W32_CALL inet_nsap_addr (const char *ascii, u_char *binary, int maxlen)
{
  u_char c, nib;
  int    len = 0;

  while ((c = *ascii++) != '\0' && len < maxlen)
  {
    if (c == '.' || c == '+' || c == '/')
       continue;

    c = toupper (c);
    if (!isxdigit(c))
       return (0);

    nib = xtob (c);
    if ((c = *ascii++) == '\0')
       return (0);

    c = toupper (c);
    if (!isxdigit(c))
       return (0);

    *binary++ = (nib << 4) | xtob(c);
    len++;
  }
  return (u_int)len;
}

char * W32_CALL inet_nsap_ntoa (int binlen, const u_char *binary, char *ascii)
{
  static char tmpbuf[255*3+1];
  char  *start;
  int    i;

  if (ascii)
     start = ascii;
  else
  {
    ascii = tmpbuf;
    start = tmpbuf;
  }

  if (binlen > 255)
      binlen = 255;

  for (i = 0; i < binlen; i++)
  {
    int nib = *binary >> 4;

    *ascii++ = nib + (nib < 10 ? '0' : '7');
    nib = *binary++ & 0x0f;
    *ascii++ = nib + (nib < 10 ? '0' : '7');
    if (((i % 2) == 0 && (i + 1) < binlen))
       *ascii++ = '.';
  }
  *ascii = '\0';
  return (start);
}

#endif

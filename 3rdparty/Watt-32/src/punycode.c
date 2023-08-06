/*
 * punycode.c from RFC 3492
 * http://www.nicemice.net/idn/
 * Adam M. Costello
 * http://www.nicemice.net/amc/
 *
 * This is ANSI C code (C89) implementing Punycode (RFC 3492).
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "wattcp.h"
#include "misc.h"
#include "misc_str.h"
#include "punycode.h"

#if defined(USE_IDNA)

/*** Bootstring parameters for Punycode ***/

enum {
  base = 36, tmin = 1, tmax = 26,
  skew = 38, damp = 700,
  initial_bias = 72,
  initial_n = 0x80,
  delimiter = 0x2D
};

/* basic(cp) tests whether cp is a basic code point:
 */
#define basic(cp) ((DWORD)(cp) < 0x80)

/* delim(cp) tests whether cp is a delimiter:
 */
#define delim(cp) ((cp) == delimiter)

/*
 * decode_digit(cp) returns the numeric value of a basic code
 * point (for use in representing integers) in the range 0 to
 * base-1, or base if cp is does not represent a value.
 */
static DWORD decode_digit (DWORD cp)
{
  return (cp - 48 < 10 ?
          cp - 22 : cp - 65 < 26 ?
          cp - 65 : cp - 97 < 26 ?
          cp - 97 : base);
}

/*
 * encode_digit(d,flag) returns the basic code point whose value
 * (when used for representing integers) is d, which needs to be in
 * the range 0 to base-1.  The lowercase form is used unless flag is
 * nonzero, in which case the uppercase form is used.  The behavior
 * is undefined if flag is nonzero and digit d has no uppercase form.
 */
static char encode_digit (DWORD d, int flag)
{
  return (char) (d + 22 + 75 * (d < 26) - ((flag != 0) << 5));
  /*  0..25 map to ASCII a..z or A..Z */
  /* 26..35 map to ASCII 0..9         */
}

/* flagged(bcp) tests whether a basic code point is flagged
 * (uppercase).  The behavior is undefined if bcp is not a
 * basic code point.
 */
#define flagged(bcp) ((DWORD)(bcp) - 65 < 26)

/*
 * encode_basic(bcp,flag) forces a basic code point to lowercase
 * if flag is zero, uppercase if flag is nonzero, and returns
 * the resulting code point.  The code point is unchanged if it
 * is caseless.  The behavior is undefined if bcp is not a basic
 * code point.
 */
static char encode_basic (DWORD bcp, int flag)
{
  bcp -= (bcp - 97 < 26) << 5;
  return (char) (bcp + ((!flag && (bcp - 65 < 26)) << 5));
}

/* maxint is the maximum value of a DWORD variable:
 */
static const DWORD maxint = (DWORD)-1;

static DWORD adapt (DWORD delta, DWORD numpoints, int firsttime)
{
  DWORD k;

  delta = firsttime ? delta / damp : delta >> 1;
  /* delta >> 1 is a faster way of doing delta / 2
   */
  delta += delta / numpoints;

  for (k = 0; delta > ((base - tmin) * tmax) / 2; k += base)
      delta /= base - tmin;
  return k + (base - tmin + 1) * delta / (delta + skew);
}

/*
 *Main encode function
 */
enum punycode_status punycode_encode (size_t input_length,
                                      const DWORD *input,
                                      const BYTE *case_flags,
                                      size_t *output_length,
                                      char *output)
{
  DWORD n, delta, h, b, out, max_out, bias, j, m, q, k, t;

  /* Initialize the state: */

  n = initial_n;
  delta = out = 0;
  max_out = *output_length;
  bias = initial_bias;

  /* Handle the basic code points: */
  for (j = 0; j < input_length; ++j)
  {
    if (basic (input[j]))
    {
      if (max_out - out < 2)
         return (punycode_big_output);
      output[out++] = case_flags ? encode_basic (input[j], case_flags[j]) :
                                   (char)input[j];
    }
#if 0
    else if (input[j] < n)
         return (punycode_bad_input);
    /* (not needed for Punycode with unsigned code points) */
#endif
  }

  h = b = out;

  /* h is the number of code points that have been handled, b is the
   * number of basic code points, and out is the number of characters
   * that have been output.
   */
  if (b > 0)
     output[out++] = delimiter;

  /* Main encoding loop:
   */
  while (h < input_length)
  {
    /* All non-basic code points < n have been
     * handled already.  Find the next larger one:
     */
    for (m = maxint, j = 0; j < input_length; ++j)
    {
#if 0
      if (basic(input[j]))
          continue;
      /* (not needed for Punycode) */
#endif
      if (input[j] >= n && input[j] < m)
         m = input[j];
    }

    /* Increase delta enough to advance the decoder's
     * <n,i> state to <m,0>, but guard against overflow:
     */
    if (m - n > (maxint - delta) / (h + 1))
       return (punycode_overflow);

    delta += (m - n) * (h + 1);
    n = m;

    for (j = 0; j < input_length; ++j)
    {
      if (input[j] < n)
      {
        if (++delta == 0)
           return (punycode_overflow);
      }

      if (input[j] == n)
      {
        /* Represent delta as a generalized variable-length integer:
         */
        for (q = delta, k = base;; k += base)
        {
          if (out >= max_out)
             return (punycode_big_output);

          t = k <= bias ? tmin :
              k >= bias + tmax ? tmax :
              k - bias;
          if (q < t)
             break;
          output[out++] = encode_digit (t + (q - t) % (base - t), 0);
          q = (q - t) / (base - t);
        }
        output[out++] = encode_digit (q, case_flags && case_flags[j]);
        bias = adapt (delta, h + 1, h == b);
        delta = 0;
        ++h;
      }
    }
    ++delta;
    ++n;
  }

  *output_length = out;
  return (punycode_success);
}

/*** Main decode function ***/

enum punycode_status punycode_decode (DWORD input_length,
                                      const char *input,
                                      size_t *output_length,
                                      DWORD *output,
                                      BYTE *case_flags)
{
  DWORD n, out, i, max_out, bias, b, j, in, oldi, w, k, digit, t;

  /* Initialize the state: */

  n = initial_n;
  out = i = 0;
  max_out = *output_length;
  bias = initial_bias;

  /* Handle the basic code points:  Let b be the number of input code
   * points before the last delimiter, or 0 if there is none, then
   * copy the first b code points to the output.
   */
  for (b = j = 0; j < input_length; ++j)
      if (delim (input[j]))
         b = j;
  if (b > max_out)
     return (punycode_big_output);

  for (j = 0; j < b; ++j)
  {
    if (case_flags)
       case_flags[out] = flagged (input[j]);
    if (!basic (input[j]))
       return (punycode_bad_input);
    output[out++] = input[j];
  }

  /* Main decoding loop:  Start just after the last delimiter if any
   * basic code points were copied; start at the beginning otherwise.
   */
  for (in = (b > 0) ? (b + 1) : 0; in < input_length; ++out)
  {
    /* in is the index of the next character to be consumed, and
     * out is the number of code points in the output array.
     */

    /* Decode a generalized variable-length integer into delta,
     * which gets added to i.  The overflow checking is easier
     * if we increase i as we go, then subtract off its starting
     * value at the end to obtain delta.
     */
    for (oldi = i, w = 1, k = base;; k += base)
    {
      if (in >= input_length)
         return (punycode_bad_input);

      digit = decode_digit (input[in++]);
      if (digit >= base)
         return (punycode_bad_input);

      if (digit > ((maxint - i) / w))
         return (punycode_overflow);

      i += digit * w;
      t = k <= bias ? tmin :
          k >= bias + tmax ? tmax :
          k - bias;
      if (digit < t)
         break;
      if (w > maxint / (base - t))
         return (punycode_overflow);

      w *= (base - t);
    }

    bias = adapt (i - oldi, out + 1, oldi == 0);

    /* i was supposed to wrap around from out+1 to 0,
     * incrementing n each time, so we'll fix that now:
     */
    if (i / (out + 1) > maxint - n)
       return (punycode_overflow);

    n += i / (out + 1);
    i %= (out + 1);

    /* Insert n at position i of the output:
     */
#if 0
    /* not needed for Punycode: */
    if (decode_digit(n) <= base)
       return (punycode_invalid_input);
#endif
    if (out >= max_out)
       return (punycode_big_output);

    if (case_flags)
    {
      memmove (case_flags + i + 1, case_flags + i, out - i);

      /* Case of last character determines uppercase flag:
       */
      case_flags[i] = flagged (input[in - 1]);
    }
    memmove (output + i + 1, output + i, (out - i) * sizeof *output);
    output[i++] = n;
  }

  *output_length = out;
  return (punycode_success);
}
#endif  /* USE_IDNA */

#if defined(TEST_PROG)
#if !defined(USE_IDNA)
int main (void)
{
  puts ("This program needs '#define USE_IDNA'");
  return (1);
}
#else  /* rest of file */

/* For testing, we'll just set some compile-time limits rather than
 * use malloc(), and set a compile-time option rather than using a
 * command-line option.
 */
#define UNICODE_MAX_LENGTH  256
#define ACE_MAX_LENGTH      256

static void usage (char **argv)
{
  fprintf (stderr,
           "\n"
           "%s -e reads code points and writes a Punycode string.\n"
           "%s -d reads a Punycode string and writes code points.\n"
           "\n"
           "Input and output are plain text in the native character set.\n"
           "Code points are in the form u+hex separated by whitespace.\n"
           "Although the specification allows Punycode strings to contain\n"
           "any characters from the ASCII repertoire, this test code\n"
           "supports only the printable characters, and needs the Punycode\n"
           "string to be followed by a newline.\n"
           "The case of the u in u+hex is the force-to-uppercase flag.\n",
           argv[0], argv[0]);
  exit (EXIT_FAILURE);
}

#define fail(msg) __fail (__LINE__, msg)
static void __fail (unsigned line, const char *msg)
{
  fprintf (stderr, "line %u: %s", line, msg);
  exit (EXIT_FAILURE);
}

static const char too_big[]       = "input or output is too large, recompile with larger limits\n";
static const char invalid_input[] = "invalid input\n";
static const char overflow[]      = "arithmetic overflow\n";
static const char io_error[]      = "I/O error\n";

/* The following string is used to convert printable
 * characters between ASCII and the native charset:
 */
static const char print_ascii[] = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                                  "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
                                  " !\"#$%&'()*+,-./" "0123456789:;<=>?"
                                  "@ABCDEFGHIJKLMNO"
                                  "PQRSTUVWXYZ[\\]^_"
                                  "`abcdefghijklmno"
                                  "pqrstuvwxyz{|}~\n";

int main (int argc, char **argv)
{
  enum punycode_status status;
  size_t input_length, output_length, j;
  BYTE   case_flags [UNICODE_MAX_LENGTH];
  int    r;

  if (argc != 2 || argv[1][0] != '-' || argv[1][2] != 0)
     usage (argv);

  if (argv[1][1] == 'e')
  {
    DWORD  input [UNICODE_MAX_LENGTH];
    u_long codept;
    char   output [ACE_MAX_LENGTH+1], uplus[3];
    int    c;

    /* Read the input code points:
     */
    input_length = 0;

    for (;;)
    {
      r = scanf ("%2s%lx", uplus, &codept);
      if (ferror (stdin))
         fail (io_error);
      if (r == EOF || r == 0)
         break;

      if (r != 2 || uplus[1] != '+' || codept > (DWORD)-1)
         fail (invalid_input);

      if (input_length == UNICODE_MAX_LENGTH)
         fail (too_big);

      if (uplus[0] == 'u')
           case_flags[input_length] = 0;
      else if (uplus[0] == 'U')
           case_flags[input_length] = 1;
      else fail (invalid_input);

      input[input_length++] = (DWORD)codept;
    }

    /* Encode:
     */
    output_length = ACE_MAX_LENGTH;
    status = punycode_encode (input_length, input, case_flags,
                              &output_length, output);
    if (status == punycode_bad_input)
       fail (invalid_input);
    if (status == punycode_big_output)
       fail (too_big);
    if (status == punycode_overflow)
       fail (overflow);
    WATT_ASSERT (status == punycode_success);

    /* Convert to native charset and output:
     */
    for (j = 0; j < output_length; ++j)
    {
      c = output[j];
      WATT_ASSERT (c >= 0 && c <= 127);
      if (print_ascii[c] == 0)
         fail (invalid_input);
      output[j] = print_ascii[c];
    }

    output[j] = '\0';
    r = puts (output);
    if (r == EOF)
       fail (io_error);
    return (EXIT_SUCCESS);
  }

  if (argv[1][1] == 'd')
  {
    char  input [ACE_MAX_LENGTH+2], *p, *pp;
    DWORD output [UNICODE_MAX_LENGTH];

    /* Read the Punycode input string and convert to ASCII: */

    fgets (input, ACE_MAX_LENGTH+2, stdin);
    if (ferror (stdin))
       fail (io_error);

    if (feof (stdin))
       fail (invalid_input);

    input_length = strlen (input) - 1;
    if (input[input_length] != '\n')
       fail (too_big);

    input[input_length] = '\0';

    for (p = input; *p != 0; ++p)
    {
      pp = strchr (print_ascii, *p);
      if (pp == 0)
         fail (invalid_input);
      *p = pp - print_ascii;
    }

    /* Decode:
     */
    output_length = UNICODE_MAX_LENGTH;
    status = punycode_decode (input_length, input, &output_length,
                              output, case_flags);
    if (status == punycode_bad_input)
       fail (invalid_input);
    if (status == punycode_big_output)
       fail (too_big);
    if (status == punycode_overflow)
       fail (overflow);
    WATT_ASSERT (status == punycode_success);

    /* Output the result:
     */
    for (j = 0; j < output_length; ++j)
        printf ("%s+%04lX\n", case_flags[j] ? "U" : "u", DWORD_CAST(output[j]));
    return (EXIT_SUCCESS);
  }
  usage (argv);
  return (EXIT_SUCCESS);
}
#endif   /* USE_IDNA */
#endif   /* TEST_PROG */

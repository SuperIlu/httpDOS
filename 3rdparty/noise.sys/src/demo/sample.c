/*

   A demo program which samples a given number of bytes from the
   /dev/random driver and writes them to a file.

   Added -a option to append data to file, 10 Jan 96
   Fixed bugs with opening /dev/random when driver uninstalled,
     and when a write error occurs with file, 14 Jan 96
   Removed suspicious pointers, fixed for use with /dev/urandom, 15 Jan 96
   Added -r and -u options to read from /dev/random and /dev/urandom,
     respectively; added -s option to read until file is num bytes in
     size, use of RNG_DEVICE environment variable for default, 6 Apr 96
   Removed C++ style comments for gcc friendliness, 8 Apr 96
   Fixed minor bug in -v message when >65535 bytes appended, 7 May 96
   Updated for v0.5.7 of NOISE.SYS, 19 May 96

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>

#include "usuals.h"

#ifdef __MSDOS__
#define SIZE 8192 /* size of buffer */
#define DEFAULT_DEV "/dev/random$"
#else
#define SIZE 1024
#define DEFAULT_DEV "/dev/random"
#endif

void usage(void) {
  fprintf(stderr,
          "usage: sample file num [-asruv] [dev]\n"
          "examples: sample foo.bar 256 -ar\n"
          "\tsample randseed.bin 408 -s\n"
          "\tsample noiz 64 -a /dev/rand\n"
          "Samples num bytes from the " DEFAULT_DEV " device\n"
          "  -a\tappend num bytes to existing file (default is overwrite)\n"
          "  -s\tappend bytes until file is num bytes long\n"
          "  -r\tread bytes from /dev/random\n"
          "  -u\tread bytes from /dev/urandom\n"
          "  -v\tverbose output\n"
          "  dev\tread bytes from alternate device or file\n\n"
          "The default device is set in the RNG_DEVICE environment\n"
          "variable (if none is set, " DEFAULT_DEV " is assumed)\n");
  exit(EXIT_FAILURE);
}

void main(int argc, char *argv[]) {

  FILE *randstream, *ofile;
  struct stat *ostatbuf;
  word32 num, total = 0;
  int i, j, verbose = 0;
  unsigned int bytesread, needed = 0;
  static char buff[SIZE], *ofname, *filemode, *devname;

  if (argc < 3)
    usage();
  /* -------------------------------------------------------------------------
     WARNING: This doesn't check if a file exists already, so it's very
     possible to overwrite a file out of carelessness!
  -------------------------------------------------------------------------- */
  else {
    filemode = "wb";
    devname = getenv("RNG_DEVICE");
    if (strcmp(devname, "") == 0)
      devname = DEFAULT_DEV;

    ofname = argv[1];
    num = 0;
    sscanf(argv[2], "%ld", &num);

    if (argc > 3)
      for (i = 3; i < argc; i++) {
        if (argv[i][0] == '-')
          for (j = 1; j < strlen(argv[i]); j++) {
#ifdef DEBUG
            printf("argv[%d][%d] = %c\n", i, j, argv[i][j]);
#endif
            switch (argv[i][j]) {
            case 'r':
#ifdef __MSDOS__
              devname = "/dev/random$";
#else
              devname = "/dev/random";
#endif
              break;
            case 'u':
#ifdef __MSDOS__
              devname = "/dev/urandom$";
#else
              devname = "/dev/urandom";
#endif
              break;
            case 's':
              if (stat(ofname, ostatbuf) == 0) {
                if (num > ostatbuf->st_size)
                  num -= ostatbuf->st_size;
                else
                  num = 0;
              }
            case 'a':
              filemode = "ab"; /* append mode */
              break;
            case 'v':
              verbose = 1;
              break;
            }
          }
        else
          devname = argv[i];
      }
    /* ----- open random stream in binary mode ----- */
    if ((randstream = fopen(devname, "rb")) <= 0) {
      /* sometimes if device is not loaded, randstream == 0 */
      fprintf(stderr, "error opening %s\n", devname);
      exit(EXIT_FAILURE);
    } else if ((ofile = fopen(ofname, filemode)) < 0) {
      fprintf(stderr, "unable to open file %s\n", ofname);
      exit(EXIT_FAILURE);
    } else {
#ifdef DEBUG
      fprintf(stderr, "open file %s (mode %s) of %ld bytes (requested)\n",
              ofname, filemode, num);
#endif
      do {
        needed = (SIZE > num) ? (needed = num) : (needed = SIZE);
        /* ----- read bytes from the random stream ----- */
        bytesread = fread(&buff, 1, needed, randstream);
#ifdef DEBUG
        fprintf(stderr, "%d bytes read from device %s\n", bytesread, devname);
#endif
        if (bytesread)
          num -= ((i = fwrite(&buff, 1, bytesread, ofile)) >= 0) ? i : 0;
        total += bytesread;
      } while ((num != 0) && (bytesread == needed) && (i == bytesread));
      fclose(ofile);
      if (verbose)
        printf("%ld bytes appended to %s\n", total, ofname);
      exit(EXIT_SUCCESS);
    }
  }
}

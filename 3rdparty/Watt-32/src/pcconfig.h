/*!\file pcconfig.h
 */
#ifndef _w32_PCCONFIG_H
#define _w32_PCCONFIG_H

#if !defined(USE_BUFFERED_IO)
#include <fcntl.h> /* Defines O_RDONLY and O_BINARY, needed by FOPEN_x macros */
#endif

#define sock_inactive      W32_NAMESPACE (sock_inactive)
#define sock_data_timeout  W32_NAMESPACE (sock_data_timeout)
#define sock_delay         W32_NAMESPACE (sock_delay)
#define multihomes         W32_NAMESPACE (multihomes)
#define print_hook         W32_NAMESPACE (print_hook)
#define usr_post_init      W32_NAMESPACE (usr_post_init)
#define Cookies            W32_NAMESPACE (Cookies)
#define last_cookie        W32_NAMESPACE (last_cookie)
#define ctrace_on          W32_NAMESPACE (ctrace_on)
#define dynamic_host       W32_NAMESPACE (dynamic_host)
#define get_argv0          W32_NAMESPACE (get_argv0)
#define parse_config_table W32_NAMESPACE (parse_config_table)
#define expand_var_str     W32_NAMESPACE (expand_var_str)

extern int  sock_inactive;
extern int  sock_data_timeout;
extern int  sock_delay;
extern WORD multihomes;

extern void (W32_CALL *print_hook)   (const char *);

extern DWORD Cookies [MAX_COOKIES];
extern WORD  last_cookie;
extern int   ctrace_on;
extern BOOL  dynamic_host;
extern int   stkwalk_details;
extern int   fortify_fail_rate;

extern const struct config_table *watt_init_cfg;

extern const char *get_argv0  (void);
extern void       _add_server (WORD *counter, DWORD *array, int array_dim, DWORD value);
extern int         netdb_init (void);
extern void        netdb_warn (const char *fname);
extern const char *expand_var_str (const char *str);

/*
 * Using buffered I/O speeds up reading config-file, but uses more data/code.
 * Non-DOSX targets where memory is tight doesn't have "#define USE_BUFFERED_IO"
 * by default (see config.h)
 */

#undef FREAD /* In CygWin's /usr/include/sys/_default_fcntl.h */

#if defined(USE_BUFFERED_IO)  /* The normal case */
  typedef FILE*                 WFILE;

  #define FOPEN_BIN(fil, name)   (fil = fopen (name, "rb"))
  #define FOPEN_TXT(fil, name)   (fil = fopen (name, "rt"))
  #define FOPEN_APP(fil, name)   (fil = fopen (name, "at"))
  #define FREAD(ptr, fil)        fread ((char*)(ptr), 1, 1, fil)
  #define FCLOSE(fil)            fclose (fil)

#else

  /* Use this simple read-ahead cache when memory is tight.
   * A 32 byte cache seems plenty to speed-up the reading of
   * the "wattcp.cfg".
   */
  struct FCFILE {
    int   fd;
    char  cache [32];
    short cache_pos;
    short cache_max;
  };

  typedef struct FCFILE*         WFILE;

  #if 0   /* Old version */
    #define FOPEN_BIN(fil, name)    (fil = open (name, O_RDONLY|O_BINARY), (fil != -1))
    #define FOPEN_TXT(fil, name)    (fil = open (name, O_RDONLY|O_TEXT),   (fil != -1))
  #else
    #define FOPEN_BIN(fil, name)    (fil = fc_open(name, O_BINARY))
    #define FOPEN_TXT(fil, name)    (fil = fc_open(name, O_TEXT))
  #endif

  #define FOPEN_APP(fil,name)   UNIMPLEMENTED()
  #define FREAD(ptr,fil)        fc_readbyte (fil, (char*)(ptr))
  #define FCLOSE(fil)           fc_close (fil)

  extern struct FCFILE *fc_open (const char *fname, int mode);
  extern void           fc_close (struct FCFILE *f);
  extern int            fc_readbyte (struct FCFILE *f, char *c);
#endif

extern long  tcp_parse_file (WFILE fil, const struct config_table *cfg);

#endif


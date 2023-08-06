#ifndef __MAIN_H__
#define __MAIN_H__

#define SYSINFO ">>> "     //!< logfile line prefix for system messages
#define LOGSTREAM logfile  //!< output stream for logging on DOS

//! printf-style write info to logfile/console
#define LOGF(str, ...)                                  \
    if (LOGSTREAM) {                                    \
        fprintf(LOGSTREAM, SYSINFO str, ##__VA_ARGS__); \
        fflush(LOGSTREAM);                              \
    }                                                   \
    fprintf(stderr, SYSINFO str, ##__VA_ARGS__);

//! write info to logfile/console
#define LOG(str)                       \
    if (LOGSTREAM) {                   \
        fputs(SYSINFO str, LOGSTREAM); \
        fflush(LOGSTREAM);             \
    }                                  \
    fputs(SYSINFO str, stderr);

extern FILE *logfile;  //!< file for log output.

#endif  // __MAIN_H__

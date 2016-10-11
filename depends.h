#ifdef __i386__
#  define FMT_SIZEOF "%u"
#  define FMT_CADDR  "%d"
#  ifdef __linux__
#    define FMT_TV     "%ld.%06ld"
#    define FMT_TS     "%ld.%09ld"
#  else
     /* FreeBSD x86 */
#    define FMT_TV     "%d.%06ld"
#    define FMT_TS     "%d.%09ld"
#  endif
#else
#  define FMT_SIZEOF "%ld"
#  define FMT_CADDR  "%ld"
#  ifdef __linux__
#    define FMT_TV     "%ld.%06ld"
#    define FMT_TS     "%ld.%09ld"
#  else
     /* APPLE, NetBSD amd64 */
#    define FMT_TV     "%ld.%06d"
#    define FMT_TS     "%ld.%09ld"
#  endif
#endif

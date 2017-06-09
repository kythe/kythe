/* mem_config.h.  Generated from mem_config.in by configure, then
 * modified to work under Linux and macOS.
 */
/* mem_config.in.  Generated from configure.ac by autoheader.  */


#pragma once

/* _SYS_FEATURE_TESTS_H is Solaris, _FEATURES_H is GCC */
#if defined( _SYS_FEATURE_TESTS_H) || defined(_FEATURES_H)
#error "You should include mem_config.h as your first include file"
#endif



/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* the location of <cinttypes> */
#define CINTTYPES_H <cinttypes>

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
/* #undef CRAY_STACKSEG_END */

/* the location of <cstdint> */
#define CSTDINT_H <cstdint>

/* Define to 1 if using `alloca.c'. */
/* #undef C_ALLOCA */

/* Define to 1 to enable debugging code. */
#define DEBUG 0

/* Support for DrizzleD */
#define DRIZZLED_BINARY 0

/* Another magical number */
/* #undef EAI_SYSTEM */

/* machine is big-endian */
/* #undef ENDIAN_BIG */

/* machine is little-endian */
/* #undef ENDIAN_LITTLE */

/* Name of the gearmand binary used in make test */
#define GEARMAND_BINARY 0

/* Support for Gearman Blobslap worker */
#define GEARMAND_BLOBSLAP_WORKER 0

/* Define to 1 if you have the `alarm' function. */
#define HAVE_ALARM 1

/* Define to 1 if you have `alloca', as a function or macro. */
#define HAVE_ALLOCA 1

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#define HAVE_ALLOCA_H 1

/* Define to 1 if you have the <arpa/inet.h> header file. */
#define HAVE_ARPA_INET_H 1

/* Define to 1 if you have the `atexit' function. */
#define HAVE_ATEXIT 1

/* Define to 1 if you have the `atomic_add_32' function. */
/* #undef HAVE_ATOMIC_ADD_32 */

/* Define to 1 if you have the `atomic_add_64' function. */
/* #undef HAVE_ATOMIC_ADD_64 */

/* Define to 1 if you have the <atomic.h> header file. */
/* #undef HAVE_ATOMIC_H */

/* Have clock_gettime */
#define HAVE_CLOCK_GETTIME 0

/* Define to 1 if you have the declaration of `strerror_r', and to 0 if you
   don't. */
#define HAVE_DECL_STRERROR_R 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Support for DrizzleD */
#define HAVE_DRIZZLED_BINARY 0

/* Define this if you have an implementation of drop_privileges() */
/* #undef HAVE_DROP_PRIVILEGES */

/* Enables DTRACE Support */
/* #undef HAVE_DTRACE */

/* Define to 1 if you have the `dup2' function. */
#define HAVE_DUP2 1

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the <execinfo.h> header file. */
#define HAVE_EXECINFO_H 1

/* Define to 1 if you have the `fcntl' function. */
#define HAVE_FCNTL 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define to 1 if you have the <features.h> header file. */
/* #undef HAVE_FEATURES_H */

/* Define to 1 if you have the `floor' function. */
/* #undef HAVE_FLOOR */

/* Define to 1 if you have the <fnmatch.h> header file. */
#define HAVE_FNMATCH_H 1

/* Enables fnv64 hashing support */
#define HAVE_FNV64_HASH 1

/* Define to 1 if you have the `fork' function. */
#define HAVE_FORK 1

/* define if the compiler supports GCC C++ ABI name demangling */
#define HAVE_GCC_ABI_DEMANGLE 1

/* Define to 1 if compiler provides atomic builtins. */
#define HAVE_GCC_ATOMIC_BUILTINS 1

/* If Gearmand binary is available */
#define HAVE_GEARMAND_BINARY 0

/* Define to 1 if you have the `getcwd' function. */
#define HAVE_GETCWD 1

/* Define to 1 if you have the `getline' function. */
#define HAVE_GETLINE 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Enables hsieh hashing support */
/* #undef HAVE_HSIEH_HASH */

/* Have ntohll */
/* #undef HAVE_HTONLL */

/* Define to 1 if you have the `inet_ntoa' function. */
#define HAVE_INET_NTOA 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if the system has the type `in_port_t'. */
#define HAVE_IN_PORT_T 1

/* Support for libcurl */
#define HAVE_LIBCURL 0

/* Support for libdrizzle */
#define HAVE_LIBDRIZZLE 0

/* Define if event_init is present in event.h. */
#define HAVE_LIBEVENT 1

/* Define to 1 to compile in libgearman support */
#define HAVE_LIBGEARMAN 0

/* Define to 1 if you have the <libintl.h> header file. */
/* #undef HAVE_LIBINTL_H */

/* Enables libmemcached Support */
#define HAVE_LIBMEMCACHED 1

/* Define to 1 if MySQL libraries are available */
#define HAVE_LIBMYSQL_BUILD 0

/* Support for Postgres */
#define HAVE_LIBPQ 0

/* Define to 1 if LIBSASL is found */
/* #undef HAVE_LIBSASL 0 */

/* Define to 1 if you have the <limits.h> header file. */
#define HAVE_LIMITS_H 1

/* Define to 1 if you have the <mach/mach.h> header file. */
/* #undef HAVE_MACH_MACH_H */

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef HAVE_MALLOC_H */

/* Define to 1 if you have the <math.h> header file. */
#define HAVE_MATH_H 1

/* Define to 1 if MCHECK is found */
/* #undef HAVE_MCHECK */

/* If Memcached binary is available */
#define HAVE_MEMCACHED_BINARY 0

/* If Memcached binary with sasl support is available */
#define HAVE_MEMCACHED_SASL_BINARY 0

/* Define to 1 if you have the `memchr' function. */
#define HAVE_MEMCHR 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define HAVE_MEMSET 1

/* Define to 1 if you have a MSG_DONTWAIT */
#define HAVE_MSG_DONTWAIT 1

/* Define to 1 if you have a MSG_MORE */
/* #undef HAVE_MSG_MORE */


/* Define to 1 if you have a MSG_NOSIGNAL */
/* #undef HAVE_MSG_NOSIGNAL */

/* Enables murmur hashing support */
#define HAVE_MURMUR_HASH 1

/* Define to 1 if MySQL libraries are available */
/* #undef HAVE_MYSQL */

/* Define to 1 if MySQLD binary is available */
#define HAVE_MYSQLD_BUILD 0

/* Define to 1 if you have the <netdb.h> header file. */
#define HAVE_NETDB_H 1

/* Define to 1 if you have the <netinet/in.h> header file. */
#define HAVE_NETINET_IN_H 1

/* Define to 1 if you have the <netinet/tcp.h> header file. */
#define HAVE_NETINET_TCP_H 1

/* Define to 1 if you have the `pipe2' function. */
/* #undef HAVE_PIPE2 */

/* Define to 1 if you have the <poll.h> header file. */
#define HAVE_POLL_H 1

/* Define if you have POSIX threads libraries and header files. */
#define HAVE_PTHREAD 1

/* Define to 1 if you have the <pthread.h> header file. */
#define HAVE_PTHREAD_H 1

/* Have PTHREAD_PRIO_INHERIT. */
#define HAVE_PTHREAD_PRIO_INHERIT 1

/* Define if pthread_timedjoin_np is present in pthread.h. */
#define HAVE_PTHREAD_TIMEDJOIN_NP 0

/* Define to 1 if the system has the type `ptrdiff_t'. */
#define HAVE_PTRDIFF_T 1

/* Define to 1 if you have the `putenv' function. */
#define HAVE_PUTENV 1

/* Define to 1 if you have a working SO_RCVTIMEO */
#define HAVE_RCVTIMEO 1

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* Define to 1 if you have the `setenv' function. */
#define HAVE_SETENV 1

/* Define to 1 if you have the `setppriv' function. */
/* #undef HAVE_SETPPRIV */

/* Define to 1 if you have the `sigignore' function. */
#define HAVE_SIGIGNORE 1

/* Define to 1 if you have a working SO_SNDTIMEO */
#define HAVE_SNDTIMEO 1

/* Define to 1 if you have the `socket' function. */
#define HAVE_SOCKET 1

/* Define to 1 if you have the <spawn.h> header file. */
#define HAVE_SPAWN_H 1

/* Define to 1 if you have the `sqrt' function. */
/* #undef HAVE_SQRT */

/* Define to 1 if you have the <stdarg.h> header file. */
#define HAVE_STDARG_H 1

/* Define to 1 if stdbool.h conforms to C99. */
#define HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strcasecmp' function. */
#define HAVE_STRCASECMP 1

/* Define to 1 if you have the `strchr' function. */
#define HAVE_STRCHR 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strerror' function. */
#define HAVE_STRERROR 1

/* Define to 1 if you have the `strerror_r' function. */
#define HAVE_STRERROR_R 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if you have the `strtol' function. */
#define HAVE_STRTOL 1

/* Define to 1 if you have the `strtoul' function. */
#define HAVE_STRTOUL 1

/* Define to 1 if you have the `strtoull' function. */
#define HAVE_STRTOULL 1

/* Define to 1 if you have the <syslog.h> header file. */
#define HAVE_SYSLOG_H 1

/* Define to 1 if you have the <sys/sdt.h> header file. */
/* #undef HAVE_SYS_SDT_H */

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/sysctl.h> header file. */
#define HAVE_SYS_SYSCTL_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/un.h> header file. */
#define HAVE_SYS_UN_H 1

/* Define to 1 if you have the <sys/wait.h> header file. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define this if you have umem.h */
/* #undef HAVE_UMEM_H */

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define if uuid_generate_time_safe is present in uuid/uuid.h. */
#define HAVE_UUID_GENERATE_TIME_SAFE 0

/* Have uuid/uuid.h */
#define HAVE_UUID_UUID_H 1

/* Define to 1 if you have the `vfork' function. */
#define HAVE_VFORK 1

/* Define to 1 if you have the <vfork.h> header file. */
/* #undef HAVE_VFORK_H */

/* Define to 1 or 0, depending whether the compiler supports simple visibility
   declarations. */
#define HAVE_VISIBILITY 1

/* Define to 1 if you have the <winsock2.h> header file. */
/* #undef HAVE_WINSOCK2_H */

/* Define to 1 if `fork' works. */
#define HAVE_WORKING_FORK 1

/* Define to 1 if `vfork' works. */
#define HAVE_WORKING_VFORK 1

/* Define to 1 if you have the <ws2tcpip.h> header file. */
/* #undef HAVE_WS2TCPIP_H */

/* Define to 1 if the system has the type `_Bool'. */
#define HAVE__BOOL 1

/* CPU of Build System */
#define HOST_CPU "x86_64"

/* OS of Build System */
#define HOST_OS "unknown"

/* Whether we are building for FreeBSD */
/* #undef HOST_OS_FREEBSD */

/* Whether we build for Linux */
/* #undef HOST_OS_LINUX */

/* Whether we are building for Solaris */
/* #undef HOST_OS_SOLARIS */

/* Whether we are building for Windows */
/* #undef HOST_OS_WINDOWS */

/* Vendor of Build System */
#define HOST_VENDOR "unknown"

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Name of the memcached binary used in make test */
/* #undef MEMCACHED_BINARY */

/* Memcached binary version */
/* #undef MEMCACHED_VERSION */

/* Name of the mysqld binary used in make test */
#define MYSQLD_BINARY 0

/* Define to 1 to disable assert'ing code. */
#define NDEBUG 1

/* Name of package */
#define PACKAGE "libmemcached"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "http://libmemcached.org/"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libmemcached"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libmemcached 1.0.18"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libmemcached"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.0.18"

/* Define to necessary symbol if this constant uses a non-standard name on
   your system. */
/* #undef PTHREAD_CREATE_JOINABLE */

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if strerror_r returns char *. */
/* #undef STRERROR_R_CHAR_P */

/* Define to true if you want to use functions from atomic.h */
/* #undef USE_ATOMIC_H */

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Define if the code was built from VCS. */
#define VCS_CHECKOUT 0

/* VCS system */
#define VCS_SYSTEM "none"

/* Version number of package */
#define VERSION "1.0.18"

/* machine is big-endian */
/* #undef WORDS_BIGENDIAN */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT64_T */

/* Define for Solaris 2.5.1 so the uint8_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT8_T */

/* Workaround for bug in FreeBSD headers */
/* #undef __APPLE_CC__ */

/* Use STDC Limit Macros in C++ */
#define __STDC_LIMIT_MACROS 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to the type of a signed integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int16_t */

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int64_t */

/* Define to the type of a signed integer type of width exactly 8 bits if such
   a type exists and the standard includes do not define it. */
/* #undef int8_t */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to the equivalent of the C99 'restrict' keyword, or to
   nothing if this is not supported.  Do not define if restrict is
   supported directly.  */
#define restrict __restrict
/* Work around a bug in Sun C++: it does not support _Restrict or
   __restrict__, even though the corresponding Sun C compiler ends up with
   "#define restrict _Restrict" or "#define restrict __restrict__" in the
   previous line.  Perhaps some future version of Sun C++ will work with
   restrict; if so, hopefully it defines __RESTRICT like Sun C does.  */
#if defined __SUNPRO_CC && !defined __RESTRICT
# define _Restrict
# define __restrict__
#endif

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef ssize_t */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint64_t */

/* Define to the type of an unsigned integer type of width exactly 8 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint8_t */

/* Define as `fork' if `vfork' does not work. */
/* #undef vfork */

/* Define to empty if the keyword `volatile' does not work. Warning: valid
   code using `volatile' can become incorrect without. Disable with care. */
/* #undef volatile */


#ifndef HAVE_SYS_SOCKET_H
# define SHUT_RD SD_RECEIVE
# define SHUT_WR SD_SEND
# define SHUT_RDWR SD_BOTH
#endif




#ifndef __STDC_FORMAT_MACROS
#  define __STDC_FORMAT_MACROS
#endif

#if defined(__cplusplus)
#  include CINTTYPES_H
#else
#  include <inttypes.h>
#endif

#if !defined(HAVE_ULONG) && !defined(__USE_MISC)
# define HAVE_ULONG 1
typedef unsigned long int ulong;
#endif

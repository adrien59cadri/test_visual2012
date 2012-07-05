/* mus-config.h.  Generated from mus-config.h.in by configure.  */
#ifndef SNDLIB_CONFIG_H_LOADED
#define SNDLIB_CONFIG_H_LOADED

/* Define to `int' or something if <sys/types.h> doesn't define.  */
/* #undef mode_t */
/* #undef pid_t */
/* #undef size_t */
/* #undef ssize_t */
/* #undef off_t */
/* #undef int64_t */

#define HAVE_GETCWD 1
/* #undef HAVE_GETWD */
#define HAVE_STRFTIME 1
#define HAVE_STRERROR 1
/* #undef HAVE_READLINK */
/* #undef HAVE_GETLINE */
#define HAVE_SETLOCALE 1
/* #undef HAVE_SLEEP */
/* #define HAVE_ACCESS 1            HKT */
#define HAVE_OPENDIR 1
#define HAVE_SIGNAL 1
/* #undef USE_STATVFS */
/* #undef USE_STATFS */
#define HAVE_DIFFTIME 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_VSNPRINTF 1
#define HAVE_SNPRINTF 1
#define HAVE_MEMMOVE 1
#define HAVE_FILENO 1
/* #undef HAVE_LSTAT */
/* #define HAVE_STRCASECMP 1        HKT */
/* #undef HAVE_PATHCONF */
#define HAVE_DECL_HYPOT 1
/* #define HAVE_DECL_ISNAN 1        HKT */
/* #define HAVE_DECL_ISINF 1        HKT */


/* #undef HAVE_COMPLEX_TRIG */
/* #define WITH_COMPLEX 1           HKT */

/* #undef HAVE_NESTED_FUNCTIONS */
/* #undef WORDS_BIGENDIAN */

#define HAVE_FCNTL_H 1
#define HAVE_LIMITS_H 1
#define HAVE_STRING_H 1
/* #define HAVE_UNISTD_H 1          HKT */
/* #define HAVE_STDBOOL_H 1         HKT */
/* #undef HAVE_SYS_SOUNDCARD_H */
/* #undef HAVE_MACHINE_SOUNDCARD_H */
/* #undef HAVE_SYS_MIXER_H */
/* #undef MUS_HAVE_USR_LIB_OSS */
/* #undef MUS_HAVE_USR_LOCAL_LIB_OSS */
/* #undef MUS_HAVE_OPT_OSS */
/* #undef MUS_HAVE_VAR_LIB_OSS */
/* #undef HAVE_LIBC_H */
/* #undef HAVE_ALSA_ASOUNDLIB_H */
/* #undef HAVE_BYTESWAP_H */
/* #define HAVE_STDINT_H 1          HKT */
/* #define HAVE_SYS_TIME_H 1        HKT */

/* #undef HAVE_PTHREAD_H */
/* #undef HAVE_PTHREADS */

#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define SIZEOF_OFF_T 4
#define SIZEOF_SSIZE_T 4
#define SIZEOF_UNSIGNED_LONG 4
#define SIZEOF_UNSIGNED_LONG_LONG 8
#define SIZEOF_VOID_P 8
#define SIZEOF_INT64_T 8

#define Float float
#define mus_float_t float
#define mus_long_t long long int
#define int64_t long long int
/* int64_t is used in xen.[ch] */

/* #undef _FILE_OFFSET_BITS */
/* #undef _LARGE_FILES */

/* #undef MUS_LINUX */
/* #undef MUS_SGI */
/* #undef MUS_ALPHA */
/* #undef MUS_SUN */
/* #undef MUS_OPENBSD */
/* #undef MUS_NETBSD */
#define MUS_WINDOZE 1
#define HAVE_WINDOZE 1
/* #undef HAVE_OSS */
/* #undef HAVE_ALSA */
/* #undef HAVE_NEW_ALSA */
/* #undef HAVE_JACK_IN_LINUX */
/* #undef MUS_JACK */
/* #undef HAVE_SAM_9407 */
/* #undef MUS_MAC_OSX */
/* #undef MUS_HPUX */
/* #undef MUS_AUDIOFILE_VERSION */
/* #undef HAVE_KAUDIODEVICEPROPERTYTRANSPORTTYPE */
/* #undef HAVE_KLINEARPCMFORMATFLAGISNONINTERLEAVED */
/* #undef HAVE_AUDIODEVICEDESTROYIOPROCID */
/* #undef MUS_JACK_VERSION */

/* #undef HAVE_GUILE */
#define HAVE_SCHEME 1
/* #undef HAVE_RUBY */
/* #undef HAVE_FORTH */
#define HAVE_S7 1
#define HAVE_EXTENSION_LANGUAGE 1

/* #undef SND_CONFIG_GET_ID_ARGS */
/* #undef HAVE_GSL */
/* #undef MUS_GSL_VERSION */
/* #undef HAVE_GSL_EIGEN_NONSYMMV_WORKSPACE */
/* #undef WITH_DOUBLES */
#define SNDLIB_USE_FLOATS 1
#define MUS_SAMPLE_BITS 24

#define WITH_RUN 1
#define WITH_MULTIDIMENSIONAL_VECTORS 1
#define WITH_R5RS_RATIONALIZE 0
#define WITH_CASE_SENSITIVE_SYMBOL_NAMES 1

/* #undef HAVE_READLINE */
/* #undef HAVE_APPLICABLE_SMOB */

/* #undef HAVE_SCM_REMEMBER_UPTO_HERE */
/* #undef HAVE_SCM_MAKE_REAL */
/* #undef HAVE_SCM_OBJECT_TO_STRING */
/* #undef HAVE_SCM_NUM2LONG_LONG */
/* #undef HAVE_SCM_C_MAKE_VECTOR */
/* #undef HAVE_SCM_C_DEFINE */
/* #undef HAVE_SCM_C_DEFINE_GSUBR */
/* #undef HAVE_SCM_C_EVAL_STRING */
/* #undef HAVE_SCM_NUM2INT */
/* #undef HAVE_SCM_LIST_N */
/* #undef HAVE_SCM_STR2SYMBOL */
/* #undef HAVE_SCM_DEFINED_P */
/* #undef HAVE_SCM_T_CATCH_BODY */
/* #undef HAVE_SCM_MEM2STRING */
#define HAVE_SCM_MAKE_RATIO 1
/* #undef HAVE_SCM_MAKE_COMPLEX */
/* #undef HAVE_SCM_CONTINUATION_P */
/* #undef HAVE_SCM_TO_SIGNED_INTEGER */
#define HAVE_SCM_C_MAKE_RECTANGULAR 1
/* #undef HAVE_SCM_CAR */
/* #undef HAVE_SCM_FROM_LOCALE_KEYWORD */
/* #undef HAVE_SCM_IS_VECTOR */
/* #undef HAVE_SCM_IS_SIMPLE_VECTOR */
/* #undef HAVE_SCM_C_PRIMITIVE_LOAD */
/* #undef HAVE_SCM_NAN_P */
/* #undef HAVE_GUILE_CALL_CC */
/* #undef HAVE_GUILE_DYNAMIC_WIND */
/* #undef HAVE_SCM_EVALREC */

/* #undef MUS_RUBY_VERSION */
/* #undef RUBY_RELEASE_DATE */
/* #undef RUBY_SEARCH_PATH */
/* #undef HAVE_RB_GC_DISABLE */
/* #undef HAVE_RB_ARY_DUP */
/* #undef HAVE_REASONABLE_RB_GC_MARK */
/* #undef RB_FIND_FILE_TAKES_VALUE */
/* #undef HAVE_RB_ERRINFO */

/* #undef WITH_MODULES */
/* #undef HAVE_KAUDIODEVICEPROPERTYTRANSPORTTYPE */
/* #undef HAVE_KLINEARPCMFORMATFLAGISNONINTERLEAVED */
/* #undef HAVE_AUDIODEVICEDESTROYIOPROCID */

/* #undef WITH_HOBBIT */

/* #undef HAVE_SCM_EVALREC */

#define USE_SND 0

#ifdef _MSC_VER
  typedef long off_t;
  #define ssize_t int 
  #define snprintf _snprintf 
  #define strtoll strtol
  #if _MSC_VER > 1200
    #define _CRT_SECURE_NO_DEPRECATE 1
    #define _CRT_NONSTDC_NO_DEPRECATE 1
    #define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
  #endif
#endif

#endif

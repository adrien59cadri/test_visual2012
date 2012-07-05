# Configuration script for sndlib (taken from Snd)

# TODO: make sndlib-config executable, install it in bindir

AC_INIT(sndlib, 20, bil@ccrma.stanford.edu, ftp://ccrma-ftp.stanford.edu/pub/Lisp/sndlib.tar.gz)
SNDLIB_VERSION=20
AC_CONFIG_SRCDIR(io.c)
AC_CANONICAL_HOST
AC_CONFIG_FILES(makefile)
AC_CONFIG_FILES(sndlib-config, [chmod +x sndlib-config])
AC_CONFIG_FILES(sndins/Makefile)
AC_CONFIG_HEADERS(mus-config.h sndlib.h)
AC_PROG_CC
AC_HEADER_STDC

audio_system="unknown"


#--------------------------------------------------------------------------------
# configuration options
#   --with-alsa           use ALSA if possible
#   --with-jack           use Jack
#   --with-static-alsa    use ALSA statically loaded (for RPM generation)
#   --with-float-samples  represent samples internally as floats 
#   --with-sample-width=N use N bits of samples (default = 24)
#   --with-portaudio      use portaudio
#   --with-ruby           use Ruby as the extension language
#   --with-ruby-prefix    set prefix for ruby.h
#   --with-forth          use Forth as extension language
#   --with-s7             use S7 as the extension language (default = yes)
#   --with-static-gsl     try to statically load GSL
#   --disable-shared      don't try to make a .so file
#--------------------------------------------------------------------------------

AC_ARG_WITH(alsa,	[  --with-alsa		  use ALSA])
AC_ARG_WITH(oss,         [  --with-oss		  use OSS])
AC_ARG_WITH(jack,       [  --with-jack		  use JACK])
AC_ARG_WITH(static-alsa,[  --with-static-alsa	  use ALSA statically loaded])
AC_ARG_WITH(float-samples, [  --with-float-samples	  use floats (or doubles) as the internal sample respresentation, default=yes])
AC_ARG_WITH(gsl,         [  --with-gsl		  use GSL, default=yes])
AC_ARG_WITH(s7,          [  --with-s7  	          use S7, default=yes])
AC_ARG_WITH(portaudio,   [  --with-portaudio 	  use portaudio, default=no])

AC_PATH_PROG(PKG_CONFIG, pkg-config, no)


# -------- internal sample data type --------

# sample-width only applies to the int case (ignored if float)
SNDLIB_BITS="0"
AC_ARG_WITH(sample-width,
	[  --with-sample-width=N   use N bits of samples],
	[AC_MSG_RESULT(Using $with_sample_width bit samples)
	 AC_DEFINE_UNQUOTED(MUS_SAMPLE_BITS, $with_sample_width)
         SNDLIB_BITS=$with_sample_width
	],
	[AC_DEFINE(MUS_SAMPLE_BITS, 24)]
	)

if test "$with_float_samples" != no ; then
  AC_DEFINE(SNDLIB_USE_FLOATS, 1)
else
  AC_DEFINE(SNDLIB_USE_FLOATS, 0)
fi

# for sndlib-config:
AC_SUBST(SNDLIB_BITS)

# if mingw add -mwindows to the load flags
# case "$host" in
#  *-*-mingw*)
#    LDFLAGS="$LDFLAGS -mwindows"
#    ;;
# esac


AC_ARG_ENABLE(shared,     [  --disable_shared 	  don't build or install the shared library])


#--------------------------------------------------------------------------------
# standard libraries, header files, functions, OSS special cases
#--------------------------------------------------------------------------------

AC_PROG_INSTALL

LIBS=""
AC_CHECK_LIB(m,main)
AC_CHECK_LIB(c,main)
AC_CHECK_LIB(dl,main)

AC_CHECK_HEADERS(fcntl.h limits.h unistd.h string.h sys/soundcard.h machine/soundcard.h sys/mixer.h byteswap.h stdbool.h)
AC_CHECK_HEADERS(libc.h stdint.h sys/param.h sys/time.h)
AC_CHECK_HEADER(/usr/local/lib/oss/include/sys/soundcard.h,[AC_DEFINE(MUS_HAVE_USR_LOCAL_LIB_OSS)])
AC_CHECK_HEADER(/usr/lib/oss/include/sys/soundcard.h,[AC_DEFINE(MUS_HAVE_USR_LIB_OSS)])
AC_CHECK_HEADER(/opt/oss/include/sys/soundcard.h,[AC_DEFINE(MUS_HAVE_OPT_OSS)])
AC_CHECK_HEADER(/var/lib/oss/include/sys/soundcard.h,[AC_DEFINE(MUS_HAVE_VAR_LIB_OSS)])
AC_CHECK_HEADER(gnu/libc-version.h,[AC_DEFINE(HAVE_GNU_LIBC_VERSION_H)])
AC_CHECK_HEADER(alsa/asoundlib.h,[AC_DEFINE(HAVE_ALSA_ASOUNDLIB_H)])
AC_HEADER_TIME

AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_CHECK_TYPE(ssize_t, int)
AC_TYPE_PID_T
AC_TYPE_INT64_T
AC_OLD_BIGENDIAN
AC_SYS_LARGEFILE
AC_TYPE_OFF_T
AM_LANGINFO_DECIMAL_POINT
AC_LIB_PROG_LD

AC_CHECK_SIZEOF(off_t)
AC_CHECK_SIZEOF(ssize_t)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(unsigned long long)
AC_CHECK_SIZEOF(unsigned long)

AC_CHECK_SIZEOF(void *)
# this won't work in version 2.66
# AC_CHECK_SIZEOF(intptr_t)
# AC_DEFINE_UNQUOTED(SIZEOF_VOID_P, $ac_cv_sizeof_intptr_t)

AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(int64_t)
AC_TYPE_SIGNAL

AC_CHECK_DECLS(hypot,,,[#include <math.h>]) 
AC_CHECK_DECLS(isnan,,,[#include <math.h>])
AC_CHECK_DECLS(isinf,,,[#include <math.h>])

AC_CHECK_FUNCS(getcwd getwd strerror readlink setlocale access opendir sleep signal statvfs statfs getline difftime gettimeofday)
AC_CHECK_FUNCS(vsnprintf vasprintf snprintf strftime memmove lstat fileno strcasecmp pathconf)

AC_MSG_CHECKING(for __func__)
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM(,
    [ char *s; 
      s = (char *)__func__
    ])],
  [AC_DEFINE(HAVE___FUNC__)
   AC_MSG_RESULT(yes)
  ],
  [AC_MSG_RESULT(no)])


# look for special functions in libm
AC_CHECK_LIB(m, jn, 
  [AC_CHECK_LIB(m, yn, 
    [AC_CHECK_LIB(m, lgamma,
      [AC_CHECK_LIB(m, erf, 
        [AC_DEFINE(HAVE_SPECIAL_FUNCTIONS)])])])])


# look for nested function support
AC_MSG_CHECKING(whether nested functions work)
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM(,
    [ double bar;
      double foo(double a, double b)
       {
        auto double square (double z);
        double square (double z) {return(z * z);}
        return(square(a) + square(b));
       }
     bar = foo(1.0, 2.0)
    ])],
  [AC_DEFINE(HAVE_NESTED_FUNCTIONS)
   AC_MSG_RESULT(yes)
  ],
  [AC_MSG_RESULT(no)])


case "$host" in
    *-*-linux*) 
	AC_DEFINE(HAVE_LINUX)
	;;
    *-*-sunos4*) 
        AC_DEFINE(HAVE_SUN)
        ;;
    *-*-solaris*) 
	AC_DEFINE(HAVE_SUN)
        ;;
    *-*-netbsd*) 
        AC_DEFINE(HAVE_NETBSD)
        ;;
    *-*-cygwin*)
        AC_DEFINE(HAVE_WINDOZE)
	;;
    *-*-mingw*)
	AC_DEFINE(HAVE_WINDOZE)
	;;
    *-apple-*)
        AC_DEFINE(HAVE_OSX)
	;;
esac



#--------------------------------------------------------------------------------
# complex trig
#--------------------------------------------------------------------------------

# having <complex.h> + a cacos declaration is not enough: C++ dies with a complaint about a "deprecated header"
#
#  I'm also using cexp and in xen creal, cimag, and _Complex_I
#  I'm currently using the data type "complex double" though the header seems to prefer "double complex" and
#  the gcc documentation mentions _Complex double.
#
# in C++, we want to set these to WITH_COMPLEX 1 and HAVE_COMPLEX_TRIG 0 no matter what
#
LIBS="$LIBS -lm"

if test "$CC" = "g++" ; then
  AC_DEFINE(WITH_COMPLEX)
  ac_snd_have_complex_trig=no
else

  AC_MSG_CHECKING(for complex numbers)
  AC_LINK_IFELSE(
  	[AC_LANG_PROGRAM(
        	[#include <complex.h>],
		[ double complex val; 
                  double rl, im;
                  val = 1.0 + 0.5 * _Complex_I;
                  rl = creal(val);
                  im = cimag(val);
                ])],
        [
 	 AC_DEFINE(WITH_COMPLEX)
	 AC_MSG_RESULT(yes)
    	],
	[AC_MSG_RESULT(no)])


  ac_snd_have_complex_trig=no
  AC_MSG_CHECKING(for complex trig)
  AC_LINK_IFELSE(
  	[AC_LANG_PROGRAM(
        	[#include <complex.h>],
		[ _Complex double val; 
                  double rl, im;
                  val = 1.0 + 0.5 * _Complex_I;
                  rl = creal(val);
                  im = cimag(val);
                  val = ccosh(cacosh(1.5) / 100.0)
                ])],
        [
 	 AC_DEFINE(HAVE_COMPLEX_TRIG)
	 ac_snd_have_complex_trig=yes
	 AC_MSG_RESULT(yes)
    	],
	[AC_MSG_RESULT(no)])
fi



#--------------------------------------------------------------------------------
# GSL
#--------------------------------------------------------------------------------

GSL_LIBS=""
GSL_CFLAGS=""
if test "$with_gsl" != no; then
AC_PATH_PROG(GSL_CONFIG, gsl-config, no)
AC_MSG_CHECKING(for GSL)
if test "$GSL_CONFIG" = "no" ; then
  AC_MSG_RESULT(no)
else
  GSL_CFLAGS=`$GSL_CONFIG --cflags`
  GSL_PREFIX=`$GSL_CONFIG --prefix`

  if test "$with_static_gsl" = yes ; then
    AC_CHECK_FILE($GSL_PREFIX/lib/libgsl.a,
	[GSL_LIBS="$GSL_PREFIX/lib/libgsl.a"],
	[GSL_LIBS=`$GSL_CONFIG --libs`
	 with_static_gsl=no
	 AC_MSG_WARN([can't find libgsl.a])])
  else
    GSL_LIBS=`$GSL_CONFIG --libs`
  fi

  gsl_version="`$GSL_CONFIG --version`"
  AC_MSG_RESULT($gsl_version)
  OPTIONAL_LIBRARIES="$OPTIONAL_LIBRARIES gsl"

  if test "`$GSL_CONFIG --version`" = "0.6" || test "`$GSL_CONFIG --version`" = "0.7" || test "`$GSL_CONFIG --version`" = "0.5"; then
    AC_MSG_WARN([Snd needs GSL 0.8 or later])
  else
    if test "`$GSL_CONFIG --version`" = "0.8" && test "$with_static_gsl" = yes ; then
      GSL_LIBS="$GSL_LIBS $GSL_PREFIX/lib/libgslcblas.a"
    fi
    AC_DEFINE(HAVE_GSL)
    AC_DEFINE_UNQUOTED(MUS_GSL_VERSION,"${gsl_version}")

    SAVELIBS=$LIBS
    LIBS="$LIBS $GSL_LIBS"
    AC_MSG_CHECKING(for gsl_eigen_nonsymmv_workspace)
    AC_LINK_IFELSE(
      [AC_LANG_PROGRAM(
         [#include <gsl/gsl_math.h>
          #include <gsl/gsl_eigen.h>],
	 [     gsl_eigen_nonsymmv_workspace *w = gsl_eigen_nonsymmv_alloc(4)
         ])],
      [
       AC_DEFINE(HAVE_GSL_EIGEN_NONSYMMV_WORKSPACE)
       AC_MSG_RESULT(yes)
      ],
      [AC_MSG_RESULT(no)])
    LIBS=$SAVELIBS
  fi

fi
fi
AC_SUBST(GSL_LIBS)
AC_SUBST(GSL_CFLAGS)


SNDLIB_LANGUAGE="None"

#--------------------------------------------------------------------------------
# Ruby
#--------------------------------------------------------------------------------

dnl AC_CHECK_RUBY([MINIMUM-VERSION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for Ruby, defines
dnl   RUBY_VERSION
dnl   RUBY_RELEASE_DATE
dnl   RUBY_SEARCH_PATH
dnl   RUBY_CFLAGS
dnl   RUBY_LIBS

AC_DEFUN([AC_CHECK_RUBY],
[
  [AC_PATH_PROGS([RUBY], [$RUBY ruby], [no])]
  RUBY_VERSION=""
  RUBY_RELEASE_DATE=""
  RUBY_SEARCH_PATH=""
  RUBY_CFLAGS=""
  RUBY_LIBS=""
  AC_MSG_CHECKING([for Ruby])
  if test "$RUBY" != no ; then
    minimum_version=ifelse([$1], [], [1.8.0], [$1])
    RUBY_VERSION=`$RUBY -e 'puts RUBY_VERSION'`
    if `$RUBY -e "exit(RUBY_VERSION >= '$minimum_version' ? 0 : 1)"` ; then
      AC_MSG_RESULT([$RUBY_VERSION])
      RUBY_RELEASE_DATE=`$RUBY -e 'puts RUBY_RELEASE_DATE'`
      RUBY_SEARCH_PATH=`$RUBY -e 'puts $:.join(":")'`
      ruby_ldflags=""
      ruby_libs=""
      if `$RUBY -e "exit(RUBY_VERSION < '1.9.0' ? 0 : 1)"` ; then
        ruby_hdrdir=`$RUBY -rrbconfig -e [["puts Config::CONFIG['archdir']"]]`
        AC_MSG_CHECKING([for ruby.h])
        if test -e "$ruby_hdrdir/ruby.h" ; then
          RUBY_CFLAGS="-I$ruby_hdrdir"
          AC_MSG_RESULT([$ruby_hdrdir/ruby.h])
        else
          RUBY_CFLAGS=`$RUBY -e 'puts "-I" + $:.join(" -I")'`
          AC_MSG_RESULT([use \$LOAD_PATH])
        fi
        ruby_ldflags=`$RUBY -rrbconfig -e [["puts Config::CONFIG['LIBRUBYARG']"]]`
        ruby_libs=`$RUBY -rrbconfig -e [["puts Config::CONFIG['LIBS']"]]`
      else
        ruby_hdrdir=`$RUBY -rrbconfig -e [["puts RbConfig::CONFIG['rubyhdrdir']"]]`
        ruby_arch=`$RUBY -rrbconfig -e [["puts RbConfig::CONFIG['arch']"]]`
        AC_MSG_CHECKING([for ruby.h])
        if test -e "$ruby_hdrdir/ruby.h" ; then
          RUBY_CFLAGS="-I$ruby_hdrdir -I$ruby_hdrdir/$ruby_arch"
          AC_MSG_RESULT([$ruby_hdrdir/ruby.h])
        else
          ruby_hdrdir=`$RUBY -rrbconfig -e [["puts RbConfig::CONFIG['archdir']"]]`
          if test -e "$ruby_hdrdir/ruby.h" ; then
            RUBY_CFLAGS="-I$ruby_hdrdir"
            AC_MSG_RESULT([$ruby_hdrdir/ruby.h])
	  else
            RUBY_CFLAGS=`$RUBY -e 'puts "-I" + $:.join(" -I")'`
            AC_MSG_RESULT([use \$LOAD_PATH])
	  fi
        fi
        ruby_ldflags=`$RUBY -rrbconfig -e [["puts RbConfig::CONFIG['LIBRUBYARG']"]]`
        ruby_libs=`$RUBY -rrbconfig -e [["puts RbConfig::CONFIG['LIBS']"]]`
      fi
      RUBY_LIBS="$ruby_ldflags $ruby_libs"
      [$2]
    else
      AC_MSG_RESULT([Ruby version $RUBY_VERSION < $minimum_version])
      [$3]
    fi
  else
    AC_MSG_RESULT([no])
    [$3]
  fi
  AC_SUBST([RUBY_VERSION])
  AC_SUBST([RUBY_RELEASE_DATE])
  AC_SUBST([RUBY_SEARCH_PATH])
  AC_SUBST([RUBY_CFLAGS])
  AC_SUBST([RUBY_LIBS])
])# AC_CHECK_RUBY

# readline (for Ruby)
AC_ARG_ENABLE(readline,
  [  --enable-readline      include readline (the default)], ,)

AC_ARG_WITH(ruby-prefix,[  --with-ruby-prefix=PFX  where Ruby is installed],
                ruby_prefix="$withval"
                RUBY="$ruby_prefix/bin/ruby",
                ruby_prefix="/usr/local")

if test "$with_ruby" = yes && test "$ac_snd_have_extension_language" = yes ; then
  with_ruby=no
  AC_MSG_WARN([You asked for both Ruby and $SNDLIB_LANGUAGE -- $SNDLIB_LANGUAGE will be used])
fi

AC_ARG_WITH(ruby,
        [  --with-ruby            use Ruby as the extension language],
        if test "$with_ruby" = yes ; then
           AC_CHECK_RUBY([1.8.0],
                         [AC_DEFINE(HAVE_RUBY)
                          AC_DEFINE(HAVE_EXTENSION_LANGUAGE)
                          AC_DEFINE_UNQUOTED(MUS_RUBY_VERSION,  "$RUBY_VERSION")
                          AC_DEFINE_UNQUOTED(RUBY_RELEASE_DATE, "$RUBY_RELEASE_DATE")
                          AC_DEFINE_UNQUOTED(RUBY_SEARCH_PATH,  "$RUBY_SEARCH_PATH")
                          XEN_CFLAGS="$XEN_CFLAGS $RUBY_CFLAGS"
                          XEN_LIBS="$XEN_LIBS $RUBY_LIBS" 
                          if test "$enable_readline" != no ; then
                            AC_CHECK_LIB(readline, readline,
                                         [AC_DEFINE(HAVE_READLINE)
                                          XEN_LIBS="$XEN_LIBS -lreadline -lncurses"], ,"-lncurses")
                          fi
                          AC_SUBST(XEN_LIBS)
                          AC_SUBST(XEN_CFLAGS)
                          OLD_LIBS="$LIBS"
                          LIBS="$XEN_LIBS $LIBS -lm"
                          OLD_CFLAGS="$CFLAGS"
                          CFLAGS="$XEN_CFLAGS $CFLAGS"
                          AC_MSG_CHECKING([for rb_proc_new])
                          AC_RUN_IFELSE(
                            [AC_LANG_PROGRAM([@%:@include <ruby.h>
                                             VALUE proc_call(VALUE args, VALUE id) {return(rb_apply(rb_mKernel, (ID)id, args));}
                                             ],
                                             [VALUE proc; 
                                              ruby_init(); 
                                              proc = rb_proc_new(proc_call, rb_intern("hi"))])
                                             ], 
                            [AC_DEFINE(HAVE_RB_PROC_NEW)
                             AC_MSG_RESULT(yes)
                            ],
                            AC_MSG_RESULT(no)
                            )
                          AC_MSG_CHECKING([for ruby_vsnprintf]) 
                          AC_RUN_IFELSE( 
                            [AC_LANG_PROGRAM([@%:@include <ruby.h>], 
                                             [ruby_init(); ruby_vsnprintf(0, 0, 0, 0); return(0)])], 
                            [AC_DEFINE(HAVE_VSNPRINTF, 0) 
                             AC_MSG_RESULT(yes) 
                            ], 
                            AC_MSG_RESULT(no) 
                            ) 
                          LIBS="$OLD_LIBS"
                          CFLAGS="$OLD_CFLAGS"
                          SNDLIB_LANGUAGE="Ruby"
                          ac_snd_have_extension_language=yes])
        fi)



#--------------------------------------------------------------------------------
# Forth
#--------------------------------------------------------------------------------

if test "$with_forth" = yes && test "$ac_snd_have_extension_language" = yes ; then
  with_forth=no
  AC_MSG_WARN([You asked for both Forth and $SNDLIB_LANGUAGE -- $SNDLIB_LANGUAGE will be used])
fi

AC_ARG_WITH(forth,
	[  --with-forth	  	  try to use Forth as the extension language],
  	if test "$with_forth" = yes ; then
	   FTH_CHECK_LIB([AC_DEFINE(HAVE_FORTH)
 	                  AC_DEFINE(HAVE_EXTENSION_LANGUAGE)
			  if test "$FTH_HAVE_COMPLEX" = yes ; then
     	      		     AC_DEFINE(HAVE_COMPLEX_TRIG)
     	      		     AC_DEFINE(HAVE_MAKE_COMPLEX)
     	      		     AC_DEFINE(HAVE_MAKE_RECTANGULAR)
			  fi
			  if test "$FTH_HAVE_RATIO" = yes ; then
     	      		     AC_DEFINE(HAVE_MAKE_RATIO)
			  fi
               		  AC_SUBST(XEN_CFLAGS, $FTH_CFLAGS)
               		  AC_SUBST(XEN_LIBS,   $FTH_LIBS)
 	      		  SNDLIB_LANGUAGE="Forth"
 	      		  ac_snd_have_extension_language=yes])
	fi)


#--------------------------------------------------------------------------------
# S7
#--------------------------------------------------------------------------------

if test "$with_s7" = yes && test "$ac_snd_have_extension_language" = yes ; then
  with_s7=no
  AC_MSG_WARN([You asked for both s7 and $SNDLIB_LANGUAGE -- $SNDLIB_LANGUAGE will be used])
fi

S7_LIB=""

if test "$with_s7" != no && test "$with_extension_language" != no && test "$ac_snd_have_extension_language" != yes ; then
    AC_DEFINE(HAVE_SCHEME)	
    ac_snd_have_extension_language=yes
    AC_DEFINE(HAVE_EXTENSION_LANGUAGE)
    AC_DEFINE(HAVE_MAKE_RATIO)
    AC_DEFINE(WITH_MULTIDIMENSIONAL_VECTORS)
    AC_CHECK_HEADER(complex.h, 
        [
         AC_DEFINE(HAVE_MAKE_RECTANGULAR)
	])

    SNDLIB_LANGUAGE="s7"
    S7_LIB="s7.o"
fi

AC_SUBST(S7_LIB)




#--------------------------------------------------------------------------------
# Audio library
#--------------------------------------------------------------------------------

AUDIO_LIB=""
LDSO_FLAGS=""
SO_FLAGS=""
SO_LD="ld"
SO_INSTALL="install"
A_INSTALL="install"
# A_LD="ld"
# A_LD_FLAGS=""
A_LD="ar"
A_LD_FLAGS="-rc"
SO_NAME="libsndlib.so"
JACK_LIBS=""
JAC_FLAGS=""
RANLIB=":"
LD_FLAGS="-r"

# we need the sndlib.h equivalents to try to find the native sound support (see config.guess)
# this only matters for those cases where we've implemented the audio code in audio.c
# test for ALSA courtesy of Paul Barton-Davis
# test for BSD courtesy of Steven Schultz
# test for Jack courtesy of Kjetil S. Matheussen

if test "$with_portaudio" = yes ; then
  AC_CHECK_LIB(portaudio, main, [
    AC_CHECK_HEADER(portaudio.h, [
          AUDIO_SYSTEM=portaudio
	  AC_DEFINE(MUS_PORTAUDIO)
	if test x$PKG_CONFIG != xno ; then
          AUDIO_LIB="`$PKG_CONFIG portaudio-2.0 --libs`"
        else 
	  AUDIO_LIB="-lportaudio"
        fi
	],
      [AC_MSG_WARN(can't find the portaudio header file)])],
    [AC_MSG_WARN(can't find the portaudio library)])
fi

if test "$audio_system" != unknown ; then
  AC_MSG_RESULT(Using the $audio_system audio system)
else

if test "$with_alsa" = yes && test "$with_oss" = yes ; then
  with_alsa=no
  AC_MSG_WARN([You asked for both ALSA and OSS -- OSS will be used])
fi

if test "$with_jack" = yes ; then 
            AC_CHECK_LIB(jack,main,[ 
	      AC_CHECK_HEADER(jack/jack.h,
                  ,
   	          [with_jack=no
                   AC_MSG_WARN(can't find the JACK header files)])],
              [with_jack=no
	       AC_MSG_WARN(can't find the JACK library)])
            AC_CHECK_LIB(samplerate,main,[ 
	      AC_CHECK_HEADER(samplerate.h,  ,
   	          [with_jack=no
                   AC_MSG_WARN(can't find the samplerate header files. JACK will not be used.)])],
              [with_jack=no
	       AC_MSG_WARN(can't find the samplerate library. JACK will not be used.)])
            if test $with_jack = yes ; then
               AUDIO_SYSTEM=JACK
	       AC_DEFINE(MUS_JACK)
	       if test x$PKG_CONFIG != xno ; then
                   jack_version="`$PKG_CONFIG jack --modversion`"
                   AC_DEFINE_UNQUOTED(MUS_JACK_VERSION,"Jack: ${jack_version}")
          	   JACK_LIBS="`$PKG_CONFIG jack --libs`"
          	   JACK_FLAGS="`$PKG_CONFIG jack --cflags`"
               fi
	    JACK_LIBS="$JACK_LIBS -lpthread"
            fi
fi

case "$host" in
    *-*-linux*) 
	AC_DEFINE(MUS_LINUX)
        LDSO_FLAGS="-shared"

	if test "$GCC" = yes ; then
	  SO_FLAGS="-fPIC $SO_FLAGS"
	  SO_LD="$CC"
        fi
        LIBS="-L/usr/lib -lm"
#     This ^ used to be "" but in redhat 7 the -lm needs to be explicit for make sndsine etc
#     it was "" to cancel -lXpm without the needed -L<dir> for the same case

        AUDIO_SYSTEM=ALSA
	if test "$with_oss" = yes ; then 
          AUDIO_SYSTEM=OSS
	else

          if test "$ac_cv_header_alsa_asoundlib_h" = yes ; then
            AC_CHECK_LIB(asound, main,
               AUDIO_SYSTEM=ALSA,
               [AC_MSG_WARN(can't find the ALSA library)])
          else
            AC_MSG_WARN(can't find the ALSA header files)
	    AUDIO_SYSTEM=OSS
          fi
	fi

	if test "$with_jack" = yes ; then 
	       if test "$with_alsa" != yes ; then
                 AUDIO_SYSTEM=JACK
               fi
	fi

	case $AUDIO_SYSTEM in
	   ALSA)
                # if alsa version < 0.9 complain and use OSS
		alsa_ok=yes
		AC_MSG_CHECKING([for ALSA 0.9 or later])
		AC_COMPILE_IFELSE(
                  [AC_LANG_PROGRAM([#include <alsa/asoundlib.h>],
			           [
			  	    #if ((SND_LIB_MAJOR == 0) && (SND_LIB_MINOR < 9))
				      #error too old
				    #endif
				   ])],
		  alsa_ok=yes,
	          alsa_ok=no)
		AC_MSG_RESULT($alsa_ok)
		if test "$alsa_ok" = yes ; then
	    	    AC_DEFINE(HAVE_ALSA)
		    if test "$with_static_alsa" = yes ; then
                      AUDIO_LIB="/usr/lib/libasound.a"
		    else
	    	      AUDIO_LIB="-lasound"
		    fi
	    	    AC_SUBST(AUDIO_LIB)

                    # snd_config_get_id argnum changed in Nov-01
                    snd_config_get_id_args=1
                    AC_MSG_CHECKING([alsa snd_config_get_id args])
                    AC_COMPILE_IFELSE(
		      [AC_LANG_PROGRAM([#include <alsa/asoundlib.h>],
	             		       [snd_config_t *c; 
					const char *id; 
		      			snd_config_get_id(c, &id)])],
       		      snd_config_get_id_args=2)
                    AC_MSG_RESULT($snd_config_get_id_args)
                    AC_DEFINE_UNQUOTED(SND_CONFIG_GET_ID_ARGS,$snd_config_get_id_args)

                    AC_MSG_CHECKING([for newer alsa calling sequences])
                    AC_COMPILE_IFELSE(
		      [AC_LANG_PROGRAM([#include <alsa/asoundlib.h>],
	             		       [snd_pcm_hw_params_t *hw_params;
			  	        unsigned int tmp;
					int dir;
				        snd_pcm_hw_params_get_rate_max(hw_params, &tmp, &dir)
				       ])],
		       [AC_DEFINE(HAVE_NEW_ALSA)
                        AC_MSG_RESULT(yes)],
		       [AC_MSG_RESULT(no)])

		    if test "$with_jack" = yes ; then 
		      if test "$with_static_alsa" = yes ; then
			 AUDIO_LIB="/usr/lib/libasound.a -lsamplerate"
		      else
			 AUDIO_LIB="-lasound -lsamplerate"
	              fi
	              A_INSTALL=":"
		      AC_DEFINE(HAVE_JACK_IN_LINUX)
		    fi
 		else
         	    AUDIO_SYSTEM=OSS
 		    AC_DEFINE(HAVE_OSS)
 		fi
 		;;
	    JACK)
		AC_DEFINE(HAVE_JACK_IN_LINUX)
		AC_DEFINE(HAVE_OSS)
	        A_LD=":"
	        A_INSTALL=":"
		AUDIO_LIB="-lsamplerate"
		;;
	    OSS)
		AC_DEFINE(HAVE_OSS)
		;;
	esac
	;;
    *-*-sunos4*) 
        AC_DEFINE(MUS_SUN)
	LIBS="-lm"
	audio_system=Sun
        ;;
    *-*-solaris*) 
	AC_DEFINE(MUS_SUN)
#	LIBS="-lm -ldl"
	LIBS="-lm"
	audio_system=Sun
	LDSO_FLAGS="-G"
        ;;
    *-*-hpux*) 
        AC_DEFINE(MUS_HPUX)
	audio_system=Hpux
	if test "$GCC" = yes ; then
	  SO_FLAGS="-fPIC $SO_FLAGS"
        fi
	LDSO_FLAGS="+z -Ae +DA1.1"
        ;;
    *-*-bsdi*) 
	AC_DEFINE(HAVE_OSS)
	LIBS="-lm"
	if test "$GCC" = yes ; then
	  SO_FLAGS="-fPIC $SO_FLAGS"
        fi
	audio_system=OSS
        ;;
    *-*-freebsd*)
	AC_DEFINE(HAVE_OSS)
	LIBS="-lm"
	audio_system=OSS
	if test "$GCC" = yes ; then
	  SO_FLAGS="-fPIC $SO_FLAGS"
          LDSO_FLAGS="-shared"
	  SO_LD="$CC"
        fi
	;;
    *-*-openbsd*) 
        AC_DEFINE(MUS_OPENBSD)
	audio_system=Sun
	if test "$GCC" = yes ; then
	  SO_FLAGS="-fPIC $SO_FLAGS"
        fi
        ;;
    *-*-netbsd*) 
        AC_DEFINE(MUS_NETBSD)
	audio_system=NetBSD
	if test "$GCC" = yes ; then
	  SO_FLAGS="-fPIC $SO_FLAGS"
          LDSO_FLAGS="-shared"
	  SO_LD="$CC"
        fi
	;;
    *-*-cygwin*)
	if test "$with_jack" != yes ; then 
	    AC_DEFINE(MUS_WINDOZE)
	    AUDIO_SYSTEM=Windoze
	fi
        A_LD_FLAGS="-rc"
	SO_INSTALL=":"
	SO_LD=":"
	;;
    *-*-mingw*)
	AC_DEFINE(MUS_WINDOZE)
	audio_system=Windoze
#	LIBS="$LIBS -lwinmm -lwsock32"
# this change thanks to Michael Gogins and Rick Taube
        LIBS="$LIBS -lkernel32 -lgdi32 -lwsock32 -lws2_32 -lole32 -luuid -lwinmm -lkernel32 -lgdi32 -lwsock32 -lws2_32 -lole32 -luuid -lwinmm"
        LDFLAGS="$LDFLAGS -Wl,--enable-stdcall-fixup -Wl,-export-dynamic"
        CFLAGS="$CFLAGS -O2 -g -DHAVE_CONFIG_H -D_WIN32_WINNT=0x0501"
	SO_INSTALL=":"
	SO_LD=":"
	A_INSTALL="ar rc"
        A_LD_FLAGS="-rc"
	RANDLIB="ranlib"
	;;
    *-apple-*)
	if test "$with_jack" != yes ; then 

           LDSO_FLAGS=""
	   SO_INSTALL=":"
	   SO_LD="$CC"	
	   LDSO_FLAGS="-dynamic -bundle -undefined suppress -flat_namespace"
	   SO_NAME="sndlib.bundle"
	   AC_DEFINE(MUS_MAC_OSX)
	   audio_system=MacOSX
	   AUDIO_LIB="-framework CoreAudio -framework CoreFoundation -framework CoreMIDI"
	   AC_SUBST(AUDIO_LIB)

	    # OSX 10.5, deprecating earlier AudioDeviceRemoveIOProc
	    AC_MSG_CHECKING([for AudioDeviceDestroyIOProcID])
	    AC_COMPILE_IFELSE(
              [AC_LANG_PROGRAM([#include <CoreServices/CoreServices.h>
                                #include <CoreAudio/CoreAudio.h>],
			       [AudioDeviceIOProcID procId;
			        AudioDeviceID device = kAudioDeviceUnknown;
        			AudioDeviceDestroyIOProcID(device, procId)
			       ])],
	      [AC_DEFINE(HAVE_AUDIODEVICEDESTROYIOPROCID)
	       AC_MSG_RESULT(yes)],
	      [AC_MSG_RESULT(no)])

	      AC_MSG_CHECKING([for kAudioDevicePropertyDeviceManufacturer])
	      AC_COMPILE_IFELSE(
              [AC_LANG_PROGRAM([#include <CoreServices/CoreServices.h>
                            #include <CoreAudio/CoreAudio.h>],
			   [AudioDeviceID deviceID; 
			    UInt32 trans_size = 0, trans;
      			    trans_size = sizeof(UInt32);
     			    AudioDeviceGetProperty(deviceID, 0, true, kAudioDevicePropertyTransportType, &trans_size, &trans)
			   ])],
	  		   [AC_DEFINE(HAVE_KAUDIODEVICEPROPERTYTRANSPORTTYPE)
	   		   AC_MSG_RESULT(yes)],
	  		   [AC_MSG_RESULT(no)])

		AC_MSG_CHECKING([for kLinearPCMFormatFlagIsNonInterleaved])
				  	   AC_COMPILE_IFELSE(
          		   [AC_LANG_PROGRAM([#include <CoreServices/CoreServices.h>
                            #include <CoreAudio/CoreAudio.h>],
			   [int i; i = kLinearPCMFormatFlagIsNonInterleaved])],
	  		   [AC_DEFINE(HAVE_KLINEARPCMFORMATFLAGISNONINTERLEAVED)
	   		   AC_MSG_RESULT(yes)],
	  		   [AC_MSG_RESULT(no)])
	   else
            AUDIO_SYSTEM=JACK
	    JACK_LIBS="-framework CoreAudio -framework CoreServices -framework AudioUnit -L/usr/local/lib -ljack -lsamplerate"
            JACK_FLAGS="-I/usr/local/include"
	    fi
	;;
esac
AC_MSG_CHECKING([for audio system])
AC_MSG_RESULT($audio_system)
fi

AUDIO_CHOICE="$audio_system"


if test "$enable_shared" = no; then
  SO_LD=":"
  SO_INSTALL=":"
fi


CFLAGS="-I. $CFLAGS"
# needed since we're looking for <mus-config.h>, I think

AC_SUBST(LDSO_FLAGS)
AC_SUBST(SO_FLAGS)
AC_SUBST(SO_INSTALL)
AC_SUBST(A_INSTALL)
AC_SUBST(SO_LD)
AC_SUBST(A_LD)
AC_SUBST(A_LD_FLAGS)
AC_SUBST(LD_FLAGS)
AC_SUBST(SNDLIB_VERSION)
AC_SUBST(SNDLIB_LANGUAGE)
AC_SUBST(AUDIO_CHOICE)
AC_SUBST(SO_NAME)
AC_SUBST(JACK_LIBS)
AC_SUBST(JACK_FLAGS)
AC_SUBST(RANLIB)

AC_OUTPUT


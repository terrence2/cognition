dnl Process this file with autoconf to produce a configure script
AC_INIT( cognition, [0.1] )
AC_CONFIG_HEADERS( [config.h] )
AM_INIT_AUTOMAKE

AC_PREREQ([2.59])

dnl Check for Programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Check for Libraries

dnl Check for headers
AC_HEADER_STDC

dnl Check for Library Functions
AC_FUNC_MALLOC
AC_FUNC_VPRINTF

dnl Check types
AC_CHECK_TYPE( BITMAPINFOHEADER, [AC_DEFINE( [HAVE_BITMAPINFOHEADER], [], [Bitmap Info Header Structure Defined] )] )
AC_CHECK_TYPE( BITMAPINFO, [AC_DEFINE( [HAVE_BITMAPINFO], [], [Bitmap Info Structure Defined] )] )
AC_CHECK_TYPE( RGBQUAD, [AC_DEFINE( [HAVE_RGBQUAD], [], [Bitmap RGBQUAD Defined] )] )
AC_CHECK_TYPE( BITMAPFILEHEADER, [AC_DEFINE( [HAVE_BITMAPFILEHEADER], [], [Bitmap File Header Defined] )] )
AC_CHECK_TYPE( WAVEFORMATEX, [AC_DEFINE( [HAVE_WAVEFORMATEX], [], [.wav File Format Header Defined] )] )
AC_CHECK_TYPE( UINT32, [AC_DEFINE( [HAVE_UINT32], [], [32-bit Unsigned Integer Defined] )] )
AC_CHECK_TYPE( INT32, [AC_DEFINE( [HAVE_INT32], [], [32-bit Signed Integer Defined] )] )
AC_CHECK_TYPE( UINT16, [AC_DEFINE( [HAVE_UINT16], [], [16-bit Unsigned Integer Defined] )] )
AC_CHECK_TYPE( INT16, [AC_DEFINE( [HAVE_INT16], [], [16-bit Signed Integer Defined] )] )
AC_CHECK_TYPE( UINT8, [AC_DEFINE( [HAVE_UINT8], [], [8-bit Unsigned Integer Defined] )] )
AC_CHECK_TYPE( INT8, [AC_DEFINE( [HAVE_INT8], [], [8-bit Signed Integer Defined] )] )

dnl Check stuff
AC_C_BIGENDIAN
dnl defines WORDS_BIGENDIAN
AC_C_CONST
dnl defines const as ' ' where not supported properly

dnl Check command line args, set certain variables for automake
AC_ARG_ENABLE( debug,
[ --enable-debug     Turn on debugging],
[case "${enableval}" in
	yes) debug=true ;;
	no)   debug=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac], [debug=false] )
AM_CONDITIONAL( DEBUG, test x$debug = xtrue )

AC_CONFIG_FILES( [Makefile cognition/Makefile] )
AC_OUTPUT


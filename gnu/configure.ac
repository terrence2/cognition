# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

dnl Process this file with autoconf to produce a configure script
AC_INIT( cognition, [0.1] )
AC_CONFIG_HEADERS( [config.h] )
AM_INIT_AUTOMAKE

AC_PREREQ([2.59])

dnl Check for Programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl Check for Libraries
MDL_HAVE_OPENGL
AM_PATH_SDL
AC_CHECK_LIB(GLEW,glewInit,[GLEW_CFLAGS="" GLEW_LIBS="-lGLEW"],[AC_MSG_ERROR([[Unable to find the GLEW library for OpenGL extension loading.]])])
AC_SUBST(GLEW_CFLAGS)
AC_SUBST(GLEW_LIBS)

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

dnl Check for SDL
SDL_VERSION=1.2.0
AM_PATH_SDL($SDL_VERSION,
            :,
            AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"


dnl Check command line args, set certain variables for automake
AC_ARG_ENABLE( debug,
[ --enable-debug     Turn on debugging],
[case "${enableval}" in
	yes) debug=true ;;
	no)   debug=false ;;
	*) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac], [debug=false] )
AM_CONDITIONAL( DEBUG, test x$debug = xtrue )

AC_CONFIG_FILES( [Makefile src/Makefile src/client/Makefile src/server/Makefile src/cognition/Makefile src/menu/Makefile src/shared/Makefile] )
AC_OUTPUT

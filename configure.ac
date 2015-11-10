#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Kayrebt::Cleaner], [0.1], [laurent.georget@inria.fr])
AM_INIT_AUTOMAKE([1.11 foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/types.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])
AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
   then AC_MSG_WARN([[Doxygen not found - continuing without Doxygen support]])
fi
AM_CONDITIONAL([HAVE_DOXYGEN],[test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([Doxyfile])])
AC_CHECK_PROGS([DOT], [dot])
AM_CONDITIONAL([HAVE_DOT],[test -n "$DOT"])
AM_COND_IF([HAVE_DOT], [AC_SUBST(DOT_IS_AVAILABLE,["YES"])],
                       [AC_SUBST(DOT_IS_AVAILABLE,["NO"])])
LT_INIT

# Checks for libraries.
PKG_CHECK_MODULES([LIBCGRAPH],[libcgraph])
# add CADP dependency here
AC_ARG_WITH([cadp],[AS_HELP_STRING([--with-cadp=<path to CADP installation dir> @<:@default=$CADP@:>@])],
	    [],[with_cadp=${CADP}])
AC_MSG_CHECKING([for CADP in $with_cadp])

USE_BCG=no
AC_COMPILE_IFELSE(
   [AC_LANG_SOURCE([ #include <$with_cadp/incl/bcg_user.h>
   ])],
   [
     old_libs=$LIBS
     LIBS="$LIBS -L${with_cadp}/bin.`${with_cadp}/com/arch` -lBCG_IO -lBCG"
     AC_LINK_IFELSE(
     [AC_LANG_SOURCE([
      #include <$with_cadp/incl/bcg_user.h>
      int main() {
        return (0);
      }
     ])],
     [
      AC_SUBST([CADP_BCG_CPPFLAGS],["-I${with_cadp}/incl"])
      AC_SUBST([CADP_BCG_LIBS],["-L${with_cadp}/bin.`${with_cadp}/com/arch` -lBCG_IO -lBCG"])
      USE_BCG=yes
      LIBS=$old_libs
      ],
     [AC_MSG_FAILURE([Could not link against CADP libraries in ${with_cadp}/bin.`${with_cadp}/com/arch`])])
   ],
   [AC_MSG_FAILURE([Could not find CADP headers in ${with_cadp}/incl])]
)
AC_MSG_RESULT($USE_BCG)
# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
# AC_CHECK_HEADER_STDBOOL # later
# AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_LIB([m],[cos])
AX_PTHREAD
BOOST_REQUIRE([1.52])
BOOST_GRAPH
BOOST_REGEX
BOOST_FILESYSTEM

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

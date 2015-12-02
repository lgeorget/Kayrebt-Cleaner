#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Kayrebt::ExaminePaths], [0.1], [laurent@lgeorget.eu])
AM_INIT_AUTOMAKE([1.11 foreign -Wall -Werror])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([src/constraint_parser.cpp])
AC_CONFIG_HEADERS([config.h])
AC_LANG([C++])
# Checks for programs.
AC_PROG_CXX
LT_INIT

# Checks for libraries.
AC_CHECK_HEADERS_ONCE([yices.h])
AX_PTHREAD
BOOST_REQUIRE([1.52])
BOOST_GRAPH
BOOST_REGEX

# Checks for header files.
AC_CHECK_LIB([yices],[yices_init])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

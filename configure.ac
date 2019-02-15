AC_INIT(instanalyzer, 1.0.0, nikita.dudko.95@gmail.com)
AC_LANG(C++)
AC_PROG_CXX

AC_SUBST(VERSION, 1.0.0)
AC_SUBST(HERE_APPID, $HERE_APPID)
AC_SUBST(HERE_APPCODE, $HERE_APPCODE)
AC_SUBST(YANDEX_API_KEY, $YANDEX_API_KEY)

AC_CHECK_PROG(CMAKE, cmake, yes, no)
test "$CMAKE" == "no" && AC_MSG_ERROR([Program "cmake" didn't find!])
AC_CHECK_PROG(CURL, curl, yes, no)
test "$CURL" == "no" && AC_MSG_ERROR([Program "curl" didn't find!])
AC_CHECK_PROG(GZIP, gzip, yes, no)
test "$GZIP" == "no" && AC_MSG_ERROR([Program "gzip" didn't find!])
AC_CHECK_PROG(TAR, tar, yes, no)
test "$TAR" == "no" && AC_MSG_ERROR([Program "tar" didn't find!])

AC_CHECK_LIB(curl, main, [], [AC_MSG_ERROR([Library "curl" didn't find (libcurl4-openssl-dev)!])])
AC_CHECK_LIB(unac, main, [], [AC_MSG_ERROR([Library "unac" didn't find (libunac1-dev)!])])
AC_CHECK_LIB(z, main, [], [AC_MSG_ERROR([Library "zlib" didn't find (zlib1g-dev)!])])
AC_CHECK_LIB(zip, main, [], [AC_MSG_ERROR([Library "zip" didn't find (libzip-dev)!])])

AC_OUTPUT(Makefile)
AC_MSG_NOTICE([Now you need to build libraries: make libs.])

AC_INIT(dbabstract/db.h)
AC_CONFIG_HEADER(config.h)
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE(dbabstract,0.2)
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AM_PROG_LIBTOOL

TEST_CXXFLAGS=""

AC_CHECK_LIB(PocoFoundation, Poco::Logger)

DOC=""
AC_ARG_ENABLE(doc, [  --enable-doc           Disables the building of documentation],
[
	DOC="doc"
	AC_CHECK_PROG(DOXYGEN, doxygen)
	AC_SUBST(DOXYGEN)
])
AC_SUBST(DOC)

MYSQL=""
AC_ARG_ENABLE(mysql,
	[  --enable-mysql=<path>     path to mysql],
[ if test "x$enableval" = "x"; then
	echo "You must specify a path for MySQL."
	exit -1
else
	MYSQL_PREFIX="$enableval"
	AC_SUBST(MYSQL_PREFIX)

	MYSQL="mysql"

	MYSQL_LDFLAGS="-L${MYSQL_PREFIX}/lib -L${MYSQL_PREFIX}/lib/mysql"
	MYSQL_CFLAGS="-I${MYSQL_PREFIX}/include -I${MYSQL_PREFIX}/include/mysql"
	MYSQL_CXXFLAGS="-I${MYSQL_PREFIX}/include -I${MYSQL_PREFIX}/include/mysql"
	MYSQL_LIBS="-lmysqlclient -lz"
	AC_SUBST(MYSQL_LDFLAGS)
	AC_SUBST(MYSQL_CFLAGS)
	AC_SUBST(MYSQL_CXXFLAGS)
	AC_SUBST(MYSQL_LIBS)

	TEST_CXXFLAGS="$TEST_CXXFLAGS -DENABLE_MYSQL"
fi
])
AC_SUBST(MYSQL)

SQLITE3=""
AC_ARG_ENABLE(sqlite3,
	[  --enable-sqlite3=<path>    path to sqlite3],
[ if test "x$enableval" = "x"; then
	echo "You must specify a path for Sqlite3."
	exit -1
else
	SQLITE3_PREFIX="$enableval"
	AC_SUBST(SQLITE3_PREFIX)

	SQLITE3="sqlite3"

	SQLITE3_LDFLAGS="-L${SQLITE3_PREFIX}/lib"
	SQLITE3_CFLAGS="-I${SQLITE3_PREFIX}/include"
	SQLITE3_CXXFLAGS="-I${SQLITE3_PREFIX}/include"
	SQLITE3_LIBS="-lsqlite3"
	AC_SUBST(SQLITE3_LDFLAGS)
	AC_SUBST(SQLITE3_CFLAGS)
	AC_SUBST(SQLITE3_CXXFLAGS)
	AC_SUBST(SQLITE3_LIBS)

	TEST_CXXFLAGS="$TEST_CXXFLAGS -DENABLE_SQLITE3"
fi
])

AC_SUBST(SQLITE3)

CFLAGS="$CFLAGS -I.." 
CXXFLAGS="$CXXFLAGS -I.."
LDFLAGS="$LDFLAGS $LIBS -lPocoFoundation"

AC_SUBST(TEST_CXXFLAGS)

AC_OUTPUT(Makefile
dbabstract/Makefile
dbabstract/mysql/Makefile
dbabstract/sqlite3/Makefile
doc/Makefile
test/Makefile)


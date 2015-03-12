/*
 * A database abstraction layer for C++ and ACE framework
 *
 * (C) 2006-2015 Thralling Penguin LLC. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "odbc_db.h"

namespace dbabstract
{

#ifdef UNICODE

#define TEXT(x)   (SQLWCHAR *) L##x
#define TEXTC(x)     (SQLWCHAR) L##x
#define TXTLEN(x) wcslen((wchar_t *) x)
#define TXTCMP(x1,x2) wcscmp((wchar_t *) x1, (wchar_t *) x2)

# ifdef WIN32
#define OPL_A2W(a, w, cb)     \
    MultiByteToWideChar(CP_ACP, 0, a, -1, w, cb)
# else
#define OPL_A2W(XA, XW, SIZE)      mbstowcs(XW, XA, SIZE)
# endif /* WIN32 */

#else

#define TEXT(x)   (SQLCHAR *) x
#define TEXTC(x)(SQLCHAR) x
            #define TXTLEN(x) strlen((char *) x)
#define TXTCMP(x1,x2) strcmp((char *) x1, (char *) x2)

#endif /* UNICODE */

#define NUMTCHAR(X)(sizeof (X) / sizeof (SQLTCHAR))

ODBC_ResultSet::~ODBC_ResultSet()
{
}

bool
ODBC_ResultSet::close(void)
{
    // eat remaining rows, if there are some
    while ((SQLMoreResults (hstmt)) == SQL_SUCCESS) {
        ;
    }

#if (ODBCVER < 0x0300)
    SQLFreeStmt (hstmt, SQL_CLOSE);
#else
    SQLCloseCursor (hstmt);
#endif

    delete this;

    return (true);
}

bool
ODBC_ResultSet::next(void)
{
    int sts = 0;
    /* execute this on the second row, not the first */
    if (record > 0) {
        sts = SQLMoreResults (hstmt);
    }
    record = 1;
    if (sts != SQL_SUCCESS)
        return false;

#if (ODBCVER < 0x0300)
    sts = SQLFetch (hstmt);
#else
    sts = SQLFetchScroll (hstmt, SQL_FETCH_NEXT, 1);
#endif
    if (sts == SQL_NO_DATA_FOUND)
        return false;
    if (sts != SQL_SUCCESS)
        return false;

    return ((sts == SQL_SUCCESS ? true : false));
}

unsigned long
ODBC_ResultSet::recordCount(void) const
{
    SQLLEN nrows = 0;
    SQLRowCount (hstmt, &nrows);
    return ((unsigned long) nrows);
}

unsigned int
ODBC_ResultSet::findColumn(const char *fld) const
{
    unsigned int i;
    short numCols = 0;
    SQLTCHAR colName[50];
    SQLSMALLINT colType;
    SQLULEN colPrecision;
    SQLSMALLINT colScale, colNullable;

    if (SQLNumResultCols (hstmt, &numCols) != SQL_SUCCESS) {
        return 0;
    }
    for (i=1; i<=numCols; i++) {
        if (SQLDescribeCol (hstmt, i, (SQLTCHAR *) colName, NUMTCHAR (colName), NULL,
                    &colType, &colPrecision, &colScale,
                    &colNullable) != SQL_SUCCESS)
            return 0;
        if (TXTCMP(colName, fld) == 0) {
            return (i);
        }
    }
    return (i);
}

const char *
ODBC_ResultSet::getString(const int idx) const
{
    int sts = 0;
    SQLLEN colIndicator;
    static SQLTCHAR fetchBuffer[1024 * 1024];
    static int index = 0;

    if (index == idx) {
        return (const char *)fetchBuffer;
    }
    index = idx;

    memset(fetchBuffer, 0, sizeof(fetchBuffer));

#ifdef UNICODE
    sts = SQLGetData (hstmt, idx, SQL_C_WCHAR, fetchBuffer,
            NUMTCHAR (fetchBuffer), &colIndicator);
#else
    sts = SQLGetData (hstmt, idx, SQL_C_CHAR, fetchBuffer,
            NUMTCHAR (fetchBuffer), &colIndicator);
#endif
    if (sts != SQL_SUCCESS_WITH_INFO && sts != SQL_SUCCESS) {
        std::cerr << "ERROR FETCHING DATA!" << std::endl;
    }

    return ((const char *) fetchBuffer);
}

int
ODBC_ResultSet::getInteger(const int idx) const
{
    return ((const int) ::atoi(getString(idx)));
}

bool
ODBC_ResultSet::getBool(const int idx) const
{
    const char *buf = getString(idx);
    if (buf && (buf[0] == '1' || buf[0] == 't')) {
        return (true);
    }
    return (false);
}

time_t
ODBC_ResultSet::getUnixTime(const int idx) const
{
    struct tm tmp;
    const char *buf = getString(idx);

    if (buf && strlen(buf)>=14) {
        // switch on the type of field
        // Basically, there are two types of returns I've seen
        // 1. has a hyphen and is fully parseable
        // 2. has no hyphen and is an older timestamp field
        // The others are raw DATE or TIME
        int Ypos = 0;
        int Mpos = 4;
        int Dpos = 6;
        int hpos = 8;
        int mpos = 10;
        int spos = 12;
        if (strchr(buf, '-') != NULL) {
            ++Mpos;
            Dpos = 8;
            hpos = 11;
            mpos = 14;
            spos = 17;
        }
        tmp.tm_wday = 0;
        tmp.tm_yday = 0;
        tmp.tm_isdst = 0;
        tmp.tm_year = (((buf[Ypos] - '0') * 1000) + ((buf[Ypos+1] - '0') * 100) + ((buf[Ypos+2] - '0') * 10) + (buf[Ypos+3] - '0')) - 1900;
        tmp.tm_mon = (((buf[Mpos] - '0') * 10) + (buf[Mpos+1] - '0')) - 1; /* 0 - 11 */
        tmp.tm_mday = (((buf[Dpos] - '0') * 10) + (buf[Dpos+1] - '0')); /* 1 - 31 */
        tmp.tm_hour = (((buf[hpos] - '0') * 10) + (buf[hpos+1] - '0')); /* 0 - 23 */
        tmp.tm_min = (((buf[mpos] - '0') * 10) + (buf[mpos+1] - '0')); /* 0-59 */
        tmp.tm_sec = (((buf[spos] - '0') * 10) + (buf[spos+1] - '0')); /* 0-59 */

        /*
          std::cerr << "y=" << tmp.tm_year << " m=" << tmp.tm_mon;
          std::cerr << " d=" << tmp.tm_mday << " h=" << tmp.tm_hour;
          std::cerr << " m=" << tmp.tm_min << " s=" << tmp.tm_sec << std::endl;
        */
        return (mktime(&tmp));
    }
    return (0);
}

double
ODBC_ResultSet::getDouble(const int idx) const
{
    char *pEnd;
    const char *buf = getString(idx);
    return ((buf ? strtod(buf, &pEnd) : 0));
}

float
ODBC_ResultSet::getFloat(const int idx) const
{
    const char *buf = getString(idx);
    return ((buf ? atof(buf) : 0));
}

long
ODBC_ResultSet::getLong(const int idx) const
{
    const char *buf = getString(idx);
    return ((buf ? atol(buf) : 0L));
}

short
ODBC_ResultSet::getShort(const int idx) const
{
    const char *buf = getString(idx);
    return ((short) (buf ? atoi(buf) : 0));
}

void *
ODBC_ResultSet::operator new (size_t bytes)
{
  return (::new char[bytes]);
}

void
ODBC_ResultSet::operator delete (void *ptr)
{
  delete [] static_cast <char *> (ptr);
}

#include <string.h>

SQLTCHAR outdsn[4096];


int
ODBC_Connection::ODBC_Errors (char *where) const
{
  SQLTCHAR buf[512];
  SQLTCHAR sqlstate[15];
  SQLINTEGER native_error = 0;
  int force_exit = 0;
  SQLRETURN sts;

#if (ODBCVER < 0x0300)
  /*
   *  Get statement errors
   */
  while (hstmt)
    {
      sts = SQLError (henv, hdbc, hstmt, sqlstate, &native_error,
      buf, NUMTCHAR (buf), NULL);
      if (!SQL_SUCCEEDED (sts))
    break;

#ifdef UNICODE
      fprintf (stderr, "%s = %S (%ld) SQLSTATE=%S\n",
      where, buf, (long) native_error, sqlstate);
#else
      fprintf (stderr, "%s = %s (%ld) SQLSTATE=%s\n",
      where, buf, (long) native_error, sqlstate);
#endif

      /*
       *  If the driver could not be loaded, there is no point in
       *  continuing, after reading all the error messages
       */
      if (!TXTCMP (sqlstate, TEXT ("IM003")))
    force_exit = 1;
    }

  /*
   *  Get connection errors
   */
  while (hdbc)
    {
      sts = SQLError (henv, hdbc, SQL_NULL_HSTMT, sqlstate, &native_error,
      buf, NUMTCHAR (buf), NULL);
      if (!SQL_SUCCEEDED (sts))
    break;

#ifdef UNICODE
      fprintf (stderr, "%s = %S (%ld) SQLSTATE=%S\n",
      where, buf, (long) native_error, sqlstate);
#else
      fprintf (stderr, "%s = %s (%ld) SQLSTATE=%s\n",
      where, buf, (long) native_error, sqlstate);
#endif

      /*
       *  If the driver could not be loaded, there is no point in
       *  continuing, after reading all the error messages
       */
      if (!TXTCMP (sqlstate, TEXT ("IM003")))
    force_exit = 1;
    }

  /*
   *  Get environment errors
   */
  while (henv)
    {
      sts SQLError (henv, SQL_NULL_HDBC, SQL_NULL_HSTMT, sqlstate,
      &native_error, buf, NUMTCHAR (buf), NULL);
      if (!SQL_SUCCEEDED (sts))
    break;

#ifdef UNICODE
      fprintf (stderr, "%s = %S (%ld) SQLSTATE=%S\n",
      where, buf, (long) native_error, sqlstate);
#else
      fprintf (stderr, "%s = %s (%ld) SQLSTATE=%s\n",
      where, buf, (long) native_error, sqlstate);
#endif

      /*
       *  If the driver could not be loaded, there is no point in
       *  continuing, after reading all the error messages
       */
      if (!TXTCMP (sqlstate, TEXT ("IM003")))
    force_exit = 1;
    }
#else /* ODBCVER */
  int i;

  /*
   *  Get statement errors
   */
  i = 0;
  while (hstmt && i < 5)
    {
      sts = SQLGetDiagRec (SQL_HANDLE_STMT, hstmt, ++i,
      sqlstate, &native_error, buf, NUMTCHAR (buf), NULL);
      if (!SQL_SUCCEEDED (sts))
    break;

#ifdef UNICODE
      fprintf (stderr, "%d: %s = %S (%ld) SQLSTATE=%S\n",
      i, where, buf, (long) native_error, sqlstate);
#else
      fprintf (stderr, "%d: %s = %s (%ld) SQLSTATE=%s\n",
      i, where, buf, (long) native_error, sqlstate);
#endif

      /*
       *  If the driver could not be loaded, there is no point in
       *  continuing, after reading all the error messages
       */
      if (!TXTCMP (sqlstate, TEXT ("IM003")))
    force_exit = 1;
    }

  /*
   *  Get connection errors
   */
  i = 0;
  while (hdbc && i < 5)
    {
      sts = SQLGetDiagRec (SQL_HANDLE_DBC, hdbc, ++i,
      sqlstate, &native_error, buf, NUMTCHAR (buf), NULL);
      if (!SQL_SUCCEEDED (sts))
    break;

#ifdef UNICODE
      fprintf (stderr, "%d: %s = %S (%ld) SQLSTATE=%S\n",
      i, where, buf, (long) native_error, sqlstate);
#else
      fprintf (stderr, "%d: %s = %s (%ld) SQLSTATE=%s\n",
      i, where, buf, (long) native_error, sqlstate);
#endif

      /*
       *  If the driver could not be loaded, there is no point in
       *  continuing, after reading all the error messages
       */
      if (!TXTCMP (sqlstate, TEXT ("IM003")))
    force_exit = 1;
    }

  /*
   *  Get environment errors
   */
  i = 0;
  while (henv && i < 5)
    {
      sts = SQLGetDiagRec (SQL_HANDLE_ENV, henv, ++i,
      sqlstate, &native_error, buf, NUMTCHAR (buf), NULL);
      if (!SQL_SUCCEEDED (sts))
    break;

#ifdef UNICODE
      fprintf (stderr, "%d: %s = %S (%ld) SQLSTATE=%S\n",
      i, where, buf, (long) native_error, sqlstate);
#else
      fprintf (stderr, "%d: %s = %s (%ld) SQLSTATE=%s\n",
      i, where, buf, (long) native_error, sqlstate);
#endif

      /*
       *  If the driver could not be loaded, there is no point in
       *  continuing, after reading all the error messages
       */
      if (!TXTCMP (sqlstate, TEXT ("IM003")))
    force_exit = 1;
    }
#endif /* ODBCVER */

  /*
   *  Force an exit status
   */
  if (force_exit)
    exit (-1);

  return -1;
}


bool
ODBC_Connection::open(const char *database, const char *host, const int port, const char *user, const char *pass)
{
    short buflen;
    SQLCHAR dataSource[1024];
    int status;
#ifdef UNICODE
    SQLWCHAR wdataSource[1024];
#endif

    strcpy((char *) dataSource, database);

#if (ODBCVER < 0x0300)
    if (SQLAllocEnv (&henv) != SQL_SUCCESS)
        return (false);

    if (SQLAllocConnect (henv, &hdbc) != SQL_SUCCESS)
        return (false);
#else
    if (SQLAllocHandle (SQL_HANDLE_ENV, NULL, &henv) != SQL_SUCCESS) {
        return (false);
    }

    SQLSetEnvAttr (henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3,
            SQL_IS_UINTEGER);

    if (SQLAllocHandle (SQL_HANDLE_DBC, henv, &hdbc) != SQL_SUCCESS) {
        return (false);
    }
#endif

#ifdef SQL_APPLICATION_NAME
    SQLSetConnectOption (hdbc, SQL_APPLICATION_NAME, (SQLULEN) TEXT ("dbabstract"));
#endif

#ifdef UNICODE
    strcpy_A2W (wdataSource, (char *) dataSource);
    status = SQLDriverConnectW (hdbc, 0, (SQLWCHAR *) wdataSource, SQL_NTS,
            (SQLWCHAR *) outdsn, NUMTCHAR (outdsn), &buflen, SQL_DRIVER_COMPLETE);
    if (status != SQL_SUCCESS)
        return (false);
#else
    status = SQLDriverConnect (hdbc, 0, (SQLCHAR *)dataSource, SQL_NTS,
            (SQLCHAR *) outdsn, NUMTCHAR (outdsn), &buflen, SQL_DRIVER_COMPLETE);
    if (status != SQL_SUCCESS && status != SQL_SUCCESS_WITH_INFO) {
        ODBC_Errors ("SQLDriverConnect");
        return (false);
    }
#endif
    connected = 1;

#if (ODBCVER < 0x0300)
    if (SQLAllocStmt (hdbc, &hstmt) != SQL_SUCCESS)
        return (false);
#else
    if (SQLAllocHandle (SQL_HANDLE_STMT, hdbc, &hstmt) != SQL_SUCCESS)
        return (false);
#endif
    return (true);
}

bool
ODBC_Connection::close(void)
{
#if (ODBCVER < 0x0300)
    if (hstmt) {
        SQLFreeStmt (hstmt, SQL_DROP);
        hstmt = NULL;
    }
    if (connected) {
        SQLDisconnect (hdbc);
        connected = 0;
    }
    if (hdbc) {
        SQLFreeConnect (hdbc);
        hdbc = NULL;
    }
    if (henv) {
        SQLFreeEnv (henv);
        henv = NULL;
    }
#else
    if (hstmt) {
        SQLCloseCursor (hstmt);
        SQLFreeHandle (SQL_HANDLE_STMT, hstmt);
        hstmt = NULL;
    }
    if (connected) {
        SQLDisconnect (hdbc);
        connected = 0;
    }
    if (hdbc) {
        SQLFreeHandle (SQL_HANDLE_DBC, hdbc);
        hdbc = NULL;
    }
    if (henv) {
        SQLFreeHandle (SQL_HANDLE_ENV, henv);
        henv = NULL;
    }
#endif
    return (true);
}

bool
ODBC_Connection::isConnected(void)
{
    return (true);
}

bool
ODBC_Connection::execute(const char *sql)
{
    int sts;

    if (!connected) return (false);

    if (SQLPrepare (hstmt, (SQLTCHAR *) sql, SQL_NTS) != SQL_SUCCESS)
        return (false);

    if ((sts=SQLExecute (hstmt)) != SQL_SUCCESS && sts != SQL_SUCCESS_WITH_INFO)
        return (false);

    /*
#if (ODBCVER < 0x0300)
    if (hstmt)
        SQLFreeStmt (hstmt, SQL_DROP);                                      
#else
    if (hstmt) {
        SQLCloseCursor (hstmt);
        SQLFreeHandle (SQL_HANDLE_STMT, hstmt);
    }
#endif
    hstmt = 0;
    */
    return (true);
}

dbabstract::ResultSet *
ODBC_Connection::executeQuery(const char *sql)
{
    if (!connected) return (NULL);

    if (SQLPrepare (hstmt, (SQLTCHAR *) sql, SQL_NTS) != SQL_SUCCESS)
        return (NULL);

    if (SQLExecute (hstmt) != SQL_SUCCESS)
        return (NULL);

    dbabstract::ResultSet *c = 0;
    c = new dbabstract::ODBC_ResultSet(hstmt);
    return (c);
}

char *
ODBC_Connection::escape(const char *str)
{
    unsigned long len = strlen(str);
    char *buf = new char[len*2+4];
    buf[0] = '\'';
    int pos = 1;
    for (int i=0; i<len; i++) {
        if (str[i] == '\'') {
            buf[pos++] = '\'';
            buf[pos++] = '\'';
        } else {
            buf[pos++] = str[i];
        }
    }
    buf[pos++] = '\'';
    buf[pos] = 0;
    return (buf);
}

const char *
ODBC_Connection::unixtimeToSql(const time_t val)
{
    char *buf = new char[22];
    struct tm *tmp = new struct tm;
    buf[0] = '\'';
    strftime(buf+1, 20, "%Y-%m-%d %H:%M:%S", gmtime_r(&val, tmp));
    buf[20] = '\'';
    buf[21] = 0;
    delete tmp;
    return (buf);
}

unsigned long
ODBC_Connection::insertId(void)
{
    /* Unsupported feature, dependent on databases implementation */
    return (0);
}

bool
ODBC_Connection::beginTrans(void)
{
    if (!connected) return (false);
    if (execute("BEGIN") == true) {
        return (true);
    }
    return (false);
}

bool
ODBC_Connection::commitTrans(void)
{
    if (!connected) return (false);
    if (execute("COMMIT") == true) {
        return (true);
    }
    return (false);
}

bool
ODBC_Connection::rollbackTrans(void)
{
    if (!connected) return (false);
    if (execute("ROLLBACK") == true) {
        return (true);
    }
    return (false);
}

bool
ODBC_Connection::setTransactionMode(const enum TRANS_MODE mode)
{
    const char *sql = "";

    switch (mode) {
    case READ_UNCOMMITTED:
        sql = "SET SESSION TRANSACTION ISOLATION LEVEL READ UNCOMMITTED";
        break;
    case READ_COMMITTED:
        sql = "SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED";
        break;
    case REPEATABLE_READ:
        sql = "SET SESSION TRANSACTION ISOLATION LEVEL REPEATABLE READ";
        break;
    case SERIALIZABLE:
        sql = "SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE";
        break;
    }
    return (execute(sql));
}

unsigned int
ODBC_Connection::errorno(void) const
{
    SQLTCHAR buf[512];
    SQLTCHAR sqlstate[15];
    SQLINTEGER native_error = 0;
    SQLRETURN sts;

#if (ODBCVER < 0x0300)
    sts = SQLError (henv, hdbc, hstmt, sqlstate, &native_error,
            buf, NUMTCHAR (buf), NULL);
    if (!SQL_SUCCEEDED (sts))
        return (0);
#else
    sts = SQLGetDiagRec (SQL_HANDLE_STMT, hstmt, 0, sqlstate, &native_error,
            buf, NUMTCHAR (buf), NULL);
    if (!SQL_SUCCEEDED (sts))
        return (0);
#endif
    return (native_error);
}

const char *
ODBC_Connection::errormsg(void) const
{
    static SQLTCHAR buf[512];
    SQLTCHAR sqlstate[15];
    SQLINTEGER native_error = 0;
    SQLRETURN sts;

    ODBC_Errors ("errormsg");

#if (ODBCVER < 0x0300)
    sts = SQLError (henv, hdbc, hstmt, sqlstate, &native_error,
            buf, NUMTCHAR (buf), NULL);
    if (!SQL_SUCCEEDED (sts))
        return (0);
#else
    sts = SQLGetDiagRec (SQL_HANDLE_STMT, hstmt, 0, sqlstate, &native_error,
            buf, NUMTCHAR (buf), NULL);
    if (!SQL_SUCCEEDED (sts))
        return (0);
#endif
    return ((const char *) buf);
}

const char *
ODBC_Connection::version(void) const
{
    static char ret[256];
    snprintf(ret, 256, "ODBC Driver v0.1");
    return ((const char *) ret);
}

void *
ODBC_Connection::operator new (size_t bytes)
{
  return (::new char[bytes]);
}

void
ODBC_Connection::operator delete (void *ptr)
{
  delete [] static_cast <char *> (ptr);
}

// Returns the Newsweek class pointer.
// The ACE_BUILD_SVC_DLL and ACE_Svc_Export directives are necessary to
// take care of exporting the function for Win32 platforms.
//

#ifndef STATIC

extern "C" LIBRARY_API Connection *create_connection (void);

dbabstract::Connection *
create_connection (void)
{
  dbabstract::ODBC_Connection *c = 0;
  c = new dbabstract::ODBC_Connection;
  return (c);
}

#else

extern "C" LIBRARY_API Connection *create_odbc_connection (void);

dbabstract::Connection *
create_odbc_connection (void)
{
  dbabstract::ODBC_Connection *c = 0;
  c = new dbabstract::ODBC_Connection;
  return (c);
}

#endif

}


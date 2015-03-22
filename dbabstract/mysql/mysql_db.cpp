/*
 * A database abstraction layer for C++ and ACE framework
 *
 * (C) 2006-2014 Thralling Penguin LLC. All rights reserved.
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

#include "mysql_db.h"

#include "mysql/mysql.h"

namespace dbabstract
{

MySQL_ResultSet::~MySQL_ResultSet()
{
}

bool
MySQL_ResultSet::close(void)
{
    if (!res_) return (false);

    // eat remaining rows, if there are some
    while ((row_ = mysql_fetch_row(res_)) != NULL) {
        ;
    }

    mysql_free_result(res_);
    res_ = NULL;
    delete this;

    return (true);
}

bool
MySQL_ResultSet::next(void)
{
    row_ = mysql_fetch_row(res_);
    return ((row_ != NULL ? true : false));
}

unsigned long
MySQL_ResultSet::recordCount(void) const
{
    /* NOTE: This call ALWAYS fails because mysql_use_result
       is used, UNTIL all rows have been read. This is documented
       in the MySQL manual.

       I still opt for using mysql_use_result as it reduces
       the traffic/memory requirements on the client side.
    */
    return ((unsigned long) mysql_num_rows(res_));
}

unsigned int
MySQL_ResultSet::findColumn(const char *fld) const
{
    unsigned int num_fields;
    unsigned int i;
    MYSQL_FIELD *field;

    num_fields = mysql_num_fields(res_);
    for (i=0; i<num_fields; i++) {
        field = mysql_fetch_field_direct(res_, i);
        if (::strcmp(field->name, fld) == 0) {
            return (i);
        }
    }
    return (i);
}

const char *
MySQL_ResultSet::getString(const int idx) const
{
    return (row_[idx]);
}

int
MySQL_ResultSet::getInteger(const int idx) const
{
    return ((const int) ::atoi(row_[idx]));
}

bool
MySQL_ResultSet::getBool(const int idx) const
{
    if (row_[idx] && (row_[idx][0] == '1' || row_[idx][0] == 't')) {
        return (true);
    }
    return (false);
}

time_t
MySQL_ResultSet::getUnixTime(const int idx) const
{
    struct tm tmp;

    if (row_[idx] && strlen(row_[idx])>=14) {
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
        if (strchr(row_[idx], '-') != NULL) {
            ++Mpos;
            Dpos = 8;
            hpos = 11;
            mpos = 14;
            spos = 17;
        }
        tmp.tm_wday = 0;
        tmp.tm_yday = 0;
        tmp.tm_isdst = 0;
        tmp.tm_year = (((row_[idx][Ypos] - '0') * 1000) + ((row_[idx][Ypos+1] - '0') * 100) + ((row_[idx][Ypos+2] - '0') * 10) + (row_[idx][Ypos+3] - '0')) - 1900;
        tmp.tm_mon = (((row_[idx][Mpos] - '0') * 10) + (row_[idx][Mpos+1] - '0')) - 1; /* 0 - 11 */
        tmp.tm_mday = (((row_[idx][Dpos] - '0') * 10) + (row_[idx][Dpos+1] - '0')); /* 1 - 31 */
        tmp.tm_hour = (((row_[idx][hpos] - '0') * 10) + (row_[idx][hpos+1] - '0')); /* 0 - 23 */
        tmp.tm_min = (((row_[idx][mpos] - '0') * 10) + (row_[idx][mpos+1] - '0')); /* 0-59 */
        tmp.tm_sec = (((row_[idx][spos] - '0') * 10) + (row_[idx][spos+1] - '0')); /* 0-59 */

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
MySQL_ResultSet::getDouble(const int idx) const
{
    char *pEnd;
    return ((row_[idx] ? strtod(row_[idx], &pEnd) : 0));
}

float
MySQL_ResultSet::getFloat(const int idx) const
{
    return ((row_[idx] ? atof(row_[idx]) : 0));
}

long
MySQL_ResultSet::getLong(const int idx) const
{
    return ((row_[idx] ? atol(row_[idx]) : 0L));
}

short
MySQL_ResultSet::getShort(const int idx) const
{
    return ((short) (row_[idx] ? atoi(row_[idx]) : 0));
}

void *
MySQL_ResultSet::operator new (size_t bytes)
{
  return (::new char[bytes]);
}

void
MySQL_ResultSet::operator delete (void *ptr)
{
  delete [] static_cast <char *> (ptr);
}

bool
MySQL_Connection::open(const char *database, const char *host, const int port, const char *user, const char *pass)
{
    if ((mysql_ = mysql_init(NULL)) == NULL) {
        return (false);
    }
    if (mysql_real_connect(
            mysql_,
            host,
            user,
            pass,
            NULL,
            (unsigned)port,
            NULL,
            0
            ) == NULL) {
        return (false);
    }
    if (mysql_select_db(mysql_, database) != 0) {
        return (false);
    }
    return (true);
}

bool
MySQL_Connection::close(void)
{
    if (!mysql_) return (false);
    mysql_close(mysql_);
    mysql_ = NULL;
    return (true);
}

bool
MySQL_Connection::isConnected(void)
{
    if (!mysql_) return (false);
    return ((mysql_ping(mysql_) == 0 ? true : false));
}

bool
MySQL_Connection::execute(const char *sql)
{
    if (!mysql_) return (false);
    if (mysql_query(mysql_, sql) == 0) {
        return (true);
    }
    return (false);
}

dbabstract::ResultSet *
MySQL_Connection::executeQuery(const char *sql)
{
    if (!mysql_) return (NULL);

    if (mysql_query(mysql_, sql)) {
        return (0);
    }
    MYSQL_RES *res = mysql_use_result(mysql_);
    if (!res) {
        return (0);
    }
    dbabstract::ResultSet *c = 0;
    c = new dbabstract::MySQL_ResultSet(res);
    return (c);
}

char *
MySQL_Connection::escape(const char *str)
{
    unsigned long len = strlen(str);
    char *buf = new char[len*2+4];
    buf[0] = '\'';
    len = mysql_escape_string(buf+1, str, len);
    buf[++len] = '\'';
    buf[++len] = 0;
    return (buf);
}

const char *
MySQL_Connection::unixtimeToSql(const time_t val)
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
MySQL_Connection::insertId(void)
{
    return ((const unsigned long) mysql_insert_id(mysql_));
}

bool
MySQL_Connection::beginTrans(void)
{
    if (!mysql_) return (false);
    if (mysql_query(mysql_, "SET AUTOCOMMIT = 0") != 0) {
        return (false);
    }
    if (mysql_query(mysql_, "BEGIN") == 0) {
        return (true);
    }
    return (false);
}

bool
MySQL_Connection::commitTrans(void)
{
    if (!mysql_) return (false);
    if (mysql_query(mysql_, "COMMIT") == 0) {
        return (true);
    }
    return (false);
}

bool
MySQL_Connection::rollbackTrans(void)
{
    if (!mysql_) return (false);
    if (mysql_query(mysql_, "ROLLBACK") == 0) {
        return (true);
    }
    return (false);
}

bool
MySQL_Connection::setTransactionMode(const enum TRANS_MODE mode)
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
MySQL_Connection::errorno(void) const
{
    return (mysql_errno(mysql_));
}

const char *
MySQL_Connection::errormsg(void) const
{
    return (mysql_error(mysql_));
}

const char *
MySQL_Connection::version(void) const
{
    static char ret[256];
    snprintf(ret, 256, "MySQL Driver v0.2 using MySQL client library v%s", mysql_get_client_info());
    return ((const char *) ret);
}

void *
MySQL_Connection::operator new (size_t bytes)
{
  return (::new char[bytes]);
}

void
MySQL_Connection::operator delete (void *ptr)
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
  dbabstract::MySQL_Connection *c = 0;
  c = new dbabstract::MySQL_Connection;
  return (c);
}

#else

extern "C" LIBRARY_API Connection *create_mysql_connection (void);

dbabstract::Connection *
create_mysql_connection (void)
{
  dbabstract::MySQL_Connection *c = 0;
  c = new dbabstract::MySQL_Connection;
  return (c);
}

#endif

}


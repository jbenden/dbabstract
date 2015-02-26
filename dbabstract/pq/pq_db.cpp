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

#include "pq_db.h"

namespace dbabstract
{

PQ_ResultSet::~PQ_ResultSet()
{
}

bool
PQ_ResultSet::close(void)
{
    if (!res_) return (false);

    PQclear(res_);
    res_ = NULL;
    delete this;

    return (true);
}

bool
PQ_ResultSet::next(void)
{
    row_++;
    return ((row_ < PQntuples(res_) ? true : false));
}

unsigned long
PQ_ResultSet::recordCount(void) const
{
    /* NOTE: This call ALWAYS fails because mysql_use_result
       is used, UNTIL all rows have been read. This is documented
       in the MySQL manual.

       I still opt for using mysql_use_result as it reduces
       the traffic/memory requirements on the client side.
    */
    return ((unsigned long) PQntuples(res_));
}

unsigned int
PQ_ResultSet::findColumn(const char *fld) const
{
    unsigned int num_fields;
    unsigned int i;
    char *field;

    num_fields = PQnfields(res_);
    for (i=0; i<num_fields; i++) {
        field = PQfname(res_, i);
        if (::strcmp(field, fld) == 0) {
            return (i);
        }
    }
    return (i);
}

const char *
PQ_ResultSet::getString(const int idx) const
{
    return (PQgetvalue(res_, row_, idx));
}

int
PQ_ResultSet::getInteger(const int idx) const
{
    return ((const int) ::atoi(getString(idx)));
}

bool
PQ_ResultSet::getBool(const int idx) const
{
    const char *res = getString(idx);
    if (res && res[0] == '1') {
        return (true);
    }
    return (false);
}

time_t
PQ_ResultSet::getUnixTime(const int idx) const
{
    struct tm tmp;

    const char *res = getString(idx);
    if (res && strlen(res) >= 14) {
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
        if (strchr(res, '-') != NULL) {
            ++Mpos;
            Dpos = 8;
            hpos = 11;
            mpos = 14;
            spos = 17;
        }
        tmp.tm_wday = 0;
        tmp.tm_yday = 0;
        tmp.tm_isdst = 0;
        tmp.tm_year = (((res[Ypos] - '0') * 1000) + ((res[Ypos+1] - '0') * 100) + ((res[Ypos+2] - '0') * 10) + (res[Ypos+3] - '0')) - 1900;
        tmp.tm_mon = (((res[Mpos] - '0') * 10) + (res[Mpos+1] - '0')) - 1; /* 0 - 11 */
        tmp.tm_mday = (((res[Dpos] - '0') * 10) + (res[Dpos+1] - '0')); /* 1 - 31 */
        tmp.tm_hour = (((res[hpos] - '0') * 10) + (res[hpos+1] - '0')); /* 0 - 23 */
        tmp.tm_min = (((res[mpos] - '0') * 10) + (res[mpos+1] - '0')); /* 0-59 */
        tmp.tm_sec = (((res[spos] - '0') * 10) + (res[spos+1] - '0')); /* 0-59 */

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
PQ_ResultSet::getDouble(const int idx) const
{
    char *pEnd;
    const char *res = getString(idx);;
    return ((res ? strtod(res, &pEnd) : 0));
}

float
PQ_ResultSet::getFloat(const int idx) const
{
    const char *res = getString(idx);
    return ((res ? atof(res) : 0));
}

long
PQ_ResultSet::getLong(const int idx) const
{
    const char *res = getString(idx);
    return ((res ? atol(res) : 0L));
}

short
PQ_ResultSet::getShort(const int idx) const
{
    const char *res = getString(idx);
    short val = 0;
    if (res) val = (short) atoi(res);
    return (val);
}

void *
PQ_ResultSet::operator new (size_t bytes)
{
  return (::new char[bytes]);
}

void
PQ_ResultSet::operator delete (void *ptr)
{
  delete [] static_cast <char *> (ptr);
}

bool
PQ_Connection::open(const char *database, const char *host, const int port, const char *user, const char *pass)
{
    pgconn_ = PQconnectdb(database);
    database_ = database;
    if (PQstatus(pgconn_) != CONNECTION_OK) {
        if (pgconn_)
            PQfinish(pgconn_);    
        pgconn_ = false;
        return (false);
    }
    return (true);
}

bool
PQ_Connection::close(void)
{
    if (!pgconn_) return (false);
    PQfinish(pgconn_);
    pgconn_ = NULL;
    return (true);
}

bool
PQ_Connection::isConnected(void)
{
    if (!pgconn_) return (false);
    return ((PQping(database_.c_str()) == PQPING_OK ? true : false));
}

bool
PQ_Connection::execute(const char *sql)
{
    if (!pgconn_) return (false);
    PGresult *res = PQexec(pgconn_, sql);
    if (PQresultStatus(res) == PGRES_COMMAND_OK) {
        PQclear(res);
        return (true);
    }
    std::cerr << "Error issuing command: " << PQerrorMessage(pgconn_) << std::endl;
    PQclear(res);
    return (false);
}

dbabstract::ResultSet *
PQ_Connection::executeQuery(const char *sql)
{
    if (!pgconn_) return (NULL);

    PGresult *res = PQexec(pgconn_, sql);
    if (PQresultStatus(res) == PGRES_COMMAND_OK) {
        return (0);
    }
    dbabstract::ResultSet *c = 0;
    c = new dbabstract::PQ_ResultSet(res);
    return (c);
}

char *
PQ_Connection::escape(const char *str)
{
    char *escaped = PQescapeLiteral(pgconn_, str, strlen(str));
    unsigned long len = strlen(escaped);
    char *buf = new char[len+4];
    //buf[0] = '\'';
    strcat(buf, escaped);
    //buf[++len] = '\'';
    buf[++len] = 0;
    PQfreemem(escaped);
    return (buf);
}

const char *
PQ_Connection::unixtimeToSql(const time_t val)
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
PQ_Connection::insertId(void)
{
    return (0);
}

bool
PQ_Connection::beginTrans(void)
{
    if (!pgconn_) return (false);
    PGresult *res = PQexec(pgconn_, "BEGIN");
    if (PQresultStatus(res) == PGRES_COMMAND_OK) {
        PQclear(res);
        return (true);
    }
    PQclear(res);
    return (false);
}

bool
PQ_Connection::commitTrans(void)
{
    if (!pgconn_) return (false);
    PGresult *res = PQexec(pgconn_, "END");
    if (PQresultStatus(res) == PGRES_COMMAND_OK) {
        PQclear(res);
        return (true);
    }
    PQclear(res);
    return (false);
}

bool
PQ_Connection::rollbackTrans(void)
{
    if (!pgconn_) return (false);
    PGresult *res = PQexec(pgconn_, "ROLLBACK");
    if (PQresultStatus(res) == PGRES_COMMAND_OK) {
        PQclear(res);
        return (true);
    }
    PQclear(res);
    return (false);
}

bool
PQ_Connection::setTransactionMode(const enum TRANS_MODE mode)
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
PQ_Connection::errorno(void) const
{
    return (PQstatus(pgconn_));
}

const char *
PQ_Connection::errormsg(void) const
{
    return (PQerrorMessage(pgconn_));
}

const char *
PQ_Connection::version(void) const
{
    static char ret[256];
    snprintf(ret, 256, "PostgreSQL Driver v0.1");
    return ((const char *) ret);
}

void *
PQ_Connection::operator new (size_t bytes)
{
  return (::new char[bytes]);
}

void
PQ_Connection::operator delete (void *ptr)
{
  delete [] static_cast <char *> (ptr);
}

// Returns the Newsweek class pointer.
// The ACE_BUILD_SVC_DLL and ACE_Svc_Export directives are necessary to
// take care of exporting the function for Win32 platforms.
//

#ifndef STATIC

extern "C" LIBRARY_API dbabstract::Connection *create_connection (void);

dbabstract::Connection *
create_connection (void)
{
  dbabstract::PQ_Connection *c = 0;
  c = new dbabstract::PQ_Connection;
  return (c);
}

#else

extern "C" LIBRARY_API dbabstract::Connection *create_pq_connection (void);

dbabstract::Connection *
create_pq_connection (void)
{
  dbabstract::PQ_Connection *c = 0;
  c = new dbabstract::PQ_Connection;
  return (c);
}

#endif

}


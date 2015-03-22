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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include "sqlite3_db.h"

#include "sqlite3.h"

namespace dbabstract
{

Sqlite3_ResultSet::~Sqlite3_ResultSet()
{
}

bool
Sqlite3_ResultSet::close(void)
{
    int rc;

    if (!res_) return (false);

    // eat remaining rows, if there are some
    do {
        rc = sqlite3_step(res_);
    } while (rc != SQLITE_DONE && rc != SQLITE_ERROR && rc != SQLITE_MISUSE);

    sqlite3_finalize(res_);
    res_ = NULL;
    delete this;

    return (true);
}

bool
Sqlite3_ResultSet::next(void)
{
    int rc;

    do {
        rc = sqlite3_step(res_);
        if (rc == SQLITE_BUSY) {
            sleep(1);
            continue;
        }
        if (rc == SQLITE_ROW) {
            return (true);
        }
    } while (rc != SQLITE_DONE && rc != SQLITE_ERROR && rc != SQLITE_MISUSE);
    return (false);
}

unsigned long
Sqlite3_ResultSet::recordCount(void) const
{
    return (0); // not supported
}

unsigned int
Sqlite3_ResultSet::findColumn(const char *fld) const
{
    unsigned int num_fields;
    unsigned int i;
    const char *field;

    num_fields = sqlite3_column_count(res_);
    for (i=0; i<num_fields; i++) {
        field = sqlite3_column_name(res_, i);
        if (::strcmp(field, fld) == 0) {
            return (i);
        }
    }
    return (i);
}

const char *
Sqlite3_ResultSet::getString(const int idx) const
{
    return ((const char *) sqlite3_column_text(res_, idx));
}

int
Sqlite3_ResultSet::getInteger(const int idx) const
{
    return ((const int) atoi((const char *) sqlite3_column_text(res_, idx)));
}

bool
Sqlite3_ResultSet::getBool(const int idx) const
{
    register const char *v = (const char *) sqlite3_column_text(res_, idx);
    if (v && (v[0] == '1' || v[0] == 't')) {
        return (true);
    }
    return (false);
}

time_t
Sqlite3_ResultSet::getUnixTime(const int idx) const
{
    struct tm tmp;
    register const char *v = (const char *) sqlite3_column_text(res_, idx);

    if (v && strlen(v)>=14) {
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
        if (strchr(v, '-') != NULL) {
            ++Mpos;
            Dpos = 8;
            hpos = 11;
            mpos = 14;
            spos = 17;
        }
        tmp.tm_wday = 0;
        tmp.tm_yday = 0;
        tmp.tm_isdst = 0;
        tmp.tm_year = (((v[Ypos] - '0') * 1000) + ((v[Ypos+1] - '0') * 100) + ((v[Ypos+2] - '0') * 10) + (v[Ypos+3] - '0')) - 1900;
        tmp.tm_mon = (((v[Mpos] - '0') * 10) + (v[Mpos+1] - '0')) - 1; /* 0 - 11 */
        tmp.tm_mday = (((v[Dpos] - '0') * 10) + (v[Dpos+1] - '0')); /* 1 - 31 */
        tmp.tm_hour = (((v[hpos] - '0') * 10) + (v[hpos+1] - '0')); /* 0 - 23 */
        tmp.tm_min = (((v[mpos] - '0') * 10) + (v[mpos+1] - '0')); /* 0-59 */
        tmp.tm_sec = (((v[spos] - '0') * 10) + (v[spos+1] - '0')); /* 0-59 */

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
Sqlite3_ResultSet::getDouble(const int idx) const
{
    char *pEnd;
    return (strtod((const char *) sqlite3_column_text(res_, idx), &pEnd));
}

float
Sqlite3_ResultSet::getFloat(const int idx) const
{
    return (atof((const char *) sqlite3_column_text(res_, idx)));
}

long
Sqlite3_ResultSet::getLong(const int idx) const
{
    return (atol((const char *) sqlite3_column_text(res_, idx)));
}

short
Sqlite3_ResultSet::getShort(const int idx) const
{
    return ((short) (atoi((const char *) sqlite3_column_text(res_, idx))));
}

void *
Sqlite3_ResultSet::operator new (size_t bytes)
{
  return (::new char[bytes]);
}

void
Sqlite3_ResultSet::operator delete (void *ptr)
{
  delete [] static_cast <char *> (ptr);
}

bool
Sqlite3_Connection::open(const char *database, const char *host, const int port, const char *user, const char *pass)
{
    if (sqlite3_open(database, &db_) != SQLITE_OK) {
        db_ = NULL;
        return (false);
    }
    return (true);
}

bool
Sqlite3_Connection::close(void)
{
    if (!db_) return (false);
    if (sqlite3_close(db_) != SQLITE_OK) {
        return (false);
    }
    db_ = NULL;
    return (true);
}

bool
Sqlite3_Connection::isConnected(void)
{
    if (!db_) return (false);
    return (true);
}

bool
Sqlite3_Connection::execute(const char *sql)
{
    bool ret = false;
    char *err = NULL;

    if (!db_) return (ret);

    if (sqlite3_exec(db_, sql, NULL, NULL, &err) == SQLITE_OK) {
        ret = true;
    }
    if (err) {
        sqlite3_free(err);
    }
    return (ret);
}

dbabstract::ResultSet *
Sqlite3_Connection::executeQuery(const char *sql)
{
    sqlite3_stmt *vm = NULL;
    const char *tail = NULL;

    if (!db_) return (NULL);

    if (sqlite3_prepare(db_, sql, -1, &vm, &tail) != SQLITE_OK) {
        return (0);
    }

    dbabstract::ResultSet *c = 0;
    c = new dbabstract::Sqlite3_ResultSet(vm);
    return (c);
}

char *
Sqlite3_Connection::escape(const char *str)
{
    /* this is overly complex so that valgrind doesn't report a mismatched
     * delete, free, delete[].
     */
    char *esc = sqlite3_mprintf("\'%q\'", str);
    char *ret = new char[strlen(esc) + 1];
    strcpy(ret, esc);
    sqlite3_free(esc);
    return (ret);

}

const char *
Sqlite3_Connection::unixtimeToSql(const time_t val)
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
Sqlite3_Connection::insertId(void)
{
    return ((const unsigned long) sqlite3_last_insert_rowid(db_));
}

bool
Sqlite3_Connection::beginTrans(void)
{
    if (!db_) return (false);
    return (execute("BEGIN TRANSACTION"));
}

bool
Sqlite3_Connection::commitTrans(void)
{
    if (!db_) return (false);
    return (execute("COMMIT TRANSACTION"));
}

bool
Sqlite3_Connection::rollbackTrans(void)
{
    if (!db_) return (false);
    return (execute("ROLLBACK TRANSACTION"));
}

bool
Sqlite3_Connection::setTransactionMode(const enum TRANS_MODE mode)
{
    const char *sql = "";

    switch (mode) {
    case READ_UNCOMMITTED:
    case READ_COMMITTED:
    case REPEATABLE_READ:
        sql = "PRAGMA read_uncommitted = true";
        break;
    case SERIALIZABLE:
        sql = "PRAGMA read_uncommitted = false";
        break;
    }
    return (execute(sql));
}

unsigned int
Sqlite3_Connection::errorno(void) const
{
    return ((unsigned int) sqlite3_errcode(db_));
}

const char *
Sqlite3_Connection::errormsg(void) const
{
    return (sqlite3_errmsg(db_));
}

std::vector<std::string>
Sqlite3_Connection::tables(void) const
{
    std::vector<std::string> vTables;
    std::stringstream ss;
    ss << "SELECT * FROM sqlite_master WHERE type='table'";
    ResultSet *rs = ((Connection*)this)->executeQuery(ss.str().c_str());
    if (rs) {
        while (rs->next()) {
            vTables.push_back(rs->getString(2));
        }
        rs->close();
    }
    return vTables;
}

const char *
Sqlite3_Connection::version(void) const
{
    static char ret[256];
    snprintf(ret, 256, "Sqlite3 Driver v0.2 using %s", sqlite3_libversion());
    return ((const char *) ret);
}

void *
Sqlite3_Connection::operator new (size_t bytes)
{
  return (::new char[bytes]);
}

void
Sqlite3_Connection::operator delete (void *ptr)
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
  dbabstract::Sqlite3_Connection *c = 0;
  c = new dbabstract::Sqlite3_Connection;
  return (c);
}
#else

extern "C" LIBRARY_API Connection *create_sqlite3_connection (void);

dbabstract::Connection *
create_sqlite3_connection (void)
{
  dbabstract::Sqlite3_Connection *c = 0;
  c = new dbabstract::Sqlite3_Connection;
  return (c);
}

#endif

}


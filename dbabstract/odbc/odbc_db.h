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
#include <string>
#include <vector>

#include "dbabstract/db.h"
#include "sql.h"
#include "sqlext.h"
#include "sqlucode.h"
//#include "iodbcext.h"

namespace dbabstract
{
    class ODBC_Connection;

    class ODBC_ResultSet : public ResultSet
    {
        friend class ODBC_Connection;
    protected:
        ODBC_ResultSet(HSTMT stmt) : hstmt(stmt), record(0) {};
        ~ODBC_ResultSet();
    private:
        ODBC_ResultSet() {};
        ODBC_ResultSet(const ODBC_ResultSet &old);
        const ODBC_ResultSet &operator=(const ODBC_ResultSet &old);

    public:
        void *handle(void) { return hstmt; }

        bool close(void);
        bool next(void);

        unsigned long recordCount(void) const;
        unsigned int findColumn(const char *field) const;

        const char *getString(const int idx) const;
        int getInteger(const int idx) const;
        bool getBool(const int idx) const;
        time_t getUnixTime(const int idx) const;
        double getDouble(const int idx) const;
        float getFloat(const int idx) const;
        long getLong(const int idx) const;
        short getShort(const int idx) const;

        // Overload the new/delete opertors so the object will be
        // created/deleted using the memory allocator associated with the
        // DLL/SO.
        void *operator new (size_t bytes);
        void operator delete (void *ptr);

    private:
        HSTMT hstmt;
        int record;
    };

    class ODBC_Connection : public Connection
    {
    private:
        ODBC_Connection(const ODBC_Connection &old);
        const ODBC_Connection &operator=(const ODBC_Connection &old);

    public:
        ODBC_Connection()
            : henv(0)
            , hdbc(0)
            , hstmt(0)
            , connected(0) {}
        ~ODBC_Connection() { close(); }

        void *handle(void) { return hstmt; }
        std::vector<std::string> tables(void) const;
        bool open(const char *database, const char *host, const int port, const char *user, const char *pass);
        bool close(void);
        bool isConnected(void);
        bool execute(const char *sql);
        ResultSet *executeQuery(const char *sql);
        char *escape(const char *);
        const char *unixtimeToSql(const time_t);
        unsigned long insertId(void);

        bool beginTrans(void);
        bool commitTrans(void);
        bool rollbackTrans(void);
        bool setTransactionMode(const enum TRANS_MODE mode);

        unsigned int errorno(void) const;
        const char *errormsg(void) const;

        const char *version(void) const;

        // Overload the new/delete opertors so the object will be
        // created/deleted using the memory allocator associated with the
        // DLL/SO.
        void *operator new (size_t bytes);
        void operator delete (void *ptr);
        int ODBC_Errors (char *where) const;

    private:
        HENV henv;
        HDBC hdbc;
        HSTMT hstmt;
        int connected;
    };
}

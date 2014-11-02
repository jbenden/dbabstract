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

#include "db.h"

#include "sqlite3.h"

namespace DB
{
    class Sqlite3_Connection;

    class Sqlite3_ResultSet : public ResultSet
    {
        friend class Sqlite3_Connection;
    protected:
        Sqlite3_ResultSet(sqlite3_stmt *res) : res_(res) {};
        ~Sqlite3_ResultSet();
    private:
        Sqlite3_ResultSet() {};
        Sqlite3_ResultSet(const Sqlite3_ResultSet &old);
        const Sqlite3_ResultSet &operator=(const Sqlite3_ResultSet &old);

    public:
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
        sqlite3_stmt *res_;
    };

    class Sqlite3_Connection : public Connection
    {
    private:
        Sqlite3_Connection(const Sqlite3_Connection &old);
        const Sqlite3_Connection &operator=(const Sqlite3_Connection &old);

    public:
        Sqlite3_Connection() : db_(NULL) {};
        ~Sqlite3_Connection() { close(); }

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

    private:
        sqlite3 *db_;
        int errorno_;
        std::string errormsg_;
    };
}

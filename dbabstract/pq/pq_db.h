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
#include <string>

#include "db.h"
#include <pg_config.h>
#include <libpq-fe.h>

namespace DB
{
    class PQ_Connection;

class PQ_ResultSet : public ResultSet
    {
        friend class PQ_Connection;
    protected:
        PQ_ResultSet(PGresult *res) : res_(res), row_(-1) {};
        ~PQ_ResultSet();
    private:
        PQ_ResultSet() {};
        PQ_ResultSet(const PQ_ResultSet &old);
        const PQ_ResultSet &operator=(const PQ_ResultSet &old);

    public:
        bool close(void);
        bool next(void);

        unsigned long recordCount(void) const;
        unsigned int findColumn(const char *field) const;

        const char *getString(const int idx) const;
        const int getInteger(const int idx) const;
        const bool getBool(const int idx) const;
        const time_t getUnixTime(const int idx) const;
        const double getDouble(const int idx) const;
        const float getFloat(const int idx) const;
        const long getLong(const int idx) const;
        const short getShort(const int idx) const;

        // Overload the new/delete opertors so the object will be
        // created/deleted using the memory allocator associated with the
        // DLL/SO.
        void *operator new (size_t bytes);
        void operator delete (void *ptr);

    private:
        PGresult *res_;
        unsigned long row_;
    };

    class PQ_Connection : public Connection
    {
    private:
        PQ_Connection(const PQ_Connection &old);
        const PQ_Connection &operator=(const PQ_Connection &old);

    public:
        PQ_Connection() : pgconn_(NULL) {};
        ~PQ_Connection() { close(); }

        bool open(const char *database, const char *host, const int port, const char *user, const char *pass);
        bool close(void);
        bool isConnected(void);
        bool execute(const char *sql);
        ResultSet *executeQuery(const char *sql);
        char *escape(const char *);
        const char *unixtimeToSql(const time_t);
        const unsigned long insertId(void);

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
        PGconn *pgconn_;
        std::string database_;
    };
}

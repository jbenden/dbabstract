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
#ifndef _DB_H
#define _DB_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

#include <Poco/SharedLibrary.h>

#if defined(_WIN32)
# define LIBRARY_API __declspec(dllexport)
#else
# define LIBRARY_API
#endif

namespace dbabstract
{
    /**
     * A ResultSet object is not updatable and has a cursor that
     * moves forward only. Thus, you can iterate through it only
     * once and only from the first row to the last row.
     *
     * The ResultSet interface provides getter methods for
     * retrieving column values from the current row. Values
     * can be retrieved using the index number of the column.
     * Columns are numbered from zero (0).
     *
     * For the getter methods, a given driver attempts to convert
     * the underlying data to the C++ type specified in the getter
     * method and returns a suitable value.
     *
     * A ResultSet object MUST be closed when done, which will
     * automatically handle freeing the memory associated with
     * the object.
     *
     * NOTE: Only getString will return NULL if the database had
     *       a NULL value stored, the rest will return ZERO!
     */
    class ResultSet
    {
    public:
        virtual ~ResultSet(void) {};

        virtual bool close(void) = 0;
        virtual bool next(void) = 0;

        virtual unsigned int findColumn(const char *field) const = 0;
        virtual unsigned long recordCount(void) const = 0;

        virtual const char *getString(const int idx) const = 0;
        virtual int getInteger(const int idx) const = 0;
        virtual bool getBool(const int idx) const = 0;
        virtual time_t getUnixTime(const int idx) const = 0;
        virtual double getDouble(const int idx) const = 0;
        virtual float getFloat(const int idx) const = 0;
        virtual long getLong(const int idx) const = 0;
        virtual short getShort(const int idx) const = 0;
    };

    /**
     * A Connection object is the base layer of database abstraction
     * and are created by using the factory interface. When the
     * Connection object returned from the factory is deleted, the
     * class handles closing the database connection, although the
     * user may do so if desired.
     *
     * The classes are partially thread-safe in that a Connection
     * object may be used in a single thread, but many Connection
     * objects may be created in different threads.  This is due to
     * most database implementations following this same style of
     * interface.  For best portability, one should follow this same
     * style of usage.
     */
    class Connection
    {
    public:
        Connection(void) : ref(1) {};
        virtual ~Connection(void) {};

        /**
         * Open a connection to a database.
         *
         * @param database Database name. Sqlite3 uses this for the
         *                 filename.
         * @param host Hostname
         * @param port Port
         * @param user Username
         * @param pass Password
         *
         * @return bool True if successful.
         */
        virtual bool open(const char *database, const char *host, const int port, const char *user, const char *pass) = 0;

        /**
         * Close the database connection
         *
         * @return bool True if successful.
         */
        virtual bool close(void) = 0;

        /**
         * Attempts to tell if we are still connected to the database.
         *
         * @return bool
         */
        virtual bool isConnected(void) = 0;

        /**
         * Executes a query to the database, discarding any result
         * data.  This function is typically used for non-SELECT
         * statements.
         *
         * @param sql
         *
         * @return bool
         */
        virtual bool execute(const char *sql) = 0;

        /**
         * Executes a query to the database which returns row data. The
         * row data is manipulated using the ResultSet object returned
         * by this method.  If the database wasn't able to return row
         * data or an error occurs, then zero is returned.  Therefore be
         * careful to check the returned value, before using it - or a
         * seg. fault is possible!
         *
         * @param sql
         *
         * @return ResultSet*
         */
        virtual ResultSet *executeQuery(const char *sql) = 0;

        /**
         * Returns a database specific escaped string from the input.
         * The string returned must be freed by the caller.  This is
         * used by the Query class.
         *
         * @return char*
         */
        virtual char *escape(const char *) = 0;

        /**
         * Returns a database specific representation of the time_t
         * value. This is used by the Query class. The string returned
         * must be freed by the caller.
         *
         * @param time_t
         *
         * @return const char*
         */
        virtual const char *unixtimeToSql(const time_t) = 0;

        /**
         * Returns the last SQL INSERT unique identifier if the
         * underlying database supports this feature.
         *
         * @return const unsigned long
         */
        virtual unsigned long insertId(void) = 0;

        /**
         * Begins a new transaction. If for some reason it fails, false
         * will be returned (for instance, if the underlying driver does
         * not support transactions.)
         *
         * @return bool
         */
        virtual bool beginTrans(void) = 0;

        /**
         * Attempts to commit the current transaction
         *
         * @return bool
         */
        virtual bool commitTrans(void) = 0;

        /**
         * Attempts to rollback the current transaction
         *
         * @return bool
         */
        virtual bool rollbackTrans(void) = 0;

        enum TRANS_MODE {
            READ_UNCOMMITTED, /** allows dirty reads, but fastest */
            READ_COMMITTED, /** default postgres, mssql, and oci8 */
            REPEATABLE_READ, /** default mysql */
            SERIALIZABLE /** slowest and most restrictive */
        };
        /**
         * Sets the database transaction mode.
         *
         * @param mode
         *
         * @return bool True or False if it succeeded.
         */
        virtual bool setTransactionMode(const enum TRANS_MODE mode) = 0;

        /**
         * Returns the last error code to occur.
         *
         * @return unsigned int
         */
        virtual unsigned int errorno(void) const = 0;

        /**
         * Returns a textual representation of the last error to occur.
         * The value returned should NOT be freed.
         *
         * @return const char*
         */
        virtual const char *errormsg(void) const = 0;

        /**
         * Returns the DB abstraction module's version number and
         * possibly the version of the underlying database client
         * library used.  The returned value should NOT be freed.
         *
         * @return const char*
         */
        virtual const char *version(void) const = 0;

#ifndef STATIC

        typedef Connection* (*Connection_Creator) (void);

        /**
         * Factory method for creating a new Connection object
         * for a database connection. This method shields the
         * logic for DLL manipulation from the caller.
         *
         * Warning: The returned Connection pointer may be NULL
         * if an error condition occurred!
         *
         * @param db_dll_name The database type to create
         *
         * @return Connection* Pointer to new Connection object
         */
        static Connection *
        factory(const char *db_dll_name)
        {
            Poco::SharedLibrary dll(db_dll_name);

            Connection_Creator cc;
            try {
                void *void_ptr = dll.getSymbol("create_connection");
                cc = reinterpret_cast<Connection_Creator>(void_ptr);
                if (!cc) {
                    std::cerr << "Shared Library contains the symbol interface for db-abstract, but returned NULL for some reason." << std::endl;
                    return (NULL);
                }
                return (cc());
            } catch (Poco::NotFoundException &ex) {
                std::cerr << "Shared Library does not contain the appropriate interface for db-abstract." << std::endl;
                return (NULL);
            }
        }

#endif

        void duplicate() {
            ref++;
        }

        void release() {
            ref--;
            if (ref == 0) {
                delete this;
            }
        }

        long ref;
    };

    inline std::ostream &unixtime_impl(std::ostream &Out, Connection& conn_, const time_t ut)
    {
        const char *buf = conn_.unixtimeToSql(ut);
        Out << buf;
        delete [] buf;
        return (Out);
    }

    inline std::ostream &qstr_impl(std::ostream &Out, Connection& conn_, const char *str)
    {
        const char *buf = conn_.escape(str);
        Out << buf;
        delete [] buf;
        return (Out);
    }

    /**
     * Stream manipulator to convert a time_t epoct timestamp
     * into a suitable date and time field according to the
     * underlying database type.
     *
     * Based on the Effector pattern
     */
    class unixtime {
        time_t t;
        Connection& conn;
    public:
        unixtime(Connection& con, time_t ti) : t(ti), conn(con) {}
        friend std::ostream& operator<<(std::ostream& Out,  const unixtime& ref) {
            return unixtime_impl(Out, ref.conn, ref.t);
        }
    };


    /**
     * Stream manipulator to escape a string according to the
     * underlying database type.
     *
     * Based on the Effector pattern
     */
    class qstr {
        std::string s;
        Connection& conn;
    public:
        qstr(Connection& con, const char *str) : s(str), conn(con) {}
        qstr(Connection& con, const std::string& str) : s(str), conn(con) {}
        friend std::ostream& operator<<(std::ostream& Out, const qstr& q) {
                return qstr_impl(Out, q.conn, q.s.c_str());
        }
    };
}; /* namespace */

#endif


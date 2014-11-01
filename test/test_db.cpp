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
#include "db.h"

#include <Poco/SharedLibrary.h>
#include <Poco/AutoPtr.h>

#include <iostream>
#include <string>

using namespace DB;

typedef Connection* (*Connection_Creator) (void);

int enable_static = 0;

extern "C" Connection * create_mysql_connection(void);
extern "C" Connection * create_sqlite3_connection(void);
extern "C" Connection * create_pq_connection(void);

int
main (int argc, const char * const argv[])
{
    int result = 0;

    if (argc > 1) {
        enable_static = 1;
        printf("Using static libraries.\n");
    }

#ifdef ENABLE_MYSQL

  {
      Poco::AutoPtr <Connection> connection;
   if (!enable_static) {
      std::string path(LIBPATH "/libmysql_dba");
#ifndef __APPLE__
      path.append(Poco::SharedLibrary::suffix());
#else
      path.append(".so");
#endif
      connection = Connection::factory(path.c_str());
    } else {
        connection = create_mysql_connection();
    }
    std::cout << "OK: Reported version: " << connection->version () << std::endl;

    if (connection->open("test", "127.0.0.1", 3306, "root", "")) {
        std::cout << "OK: Connected to database." << std::endl;

        time_t now = time(NULL);
        Query q((*connection));
        q << "SELECT id, data FROM testing WHERE data=" << qstr("ben'den") << " and 1=1 and added=" << unixtime(now);
        std::cout << "SQL = " << q.str() << std::endl;

        const char *s1 = "CREATE TABLE testing (id int unsigned not null auto_increment primary key, data varchar(255) not null, added datetime, cost decimal(10,2)) ENGINE=MyISAM";
        if (connection->execute(s1)) {
            std::cout << "OK: Created table testing" << std::endl;

            ResultSet *rs3 = connection->executeQuery("SELECT id, data FROM testing");
            if (rs3) {
                while (rs3->next()) {
                    std::cout << "READ: id=" << rs3->getInteger(0) << " data=" << rs3->getString(1) << std::endl;
                }
                rs3->close();
            }
            std::cout << "OK: Read nothing from table" << std::endl;

            const char *s2 = "INSERT INTO testing SET data='joe', added=now(), cost=1.99";
            if (!connection->execute(s2)) {
                result = 1;
                std::cout << "FAILED: Insert 1 failed" << std::endl;
            }

            ResultSet *rs = connection->executeQuery("SELECT id, data, added, cost FROM testing");
            if (rs) {
                while (rs->next()) {
                    std::cout << "READ: id=" << rs->getInteger(0) << " data=" << rs->getString(1) << " added=" << rs->getUnixTime(2) << " cost=" << rs->getDouble(3) << std::endl;
                    time_t added = rs->getUnixTime(2);
                    std::cout << "READ: added formatted time: " << ctime(&added) << std::endl;
                }
                rs->close();
            }
            std::cout << "OK: Single record" << std::endl;

            const char *s3 = "INSERT INTO testing SET data='benden'";
            if (!connection->execute(s3)) {
                result = 1;
                std::cout << "FAILED: Insert 2 failed" << std::endl;
            }

            ResultSet *rs1 = connection->executeQuery("SELECT id, data FROM testing");
            if (rs1) {
                while (rs1->next()) {
                    std::cout << "READ: id=" << rs1->getInteger(0) << " data=" << rs1->getString(1) << std::endl;
                }
                rs1->close();
            }
            std::cout << "OK: Two records" << std::endl;

            const char *e1 = "DROP TABLE testing";
            if (connection->execute(e1)) {
                std::cout << "OK: Dropped table testing" << std::endl;
            } else {
                result = 1;
                std::cout << "FAILED: Could not drop table testing" << std::endl;
            }
        } else {
            result = 1;
            std::cout << "FAILED: Creating table testing" << std::endl;
        }
    } else {
        result = 1;
        std::cout << "FAILED: Could not connect to database." << std::endl;
    }
  }

#endif


#ifdef ENABLE_SQLITE3
  {
      std::string path(LIBPATH "/libsqlite3_dba");
#ifndef __APPLE__
      path.append(Poco::SharedLibrary::suffix());
#else
      path.append(".so");
#endif
      Poco::AutoPtr <Connection> connection;
      if (!enable_static) {
          connection = Connection::factory(path.c_str());
      } else {
          connection = create_sqlite3_connection();
      }

    std::cout << "OK: Reported version: " << connection->version () << std::endl;

    if (connection->open("sqlite3_test", NULL, 0, NULL, NULL)) {
        std::cout << "OK: Connected to database." << std::endl;

        time_t now = time(NULL);
        Query q((*connection));
        q << "SELECT id, data FROM testing WHERE data=" << qstr("ben'den") << " and 1=1 and added=" << unixtime(now);
        std::cout << "SQL = " << q.str() << std::endl;

        const char *s1 = "CREATE TABLE testing (id integer not null primary key autoincrement, data text not null, added integer, cost real)";
        if (connection->execute(s1)) {
            std::cout << "OK: Created table testing" << std::endl;

            ResultSet *rs3 = connection->executeQuery("SELECT id, data FROM testing");
            if (rs3) {
                while (rs3->next()) {
                    std::cout << "READ: id=" << rs3->getInteger(0) << " data=" << rs3->getString(1) << std::endl;
                }
                rs3->close();
            }
            std::cout << "OK: Read nothing from table" << std::endl;

            const char *s2 = "INSERT INTO testing VALUES (NULL,'joe',datetime('now'),1.99)";
            if (!connection->execute(s2)) {
                result = 1;
                std::cout << "FAILED: Insert 1 failed" << std::endl;
            }

            ResultSet *rs = connection->executeQuery("SELECT id, data, added, cost FROM testing");
            if (rs) {
                while (rs->next()) {
                    std::cout << "READ: id=" << rs->getInteger(0) << " data=" << rs->getString(1) << " added=" << rs->getUnixTime(2) << " cost=" << rs->getDouble(3) << std::endl;
                    time_t added = rs->getUnixTime(2);
                    std::cout << "READ: added formatted time: " << ctime(&added) << std::endl;
                }
                rs->close();
            }
            std::cout << "OK: Single record" << std::endl;

            const char *s3 = "INSERT INTO testing VALUES (NULL,'benden',0,0)";
            if (!connection->execute(s3)) {
                result = 1;
                std::cout << "FAILED: Insert 2 failed" << std::endl;
            }

            ResultSet *rs1 = connection->executeQuery("SELECT id, data FROM testing");
            if (rs1) {
                while (rs1->next()) {
                    std::cout << "READ: id=" << rs1->getInteger(0) << " data=" << rs1->getString(1) << std::endl;
                }
                rs1->close();
            }
            std::cout << "OK: Two records" << std::endl;

            const char *e1 = "DROP TABLE testing";
            if (connection->execute(e1)) {
                std::cout << "OK: Dropped table testing" << std::endl;
            } else {
                result = 1;
                std::cout << "FAILED: Could not drop table testing" << std::endl;
            }
        } else {
            result = 1;
            std::cout << "FAILED: Creating table testing" << std::endl;
        }
    } else {
        result = 1;
        std::cout << "FAILED: Could not connect to database." << std::endl;
    }
  }

#endif

#ifdef ENABLE_PQ
  {
      std::string path(LIBPATH "/libpq_dba");
#ifndef __APPLE__
      path.append(Poco::SharedLibrary::suffix());
#else
      path.append(".so");
#endif
      Poco::AutoPtr <Connection> connection;
     
      if (!enable_static) {
          connection = Connection::factory(path.c_str());
      } else {
          connection = create_pq_connection();
      }

    std::cout << "OK: Reported version: " << connection->version () << std::endl;

    if (connection->open("dbname = postgres", NULL, 0, NULL, NULL)) {
        std::cout << "OK: Connected to database." << std::endl;

        time_t now = time(NULL);
        Query q((*connection));
        q << "SELECT id, data FROM testing WHERE data=" << qstr("ben'den") << " and 1=1 and added=" << unixtime(now);
        std::cout << "SQL = " << q.str() << std::endl;

        const char *s1 = "CREATE TABLE testing (id serial, data text not null, added timestamp, cost real)";
        if (connection->execute(s1)) {
            std::cout << "OK: Created table testing" << std::endl;

            ResultSet *rs3 = connection->executeQuery("SELECT id, data FROM testing");
            if (rs3) {
                while (rs3->next()) {
                    std::cout << "READ: id=" << rs3->getInteger(0) << " data=" << rs3->getString(1) << std::endl;
                }
                rs3->close();
            }
            std::cout << "OK: Read nothing from table" << std::endl;

            char s2[512];
            sprintf(s2, "INSERT INTO testing (data,added,cost) VALUES ('joe',NOW(),1.99)");
            if (!connection->execute(s2)) {
                result = 1;
                std::cout << "FAILED: Insert 1 failed" << std::endl;
            }

            ResultSet *rs = connection->executeQuery("SELECT id, data, added, cost FROM testing");
            if (rs) {
                while (rs->next()) {
                    std::cout << "READ: id=" << rs->getInteger(0) << " data=" << rs->getString(1) << " added=" << rs->getUnixTime(2) << " cost=" << rs->getDouble(3) << std::endl;
                    time_t added = rs->getUnixTime(2);
                    std::cout << "READ: added formatted time: " << ctime(&added) << std::endl;
                }
                rs->close();
            }
            std::cout << "OK: Single record" << std::endl;

            const char *s3 = "INSERT INTO testing (data,added,cost) VALUES ('benden',NOW(),0)";
            if (!connection->execute(s3)) {
                result = 1;
                std::cout << "FAILED: Insert 2 failed" << std::endl;
            }

            ResultSet *rs1 = connection->executeQuery("SELECT id, data FROM testing");
            if (rs1) {
                while (rs1->next()) {
                    std::cout << "READ: id=" << rs1->getInteger(0) << " data=" << rs1->getString(1) << std::endl;
                }
                rs1->close();
            }
            std::cout << "OK: Two records" << std::endl;

            const char *e1 = "DROP TABLE testing";
            if (connection->execute(e1)) {
                std::cout << "OK: Dropped table testing" << std::endl;
            } else {
                result = 1;
                std::cout << "FAILED: Could not drop table testing" << std::endl;
            }
        } else {
            result = 1;
            std::cout << "FAILED: Creating table testing" << std::endl;
        }
    } else {
        result = 1;
        std::cout << "FAILED: Could not connect to PostgreSQL database." << std::endl;
    }
  }

#endif

  return (result);
}


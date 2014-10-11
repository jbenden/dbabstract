/*                                                              
 * A database abstraction layer for C++ and ACE framework
 * 
 * (C) 2006-2007 Thralling Penguin LLC. All rights reserved.
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
#include "ace/DLL.h"
#include "ace/Auto_Ptr.h"
#include "ace/Log_Msg.h"

using namespace DB;

typedef Connection* (*Connection_Creator) (void);

int
ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
  ACE_UNUSED_ARG (argc);
  ACE_UNUSED_ARG (argv);

#ifdef ENABLE_MYSQL

  {
    auto_ptr <Connection> connection (Connection::factory(LIBPATH "/libdba_mysql"));

    ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Reported version: %s\n"),connection->version ()));

    if (connection->open("sqlite3_test", "localhost", 3306, "root", "")) {
        ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Connected to database.\n")));

        time_t now = time(NULL);
        Query q((*connection));
        q << "SELECT id, data FROM testing WHERE data=" << qstr("ben'den") << " and 1=1 and added=" << unixtime(now);
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("SQL = %s\n"), q.str()));

        const char *s1 = "CREATE TABLE testing (id int unsigned not null auto_increment primary key, data varchar(255) not null, added datetime, cost decimal(10,2)) ENGINE=MyISAM";
        if (connection->execute(s1)) {
            ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Created table testing\n")));

            ResultSet *rs3 = connection->executeQuery("SELECT id, data FROM testing");
            if (rs3) {
                while (rs3->next()) {
                    ACE_DEBUG((LM_ERROR,ACE_TEXT("READ: id=%d data=%s\n"), rs3->getInteger(0), rs3->getString(1)));
                }
                rs3->close();
            }
            ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Read nothing from table\n")));

            const char *s2 = "INSERT INTO testing SET data='joe', added=now(), cost=1.99";
            if (!connection->execute(s2)) {
                ACE_DEBUG((LM_ERROR,ACE_TEXT("FAILED: Insert 1 failed\n")));
            }

            ResultSet *rs = connection->executeQuery("SELECT id, data, added, cost FROM testing");
            if (rs) {
                while (rs->next()) {
                    ACE_DEBUG((LM_ERROR,ACE_TEXT("READ: id=%d data=%s added=%d cost=%f\n"), rs->getInteger(0), rs->getString(1), rs->getUnixTime(2), rs->getDouble(3)));
                    time_t added = rs->getUnixTime(2);
                    ACE_DEBUG((LM_ERROR,ACE_TEXT("READ: added formatted time: %s"), ctime(&added)));
                }
                rs->close();
            }
            ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Single record\n")));

            const char *s3 = "INSERT INTO testing SET data='benden'";
            if (!connection->execute(s3)) {
                ACE_DEBUG((LM_ERROR,ACE_TEXT("FAILED: Insert 2 failed\n")));
            }

            ResultSet *rs1 = connection->executeQuery("SELECT id, data FROM testing");
            if (rs1) {
                while (rs1->next()) {
                    ACE_DEBUG((LM_ERROR,ACE_TEXT("READ: id=%d data=%s\n"), rs1->getInteger(0), rs1->getString(1)));
                }
                rs1->close();
            }
            ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Two records\n")));

            const char *e1 = "DROP TABLE testing";
            if (connection->execute(e1)) {
                ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Dropped table testing\n")));
            } else {
                ACE_DEBUG((LM_ERROR,ACE_TEXT("FAILED: Could not drop table testing\n")));
            }
        } else {
            ACE_DEBUG((LM_ERROR,ACE_TEXT("FAILED: Creating table testing\n")));
        }
    } else {
        ACE_DEBUG((LM_ERROR,ACE_TEXT("FAILED: Could not connect to database.\n")));
    }
  }

#endif


#ifdef ENABLE_SQLITE3
  {
    auto_ptr <Connection> connection (Connection::factory(LIBPATH "/libdba_sqlite3"));

    ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Reported version: %s\n"),connection->version ()));

    if (connection->open("sqlite3_test", NULL, 0, NULL, NULL)) {
        ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Connected to database.\n")));

        time_t now = time(NULL);
        Query q((*connection));
        q << "SELECT id, data FROM testing WHERE data=" << qstr("ben'den") << " and 1=1 and added=" << unixtime(now);
        ACE_DEBUG((LM_DEBUG, ACE_TEXT("SQL = %s\n"), q.str()));

        const char *s1 = "CREATE TABLE testing (id integer not null primary key autoincrement, data text not null, added integer, cost real)";
        if (connection->execute(s1)) {
            ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Created table testing\n")));

            ResultSet *rs3 = connection->executeQuery("SELECT id, data FROM testing");
            if (rs3) {
                while (rs3->next()) {
                    ACE_DEBUG((LM_ERROR,ACE_TEXT("READ: id=%d data=%s\n"), rs3->getInteger(0), rs3->getString(1)));
                }
                rs3->close();
            }
            ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Read nothing from table\n")));

            const char *s2 = "INSERT INTO testing VALUES (NULL,'joe',datetime('now'),1.99)";
            if (!connection->execute(s2)) {
                ACE_DEBUG((LM_ERROR,ACE_TEXT("FAILED: Insert 1 failed\n")));
            }

            ResultSet *rs = connection->executeQuery("SELECT id, data, added, cost FROM testing");
            if (rs) {
                while (rs->next()) {
                    ACE_DEBUG((LM_ERROR,ACE_TEXT("READ: id=%d data=%s added=%d cost=%f\n"), rs->getInteger(0), rs->getString(1), rs->getUnixTime(2), rs->getDouble(3)));
                    time_t added = rs->getUnixTime(2);
                    ACE_DEBUG((LM_ERROR,ACE_TEXT("READ: added formatted time: %s"), ctime(&added)));
                }
                rs->close();
            }
            ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Single record\n")));

            const char *s3 = "INSERT INTO testing VALUES (NULL,'benden',0,0)";
            if (!connection->execute(s3)) {
                ACE_DEBUG((LM_ERROR,ACE_TEXT("FAILED: Insert 2 failed: %s\n"), connection->errormsg()));
            }

            ResultSet *rs1 = connection->executeQuery("SELECT id, data FROM testing");
            if (rs1) {
                while (rs1->next()) {
                    ACE_DEBUG((LM_ERROR,ACE_TEXT("READ: id=%d data=%s\n"), rs1->getInteger(0), rs1->getString(1)));
                }
                rs1->close();
            }
            ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Two records\n")));

            const char *e1 = "DROP TABLE testing";
            if (connection->execute(e1)) {
                ACE_DEBUG((LM_ERROR,ACE_TEXT("OK: Dropped table testing\n")));
            } else {
                ACE_DEBUG((LM_ERROR,ACE_TEXT("FAILED: Could not drop table testing\n")));
            }
        } else {
            ACE_DEBUG((LM_ERROR,ACE_TEXT("FAILED: Creating table testing\n")));
        }
    } else {
        ACE_DEBUG((LM_ERROR,ACE_TEXT("FAILED: Could not connect to database.\n")));
    }
#endif
  }

  return 0;
}


#include <Poco/AutoPtr.h>

#include <gtest/gtest.h>
#include <exception>
#include <iostream>
#include <string>
#include <strstream>

#include "dbabstract/db.h"

#ifdef ENABLE_ODBC

extern "C" {
    extern dbabstract::Connection *create_odbc_connection(void);
};

class ODBCDefaultTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            connection = create_odbc_connection();
            connection->open("DSN=test_db", "127.0.0.1", 3306, "root", "");
            connection->execute("set time_zone='+0:00'");
        }

        virtual void TearDown() {
            connection->release();
        }

        Poco::AutoPtr <dbabstract::Connection> connection;
};

TEST_F(ODBCDefaultTest, CanConnectToDatabase) {
    EXPECT_EQ(connection->isConnected(), true);
}

TEST_F(ODBCDefaultTest, CanExecuteSimpleQuery) {
    EXPECT_EQ(connection->execute("CREATE TABLE testing (id INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, text VARCHAR(128), num INT, fl FLOAT, createdOn TIMESTAMP, updatedOn TIMESTAMP) ENGINE=InnoDB"), true);
    EXPECT_EQ(connection->execute("DROP TABLE testing"), true);
}

TEST_F(ODBCDefaultTest, CannotExecuteSimpleQuery) {
    EXPECT_EQ(connection->execute("BYE"), false);
}

TEST_F(ODBCDefaultTest, EscapeCharacters) {
    EXPECT_STREQ(connection->escape("be'nden"), "'be''nden'");
}

TEST_F(ODBCDefaultTest, UnixTimeToSQL) {
    const char *t = connection->unixtimeToSql((time_t) 1414965631);
    EXPECT_STREQ(t, "'2014-11-02 22:00:31'");
}

TEST_F(ODBCDefaultTest, ErrorNumberAndMessage) {
    EXPECT_EQ(connection->errorno(), 0);
    EXPECT_STREQ(connection->errormsg(), NULL);
}

TEST_F(ODBCDefaultTest, VersionString) {
    const char *v = connection->version();
    bool correct = false;
    if (strstr(v, "ODBC Driver v0.1")) {
        correct = true;
    }
    EXPECT_EQ(correct, true);
}

class ODBCTransactionTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            connection = create_odbc_connection();
            connection->open("DSN=test_db", "127.0.0.1", 3306, "root", "");
            connection->execute("set time_zone='+0:00'");
            connection->execute("SET AUTOCOMMIT=false");
            connection->execute("CREATE TABLE testing (id INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, text VARCHAR(128), num INT, fl FLOAT, createdOn TIMESTAMP DEFAULT CURRENT_TIMESTAMP, updatedOn TIMESTAMP) ENGINE=InnoDB");
            connection->beginTrans();
        }

        virtual void TearDown() {
            connection->commitTrans();
            connection->execute("DROP TABLE testing");
            connection->close();
            connection->release();
        }

        Poco::AutoPtr <dbabstract::Connection> connection;
};

TEST_F(ODBCTransactionTest, SingleInsert) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',42)"), true);
}

TEST_F(ODBCTransactionTest, SingleSelect) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',42)"), true);
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',42)"), true);
    EXPECT_EQ(connection->commitTrans(), true);

    std::stringstream q;
    q << "SELECT * FROM testing";
    dbabstract::ResultSet *rs = connection->executeQuery(q.str().c_str());
    EXPECT_NE(rs, (dbabstract::ResultSet *) NULL);
    rs->next();
    EXPECT_EQ(rs->findColumn("text"), 2);
    EXPECT_STREQ(rs->getString(2), "benden");
    EXPECT_EQ(rs->findColumn("fl"), 4);
    EXPECT_EQ(rs->findColumn("r"), 7);
    EXPECT_EQ(rs->getInteger(4), 42);
    EXPECT_EQ(rs->getFloat(4), 42.0f);
    EXPECT_EQ(rs->getDouble(4), 42.0F);
    EXPECT_EQ(rs->getLong(4), 42l);
    EXPECT_EQ(rs->getBool(4), false);
    EXPECT_EQ(rs->getShort(4), (short) 42);
    EXPECT_NE(rs->getUnixTime(5), -1);
    EXPECT_EQ(rs->recordCount(), 2);
    rs->close();
}

TEST_F(ODBCTransactionTest, DoubleSelect) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',1)"), true);
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',42)"), true);
    EXPECT_EQ(connection->commitTrans(), true);

    std::stringstream q;
    q << "SELECT * FROM testing;";
    dbabstract::ResultSet *rs = connection->executeQuery(q.str().c_str());
    EXPECT_NE(rs, (dbabstract::ResultSet *) NULL);
    rs->next();
    EXPECT_EQ(rs->findColumn("text"), 2);
    EXPECT_STREQ(rs->getString(2), "benden");
    EXPECT_EQ(rs->findColumn("fl"), 4);
    EXPECT_EQ(rs->findColumn("r"), 7);
    EXPECT_EQ(rs->getInteger(4), 1);
    EXPECT_EQ(rs->getFloat(4), 1.0f);
    EXPECT_EQ(rs->getDouble(4), 1.0F);
    EXPECT_EQ(rs->getLong(4), 1l);
    EXPECT_EQ(rs->getBool(4), true);
    EXPECT_EQ(rs->getShort(4), (short) 1);
    EXPECT_NE(rs->getUnixTime(5), -1);
    EXPECT_EQ(rs->getUnixTime(2), 0);
    rs->close();
}

TEST_F(ODBCTransactionTest, QueryString) {
    connection->setTransactionMode(dbabstract::Connection::READ_UNCOMMITTED);
    connection->setTransactionMode(dbabstract::Connection::READ_COMMITTED);
    connection->setTransactionMode(dbabstract::Connection::REPEATABLE_READ);
    connection->setTransactionMode(dbabstract::Connection::SERIALIZABLE);

    std::stringstream q;
    q << "INSERT INTO testing (text,fl) VALUES (" << dbabstract::qstr(*connection, "benden");
    q << "," << 42.0f << ");";
    EXPECT_EQ(connection->execute(q.str().c_str()), true);
    unsigned long id = connection->insertId();
    EXPECT_EQ(id, 0);
    EXPECT_EQ(connection->commitTrans(), true);
}

TEST_F(ODBCTransactionTest, RollbackTransaction) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text) VALUES ('benden');"), true);
    EXPECT_EQ(connection->rollbackTrans(), true);
}

TEST_F(ODBCTransactionTest, QueryStringTypes) {
    std::stringstream q;

    q << "INSERT INTO test (text,fl,updatedOn) VALUES (";
    q << dbabstract::qstr(*connection, "benden") << "," << 42l << "," << dbabstract::unixtime(*connection, time(NULL)) << ");";
    EXPECT_EQ(strlen(q.str().c_str()), 80);
}

TEST_F(ODBCTransactionTest, QueryStringTypes2) {
    std::stringstream q;
    std::string world;
    world = " World";

    q << "Hello" << world;

    EXPECT_EQ(strlen(q.str().c_str()), 11);
}

TEST_F(ODBCTransactionTest, QueryNumberTypes) {
    std::stringstream q;
    double d = 42.2;
    short s = 42;
    unsigned short su = 42;

    q << 42 << " " << d << " " << s << (const unsigned char *) " ";
    q << 42u << " " << 42ul << " " << su;
}


#endif


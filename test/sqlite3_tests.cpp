#include <Poco/AutoPtr.h>

#include <gtest/gtest.h>
#include <exception>
#include <iostream>
#include <string>
#include <strstream>

#include "db.h"

#ifdef ENABLE_SQLITE3

extern "C" {
    extern dbabstract::Connection *create_sqlite3_connection(void);
};

class SqliteInvalidTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            connection = create_sqlite3_connection();
            connection->open(".", NULL, 0, NULL, NULL);
        }

        virtual void TearDown() {
            connection->release();
        }

        Poco::AutoPtr <dbabstract::Connection> connection;
};

TEST_F(SqliteInvalidTest, CannotConnectToDatabase) {
    ASSERT_EQ(connection->isConnected(), false);
}

class SqliteDefaultTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            connection = create_sqlite3_connection();
            connection->open("test.db", NULL, 0, NULL, NULL);
        }

        virtual void TearDown() {
            connection->release();
        }

        Poco::AutoPtr <dbabstract::Connection> connection;
};

TEST_F(SqliteDefaultTest, CanConnectToDatabase) {
    EXPECT_EQ(connection->isConnected(), true);
}

TEST_F(SqliteDefaultTest, CanExecuteSimpleQuery) {
    EXPECT_EQ(connection->execute("CREATE TABLE testing (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, text VARCHAR(128), num INTEGER, fl FLOAT, createdOn TIMESTAMP, updatedOn TIMESTAMP)"), true);
    EXPECT_EQ(connection->execute("DROP TABLE testing"), true);
}

TEST_F(SqliteDefaultTest, CannotExecuteSimpleQuery) {
    EXPECT_EQ(connection->execute("BYE"), false);
}

TEST_F(SqliteDefaultTest, EscapeCharacters) {
    EXPECT_STREQ(connection->escape("be'nden"), "'be''nden'");
}

TEST_F(SqliteDefaultTest, UnixTimeToSQL) {
    const char *t = connection->unixtimeToSql((time_t) 1414965631);
    EXPECT_STREQ(t, "'2014-11-02 22:00:31'");
}

TEST_F(SqliteDefaultTest, ErrorNumberAndMessage) {
    EXPECT_EQ(connection->errorno(), 0);
    EXPECT_STREQ(connection->errormsg(), "not an error");
}

TEST_F(SqliteDefaultTest, VersionString) {
    const char *v = connection->version();
    bool correct = false;
    if (strstr(v, "Sqlite3 Driver v0.2")) {
        correct = true;
    }
    EXPECT_EQ(correct, true);
}

class SqliteTransactionTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            connection = create_sqlite3_connection();
            connection->open("test.db", NULL, 0, NULL, NULL);
            EXPECT_EQ(connection->execute("CREATE TABLE testing (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, text VARCHAR(128), num INTEGER, fl FLOAT, createdOn TIMESTAMP DEFAULT CURRENT_TIMESTAMP, updatedOn TIMESTAMP)"), true);
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

TEST_F(SqliteTransactionTest, SingleInsert) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',42)"), true);
}

TEST_F(SqliteTransactionTest, SingleSelect) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',42)"), true);
    EXPECT_EQ(connection->commitTrans(), true);

    dbabstract::ResultSet *rs = connection->executeQuery("SELECT * FROM testing;");
    EXPECT_NE(rs, (dbabstract::ResultSet *) NULL);
    if (rs) {
    rs->next();
    EXPECT_EQ(rs->findColumn("text"), 1);
    EXPECT_STREQ(rs->getString(1), "benden");
    EXPECT_EQ(rs->findColumn("fl"), 3);
    EXPECT_EQ(rs->findColumn("r"), 6);
    EXPECT_EQ(rs->getInteger(3), 42);
    EXPECT_EQ(rs->getFloat(3), 42.0f);
    EXPECT_EQ(rs->getDouble(3), 42.0F);
    EXPECT_EQ(rs->getLong(3), 42l);
    EXPECT_EQ(rs->getBool(3), false);
    EXPECT_EQ(rs->getShort(3), (short) 42);
    EXPECT_NE(rs->getUnixTime(4), -1);
    EXPECT_EQ(rs->getUnixTime(5), 0);
    EXPECT_EQ(rs->recordCount(), 0);
    rs->close();
    }
}

TEST_F(SqliteTransactionTest, DoubleSelect) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',1)"), true);
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',1)"), true);
    EXPECT_EQ(connection->commitTrans(), true);

    dbabstract::ResultSet *rs = connection->executeQuery("SELECT * FROM testing");
    EXPECT_NE(rs, (dbabstract::ResultSet *) NULL);
    if (rs) {
    rs->next();
    EXPECT_EQ(rs->findColumn("text"), 1);
    EXPECT_STREQ(rs->getString(1), "benden");
    EXPECT_EQ(rs->findColumn("fl"), 3);
    EXPECT_EQ(rs->findColumn("r"), 6);
    EXPECT_EQ(rs->getInteger(3), 1);
    EXPECT_EQ(rs->getFloat(3), 1.0f);
    EXPECT_EQ(rs->getDouble(3), 1.0F);
    EXPECT_EQ(rs->getLong(3), 1l);
    EXPECT_EQ(rs->getBool(3), true);
    EXPECT_EQ(rs->getShort(3), (short) 1);
    EXPECT_NE(rs->getUnixTime(4), -1);
    EXPECT_EQ(rs->getUnixTime(5), 0);
    rs->close();
    }
}

TEST_F(SqliteTransactionTest, QueryString) {
    EXPECT_EQ(connection->setTransactionMode(dbabstract::Connection::READ_UNCOMMITTED), true);
    EXPECT_EQ(connection->setTransactionMode(dbabstract::Connection::READ_COMMITTED), true);
    EXPECT_EQ(connection->setTransactionMode(dbabstract::Connection::REPEATABLE_READ), true);
    EXPECT_EQ(connection->setTransactionMode(dbabstract::Connection::SERIALIZABLE), true);

    EXPECT_EQ(connection->beginTrans(), false);

    dbabstract::Query q(*connection);
    q << "INSERT INTO testing (text,fl) VALUES (" << dbabstract::qstr("benden");
    q << "," << 42.0f << ");";
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',42.0);"), true);
    std::cout << q.str() << std::endl;
    std::cout << connection->errormsg() << std::endl;
    unsigned long id = connection->insertId();
    EXPECT_EQ(id, 1);
    EXPECT_EQ(connection->commitTrans(), true);
}

TEST_F(SqliteTransactionTest, RollbackTransaction) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text) VALUES ('benden');"), true);
    EXPECT_EQ(connection->rollbackTrans(), true);
}

TEST_F(SqliteTransactionTest, QueryStringTypes) {
    dbabstract::Query q(*connection);

    q << "INSERT INTO test (text,fl,updatedOn) VALUES (";
    q << dbabstract::qstr("benden") << "," << 42l << "," << dbabstract::unixtime(time(NULL)) << ");";
    EXPECT_EQ(strlen(q.str()), 80);
}

TEST_F(SqliteTransactionTest, QueryStringTypes2) {
    dbabstract::Query q(*connection);
    std::stringstream world;
    world << " World" << std::ends;

    q << std::string("Hello") << world;

    EXPECT_EQ(strlen(q.str()), 11);
}

TEST_F(SqliteTransactionTest, QueryNumberTypes) {
    dbabstract::Query q(*connection);
    double d = 42.2;
    short s = 42;
    unsigned short su = 42;

    q << 42 << " " << d << " " << s << (const unsigned char *) " ";
    q << 42u << " " << 42ul << " " << su;
}


#endif


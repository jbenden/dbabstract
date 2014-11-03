#include <Poco/AutoPtr.h>

#include <gtest/gtest.h>
#include <exception>
#include <iostream>
#include <string>
#include <strstream>

#include "db.h"

#ifdef ENABLE_PQ

extern "C" {
    extern DB::Connection *create_pq_connection(void);
};

class PqDefaultTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            connection = create_pq_connection();
            connection->open("host = 127.0.0.1 dbname = postgres", NULL, 0, NULL, NULL);
        }

        virtual void TearDown() {
            connection->release();
        }

        Poco::AutoPtr <DB::Connection> connection;
};

TEST_F(PqDefaultTest, CanConnectToDatabase) {
    EXPECT_EQ(connection->isConnected(), true);
}

TEST_F(PqDefaultTest, CanExecuteSimpleQuery) {
    EXPECT_EQ(connection->execute("CREATE TABLE testing (id SERIAL, text VARCHAR(128), num INTEGER, fl FLOAT, createdOn TIMESTAMP, updatedOn TIMESTAMP)"), true);
    EXPECT_EQ(connection->execute("DROP TABLE testing"), true);
}

TEST_F(PqDefaultTest, CannotExecuteSimpleQuery) {
    EXPECT_EQ(connection->execute("BYE"), false);
}

TEST_F(PqDefaultTest, EscapeCharacters) {
    EXPECT_STREQ(connection->escape("be'nden"), "'be''nden'");
}

TEST_F(PqDefaultTest, UnixTimeToSQL) {
    const char *t = connection->unixtimeToSql((time_t) 1414965631);
    EXPECT_STREQ(t, "'2014-11-02 15:00:31'");
}

TEST_F(PqDefaultTest, ErrorNumberAndMessage) {
    EXPECT_EQ(connection->errorno(), 0);
    EXPECT_STREQ(connection->errormsg(), "");
}

TEST_F(PqDefaultTest, VersionString) {
    const char *v = connection->version();
    bool correct = false;
    if (strstr(v, "PostgreSQL Driver v0.1")) {
        correct = true;
    }
    EXPECT_EQ(correct, true);
}

class PqTransactionTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            connection = create_pq_connection();
            connection->open("host = 127.0.0.1 dbname = postgres", NULL, 0, NULL, NULL);
            connection->execute("CREATE TABLE testing (id SERIAL, text VARCHAR(128), num INTEGER, fl FLOAT, createdOn TIMESTAMP DEFAULT CURRENT_TIMESTAMP, updatedOn TIMESTAMP)");
            connection->beginTrans();
        }

        virtual void TearDown() {
            connection->commitTrans();
            connection->execute("DROP TABLE testing");
            connection->close();
            connection->release();
        }

        Poco::AutoPtr <DB::Connection> connection;
};

TEST_F(PqTransactionTest, SingleInsert) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',42)"), true);
}

TEST_F(PqTransactionTest, SingleSelect) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',42)"), true);
    EXPECT_EQ(connection->commitTrans(), true);

    DB::Query q(*connection);
    q << "SELECT * FROM testing;";
    DB::ResultSet *rs = connection->executeQuery(q.str());
    EXPECT_NE(rs, (DB::ResultSet *) NULL);
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
    EXPECT_EQ(rs->getUnixTime(5), -1);
    EXPECT_EQ(rs->recordCount(), 1);
    rs->close();
}

TEST_F(PqTransactionTest, QueryString) {
    connection->setTransactionMode(DB::Connection::READ_UNCOMMITTED);
    connection->setTransactionMode(DB::Connection::READ_COMMITTED);
    connection->setTransactionMode(DB::Connection::REPEATABLE_READ);
    connection->setTransactionMode(DB::Connection::SERIALIZABLE);

    DB::Query q(*connection);
    q << "INSERT INTO testing (text,fl) VALUES (" << DB::qstr("benden");
    q << "," << 42.0f << ");";
    EXPECT_EQ(connection->execute(q.str()), true);
    unsigned long id = connection->insertId();
    EXPECT_EQ(id, 0);
    EXPECT_EQ(connection->commitTrans(), true);
}

TEST_F(PqTransactionTest, RollbackTransaction) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text) VALUES ('benden');"), true);
    EXPECT_EQ(connection->rollbackTrans(), true);
}

TEST_F(PqTransactionTest, QueryStringTypes) {
    DB::Query q(*connection);

    q << "INSERT INTO test (text,fl,updatedOn) VALUES (";
    q << DB::qstr("benden") << "," << 42l << "," << DB::unixtime(time(NULL)) << ");";
    EXPECT_EQ(strlen(q.str()), 80);
}

TEST_F(PqTransactionTest, QueryStringTypes2) {
    DB::Query q(*connection);
    std::stringstream world;
    world << " World" << std::ends;

    q << std::string("Hello") << world;

    EXPECT_EQ(strlen(q.str()), 11);
}

TEST_F(PqTransactionTest, QueryNumberTypes) {
    DB::Query q(*connection);
    double d = 42.2;
    short s = 42;
    unsigned short su = 42;

    q << 42 << " " << d << " " << s << (const unsigned char *) " ";
    q << 42u << " " << 42ul << " " << su;
}


#endif


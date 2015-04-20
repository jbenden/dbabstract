#include <gtest/gtest.h>
#include <exception>
#include <iostream>
#include <string>
#include <strstream>

#include "dbabstract/db.h"

#ifdef ENABLE_PQ

extern "C" {
    extern dbabstract::Connection *create_pq_connection(void);
};

class PqInvalidTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            connection = create_pq_connection();
            connection->open("host = 127.0.0.1 dbname = postgres1", NULL, 0, NULL, NULL);
        }

        virtual void TearDown() {
            connection->release();
        }

        dbabstract::Connection * connection;
};

TEST_F(PqInvalidTest, CannotConnectToDatabase) {
    ASSERT_EQ(connection->isConnected(), false);
}

class PqInvalid2Test : public ::testing::Test {
    protected:
        virtual void SetUp() {
            connection = create_pq_connection();
            ASSERT_EQ(connection->open("host=127.0.0.1 dbname=postgres2", NULL, 0, NULL, NULL), false);
        }

        virtual void TearDown() {
            connection->release();
        }

        dbabstract::Connection * connection;
};

TEST_F(PqInvalid2Test, CannotConnectToDatabase) {
    ASSERT_EQ(connection->isConnected(), false);
}

TEST_F(PqInvalid2Test, BadQuery) {
    ASSERT_EQ(connection->executeQuery("SELECT sjot frm fs"), (dbabstract::ResultSet*)NULL);
    ASSERT_EQ(connection->executeQuery("BEGIN"), (dbabstract::ResultSet*)NULL);
}

class PqDefaultTest : public ::testing::Test {
    protected:
        virtual void SetUp() {
            connection = create_pq_connection();
            EXPECT_EQ(connection->open("host = 127.0.0.1 dbname = postgres", NULL, 0, NULL, NULL), true);
        }

        virtual void TearDown() {
            connection->release();
        }

        dbabstract::Connection * connection;
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
    EXPECT_STREQ(t, "'2014-11-02 22:00:31'");
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

        dbabstract::Connection * connection;
};

TEST_F(PqTransactionTest, SingleInsert) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',42)"), true);
}

TEST_F(PqTransactionTest, SingleSelect) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',42)"), true);
    EXPECT_EQ(connection->commitTrans(), true);

    std::stringstream q;
    q << "SELECT * FROM testing;";
    dbabstract::ResultSet *rs = connection->executeQuery(q.str().c_str());
    EXPECT_NE(rs, (dbabstract::ResultSet *) NULL);
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
    EXPECT_EQ(rs->recordCount(), 1);
    rs->close();
}

TEST_F(PqTransactionTest, DoubleSelect) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',1)"), true);
    EXPECT_EQ(connection->execute("INSERT INTO testing (text,fl) VALUES ('benden',1)"), true);
    EXPECT_EQ(connection->commitTrans(), true);

    std::stringstream q;
    q << "SELECT * FROM testing;";
    dbabstract::ResultSet *rs = connection->executeQuery(q.str().c_str());
    EXPECT_NE(rs, (dbabstract::ResultSet *) NULL);
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
    EXPECT_EQ(rs->getUnixTime(1), 0);
    rs->close();
}
TEST_F(PqTransactionTest, QueryString) {
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

TEST_F(PqTransactionTest, RollbackTransaction) {
    EXPECT_EQ(connection->execute("INSERT INTO testing (text) VALUES ('benden');"), true);
    EXPECT_EQ(connection->rollbackTrans(), true);
}

TEST_F(PqTransactionTest, QueryStringTypes) {
    std::stringstream q;

    q << "INSERT INTO test (text,fl,updatedOn) VALUES (";
    q << dbabstract::qstr(*connection, "benden") << "," << 42l << "," << dbabstract::unixtime(*connection, time(NULL)) << ");";
    EXPECT_EQ(strlen(q.str().c_str()), 80);
}

TEST_F(PqTransactionTest, QueryStringTypes2) {
    std::stringstream q;
    std::string world;
    world = " World";

    q << "Hello" << world;

    EXPECT_EQ(strlen(q.str().c_str()), 11);
}

TEST_F(PqTransactionTest, QueryNumberTypes) {
    std::stringstream q;
    double d = 42.2;
    short s = 42;
    unsigned short su = 42;

    q << 42 << " " << d << " " << s << (const unsigned char *) " ";
    q << 42u << " " << 42ul << " " << su;
}


#endif


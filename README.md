---

A database abstraction layer for C++ and POCO framework
=======================================================
 
(C) 2006-2014 Thralling Penguin LLC. All rights reserved.

---

For information about usage, consult the class reference available
in the doc directory (if you have Doxygen) or view the online one
available at Thralling Penguin's website.

Additional information is available [here](82b4790abe34f97704f727bad1e222c83b2ee862). Doxygen documentation for the original version is available [here](http://www.thrallingpenguin.com/resources/dbabstract/).

Building On OS X
----------------

As a requirement to building you must have Homebrew and the following
modules installed:

* poco
* mysql
* sqlite3

Then inside a git checkout of this project, do the following:

    $ LIBPATH=/usr/local/lib CXX=clang++ ./configure --prefix=/usr/local --enable-mysql=/usr/local --enable-sqlite3=/usr/local
    $ make
    $ make install
    $ /usr/local/bin/test_db

Postgres.app Support
--------------------

It is possible to use Postgres.app as your PostgreSQL database. Use a
terminal line as follows:

    $ CXX=clang++ LIBPATH="/usr/local/lib" ./configure --prefix=/usr/local --enable-mysql=/usr/local --enable-sqlite3=/usr/local --enable-doc --enable-pq=/Applications/Postgres.app/Contents/Versions/9.3/
    $ make
    $ make install
    $ /usr/local/bin/test_db


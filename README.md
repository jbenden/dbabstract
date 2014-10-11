---

A database abstraction layer for C++ and POCO framework
=======================================================
 
(C) 2006-2014 Thralling Penguin LLC. All rights reserved.

---

For information about usage, consult the class reference available
in the doc directory (if you have Doxygen) or view the online one
available at Thralling Penguin's website.

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


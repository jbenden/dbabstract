[![Build Status](https://travis-ci.org/jbenden/dbabstract.svg?style=flat&branch=master)](https://travis-ci.org/jbenden/dbabstract)
A database abstraction layer for C++ and POCO framework
=======================================================
 
(C) 2006-2014 Thralling Penguin LLC. All rights reserved.

---

For information about usage, consult the class reference available
in the doc directory (if you have Doxygen) or view the online one
available at Thralling Penguin's website.

There is an article at [http://benden.us/](http://benden.us/journal/2014/dbabstract-a-database-abstraction-library-for-cpp/) about DBAbstract. Doxygen documentation for the original version is available [here](http://www.thrallingpenguin.com/resources/dbabstract/).

Building On OS X
----------------

As a requirement to building you must have Homebrew and the following
modules installed:

* cmake
* poco
* gtest (included)
* mysql (optional)
* sqlite3 (optional)
* postgresql (optional) [Postgres.app will be automatically detected, if installed.]

Then inside a git checkout of this project, do the following:

    $ cmake .
    $ make
    $ make install
    $ /usr/local/bin/test_db


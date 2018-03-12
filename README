This is a set of UDF functions for MySQL originally written by Jan Steeman.

http://mysql-udf.sourceforge.net/

These functions have been brought up to date with the MySQL API, and I added some functions (such as STEYX from Excel).
I added a build system (simple Makefile) to easily compile all functions as a single library which can be installed with
other MySQL plugins. The development environment for this library is Ubuntu Linux (64bit). The library should build cleanly on any
standard Linux development machine with g++ and the MySQL development libraries and headers installed.

The import.sql script provides the necessary commands to import the UDF functions into MySQL. You need to be an administrator
to be able to do that.

The repository on GitHub is at http://github.com/megastep/mysql-udf


--
Stephane Peter
October 2011



----------



Then:

1. median() were merged with Steemann's (http://sourceforge.net/u/steemann/profile/) patch for available here:

	http://sourceforge.net/p/mysql-udf/patches/2/

2. Misc functions merged with Paul's ("paul.w.b...") fixes from:

	http://code.google.com/p/mysql-udf-moments

3. All remaining functions were patched to work against MySQL 5.5

4. Makefile fixed to run correctly against MySQL 5.5

5. Fixing import.sql for installing the functions.

6. Made drop.sql script for removing the functions.

The repository on GitHub is at https://github.com/nmmmnu/mysql-udf


--
Nikolay Mihaylov
nmmm@nmmm.nu
https://github.com/nmmmnu/
21.September.2014



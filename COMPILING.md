To compile Renga you need to be running Haiku.

Dependencies
============

    pkgman install gloox_x86_devel

Building
========

Renga is built using cmake. There is nothing special about the build process,
this should work without problems:

    setarch x86
    mkdir build
	cd build
	cmake ..
	make

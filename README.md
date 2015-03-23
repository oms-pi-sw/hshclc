# hshclc
HSH-CLC: hashes calculator

HSH-CLC 1.1u8r10r_Release: a hash calculator.

It supports md5, sha1, sha224, sha256, sha384, sha512.

Those algorithms are written by:

	md5: Ulrich Drepper

	sha1: Scott G. Miller [Robert Klep - expansion function fix]

	sha224, sha256, sha384, sha512: David Madore

All sources and binaries are under GNU GPL v3 LICENSE (READ CAREFULLY LICENSE)

Complete repository is at https://minegrado.info/DWN/.

#build
build from sources:

use a c compiler like gcc or clang.

For example:

gcc -Wall -O2 *.c lib/*.c -ohshclc

#executing
Executing from command line:

please read `hshsclc --help' for more help or readme.txt

HSH-CLC:

ABSTRACT:
hshclc is an hash calculator, it support md5, sha1, sha224, sha256, sha384, sha512.

USE:
	COMMAND LINE:
You can calculate a single hash by command line with this sintax:
hshclc -i <input_file> -a <algorithm: md5|sha1|sha224|...> -o [<output_file> [--tee]]

	CONFIGURATION FILE:
You can calculate and check multiple hashes on multiple files with a simple configuration file.
This is an example:

**********  config file: config.cfg  **********
-file <bar/foo1.ext> {
	-calculate md5 sha1 sha512;
	-check {
		md5	<md5_sum>;
		sha256	<sha256_sum>;
	}
	-tee <output_file>;
}
-file <foo2.ext> {
	-calculate sha256;
	-check {
		sha384	<sha384_sum>;
	}
	-out <output_file>;
}
**********  config file: config.clc  **********

and then:
hshclc -c config.clc

		NB:	
		you must use semicolon at the end of any line.
		With configuration file it will append to output file, if exists, else it will create the new file and then append.
				
For other informations use "hshclc --help" or "hshclc -h".

LICENSE:
	THIS SOFTWARE IS UNDER GNU GPL LICENSE.
	READ CAREFULLY LICENSE.txt and README.txt and then "hshclc -l".
	
	Hash sum algorithm are considerably copy-pasted from those written by:
	md5: Ulrich Drepper
	sha1: Scott G. Miller [Robert Klep - expansion function fix]
	sha224, sha256, sha384, sha512: David Madore

AUTHOR:
	Niccolò Ferrari: niccolo.ferrari@hotmail.it
	
	Hash sum algorithm are considerably copy-pasted from those written by:
	md5: Ulrich Drepper
	sha1: Scott G. Miller [Robert Klep - expansion function fix]
	sha224, sha256, sha384, sha512: David Madore.

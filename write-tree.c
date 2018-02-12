/*
 *  All documentation (comments) unless explicitly noted:
 *
 *      Copyright 2018, AnalytixBar LLC, Jacob Stopak
 *
 *  All code & explicitly labelled documentation (comments):
 *      
 *      Copyright 2005, Linus Torvalds
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************
 *
 *  The purpose of this file is to be compiled into an executable
 *  called `write-tree`. When `write-tree` is run from the command line
 *  it does not take any command line arguments.
 *
 *  The `write-tree` command takes the changes that have been staged in
 *  the index and creates a tree object in the object store recording
 *  these changes.
 *
 *  Everything in the main function in this file will run
 *  when ./write-tree executable is run from the command line.
 */

#include "cache.h"
/*  The above 'include' allows use of the following functions and
    variables from <cache.h> header file, ranked in order of first use
    in this file. Most are functions/macros from standard C libraries
    that are `#included` in <cache.h>. Function names are followed by
    parenthesis whereas variable/struct names are not:

	sha1_file_name(sha1): TODO

	access(path, amode): Check whether the file at `path` is accessible
						 via the permission set descibed by `amode`.

	-perror(message): Write `message` to standard error output.
                      Sourced from <stdio.h>.

	-read_cache(): Read in the contents of the `.dircache/index` file into
                   the `active_cache`. The number of caches entries is returned.

    -fprintf(stream, message): Place `message` on the named output
                               `stream`. Sourced from <stdio.h>.

    -exit(status): Stop execution of the program and exit with code

	-malloc(size): Allocate unused space for an object whose
                   size in bytes is specified by `size` and whose
                   value is unspecified. Sourced from <stdlib.h>.

	-alloc_nr(x): This is a macro sourced from <cache.h>.

    -realloc(pointer, size): Update the size of the memory object pointed
                             to by `pointer` to `size`. Sourced from <stdlib.h>.

    -sprintf(s, message): Place output followed by the null byte, '\0', in
                          consecutive bytes starting at *s. Sourced from
                          <stdio.h>.

	-memcpy(s1, s2, n): Copy n bytes from the object pointed to
                        by s2 into the object pointed to by s1.

	-write_sha1_file(buf, len): Compress file content stored in `buf`, hash
                                compressed output, and write to object store.

****************************************************************

The following variables and functions are defined locally.

    -main(): The main function runs each time the ./read-tree
             command is run.

    -check_valid_sha1(): TODO

	-prepend_integer(): TODO

	-ORIG_OFFSET: TODO

*/

/*
 * Function: `check_valid_sha1`
 * Parameters:
 *      -sha1: The hash to check if there is an valid file for.
 * Purpose: To check whether or not there is a file in the working directory
 *          corresponding to the passed-in hash.
 */
static int check_valid_sha1(unsigned char *sha1)
{
    /* Get the path/filename corresponding to a specific hash. */
	char *filename = sha1_file_name(sha1);
	int ret; /* Return code. */

    /* Check whether the file is accessible in the working directory. */
	ret = access(filename, R_OK);

    /* Error if the file is not accessible. */
	if (ret)
		perror(filename);

	return ret;
}

/*
 * Function: `prepend_integer`
 * Parameters:
 *      -buffer: A string to prepend an integer to.
 *      -val: TODO
 *      -i: The integer to prepend to `buffer`.
 */
static int prepend_integer(char *buffer, unsigned val, int i)
{
    /* Decrement `i` and set the new ith element in `buffer` to the null byte. */
	buffer[--i] = '\0';

	do {
		buffer[--i] = '0' + (val % 10);
		val /= 10;
	} while (val);
	return i;
}

/* Linus Torvalds: Enough space to add the header of "tree <size>\0" */
#define ORIG_OFFSET (40)

/*
 * Function: `main`
 * Parameters:
 *      -argc: The number of command-line arguments supplied, inluding the command itself. 
 *      -argv: An array of the command line arguments, including the command itself.
 * Purpose: Standard `main` function definition. Runs when the executable `write-tree` is
 *          run from the command line. 
 */
int main(int argc, char **argv)
{
	/* The size of the tree to write to the object store. */
	unsigned long size;

    /* Declare an offset used to store the header of the tree. */
	unsigned long offset;

    /* Not used. Even Linus Torvalds makes mistakes. */
	unsigned long val;

	/* Iterator used in for loop. */
	int i;

	/*
	 * Read in the contents of the `.dircache/index` file into the `active_cache`.
	 * The number of caches entries is stored in `entries`.
	 */
	int entries = read_cache();

	/* String to hold the tree's content. */
	char *buffer;

	/*
	 * If there are no active cache entries, throw an error message since there is
	 * nothing staged in the index to write.
	 */
	if (entries <= 0) {
		fprintf(stderr, "No file-cache to create a tree of\n");
		exit(1);
	}

	/* Linus Torvalds: Guess at an initial size of the tree to write. */
	size = entries * 40 + 400;

	/* Allocate `size` bytes to store the tree's content. */
	buffer = malloc(size);

    /* Set the offset using the macro defined in this file. */
	offset = ORIG_OFFSET;

    /*
     * Iterate over each cache entry, adding the relevant info to the buffer
     * representing the tree.
     */
	for (i = 0; i < entries; i++) {
        /* Pick out the ith cache entry from the active cache. */
		struct cache_entry *ce = active_cache[i];

        /* Make sure each cache entry has a valid SHA1, or exit. */
		if (check_valid_sha1(ce->sha1) < 0)
			exit(1);

        /* If needed, readjust the size allocated for each cache entry in the buffer. */
		if (offset + ce->namelen + 60 > size) {
			size = alloc_nr(offset + ce->namelen + 60);
			buffer = realloc(buffer, size);
		}

        /* Add the file type and name to the tree for the current cache entry. */
		offset += sprintf(buffer + offset, "%o %s", ce->st_mode, ce->name);

        /* Add a `0` to the tree buffer as a null byte separator. */
		buffer[offset++] = 0;

        /* Add the cache entry's SHA1 to the tree buffer. */
		memcpy(buffer + offset, ce->sha1, 20);

        /* Increment the offset by 20 bytes based on the size of the previously added SHA1. */
		offset += 20;
	}

    /*
     * Create space at the beginning of the buffer to add the text `tree `, to identify this
     * object as a tree in the object store.
     */
	i = prepend_integer(buffer, offset - ORIG_OFFSET, ORIG_OFFSET);
	i -= 5;
	memcpy(buffer+i, "tree ", 5);

	/* Not sure why this increment/decrement is required. */
	buffer += i;
	offset -= i;

	/* Compress file content stored in `buf`, hash compressed output, and write to object store. */
	write_sha1_file(buffer, offset);

	/* Return success. */
	return 0;
}

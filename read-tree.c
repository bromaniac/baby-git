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
 *  called `read-tree`. When `read-tree` is run from the command line
 *	it takes the hash of a tree object as an argument, for example:
 *	`./read-tree c21baf88bb59f4376b242edcd915d82e9fef4913`
 *
 *  Note the tree object must exist in the object store for this to work.
 *
 *  It will then print out the contents of that tree object, which includes:
 *      1) The file-mode (file-type/permissions/attributes) of the content
 *         referenced by the tree. I.e. the file-mode of the file that was committed.
 *      2) The name of the file that the tree object references.
 *      3) The hash of the blob that the tree object references.
 *
 *  This whole file (i.e. everything in the main function) will run
 *  when ./read-tree executable is run from the command line.
 */

#include "cache.h"
/*  The above 'include' allows use of the following functions and
    variables from <cache.h> header file, ranked in order of first use
    in this file. Most are functions/macros from standard C libraries
    that are `#included` in <cache.h>. Function names are followed by
    parenthesis whereas variable/struct names are not:

    -getenv(name): Get value of the environment variable `name`.
                   Sourced from <stdlib.h>.

    -DB_ENVIRONMENT: Constant string (defined via macro in <cache.h>)
                     used to specify an environment variable to set
                     the path/name of the object store.

    -DEFAULT_DB_ENVIRONMENT: Constant string (defined via macro in
                             <cache.h>) used to customize the name
                             of object store.

    -strcmp(s1, s2): Compares the string pointed to by `s1` to the
                     string pointed to by `s2`.

    -strlen(string): Return the length of `string` in bytes.

    -strchr(string, c): Locate the first occurrence of `c` (converted
                        to a char) in the string pointed to by `string`.

    -sscanf(string, format, args): Read input from `string` into `args`
                                   using input format `format`. Sourced
                                   from <stdio.h>.

    -printf(message): Place `message` on standard output stream
                               stdout.  Sourced from <stdio.h>.

	-usage(): Print a usage message in the event of incorrect usage.
              Sourced from <cache.h>.

    -sha1_to_hex(): TODO

    -get_sha1_hex(): TODO

    -read_sha1_file(): TODO

****************************************************************

    The following variables and functions are defined locally.

    -main(): The main function runs each time the ./read-tree
             command is run.

    -unpack(): Retrieve the tree content for the tree in the object
               store. identified by the passed in binary `sha1` hash.

*/

/*
 * Function: `unpack`
 * Parameters:
 *		-sha1: The binary hash of a tree object in the object store. 
 * Purpose: Retrieve the tree content for the tree in the object store
 * 			identified by the passed in binary `sha1` hash.
 */
static int unpack(unsigned char *sha1)
{
	void *buffer; /* Will hold the tree's content. */
	unsigned long size; /* Will hold the size of the tree in bytes. */
	char type[20]; /* Will hold the type of the file, which must be "tree". */

    /*
     * Read the content of the file in the object store identified by `sha1`
     * and store it in `buffer`.
     */
	buffer = read_sha1_file(sha1, type, &size);

    /* Print usage message if `buffer` is empty or null. */
	if (!buffer)
		usage("unable to read sha1 file");

    /* Print usage message if the file identified by `sha1` is not a tree. */
	if (strcmp(type, "tree"))
		usage("expected a 'tree' node");

    /*
     * Since a tree can reference multiple trees and blobs, loop to
     * iterate over each referenced item.
     */
	while (size) {
		int len = strlen(buffer)+1; /* The # of characters in the tree. */
		unsigned char *sha1 = buffer + len; /* Repoint `sha1` to the end of the buffer in memory. */
		char *path = strchr(buffer, ' ')+1; /* Set the `path` pointer to the location of the first space in `buffer`. */
		unsigned int mode; /* Will hold the file-mode of the tree. */
        
        /*
         * Verify the size of the content and read in the file-mode.
         * If either fails, display usage message.
         */
		if (size < len + 20 || sscanf(buffer, "%o", &mode) != 1)
			usage("corrupt 'tree' file");

		buffer = sha1 + 20; /* Update `buffer` by `sha1` + 20. */
		size -= len + 20; /* Decrement size by `len` + 20. */

        /* Display the file-mode, file name, and hash of the object referenced by the tree. */
		printf("%o %s (%s)\n", mode, path, sha1_to_hex(sha1));
	}
	return 0;
}

/*
 * Function: `main`
 * Parameters:
 * 		-argc: The number of command-line arguments supplied, inluding the command itself. 
 *		-argv: An array of the command line arguments, including the command itself.
 * Purpose: Standard `main` function definition. Runs when the executable `read-tree` is
 * 			run from the command line. Ensures proper command syntax and calls `unpack()`
 *			function above to print a tree's content to the screen.
 */
int main(int argc, char **argv)
{
	int fd; /* Declare an integer to hold the file descriptor. */
	unsigned char sha1[20]; /* String to hold the 20 byte binary hash of the tree. */

	/*  
     * Make sure program was passed in one command line argument
     * (it technically looks for 2 since the initial command itself
     * counts). If not, print a usage message.
     */
	if (argc != 2)
		usage("read-tree <key>");

	/*
     * Convert hexadecimal string from command line argument to
     * binary SHA1 representation. If conversion fails, (for example
     * if the hex string passed-in has a character outside the valid
     * hex range of 0-9, a-f, or A-F), print usage message.
     */
	if (get_sha1_hex(argv[1], sha1) < 0)
		usage("read-tree <key>");

	/*  
     * Set the `sha1_file_directory` (i.e. the path to the object store)
     * to the value of the `DB_ENVIRONMENT` environment
     * variable which defaults to `SHA1_FILE_DIRECTORY` as 
     * defined in <cache.h>. If the environment variable is
     * not set (and it most likely won't be), `sha1_file_directory`
     * will be null (technically a null pointer, but whatever).
     */
	sha1_file_directory = getenv(DB_ENVIRONMENT);

	/*  
     * If object store path was not set from environment variable above,
	 * set it to the default value `.dircache/objects` from <cache.h>.
     */
	if (!sha1_file_directory)
		sha1_file_directory = DEFAULT_DB_ENVIRONMENT;

	/* Call `unpack()` with the binary hash of the tree. */
	if (unpack(sha1) < 0)
		usage("unpack failed");

	return 0;
}

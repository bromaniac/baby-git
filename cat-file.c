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
 *  version 2 as published by the Free Software Foundation.
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
 *  called `cat-file`. When `cat-file` is run from the command line
 *  it will take as an argument the 20 hexadecimal of any object in
 *  the object store, and output the contents of that object into a
 *  file named `temp_git_file_XXXXXX`, where XXXXXX is a randomly
 *  generated character string, so you can view the object's content.
 *  Note that this implies that at least one object already exists in
 *  the object store (which means at least one file was added to the
 *  index and committed into the repository using the `update-cache`,
 *  `write-tree`, and `commit-tree` commands consecutively.
 *
 *  This whole file (i.e. everything in the main function) will run
 *  when ./cat-file executable is run from the command line.
 */

#include "cache.h"
/*  The above 'include' allows use of the following functions and
    variables from <cache.h> header file, ranked in order of first use
    in this file. Most are functions/macros from standard C libraries
    that are `#included` in <cache.h>. Function names are followed by
    parenthesis whereas variable/struct names are not:

    -get_sha1_hex(): Convert the SHA1 passed in as a command line
                     argument to the proper hexadecimal format.
                     Sourced from <cache.h> and defined in
                     <read-cache.c>.

	-usage(): Print a read-cache error to the screen. Sourced from
			  <cache.h> and defined in <read-cache.c>.

	-read_sha1_file(): Read-in the contents of the file identified by
					   the SHA1 passed in as a command line argument
					   into a buffer to be ouputted later.

	-mkstemp(template): Use a `template` filename to create a file
						descriptor to a newly opened file for writing.
						Sourced from <stdlib.h>.

	-write(fd, buf, n): Write `n` bytes from buffer `buf` to file
						described by file descriptor `fd`.

	-strcpy(str1, str2): Copy the value of str2 into the memory
						 allocated for str1.

	-printf(message): Print message to standard output stdout.
					  Sourced from <stdio.h>.

	****************************************************************

    The following variables and functions are defined locally in
    this file:

    -main(argc, argv): The main function which runs each time the
                       ./cat-file command is run.

    -argc: The number of command line arguments supplied when
           executing ./cat-file.

    -argv: Array containing command line argument strings.

	-sha1: String represeting the SHA1 of the object desired to
           be retrieved from the object store.

	-type: The type of the object being retrieved from the object
           store, i.e. `blob` or `tree`, etc.

	-buf: A buffer to temporarily store the contents of the
          retrieved object from the object store before it is
          written to the output file `temp_git_file_XXXXXX`.

	-size: The size in bytes of the retrieved content.

	-template: A template used to generate output file names.

	-fd: A file descriptor respresenting the output file to be
         created.

*/

/*
 * Function: `main`
 * Parameters:
 *      -argc: The number of command-line arguments supplied, inluding the command itself.
 *      -argv: An array of the command line arguments, including the command itself.
 * Purpose: Standard `main` function definition. Runs when the executable `commit-tree` is
 *          run from the command line.
 */
int main(int argc, char **argv)
{
    /*
     * String to hold SHA1 passed-in as command-line argument.
     * Note it is empty at first and gets populated by the
     * get_sha1_hex() function which takes the empty variable
     * as an argument and populates it with the converted SHA1.
     */
	unsigned char sha1[20];

    /*
     * String to hold the type (`blob` or `tree`) of the
     * requested content. Note it is empty at first and gets
     * populated by read_sha1_file() function which takes the
     * empty variable as an argument and populates it.
     */
	char type[20];

	/*  
     * Buffer to hold the content of the requested blob or tree.
     * Note it is empty at first and gets populated by the return
	 * of the read_sha1_file() function. 
     */
	void *buf;

	/*
     * Integer to hold the size in bytes of the requested content
     * Note it is empty at first and gets populated by the
	 * read_sha1_file() function which takes a pointer to the empty
	 * variable as an argument and populates it.
     */
	unsigned long size;

	/*
	 * Template filename used to create a unique output file name
	 * to store the requested output content.
	 */
	char template[] = "temp_git_file_XXXXXX";

	/*
	 * File descriptor representing the output file to be created. 
	 */
	int fd;

	/*
	 * Make sure program was passed in one command line argument
	 * (it technically looks for 2 since the initial command itself
	 * counts) and that the SHA1 has a valid hexadecimal conversion.
	 * Or else output a usage message. If successful this also
	 * populates the `sha1` variable with the converted hex value.
	 */
	if (argc != 2 || get_sha1_hex(argv[1], sha1))
		usage("cat-file: cat-file <sha1>");

	/*
	 * Pass the now populated `sha1` variable into the
	 * `read_sha1_file()` function along with still empty `type`
	 * and `size` variables, which will be populated as a part of
	 * the function. Store the output in our `buf` buffer variable.
	 */
	buf = read_sha1_file(sha1, type, &size);
	
	/*
	 * If the buffer wasn't populated correctly, exit the program.
	 */
	if (!buf)
		exit(1);

	/*
	 * Create the file descriptor `fd` based on the `template`
	 * defined above. The `XXXXXX` in the template will be relaced
	 * with a randomly generated alphanumeric string to create a
	 * unique filename.
	 */
	fd = mkstemp(template);

	/*  
     * If file descriptor creation fails, print usage message.
     */
	if (fd < 0)
		usage("unable to create tempfile");

	/*
	 * Write `size` bytes from our `buffer` which contains the
	 * requested content from the object store to our file
	 * descriptor `fd` and make sure that size matches up. If not,
	 * set the `type` to "bad".
	 */
	if (write(fd, buf, size) != size)
		strcpy(type, "bad");

	/*
	 * Print the generated `template` filename and `type` to screen.
	 */
	printf("%s: %s\n", template, type);
}

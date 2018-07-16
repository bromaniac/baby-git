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
/* The above 'include' allows use of the following functions and
   variables from "cache.h" header file, ranked in order of first use
   in this file. Most are functions/macros from standard C libraries
   that are `#included` in "cache.h". Function names are followed by
   parenthesis whereas variable/struct names are not:

   -get_sha1_hex(): Convert a 40-character hexadecimal representation of an 
                    SHA1 hash value to the equivalent 20-byte representation.

   -usage(): Print an error message and exit.

   -read_sha1_file(): Locate an object in the object database, read and 
                      inflate it, then return the inflated object data 
                      (without the prepended metadata).

   -mkstemp(template): Modifies `template` to generate a unique filename, then
                       opens the file for reading and writing and returns a 
                       file descriptorfor the file. Sourced from <stdlib.h>.

   -write(fd, buf, n): Write `n` bytes from buffer `buf` to file associated
                       with file descriptor `fd`.

   -strcpy(str1, str2): Copy string str2 to string str1, including the
                        terminating null character.

   -printf(message, ...): Write `message` to standard output stream stdout.  
                          Sourced from <stdio.h>.

   ****************************************************************

   The following variables and functions are defined in this source file:

   -main(argc, argv): The main function which runs each time the ./cat-file 
                      command is run.

   -argc: The number of command line arguments supplied when executing 
          ./cat-file.

   -argv: Array containing command line argument strings.

   -sha1: 20-byte representation of an SHA1 hash.

   -type: The type of the object that was read from the object store (blob, 
          tree, or commit).

   -buf: A buffer to store the object data.

   -size: The size in bytes of the object data.

   -template: A template string used to generate a unique output filename.

   -fd: A file descriptor associated with the output file.
*/

/*
 * Function: `main`
 * Parameters:
 *      -argc: The number of command line arguments supplied, inluding the 
 *             command itself.
 *      -argv: An array of the command line arguments, including the command 
 *             itself.
 * Purpose: Standard `main` function definition. Runs when the executable
 *          `cat-file` is run from the command line.
 */
int main(int argc, char **argv)
{
    /* Used to store the 20-byte representation of an SHA1 hash. */
    unsigned char sha1[20];
    /* Used to store the object type (blob, tree, or commit). */
    char type[20];
    /* Buffer to store the object data. */
    void *buf;
    /* The size in bytes of the object data. */
    unsigned long size;
    /* A template string used to generate a unique output filename. */
    char template[] = "temp_git_file_XXXXXX";
    /* File descriptor for the output file. */
    int fd;

    /*  
     * Validate the number of command line arguments and convert the given 
     * 40-character hexadecimal representation of an SHA1 hash value to the 
     * equivalent 20-byte representation. If either one fails, display usage
     * and exit.
     */
    if (argc != 2 || get_sha1_hex(argv[1], sha1))
        usage("cat-file: cat-file <sha1>");

    /*
     * Read the object whose SHA1 hash is `sha1` from the object store, 
     * inflate it, and return a pointer to the object data (without the 
     * prepended metadata). Store the object type and object data size in 
     * `type` and `size` respectively.
     */
    buf = read_sha1_file(sha1, type, &size);
    
    /*
     * Exit if `buf` is a null pointer, i.e., if reading the object from the
     * object store failed.
     */
    if (!buf)
        exit(1);

    /*
     * Modify `template` to generate a unique filename, then open the file for 
     * reading and writing and return a file descriptor for the file. The 
     * `XXXXXX` in the template is replaced with a randomly generated 
     * alphanumeric string to generate a unique filename.
     */
    fd = mkstemp(template);

    /* If mkstemp() fails, print error message and exit. */
    if (fd < 0)
        usage("unable to create tempfile");

    /*
     * Write the object data, which has length `size` bytes, to the output
     * file associated with `fd`. If the number of bytes written does not 
     * equal the object data size, then set object `type` to "bad".
     */
    if (write(fd, buf, size) != size)
        strcpy(type, "bad");

    /* Print the output filename and object type to screen. */
    printf("%s: %s\n", template, type);
}

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
 *  called `read-tree`. When `read-tree` is run from the command line
 *  it takes the hash of a tree object as an argument, for example:
 *  `./read-tree c21baf88bb59f4376b242edcd915d82e9fef4913`
 *
 *  Note the tree object must exist in the object store for this to work.
 *
 *  It will then print out the contents of that tree object, which includes:
 *      1) The file-mode (file-type/permissions/attributes) of the content
 *         referenced by the tree. I.e. the file-mode of the file that was 
 *         committed.
 *      2) The name of the file that the tree object references.
 *      3) The hash of the blob that the tree object references.
 *
 *  This whole file (i.e. everything in the main function) will run
 *  when ./read-tree executable is run from the command line.
 */

#include "cache.h"
/* The above 'include' allows use of the following functions and
   variables from "cache.h" header file, ranked in order of first use
   in this file. Most are functions/macros from standard C libraries
   that are `#included` in "cache.h". Function names are followed by
   parenthesis whereas variable/struct names are not:

   -getenv(name): Get value of the environment variable `name`. Sourced from
                  <stdlib.h>.

   -DB_ENVIRONMENT: Constant string (defined via macro in "cache.h") used to 
                    specify an environment variable to set the path/name of 
                    the object store.

   -DEFAULT_DB_ENVIRONMENT: Constant string (defined via macro in "cache.h") 
                            with the default path of the object store.

   -strcmp(s1, s2): Compares the string pointed to by `s1` to the string 
                    pointed to by `s2`.

   -strlen(string): Return the length of `string` in bytes.

   -strchr(string, c): Return pointer to the first occurrence of character `c` 
                       in `string`.

   -sscanf(string, format, args): Read input from `string` into `args` using 
                                  input format `format`. Sourced from 
                                  <stdio.h>.

   -printf(message, ...): Write `message` to standard output stream stdout.  
                          Sourced from <stdio.h>.

   -usage(): Print an error message and exit. 

   -sha1_to_hex(): Convert a 20-byte representation of an SHA1 hash value to 
                   the equivalent 40-character hexadecimal representation.

   -get_sha1_hex(): Convert a 40-character hexadecimal representation of an 
                    SHA1 hash value to the equivalent 20-byte representation.

   -read_sha1_file(): Locate an object in the object database, read and 
                      inflate it, then return the inflated object data 
                      (without the prepended metadata).

   ****************************************************************

   The following variables and functions are defined in this source file.

   -main(): The main function runs each time the ./read-tree command is run.

   -unpack(): Call the read_sha1_file() function to read and inflate a tree
              object from the object store, and then output the tree data to
              the screen.
*/

/*
 * Function: `unpack`
 * Parameters:
 *        -sha1: The SHA1 hash of a tree object in the object store. 
 * Purpose: Call the read_sha1_file() function to read and inflate a tree
 *          object from the object store, and then output the tree data to
 *          the screen.
 */
static int unpack(unsigned char *sha1)
{
    void *buffer;         /* The tree data buffer. */
    unsigned long size;   /* The size of the tree object data in bytes. */
    char type[20];        /* The object type. */

    /*
     * Read an object with hash value `sha1` from the object store, inflate 
     * it, and return a pointer to the object data (without the prepended 
     * metadata). Store the object type and object data size in `type` and 
     * `size` respectively.
     */
    buffer = read_sha1_file(sha1, type, &size);

    /* Print usage message if `buffer` is empty or null, then exit. */
    if (!buffer)
        usage("unable to read sha1 file");

    /*
     * Print usage message if the object corresponding to the hash `sha1` is 
     * not a tree, then exit. 
     */
    if (strcmp(type, "tree"))
        usage("expected a 'tree' node");

    /*
     * Read metadata about each blob object from the tree object data buffer. 
     */
    while (size) {
        /* Calculate offset to the current blob object's SHA1 hash. */
        int len = strlen(buffer) + 1;           
        /* Point to the current blob object's SHA1 hash. */
        unsigned char *sha1 = buffer + len;  
        /*
         * Point to the path of the file corresponding to the current blob
         * object. 
         */
        char *path = strchr(buffer, ' ') + 1;   
        unsigned int mode; 
        
        /*
         * Verify the current size of the buffer and get the mode of the file
         * corresponding to the current blob object. If either fails, display 
         * error message then exit.
         */
        if (size < len + 20 || sscanf(buffer, "%o", &mode) != 1)
            usage("corrupt 'tree' file");

        /*
         * Adjust buffer to point to the start of the next blob object's 
         * metadata. 
         */
        buffer = sha1 + 20; 
        /*
         * Decrement `size` by the length of the metadata that was read for 
         * the current blob object. 
         */
        size -= len + 20; 

        /*
         * Display the mode and path of the file corresponding to the current
         * blob object, and the 40-character representation of the current 
         * blob object's SHA1 hash.
         */
        printf("%o %s (%s)\n", mode, path, sha1_to_hex(sha1));
    }
    return 0;
}

/*
 * Function: `main`
 * Parameters:
 *        -argc: The number of command-line arguments supplied, inluding the 
 *               command itself. 
 *        -argv: An array of the command line arguments, including the command 
 *               itself.
 * Purpose: Standard `main` function definition. Runs when the executable 
 *          `read-tree` is run from the command line. Ensures proper command 
 *          syntax and calls `unpack()` function above to print a tree's 
 *          content to the screen.
 */
int main(int argc, char **argv)
{
    /* A file descriptor. */
    int fd; 
    /* String to hold the 20-byte representation of a hash value. */
    unsigned char sha1[20]; 

    /*  
     * Validate the number of command line arguments, which should be equal to
     * 2 since the command itself is also counted. If not, print a usage 
     * message and exit.
     */
    if (argc != 2)
        usage("read-tree <key>");

    /* 
     * Convert the given 40-character hexadecimal representation of an SHA1 
     * hash value to the equivalent 20-byte representation. If conversion 
     * fails (for example, if the hexadecimal representation has a character 
     * outside the valid hexadecimal range of 0-9, a-f, or A-F), print usage 
     * message and exit.
     */
    if (get_sha1_hex(argv[1], sha1) < 0)
        usage("read-tree <key>");

    /*
     * Set `sha1_file_directory` (i.e. the path to the object store) to the 
     * value of the `DB_ENVIRONMENT` environment variable, which defaults to 
     * `SHA1_FILE_DIRECTORY` as defined in "cache.h". If the environment 
     * variable is not set (and it most likely won't be), getenv() will return
     * a null pointer. 
     */
    sha1_file_directory = getenv(DB_ENVIRONMENT);

    /*  
     * If object store path was not set from the environment variable above, 
     * set it to the default value, `.dircache/objects`, the definition of the
     * token `DEFAULT_DB_ENVIRONMENT` in "cache.h".
     */
    if (!sha1_file_directory)
        sha1_file_directory = DEFAULT_DB_ENVIRONMENT;

    /*
     * Call `unpack()` function with the binary SHA1 hash of the tree object
     * as the function parameter. 
     */
    if (unpack(sha1) < 0)
        usage("unpack failed");

    return 0;
}

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
 *  called `init-db`. When `init-db` is run from the command line
 *  it will initialize a Git repository by creating the object
 *  store (a directory called `.dircache/objects` by default)
 *  which will store the content that users commit in order to
 *  track the history of the repository over time.
 *
 *  This whole file (i.e. everything in the main function) will run
 *  when ./init-db executable is run from the command line.
 */

#include "cache.h"

#ifndef BGIT_WINDOWS
    #define MKDIR( path ) ( mkdir( path, 0700 ) )
#else
    #define MKDIR( path ) ( _mkdir( path ) )
#endif

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

   -mkdir(name, mode): Create a new directory `name` with permissions derived 
                       from `mode`. Sourced from <sys/stat.h>.

   -perror(message): Write `message` to standard error stream. Sourced from 
                     <stdio.h>.

   -exit(status): Stop execution of the program and exit with code `status`. 
                  Sourced from <stdlib.h>.

   -stat: Structure pointer used by stat() function to store information
          related to a filesystem file. Sourced from <sys/stat.h>.

   -stat(name, buf): Obtain information about file `name` and store it in the 
                     area pointed to by `buf`, which is a pointer to a
                     `stat` structure.
                      
   -S_ISDIR(mode): Determines whether a file is a directory based on its 
                   `mode`. Sourced from <sys/stat.h>.

   -fprintf(stream, message, ...): Write `message` to the output `stream`. 
                                   Sourced from <stdio.h>.

   -stderr: The stadard error stream. Sourced from <stdio.h>.

   -DEFAULT_DB_ENVIRONMENT: Constant string (defined via macro in "cache.h") 
                            with the default path of the object store.

   -strlen(string): Return the length of `string` in bytes.

   -errno: Error number of last error. Sourced from <errno.h>.

   -EEXIST: Error macro indicating that an existing file was specified in a 
            context where it only makes sense to specify a new file. Sourced 
            from <errno.h>.

   -malloc(size): Allocate unused space for an object whose size in bytes is 
                  specified by `size` and whose value is unspecified. Sourced 
                  from <stdlib.h>.

   -memcpy(s1, s2, n): Copy n bytes from the object pointed to by s2 into the 
                       object pointed to by s1. Sourced from <string.h>.

   -sprintf(s, message, ...): Writes `message` string constant to string 
                              variable `s` followed by the null character 
                              '\0'. Sourced from <stdio.h>.

   ****************************************************************

   The following variables and functions are defined in this source file:

   -main(argc, argv): The main function which runs each time the init-db 
                      command is run.

   -argc: The number of command line arguments supplied when executing 
          init-db.

   -argv: Array containing command line argument strings.

   -sha1_dir: The path to the object store.

   -path: A dynamic character string that is used to build the path to each 
          subdirectory in the object store, i.e., one subdirectory for each 
          number between 0 and 255 in hexadecimal. The name of each 
          subdirectory will represent the first two digits of the SHA1 hashes
          of the objects stored in that subdirectory.

   -len: The length of the constant string in the `sha1_dir` variable.

   -i: For loop counter used to create subdirectories in object store.

   -fd: Declared but not used. Linus Torvalds is mortal too :D.

   -st: `stat` structure used to store file information obtained from `stat()` 
        function call.
*/

/*
 * Function: `main`
 * Parameters:
 *      -argc: The number of command-line arguments supplied, inluding the 
 *             command itself.
 *      -argv: An array of the command line arguments, including the command 
 *             itself.
 * Purpose: Standard `main` function definition. Runs when the executable 
 *          `init-db` is run from the command line.
 */
int main(int argc, char **argv)
{
    /* 
     * The `char *` format of the variables below allows them to be used as 
     * strings of characterss instead of just holding one single character. 
     * Just think of these as strings.
     */
    char *sha1_dir, *path;

    /* Declaring three integers to be used later. */
    int len, i, fd;

    /*
     * Attempt to create a directory called `.dircache` in the current 
     * directory. If it fails, `mkdir()` will return -1 and the program will 
     * print an error message and exit.
     */
    if (MKDIR(".dircache") < 0) {
        perror("unable to create .dircache");
        exit(1);
    }
    
    /*
     * Set `sha1_dir` (i.e. the path to the object store) to the value of the
     * `DB_ENVIRONMENT` environment variable, which defaults to 
     * `SHA1_FILE_DIRECTORY` as defined in "cache.h". If the environment 
     * variable is not defined (and it most likely won't be), getenv() will 
     * return a null pointer. 
     */
    sha1_dir = getenv(DB_ENVIRONMENT);

    /*
     * This code block will only be executed if `sha1_dir` is NOT null, i.e., 
     * if the environment variable above was defined.
     */
    if (sha1_dir) {
        struct stat st;
        if (!(stat(sha1_dir, &st) < 0) && S_ISDIR(st.st_mode))
            return 1;
        fprintf(stderr, "DB_ENVIRONMENT set to bad directory %s: ", sha1_dir);
    }

    /*
     * Fall through here if a custom object store path was not specified or
     * was not valid. 
     */

    /*
     * Set `sha1_dir` to the default value `.dircache/objects` as defined in 
     * "cache.h", then print a message to the screen conveying this.
     */
    sha1_dir = DEFAULT_DB_ENVIRONMENT;
    fprintf(stderr, "defaulting to private storage area\n");

    /*
     * Set `len` to the length of the string in `sha1_dir`, i.e., the length
     * of the string `.dircache/objects`. This will be used later to build the 
     * subdirectories in the object database, where hash-indexed objects will 
     * be stored.
     */
    len = strlen(sha1_dir);

    /*
     * Attempt to create a directory inside `.dircache` called `objects`. If 
     * it fails, `mkdir()` will return `-1` and the program will print a 
     * message and exit.
     */
    if (MKDIR(sha1_dir) < 0) {
        if (errno != EEXIST) {
            perror(sha1_dir);
            exit(1);
        }
    }

    /*
     * Allocate space for `path` with size len` (size in bytes of `sha1_dir`) 
     + + 40 bytes.
     */
    path = malloc(len + 40);

    /* Copy the `sha1_dir` to `path`. */
    memcpy(path, sha1_dir, len);

    /*
     * Execute this loop 256 times to create the 256 subdirectories inside the 
     * `.dircache/objects/` directory. The subdirectories will be named `00`
     * to `ff`, which are the hexadecimal representations of the numbers 0 to 
     * 255. Each subdirectory will be used to hold the objects whose SHA1 hash 
     * values in hexadecimal representation start with those two digits.
     */
    for (i = 0; i < 256; i++) {
        /*
         * Convert `i` to a two-digit hexadecimal number and append it to the 
         * path variable after the `.dircache/objects/` part. That way, each 
         * time through the loop we build up one of the following paths: 
         * `.dircache/objects/00`, `.dircache/objects/01`, ...,
         * `.dircache/objects/fe`, `.dircache/objects/ff`.
         */
        sprintf(path+len, "/%02x", i);

        /*
         * Attempt to create the current subdirectory. If it fails, `mkdir()` 
         * will return -1 and the program will print a message and exit.
         */
        if (MKDIR(path) < 0) {
            if (errno != EEXIST) {
                perror(path);
                exit(1);
            }
        }
    }
    return 0;
}

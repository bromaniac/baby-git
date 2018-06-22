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
/* The above 'include' allows use of the following functions and
   variables from "cache.h" header file, ranked in order of first use
   in this file. Most are functions/macros from standard C libraries
   that are `#included` in "cache.h". Function names are followed by
   parenthesis whereas variable/struct names are not:

   -sha1_file_name(): Build the path of an object in the object database
                      using the object's SHA1 hash value.

   -access(path, amode): Check whether the file at `path` can be accessed
                         according to the permissions specified in `amode`.

   -perror(message): Write `message` to standard error output stream. Sourced
                     from <stdio.h>.

   -read_cache(): Read the contents of the `.dircache/index` file into the
                  `active_cache` array. The number of caches entries is 
                  returned.

   -fprintf(stream, message, ...): Write `message` to the output `stream`. 
                                   Sourced from <stdio.h>.

   -exit(status): Stop execution of the program and exit with code `status`.
                  Sourced from <stdlib.h>.

   -malloc(size): Allocate unused space for an object whose size in bytes is 
                  specified by `size` and whose value is unspecified. Sourced 
                  from <stdlib.h>.

   -alloc_nr(x): This is a macro in "cache.h" that's used to calculate the
                 maximum number of elements to allocate to the active_cache 
                 array.

   -realloc(pointer, size): Update the size of the memory object pointed to by
                            `pointer` to `size`. Sourced from <stdlib.h>.

   -sprintf(s, message, ...): Writes `message` string constant to string 
                              variable `s` followed by the null character 
                              '\0'. Sourced from <stdio.h>.

   -memcpy(s1, s2, n): Copy n bytes from the object pointed to by s2 into the 
                       object pointed to by s1.

   -write_sha1_file(): Deflate an object, calculate the hash value, then call
                       the write_sha1_buffer function to write the deflated
                       object to the object database.

   ****************************************************************

   The following variables and functions are defined in this source file.

   -main(): The main function runs each time the ./write-tree command is run.

   -check_valid_sha1(): Check if user-supplied SHA1 hash corresponds to an
                        object in the object database.

   -prpend_integer(): Prepend a string containing the decimal form of the size 
                      of the tree data in bytes to the buffer.

   -ORIG_OFFSET: Token that defines the number of bytes at the beginning of 
                 the buffer that are allocated for the object tag and the 
                 object data size.
*/

/*
 * Function: `check_valid_sha1`
 * Parameters:
 *      -sha1: An SHA1 hash to check.
 * Purpose: Check if user-supplied SHA1 hash corresponds to an object in the 
 *          object database and if the process has read access to it.
 */
static int check_valid_sha1(unsigned char *sha1)
{
    /*
     * Build the path of an object in the object database using the object's 
     * SHA1 hash value.
     */
    char *filename = sha1_file_name(sha1);
    int ret;   /* Return code. */

    /*
     * Check whether the process has read access to the object in the object 
     * database. 
     */
    ret = access(filename, R_OK);

    /* Error if the file is not accessible. */
    if (ret)
        perror(filename);

    return ret;
}

/*
 * Function: `prepend_integer`
 * Parameters:
 *      -buffer: Pointer to buffer that holds the tree data.
 *      -val: Size in bytes of the tree data.
 *      -i: Number of bytes at the beginning of `buffer` that are allocated 
 *          for the object tag and object data size.
 * Purpose: Prepend a string containing the decimal form of the size of the 
 *          tree data in bytes to the buffer.
 */
static int prepend_integer(char *buffer, unsigned val, int i)
{
    /* Prepend a null character to the tree data in the buffer. */
    buffer[--i] = '\0';

    /*
     * Prepend a string containing the decimal form of the size of the tree 
     * data in bytes before the null character.
     */
    do {
        buffer[--i] = '0' + (val % 10);
        val /= 10;
    } while (val);
    /*
     * The value of `i` is now the index of the buffer element that contains 
     * the most significant digit of the decimal form of the tree data size.
     */
    return i;
}

/* Linus Torvalds: Enough space to add the header of "tree <size>\0" */
#define ORIG_OFFSET (40)

/*
 * Function: `main`
 * Parameters:
 *      -argc: The number of command-line arguments supplied, inluding the 
 *             command itself. 
 *      -argv: An array of the command line arguments, including the command 
 *             itself.
 * Purpose: Standard `main` function definition. Runs when the executable 
 *          `write-tree` is run from the command line. 
 */
int main(int argc, char **argv)
{
    /* The size to be allocated to the buffer. */
    unsigned long size;
    /* Index of the buffer element to be filled next. */
    unsigned long offset;
    /* Not used. Even Linus Torvalds makes mistakes. */
    unsigned long val;
    /* Iterator used in for loop. */
    int i;
    /*
     * Read in the contents of the `.dircache/index` file into the 
     * `active_cache` array. The number of cache entries is returned and 
     * stored in `entries`.
     */
    int entries = read_cache();

    /* String to hold the tree's content. */
    char *buffer;

    /*
     * If there are no active cache entries or if there was an error reading
     * the cache, display an error message and exit since there is nothing to 
     * write to a tree.
     */
    if (entries <= 0) {
        fprintf(stderr, "No file-cache to create a tree of\n");
        exit(1);
    }

    /* Linus Torvalds: Guess at an initial size */
    size = entries * 40 + 400;
    /* Allocate `size` bytes to buffer to store the tree content. */
    buffer = malloc(size);
    /*
     * Set the offset index using the macro defined in this file. The tree
     * data will be written starting at this offset.  The tree metadata will
     * be written before it.
     */
    offset = ORIG_OFFSET;

    /*
     * Loop over each cache entry and build the tree object by adding the 
     * data from the cache entry to the buffer.
     */
    for (i = 0; i < entries; i++) {
        /* Pick out the ith cache entry from the active_cache array. */
        struct cache_entry *ce = active_cache[i];

        /* Check if the cache entry's SHA1 hash is valid. Otherwise, exit. */
        if (check_valid_sha1(ce->sha1) < 0)
            exit(1);

        /* If needed, increase the size of the buffer. */
        if (offset + ce->namelen + 60 > size) {
            size = alloc_nr(offset + ce->namelen + 60);
            buffer = realloc(buffer, size);
        }

        /*
         * Write the cache entry's file mode and name to the buffer and
         * increment `offset` by the number of characters that were written.
         */
        offset += sprintf(buffer + offset, "%o %s", ce->st_mode, ce->name);

        /*
         * Write a null character to the buffer as a separator and increment
         * `offset`. 
         */
        buffer[offset++] = 0;

        /* Add the cache entry's SHA1 hash to the buffer. */
        memcpy(buffer + offset, ce->sha1, 20);

        /*
         * Increment the offset by 20 bytes, the length of an SHA1 hash.
         */
        offset += 20;
    }

    /*
     * Prepend a string containing the decimal form of the size of the tree 
     * data in bytes to the buffer.
     */
    i = prepend_integer(buffer, offset - ORIG_OFFSET, ORIG_OFFSET);
    /*
     * Prepend the string `tree ` to the buffer to identify this object as a 
     * tree in the object store.
     */
    i -= 5;
    memcpy(buffer+i, "tree ", 5);

    /*
     * Adjust buffer to start at the first character of the `tree` object 
     * tag. 
     */
    buffer += i;
    /* Calculate final total size of this buffer. */
    offset -= i;

    /* Compress the contents of buffer, calculate SHA1 hash of compressed 
     * output, and write the tree object to the object store. 
     */
    write_sha1_file(buffer, offset);

    /* Return success. */
    return 0;
}

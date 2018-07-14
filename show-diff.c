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
 *  called `show-diff`. When `show-diff` is run from the command line
 *  it does not take any command line arguments.
 *
 *  The `show-diff` command is used to show the differences between
 *  files staged in the index and the current versions of those files
 *  as they exist in the filesystem.
 *
 *  Everything in the main function in this file will run
 *  when ./show-diff executable is run from the command line.
 */

#include "cache.h"
/* The above 'include' allows use of the following functions and
   variables from "cache.h" header file, ranked in order of first use
   in this file. Most are functions/macros from standard C libraries
   that are `#included` in "cache.h". Function names are followed by
   parenthesis whereas variable/struct names are not:

   -snprintf(char *s, size_t n, const char *format, ...):
        Write the formatted string `format` into `s`, with `n` specifying the 
        maximum number of characters that should be written to `s`, including 
        the null terminating character.

   -popen(command, mode): Create a pipe between the calling program and a 
                          shell command and return a pointer to a stream that 
                          enables read or write access to the command 
                          depending on the value of mode. Sourced from 
                          <stdio.h>.

   -fwrite(data, size, nitems, stream): Write up to `nitems`, each of size 
                                        `size`, from array `data` to `stream`.
                                        Sourced from <stdio.h>.

   -pclose(stream): Close a stream that was opened by popen(). Sourced from 
                    <stdio.h>.

   -read_cache(): Reads the contents of the `.dircache/index` file into the 
                  `active_cache` array. 

   -printf(message, ...): Write `message` to standard output stream stdout.  
                          Sourced from <stdio.h>.

   -free(ptr): Deallocates the space pointed to by `ptr`. Sourced from 
               <stdlib.h>.
*/

#define MTIME_CHANGED   0x0001
#define CTIME_CHANGED   0x0002
#define OWNER_CHANGED   0x0004
#define MODE_CHANGED    0x0008
#define INODE_CHANGED   0x0010
#define DATA_CHANGED    0x0020

/*
 * Function: `match_stat`
 * Parameters:
 *      -ce: Pointer to a cache entry structure.
 *      -st: Pointer to a stat structure containing metadata of the working 
 *           file that corresponds to the cache entry. 
 * Purpose: Compare metadata stored in a cache entry to the metadata of the 
 *          corresponding working file to check if they are the same or if
 *          anything changed.
 */
static int match_stat(struct cache_entry *ce, struct stat *st)
{
    /* Flag to indicate which file metadata changed, if any. */
    unsigned int changed = 0;

    /*
     * Compare metadata stored in a cache entry to those of the corresponding
     * working file to check if they are the same.
     */

    /* Check last modification time. */
    if (ce->mtime.sec  != (unsigned int)STAT_TIME_SEC( st, st_mtim ) ||
        ce->mtime.nsec != (unsigned int)STAT_TIME_NSEC( st, st_mtim ))
            changed |= MTIME_CHANGED;
    /* Check time of last status change. */
    if (ce->ctime.sec  != (unsigned int)STAT_TIME_SEC( st, st_ctim ) ||
        ce->ctime.nsec != (unsigned int)STAT_TIME_NSEC( st, st_ctim ))
            changed |= CTIME_CHANGED;

    /* Check file user ID and group ID. */
    if (ce->st_uid != (unsigned int)st->st_uid ||
        ce->st_gid != (unsigned int)st->st_gid)
        changed |= OWNER_CHANGED;
    /* Check file mode. */
    if (ce->st_mode != (unsigned int)st->st_mode)
        changed |= MODE_CHANGED;
    #ifndef BGIT_WINDOWS
    /* Check device ID and file inode number. */
    if (ce->st_dev != (unsigned int)st->st_dev ||
        ce->st_ino != (unsigned int)st->st_ino)
        changed |= INODE_CHANGED;
    #endif
    /* Check file size. */
    if (ce->st_size != (unsigned int)st->st_size)
        changed |= DATA_CHANGED;
    return changed;
}

/*
 * Function: `show_differences`
 * Parameters:
 *      -ce: Pointer to a cache entry structure.
 *      -cur: Pointer to a stat structure containing metadata of the working 
 *            file that corresponds to the cache entry. 
 *      -old_contents: The blob data corresponding to the cache entry.
 *      -old_size: The size of the blob data in bytes.
 * Purpose: Use the diff shell command to display the differences between the 
 *          blob data corresponding to the cache entry and the contents of the 
 *          corresponding working file.
 */
static void show_differences(struct cache_entry *ce, struct stat *cur,
                             void *old_contents, unsigned long long old_size)
{
    static char cmd[1000];   /* String to store the diff command. */
    FILE *f;                 /* Declare a file pointer. */

    /*
     * Construct the diff command for this cache entry, which will be used to 
     * display the differences between the blob data corresponding to the
     * cache entry and the contents of the corresponding working file.
     * Store the command string in `cmd`. 
     */
    snprintf(cmd, sizeof(cmd), "diff --strip-trailing-cr -u - %s", ce->name);

    /*
     * Create a pipe to the diff command and return a pointer to the
     * corresponding stream for writing. 
     */
    f = popen(cmd, "w");

    /*
     * Write the blob object data corresponding to the current cache entry to 
     * the command stream to complete the command, thus effectively executing 
     * the diff command.
     */
    fwrite(old_contents, old_size, 1, f);

    /* Close the command stream. */
    pclose(f);
}

/*
 * Function: `main`
 * Parameters:
 *      -argc: The number of command-line arguments supplied, inluding the 
 *             command itself. 
 *      -argv: An array of the command line arguments, including the command 
 *             itself.
 * Purpose: Standard `main` function definition. Runs when the executable 
 *          `show-diff` is run from the command line. 
 */
int main(int argc, char **argv)
{
    /*
     * Reads the contents of the `.dircache/index` file into the 
     * `active_cache` array and returns the number of cache entries.
     */
    int entries = read_cache();

    /* For loop counter. */
    int i;

    /*
     * If there was an error reading the cache, display an error message and 
     * exit. 
     */
    if (entries < 0) {
        perror("read_cache");
        exit(1);
    }

    /* Loop through the cache entries in the active_cache array. */
    for (i = 0; i < entries; i++) {
        /* Declare a stat structure to store file metadata. */
        struct stat st;
        /* The current cache entry. */
        struct cache_entry *ce = active_cache[i];
        /* For loop counter. */
        int n;
        /* Flag to indicate which file metadata changed, if any. */
        int changed;
        /* Not used. */
        unsigned int mode;
        /* Blob object data size. */
        unsigned long size;
        /* Used to store the object type (blob in this case ). */
        char type[20];
        /* Used to store the blob object data. */
        void *new;

        /*
         * Use the stat() function to obtain information about the working 
         * file corresponding to the current cache entry and store it in the 
         * `st` stat structure. If the stat() call fails, display an error
         * message and continue to the next cache entry.
         */
        if (stat(ce->name, &st) < 0) {
            printf("%s: %s\n", ce->name, strerror(errno));
            continue;
        }

        /*
         * Compare the metadata stored in the cache entry to those of the 
         * corresponding working file to check if they are the same or if
         * anything changed. 
         */
        changed = match_stat(ce, &st);

        /*
         * If no metadata changed, display an ok message and continue to the 
         * next cache entry in the active_cache array. 
         */
        if (!changed) {
            printf("%s: ok\n", ce->name);
            continue;
        }

        /* Fall through here if any metadata changed. */

        /*
         * Display the path of the file corresponding to the current cache
         * entry.
         */
        printf("%.*s:  ", ce->namelen, ce->name);

        /*
         * Display the hexadecimal representation of the SHA1 hash of the blob 
         * object corresponding to the current cache entry. 
         */
        for (n = 0; n < 20; n++)
            printf("%02x", ce->sha1[n]);

        printf("\n");   /* Print a newline. */

        /*
         * Read the blob object from the object store using its SHA1 hash,
         * inflate it, and return a pointer to the object data (without the 
         * prepended metadata). Store the object type and object data size in 
         * `type` and `size` respectively.
         */
        new = read_sha1_file(ce->sha1, type, &size);

        /*
         * Use the diff shell command to display the differences between the 
         * blob data corresponding to the current cache entry and the contents 
         * of the corresponding working file.
         */
        show_differences(ce, &st, new, size);

        /* Deallocate the space pointed to by `new`. */
        free(new);
    }
    return 0;
}

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

/*  The above 'include' allows use of the following functions and
    variables from <cache.h> header file, ranked in order of first use
    in this file. Most are functions/macros from standard C libraries
    that are `#included` in <cache.h>. Function names are followed by
    parenthesis whereas variable/struct names are not:

	-snprintf(s, n, f): Add `n` bytes of format string `f` to memory
                        location identified by `s`. Sourced from <stdio.h>.

    -popen(command, mode): Execute the command specified by the string `command`.
                           It shall create a pipe between the calling program and
                           the executed command, and shall return a pointer to a
                           stream that can be used to either read from or write to
                           the pipe, depending on the value of `mode`. Sourced from
						   <stdio.h>.

    -fwrite(ptr, size, nitems, stream): Write, from the array pointed to by ptr,
                                        up to nitems elements whose size is specified
                                        by size, to the stream pointed to by stream.
										Sourced from <stdio.h>.

    -pclose(stream): Close a stream that was opened by popen(). Sourced from <stdio.h>.

	-read_cache(): Read in the contents of the `.dircache/index` file into
                   the `active_cache`. The number of caches entries is returned.

    -printf(message): Print message to standard output stdout.
                      Sourced from <stdio.h>.

	-free(ptr): Cause the space pointed to by `ptr` to be deallocated. Sourced from <stdlib.h>.

*/

#define MTIME_CHANGED	0x0001
#define CTIME_CHANGED	0x0002
#define OWNER_CHANGED	0x0004
#define MODE_CHANGED    0x0008
#define INODE_CHANGED   0x0010
#define DATA_CHANGED    0x0020

/*
 * Function: `match_stat`
 * Parameters:
 *      -ce: The cache entry to compare contents of to the file in the working directory.
 *		-st: The stat object of the working directory file to compare to the cached version.
 * Purpose: To check whether a previously staged file has been changed in the working directory.
 */
static int match_stat(struct cache_entry *ce, struct stat *st)
{
    /* This is used to track whether the cached file matches the current file in the working directory. */
	unsigned int changed = 0;

    /* Compare each cached file parameter to those in working directory file, to determine whether files match.  */
	if (ce->mtime.sec  != (unsigned int)st->st_mtimespec.tv_sec ||
	    ce->mtime.nsec != (unsigned int)st->st_mtimespec.tv_nsec)
		changed |= MTIME_CHANGED;
	if (ce->ctime.sec  != (unsigned int)st->st_ctimespec.tv_sec ||
	    ce->ctime.nsec != (unsigned int)st->st_ctimespec.tv_nsec)
		changed |= CTIME_CHANGED;
	if (ce->st_uid != (unsigned int)st->st_uid ||
	    ce->st_gid != (unsigned int)st->st_gid)
		changed |= OWNER_CHANGED;
	if (ce->st_mode != (unsigned int)st->st_mode)
		changed |= MODE_CHANGED;
	if (ce->st_dev != (unsigned int)st->st_dev ||
	    ce->st_ino != (unsigned int)st->st_ino)
		changed |= INODE_CHANGED;
	if (ce->st_size != (unsigned int)st->st_size)
		changed |= DATA_CHANGED;
	return changed;
}

/*
 * Function: `show_differences`
 * Parameters:
 *      -ce: The cache entry to compare contents of to the file in the working directory.
 *      -st: The stat object of the working directory file to compare to the cached version.
 * Purpose: Generate the diff between the staged version of the file and the version in the
 * 			working directory.
 */
static void show_differences(struct cache_entry *ce, struct stat *cur,
	void *old_contents, unsigned long long old_size)
{
    /* String to store the diff command which will be built up using the file name. */
	static char cmd[1000];

    /* Declare a FILE object. */
	FILE *f;

    /* Build up the diff command and store it in `cmd`. */
	snprintf(cmd, sizeof(cmd), "diff -u - %s", ce->name);

    /* Run the diff command and return as `f` pointer to open stream. */
	f = popen(cmd, "w");

    /* Write `old_contents` to the open stream referred to by `f`. */
	fwrite(old_contents, old_size, 1, f);

    /* Close the stream. */
	pclose(f);
}

/*
 * Function: `main`
 * Parameters:
 *      -argc: The number of command-line arguments supplied, inluding the command itself. 
 *      -argv: An array of the command line arguments, including the command itself.
 * Purpose: Standard `main` function definition. Runs when the executable `show-diff` is
 *          run from the command line. 
 */
int main(int argc, char **argv)
{
	/*  
     * Read in the contents of the `.dircache/index` file into the `active_cache`.
     * The number of caches entries is stored in `entries`.
     */
	int entries = read_cache();

	/* For loop counter. */
	int i;

	/*  
     * If there are no active cache entries, throw an error message since there is
     * nothing staged in the index to commit.
     */
	if (entries < 0) {
		perror("read_cache");
		exit(1);
	}

	/* Iterate through of cache entries in the active cache. */
	for (i = 0; i < entries; i++) {

		/* Declare stat to store file metadata. */
		struct stat st;

		/* Pick out the ith cache_entry inthe active cache. */
		struct cache_entry *ce = active_cache[i];

		/* For loop counter. */
		int n;

		/* Used to specify whether working directory file has changed from cached version. */
		int changed;

		/* File mode (permissions). */
		unsigned int mode;

		/* File size. */
		unsigned long size;

		/* Used to store the `type` of the working directory file (`blob` or `tree`). */
		char type[20];

		/* Used to store content of working directory file. */
		void *new;

		/*
		 * Print error message if the cached file no longer exists in the working directory.
		 * Move on to next cache entry in the active cache.
		 */
		if (stat(ce->name, &st) < 0) {
			printf("%s: %s\n", ce->name, strerror(errno));
			continue;
		}

		/* Determine whether or not the working directory file is different from cached version. */
		changed = match_stat(ce, &st);

		/* If not changed, print OK message and move on to next cache entry in active cache. */
		if (!changed) {
			printf("%s: ok\n", ce->name);
			continue;
		}

		/* Print out the name of the changed file and it's length. */
		printf("%.*s:  ", ce->namelen, ce->name);

		/* Print out the SHA1 of the changed file. */
		for (n = 0; n < 20; n++)
			printf("%02x", ce->sha1[n]);

		/* Print a newline. */
		printf("\n");

		/* Read in the working directory file contents. */
		new = read_sha1_file(ce->sha1, type, &size);

		/* Compare the cache entry to the working directory version of the file. */
		show_differences(ce, &st, new, size);

		/* Free up the memory used by `new`. */
		free(new);
	}
	return 0;
}

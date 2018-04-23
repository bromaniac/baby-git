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
 *  called `update-cache`. This command is the equivalent to `git
 *	add` in current versions of Git. When `update-cache` is run
 *	from the command-line it will take as an argument the name of
 *  a file to be added to the index (or as Mr. Torvalds calls it
 *  in the README, the "Current Directory Cache"), which is stored 
 *	in the .dircache/index file by default. This can be thought of
 *  as the "Staging Area" where changes ready to be committed are
 *  built up.
 *
 *  The `main` function in this file will run when ./update-cache
 *  executable is run from the command line.
 */

#include "cache.h"
/*  The above 'include' allows use of the following functions and
    variables from <cache.h> header file, ranked in order of first use
    in this file. Most are functions/macros from standard C libraries
    that are `#included` in <cache.h>. Function names are followed by
    parenthesis whereas variable/struct names are not:

    -memcmp(str1, str2, n): Compare first n bytes of str1 to str2
							and return an integer greater than, equal
							to, or less than 0, if the object pointed
							to by s1 is greater than, equal to, or less
							than the object pointed to by s2, respectively.
                            Sourced from 

	-active_nr: The number of entries in the active cache/index.

	-cache_entry: Structure representing the a single cached/staged file.

	-active_cache: Cache_Entry representing the active cache/index.

	-memmove(str1, str2, n): The memmove() function shall copy n bytes from
							 the object pointed to by s2 into the object
							 pointed to by s1.

	-sizeof(datatype): Generates the size of a variable or datatype,
					   measured in the number of char size storage
					   units required for the type.

	-z_stream: General purpose compression/decompression stream. Sourced
               from <zlib.h>.

	-malloc(size): Allocate unused space for an object whose
                   size in bytes is specified by `size` and whose
                   value is unspecified. Sourced from <stdlib.h>.

	-mmap(addr, len, prot, flags,
        file_descriptor, offset): Establish a mapping between a process'
                                  address space and a file, shared memory
                                  object, or typed memory object. Sourced
                                  from <sys/mman.h>.

	-PROT_READ: Flag for the mmap() function `prot` parameter describing
                the desired memory protection of the mapping. `PROT_READ`
                indicates that pages may be read. Sourced from <sys/mman.h>.
                
	-MAP_PRIVATE: Flag for the mmap() function `prot` parameter describing
                  the desired memory protection of the mapping. `MAP_PRIVATE`
                  indicates a private copy-on-write mapping. Sourced from
                  <sys/mman.h>.

	-SHA_CTX: SHA context struct used to store information related to the
              process of hashing the content. Sourced from <sha.h>.

	-close(file_descriptor): Deallocate the file descriptor. The fd will be
                             made available to subsequent called to open()
                             or other calls that allocate fd's. Remove all
                             locks owned by the process on the file
                             associated with the fd. Sourced from 

	-memset(void *s, int c, size_t n): Copies `c` (converted to an unsigned
                                       char) into each of the first `n` bytes
                                       of the object pointed to by `s`.

	-deflateInit(z_stream, level): Initializes the internal `z_stream` state
                                   for compression at `level` which indicates
                                   scale of speed vs compression on a scale from
                                   0-9. Sourced from <zlib.h>.

	-Z_BEST_COMPRESSION: Translates to integer `9` which as an input to
                         `deflateInit()` indicates to optimize compressed
                         size as opposed to compression speed. Sourced from
                         <zlib.h>.

    -sprintf(s, message): Place output followed by the null byte, '\0', in
						  consecutive bytes starting at *s. Sourced from
						  <stdio.h>.

	-deflate(z_stream, flush): Compresses as much data as possible, and stops
							   when the input buffer becomes empty or the
							   output buffer becomes full. Sourced from <zlib.h>.

	-Z_OK: Successful zlib return code. It is equal to 0. Sourced from <zlib.h>.

	-Z_FINISH: Flush value for zlib. It is equal to 4. Sourced from <zlib.h>.

	-deflateEnd(z_stream): All dynamically allocated data structures for
						   `z_stream` are freed. Sourced from <zlib.h>.

	-SHA1_Init(): Initializes a SHA_CTX structure. Sourced from <sha.h>. 

	-SHA1_Update(): Can be called repeatedly with chunks of the message to be
					hashed (len bytes at data). Sourced from <sha.h>.

	-SHA1_Final(): Places the message digest in md, which must have space for
				   SHA_DIGEST_LENGTH == 20 bytes of output, and erases the
				   SHA_CTX. Sourced from <sha.h>.

    -stat: Structure to be returned by stat() function. Holds info
           related to a filesystem file. Sourced from <sys/stat.h>.

	-open(path): Establishes the connection between a file and a file descriptor.
                 It creates an open file description that refers to a file and a
                 file descriptor that refers to that open file description. The
                 file descriptor is used by other I/O functions to refer to that
                 file. The `path` argument points to a pathname naming the file.
                 Upon successful completion, the function shall open the file and
                 return a non-negative integer representing the lowest numbered
                 unused file descriptor. Otherwise, -1 shall be returned and errno
                 set to indicate the error. No files shall be created or modified
                 if the function returns -1. Sourced from <fcntl.h>.

	-O_RDONLY: Flag for the open() function indicating to open the file for
               reading only. Sourced from <fcntl.h>.

	-ENOENT: Failure code set as errno by the open() function when O_CREAT is not
             set and the named file does not exist; or O_CREAT is set and either
             the path prefix does not exist or the path argument points to an empty
             string. Sourced from <fcntl.h>.

	-fstat(fd, buf): Obtain information about an open file associated with the file
                     descriptor `fd`, and shall write it to the area pointed to by
                     `buf` which is a `stat` struct. Sourced from <sys/stat.h>.

    -strlen(string): Return the length of `string` in bytes.

    -memcpy(s1, s2, n): Copy n bytes from the object pointed to
                        by s2 into the object pointed to by s1.

	-cache_header: Struct representing header information for cache entries. Sourced
                   from <cache.h>.

	-CACHE_SIGNATURE: #Defined as `0x44495243`. Sourced from <cache.h>.

	-offsetof(type, member): Macro that expands to an integral constant expression
                             of type std::size_t, the value of which is the offset,
                             in bytes, from the beginning of an object of specified
                             type to its specified member, including padding if any.
                             Sourced from <stddef.h>.

    -perror(message): Write `message` to standard error output.
                      Sourced from <stdio.h>.

	-O_RDWR: Flag for the open() function indicating to open the file for
			 reading and writing. Sourced from <fcntl.h>.

	-O_CREAT: Flag for the open() function indicating if the file exists,
			  this flag has no effect except as noted under O_EXCL below.
			  Otherwise, the file shall be created. Sourced from <fcntl.h>.

	-O_EXCL: Flag for the open() function indicating if O_CREAT and O_EXCL
			 are set, open() shall fail if the file exists.

    -fprintf(stream, message): Place `message` on the named output
                               `stream`. Sourced from <stdio.h>.

	-rename(old, new): Change the name of a file. `old` points to
                       the pathname of the file to be renamed. `new`
                       points to the new pathname of the file. Sourced
                       from <stdio.h>.

    ****************************************************************

    The following variables and functions are defined locally.

    -main(argc, argv): The main function which runs each time the
                       ./cat-file command is run.

    -argc: The number of command line arguments supplied when
           executing ./cat-file.

    -argv: Array containing command line argument strings.

    -index_fd(): Compress file's contents and add to index.

*/

#ifndef BGIT_WINDOWS
    #define RENAME( src_file, target_file ) rename( src_file, target_file )
    #define RENAME_FAIL -1 
#else
    #define RENAME( src_file, target_file ) MoveFileEx( src_file, \
                                                target_file, \
                                                MOVEFILE_REPLACE_EXISTING )
    #define RENAME_FAIL 0 
#endif

/*
 * Function: `cache_name_compare`
 * Parameters:
 *      -name1: The name of the first file to compare.
 *      -len1: The length of name1.
 *      -name2: The name of the second file to compare.
 *      -len2: The length of name2.
 * Purpose: Determine whether or not 2 passed in cache entry names are the same.
 */
static int cache_name_compare(const char *name1, int len1, const char *name2, int len2)
{
	int len = len1 < len2 ? len1 : len2;
	int cmp;

	cmp = memcmp(name1, name2, len);
	if (cmp)
		return cmp;
	if (len1 < len2)
		return -1;
	if (len1 > len2)
		return 1;
	return 0;
}

/*
 * Function: `cache_name_pos`
 * Parameters:
 *      -name: The name of the file to be cached.
 *      -namelen: Then length of the name.
 * Purpose: Determine the position of particular cache entry in the active cache.
 */
static int cache_name_pos(const char *name, int namelen)
{
	/*
     * Declare and initialize the first and last entry indexes in the cache.
     */
    int first, last;
	first = 0;
	last = active_nr;

    /*
     * Compare passed in filename to each filename in the active cache and
     * if a match is found, return the position of the match active cache entry.
     */
	while (last > first) {
		int next = (last + first) >> 1;
		struct cache_entry *ce = active_cache[next];
		int cmp = cache_name_compare(name, namelen, ce->name, ce->namelen);
		if (!cmp)
			return -next-1;
		if (cmp < 0) {
			last = next;
			continue;
		}
		first = next+1;
	}
	return first;
}

/*
 * Function: `remove_file_from_cache`
 * Parameters:
 *      -path: The path/filename of the file to remove from the active cache.
 * Purpose: Remove file at `path` from the active cache.
 */
static int remove_file_from_cache(char *path)
{
	int pos = cache_name_pos(path, strlen(path));
	if (pos < 0) {
		pos = -pos-1;
		active_nr--;
		if (pos < active_nr)
			memmove(active_cache + pos, active_cache + pos + 1, (active_nr - pos - 1) * sizeof(struct cache_entry *));
	}
}

/*
 * Function: `add_cache_entry`
 * Parameters:
 *      -ce: The cache_entry to be added to the `active_cache`.
 * Purpose: Add a cache entry to `active_cache`.
 */
static int add_cache_entry(struct cache_entry *ce)
{
	int pos; /* The position of the cache_entry when compared to the active cache. */
	pos = cache_name_pos(ce->name, ce->namelen);

	/* Linus Torvalds: existing match? Just replace it */
	if (pos < 0) {
		active_cache[-pos-1] = ce;
		return 0;
	}

	/* Make sure the `active_cache` array is big enough to hold all entries. */
	if (active_nr == active_alloc) {
		active_alloc = alloc_nr(active_alloc);
		active_cache = realloc(active_cache, active_alloc * sizeof(struct cache_entry *));
	}

	/* Add the new cache_entry into the active_cache. */
	active_nr++;
	if (active_nr > pos)
		memmove(active_cache + pos + 1, active_cache + pos, (active_nr - pos - 1) * sizeof(ce));
	active_cache[pos] = ce;
	return 0;
}

/*
 * Function: `index_fd`
 * Parameters:
 *      -path: The path of the file to add to the object store.
 *      -namelen: The length of the path/filename to add.
 *      -ce: The cache_entry to set the SHA1 for.
 *      -fd: The file descriptor associated with the file to be added.
 *      -st: The `stat` object containing info about the file to be added.
 * Purpose: Compress a file's content, hash it, and add it to the object store.
 */ 
static int index_fd(const char *path, int namelen, struct cache_entry *ce, int fd, struct stat *st)
{
	z_stream stream; /* Declare zlib compression stream. */
	int max_out_bytes = namelen + st->st_size + 200; /* The max # of bytes to be compressed. */
	void *out = malloc(max_out_bytes); /* Allocate `max_out_bytes` space to `out` variable. */
	void *metadata = malloc(namelen + 200); /* Allocate space for file metadata. */

    /*
     * Set up memory location to store the contents of the file to be added to cache.
     */
	void *in = mmap(NULL, st->st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	SHA_CTX c; /* Declate a SHA context variable. */

	close(fd); /* Release the file descriptor `fd` since we no longer need it. */

	if (!out || (int)(long)in == -1) /* Return -1 (failure) if `out` or `in` setup failed. */
		return -1;

    /*
     * Allocate `sizeof(stream)`'s worth of unsigned char space to the compression stream.
     */
	memset(&stream, 0, sizeof(stream));

    /* Initialize compression stream optimized for compression (as opposed to speed). */
	deflateInit(&stream, Z_BEST_COMPRESSION);

	/*
	 * Linus Torvalds: ASCII size + nul byte
	 */	
	stream.next_in = metadata; /* Set file metadata as the first addition to the compression stream. */

    /* Populate the file metadata with the text `blob` followed by the size of the file being added. */
	stream.avail_in = 1+sprintf(metadata, "blob %lu", (unsigned long) st->st_size);
	stream.next_out = out; /* Set `out` as the next location to write compressed output. */
	stream.avail_out = max_out_bytes; /* Set the maximum # of bytes to output. */
	while (deflate(&stream, 0) == Z_OK) /* Compress the data, which so far is just the file metadata. */
		/* Linus Torvalds: nothing */;

	/* Add the file content to the compression stream. */
    stream.next_in = in;
	stream.avail_in = st->st_size;
	while (deflate(&stream, Z_FINISH) == Z_OK) /* Compress the file content. */
		/* Linus Torvalds: nothing */;

	deflateEnd(&stream); /* Free memory structures that were dynamically allocated for compression. */
	
	SHA1_Init(&c); /* Initialize the SHA context `c`. */
	SHA1_Update(&c, out, stream.total_out); /* Hash the compressed output. `stream.total_out` is the size. */
	SHA1_Final(ce->sha1, &c); /* Store the SHA1 hash of the compressed output in the cache_entry's `sha1` member. */

	return write_sha1_buffer(ce->sha1, out, stream.total_out); /* Write compressed content to the object store. */
}

/*
 * Function: `add_file_to_cache`
 * Parameters:
 *      -path: The path of the file to add to the object store and index.
 * Purpose: Open the file to be added to object store and index. Then call
 *          the functions that compress it, hash it, add it to the object
 *          store, active cache, and write the updated index.
 */
static int add_file_to_cache(char *path)
{
	int size, namelen;
	struct cache_entry *ce; /* Used to reference a cache entry. */
	struct stat st; /* Used to store file info returned from `fstat()` command. */
	int fd; /* Reference to a file descriptor representing a link to a file. */

	fd = open(path, O_RDONLY); /* Associate file descriptor with file to add to cache. */

    /* If file association fails, remove file from cache if necessary and return -1. */
	if (fd < 0) {
		if (errno == ENOENT)
			return remove_file_from_cache(path);
		return -1;
	}

    /*
     * Populate the `st` buffer with file information via fstat(). If fstat()
     * fails return -1 and release the file descriptor.
     */
	if (fstat(fd, &st) < 0) {
		close(fd);
		return -1;
	}

	namelen = strlen(path); /* The length of the path/filename character string. */
	size = cache_entry_size(namelen); /* The size in bytes of the cache_entry. */
	ce = malloc(size); /* Allocate memory of size `size` for the cache_entry. */
	memset(ce, 0, size); /* Convert `0` to unsigned char and store `size` times in `ce`. */
	memcpy(ce->name, path, namelen); /* Copy `path` into the cache_entry's `name` member. */

    /* Load the info returned by fstat() call into the cache_entry's members. */
	ce->ctime.sec = STAT_TIME_SEC( &st, st_ctim );
        ce->ctime.nsec = STAT_TIME_NSEC( &st, st_ctim );
	ce->mtime.sec = STAT_TIME_SEC( &st, st_mtim );
        ce->mtime.nsec = STAT_TIME_NSEC( &st, st_mtim );
	ce->st_dev = st.st_dev;
	ce->st_ino = st.st_ino;
	ce->st_mode = st.st_mode;
	ce->st_uid = st.st_uid;
	ce->st_gid = st.st_gid;
	ce->st_size = st.st_size;
	ce->namelen = namelen;

    /*
     * Use the index_fd() function to compress the file content, hash it, and write a new
     * file to the object store to represent that content.
     */
	if (index_fd(path, namelen, ce, fd, &st) < 0)
		return -1;

	return add_cache_entry(ce);
}

/*
 * Function: `write_cache`
 * Parameters:
 *      -newfd: File descriptor associated with the index file.
 *      -cache: The array of cache_entries to write to the file identified by `newfd`.
 *      -entries: The number of entries in the passed-in `cache`.
 * Purpose: 
 */
static int write_cache(int newfd, struct cache_entry **cache, int entries)
{
	SHA_CTX c; /* Declare a SHA context. */
	struct cache_header hdr; /* Declare a cache_header. */
	int i; /* For loop iterator. */

	hdr.signature = CACHE_SIGNATURE; /* Set this to the signature defined in <cache.h>. */
	hdr.version = 1; /* The version is always set to 1 in this release. */
	hdr.entries = entries; /* Set the number of entries in the cache header. */

	SHA1_Init(&c); /* Initialize the SHA context `c`. */

    /* Add the cache_header to the content to be hashed. */
	SHA1_Update(&c, &hdr, offsetof(struct cache_header, sha1));

    /* Add each of the cache_entries to the content to be hashed. */
	for (i = 0; i < entries; i++) {
		struct cache_entry *ce = cache[i];
		int size = ce_size(ce);
		SHA1_Update(&c, ce, size);
	}

    /* Hash the content and store the SHA1 hash in the header. */
	SHA1_Final(hdr.sha1, &c);

    /* Write the cache header to the index file. */
	if (write(newfd, &hdr, sizeof(hdr)) != sizeof(hdr))
		return -1;

    /* Write each of the cache_entries to the index file. */
	for (i = 0; i < entries; i++) {
		struct cache_entry *ce = cache[i];
		int size = ce_size(ce);
		if (write(newfd, ce, size) != size)
			return -1;
	}
	return 0;
}		

/*
 * Linus Torvalds: We fundamentally don't like some paths: we don't want
 * dot or dot-dot anywhere, and in fact, we don't even want
 * any other dot-files (.dircache or anything else). They
 * are hidden, for chist sake.
 *
 * Linus Torvalds: Also, we don't want double slashes or slashes at the
 * end that can make pathnames ambiguous. 
 */
static int verify_path(char *path)
{
	char c;

	goto inside;
	for (;;) {
		if (!c)
			return 1;
		if (c == '/') {
inside:
			c = *path++;
			if (c != '/' && c != '.' && c != '\0')
				continue;
			return 0;
		}
		c = *path++;
	}
}

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
	int i; /* Iterator for `for` loop below. */
    int newfd; /* `New file descriptor` to reference cache/index file. */
    int entries; /* The # of entries in the cache, as returned by read_cache(). */
    char cache_file[]      = ".dircache/index";
    char cache_lock_file[] = ".dircache/index.lock";
    

    /*
     * Read in the contents of the `.dircache/index` file into the `active_cache`.
     * The number of caches entries will be stored in `entries`. Throw error if
     * number of entries is < 0, indicating error reading the cache.
     */
	entries = read_cache();
	if (entries < 0) {
		perror("cache corrupted");
		return -1;
	}

    /*
     * Open a new file descriptor that references the `.dircache/index.lock` file.
     * which is likely a new file. Throw error if it is < 0, indicating failure.
     */
	newfd = OPEN_FILE(cache_lock_file, O_RDWR | O_CREAT | O_EXCL, 0600);
	if (newfd < 0) {
		perror("unable to create new cachefile");
		return -1;
	}

    /*
     * Iterate over the filenames to add to the cache, which were passed in as
     * command line arguments like:
     *
     * ./update-cache filename1 filename2 ...
     */
	for (i = 1 ; i < argc; i++) {
		char *path = argv[i]; /* Store the ith filename passed in as command line argument. */

        /* Verify each path/filename that was passed in. Ignore the invalid ones. */
		if (!verify_path(path)) {
			fprintf(stderr, "Ignoring path %s\n", argv[i]);
			continue;
		}

        /*
         * This calls `add_file_to_cache()` which does a bunch of stuff:
         *      1) Opens the file at `path`.
         *      2) Creates a string of file metadata and pulls in the file's content.
         *      3) Compresses the metadata + content using zlib.
         *      4) Hashes the compressed content to get the SHA1 hash.
         *      5) Writes this content to the object store identified by it's SHA1 hash.
         *      6) Adds the new cache entry into the `active_cache`.
         *
         *      If any of these steps leads to a non-zero return code (i.e. fails), jump
         *      to the `out` section below.
         */
		if (add_file_to_cache(path)) {
			fprintf(stderr, "Unable to add %s to database\n", path);
			goto out;
		}
	}

    /*
     * This does a few things as well:
     *      1) Calls `write_cache` to set up a cache_header, add the cache_entries, hash it all, and write to index file.
     *      2) Renames the index file from `.dircache/index.lock` to simply `.dircache/index`.
     */
	if (!write_cache(newfd, active_cache, active_nr)) {
            close( newfd );
            if (RENAME(cache_lock_file, cache_file) != RENAME_FAIL) {
		return 0;
            }
        }

/* Unlink the `.dircache/index.lock` file since it won't be used due to some failure. */
out:
	unlink(cache_lock_file);
}

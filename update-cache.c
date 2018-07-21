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
 *  add` in current versions of Git. When `update-cache` is run
 *  from the command-line it will take as an argument the name of
 *  a file to be added to the index (or as Mr. Torvalds calls it
 *  in the README, the "Current Directory Cache"), which is stored 
 *  in the .dircache/index file by default. This can be thought of
 *  as the "Staging Area" where changes ready to be committed are
 *  built up.
 *
 *  The `main` function in this file will run when ./update-cache
 *  executable is run from the command line.
 */

#include "cache.h"
/* The above 'include' allows use of the following functions and
   variables from "cache.h" header file, ranked in order of first use
   in this file. Most are functions/macros from standard C libraries
   that are `#included` in "cache.h". Function names are followed by
   parenthesis whereas variable/struct names are not:

   -memcmp(str1, str2, n): Compare first n bytes of str1 to str2 and return an 
                           integer greater than, equal to, or less than 0, if 
                           the object pointed to by str1 is greater than, 
                           equal to, or less than the object pointed to by 
                           str2, respectively. Sourced from <string.h>.

   -active_nr: The number of entries in the active cache/index.

   -cache_entry: Structure representing a single cached/staged file.

   -active_cache: Array representing the active cache/index.

   -memmove(str1, str2, n): Copy n bytes from the object pointed to by str2 
                            to the object pointed to by str1.

   -sizeof(datatype): Operator that gives the number of bytes needed to store 
                      a datatype or variable. 

   -z_stream: General-purpose compression/decompression stream. Sourced from
              <zlib.h>.

   -malloc(size): Allocate unused space for an object whose size in bytes is 
                  specified by `size` and whose value is unspecified. Sourced 
                  from <stdlib.h>.

   -mmap(addr, len, prot, flags, file_descriptor, offset): 
        Establish a mapping between a process' address space and a file, 
        shared memory object, or typed memory object. Sourced from 
        <sys/mman.h>.

   -PROT_READ: Flag for the mmap() function's `prot` parameter describing
               the desired memory protection of the mapping. `PROT_READ`
               indicates that pages may be read. Sourced from <sys/mman.h>.
                
   -MAP_PRIVATE: Flag for the mmap() function's `flags` parameter describing
                 the desired memory protection of the mapping. `MAP_PRIVATE`
                 indicates that changes to the mapped data are private to the
                 calling process and should not change the underlying object.
                 Sourced from <sys/mman.h>.

   -SHA_CTX: SHA context structure used to store information related to the
             process of hashing the content. Sourced from <openssl/sha.h>.

   -close(fd): Deallocate the file descriptor `fd`. The file descriptor `fd` 
               will be made available to subsequent calls to open() or other 
               function calls that allocate `fd`. Remove all locks owned by 
               the process on the file associated with `fd`. Sourced from
               <unistd.h>.

   -memset(void *s, int c, size_t n): Copies `c` (converted to an unsigned
                                      char) into each of the first `n` bytes
                                      of the object pointed to by `s`.

   -deflateInit(z_stream, level): Initializes the internal `z_stream` state 
                                  for compression at `level`, which indicates 
                                  the scale of speed versus compression on a 
                                  scale from 0 to 9. Sourced from <zlib.h>.

   -Z_BEST_COMPRESSION: Translates to compression level 9, which, as an input 
                        to `deflateInit()`, indicates optimizing compression
                        size as opposed to compression speed. Sourced from
                        <zlib.h>.

   -sprintf(s, message, ...): Writes `message` string constant to string 
                              variable `s` followed by the null character 
                              '\0'. Sourced from <stdio.h>.

   -deflate(z_stream, flush): Compresses as much data as possible and stops
                              when the input buffer becomes empty or the
                              output buffer becomes full. Sourced from 
                              <zlib.h>.

   -Z_OK: Successful zlib return code. It is equal to 0. Sourced from 
          <zlib.h>.

   -Z_FINISH: Flush value for zlib that specifies processing of remaining 
              input. It is equal to 4. Sourced from <zlib.h>.

   -deflateEnd(z_stream): All dynamically allocated data structures for
                          `z_stream` are freed. Sourced from <zlib.h>.

   -SHA1_Init(SHA_CTX *c): Initializes a SHA_CTX structure. Sourced from 
                           <openssl/sha.h>. 

   -SHA1_Update(SHA_CTX *c, const void *data, size_t len): 
        Can be called repeatedly to calculate the hash value of chunks of data 
        (len bytes from data). Sourced from <openssl/sha.h>.

   -SHA1_Final(unsigned char *md, SHA_CTX *c): 
        Places the message digest in md, which must have space for 20 bytes of 
        output, and erases the `c` SHA_CTX structure. Sourced from 
        <openssl/sha.h>.

   -stat: Structure pointer used by stat() function to store information
          related to a filesystem file. Sourced from <sys/stat.h>.

   -open(path, flags, perms): Open file in `path` for reading and/or writing 
                              as specified in `flags` and return a file 
                              descriptor that refers to the open file
                              description. If the file does not exist, it is
                              created with the permssions in `perms`. Sourced 
                              from <fcntl.h>.

   -O_RDONLY: Flag for the open() function indicating to open the file for
              reading only. Sourced from <fcntl.h>.

   -ENOENT: Failure code set as errno by the open() function when O_CREAT is 
            not set and the named file does not exist; or O_CREAT is set and 
            either the path prefix does not exist or the path argument points 
            to an empty string. Sourced from <fcntl.h>.

   -fstat(fd, buf): Obtain information about an open file associated with the 
                    file descriptor `fd` and write it to the area pointed to 
                    by `buf`, which is a pointer to a `stat` structure. 
                    Sourced from <sys/stat.h>.

   -strlen(string): Return the length of `string` in bytes.

   -memcpy(s1, s2, n): Copy n bytes from the object pointed to by s2 into the 
                       object pointed to by s1.

   -cache_header: Structure representing header information for the cache.
                  Sourced from "cache.h".

   -CACHE_SIGNATURE: #Defined as `0x44495243`. Sourced from "cache.h".

   -offsetof(type, member): Macro that expands to an integral constant 
                            expression of type std::size_t, the value of which 
                            is the offset, in bytes, from the beginning of an 
                            object of specified type to its specified member, 
                            including padding if any. Sourced from <stddef.h>.

   -perror(message): Write `message` to standard error stream. Sourced from 
                     <stdio.h>.

   -O_RDWR: Flag for the open() function indicating to open the file for
            reading and writing. Sourced from <fcntl.h>.

   -O_CREAT: Flag for the open() function. If the file exists, this flag has 
             no effect except as noted under O_EXCL below. Otherwise, the file 
             shall be created. Sourced from <fcntl.h>.

   -O_EXCL: Flag for the open() function. If O_CREAT and O_EXCL are set,
            open() shall fail if the file exists.

   -fprintf(stream, message, ...): Write `message` to the output `stream`. 
                                   Sourced from <stdio.h>.

   -rename(old, new): Change the name of a file. `old` points to the pathname 
                      of the file to be renamed. `new` points to the new 
                      pathname of the file. Sourced from <stdio.h>.

   ****************************************************************

   The following variables and functions are defined in this source file.

   -main(argc, argv): The main function which runs each time the
                      update-cache command is run.

   -argc: The number of command line arguments supplied when executing 
          update-cache.

   -argv: Array containing command line argument strings.

   -verify_path(): Checks if a file path is a valid path.

   -add_file_to_cache(): Get information about the file to add to the cache, 
                         store the file metadata in a cache_entry structure, 
                         then call the `index_fd()` function to construct a 
                         blob object and write it to the object store, and the 
                         `add_cache_entry()` function to insert the cache 
                         entry into the `active_cache` array 
                         lexicographically.

   -index_fd(): Constructs a blob object, compresses it, calculates the SHA1 
                hash of the compressed blob object, then write the blob object 
                to the object database.

   -add_cache_entry(): Inserts a cache entry into the active_cache array
                       lexicographically.

   -cache_name_pos(): Determines the lexicographic position of a cache entry 
                      in the active_cache array.

   -cache_name_compare(): Compares the names of two cache entries
                          lexicographically.

   -remove_file_from_cache(): Removes a file's cache entry from the
                              active_cache array.

   -write_cache(): Constructs the cache header, calculates the SHA1 hash of 
                   the cache, and then writes them to the 
                   `.dircache/index.lock` file.
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
 * Purpose: Compare the names of two cache entries lexicographically.
 */
static int cache_name_compare(const char *name1, int len1, const char *name2, 
                              int len2)
{
    int len = len1 < len2 ? len1 : len2;   /* len is the shorter length. */
    int cmp;

    cmp = memcmp(name1, name2, len);
    if (cmp)           /* First len characters are different. */
        return cmp;
    if (len1 < len2)   /* First len characters are the same. */
        return -1;
    if (len1 > len2)   /* First len characters are the same. */
        return 1;
    return 0;          /* Exact match. */
}

/*
 * Function: `cache_name_pos`
 * Parameters:
 *      -name: The path of the file to be cached.
 *      -namelen: The length of the path.
 * Purpose: Determine the lexicographic position of a cache entry in the
 *          active_cache array.
 */
static int cache_name_pos(const char *name, int namelen)
{
    /* Declare and initialize the indexes for the binary search. */
    int first, last;
    first = 0;
    last = active_nr;

    /*
     * Perform a binary search to determine the lexicographic position of the 
     * cache entry in the active_cache array.
     */
    while (last > first) {
        int next = (last + first) >> 1;   /* Division by 2. */
        struct cache_entry *ce = active_cache[next];
        int cmp = cache_name_compare(name, namelen, ce->name, ce->namelen);
        if (!cmp)            /* Exact match found. */
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
 * Purpose: Remove a file's cache entry from the active_cache array.
 */
static int remove_file_from_cache(char *path)
{
    int pos = cache_name_pos(path, strlen(path));
    if (pos < 0) {   /* If exact match found. */
        pos = -pos-1;
        active_nr--;
        if (pos < active_nr)
            memmove(active_cache + pos, active_cache + pos + 1, 
                    (active_nr - pos - 1) * sizeof(struct cache_entry *));
    }
}

/*
 * Function: `add_cache_entry`
 * Parameters:
 *      -ce: The cache entry to be added to the `active_cache` array.
 * Purpose: Insert a cache entry into the `active_cache` array
 *          lexicographically.
 */
static int add_cache_entry(struct cache_entry *ce)
{
    /*
     * Get the index where the cache entry will be inserted in the 
     * active_cache array. 
     */
    int pos;   
    pos = cache_name_pos(ce->name, ce->namelen);

    /* Linus Torvalds: existing match? Just replace it */
    if (pos < 0) {
        active_cache[-pos-1] = ce;
        return 0;
    }

    /*
     * Make sure the `active_cache` array has space for the additional cache
     * entry.
     */
    if (active_nr == active_alloc) {
        active_alloc = alloc_nr(active_alloc);
        active_cache = realloc(active_cache, 
                               active_alloc * sizeof(struct cache_entry *));
    }

    /* Insert the new cache entry into the active_cache array. */
    active_nr++;
    if (active_nr > pos)
        memmove(active_cache + pos + 1, active_cache + pos, 
                (active_nr - pos - 1) * sizeof(ce));
    active_cache[pos] = ce;
    return 0;
}

/*
 * Function: `index_fd`
 * Parameters:
 *      -path: The path of the file to add to the object store.
 *      -namelen: The length of the path/filename to add.
 *      -ce: The cache entry structure corresponding to the file.
 *      -fd: The file descriptor associated with the file to be added.
 *      -st: The `stat` object containing info about the file to be added.
 * Purpose: Construct a blob object, compress it, calculate the SHA1 hash of
 *          the compressed blob object, then write the blob object to the 
 *          object database.
 */ 
static int index_fd(const char *path, int namelen, struct cache_entry *ce, 
                    int fd, struct stat *st)
{
    /* Declare zlib z_stream structure. */
    z_stream stream;
    /* Number of bytes to allocate for next compressed output. */
    int max_out_bytes = namelen + st->st_size + 200; 
    /* Allocate `max_out_bytes` of space to store next compressed output. */
    void *out = malloc(max_out_bytes);
    /* Allocate space to store file metadata. */
    void *metadata = malloc(namelen + 200);

    /* Map contents of file to be cached to memory. */
    #ifndef BGIT_WINDOWS
    void *in = mmap(NULL, st->st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    #else
    void *fhandle = CreateFileMapping( (HANDLE) _get_osfhandle(fd), NULL, 
                                       PAGE_READONLY, 0, 0, NULL );
    if (!fhandle)
        return -1;

    void *in = MapViewOfFile( fhandle, FILE_MAP_READ, 0, 0, st->st_size );
    CloseHandle( fhandle );
    #endif

    /* Declare an SHA context structure. */
    SHA_CTX c;

    /* Release the file descriptor `fd` since we no longer need it. */
    close(fd);

    #ifndef BGIT_WINDOWS
    /* Return -1 if memory allocation for the `out` or `in` memory failed. */
    if (!out || (int)(long)in == -1)
        return -1;
    #else
    if (!out || in == (void *) NULL)
        return -1;
    #endif

    /* Initialize the zlib stream to contain null characters. */
    memset(&stream, 0, sizeof(stream));

    /*
     * Initialize the compression stream for optimized compression 
     * (as opposed to speed). 
     */
    deflateInit(&stream, Z_BEST_COMPRESSION);

    /*
     * Linus Torvalds: ASCII size + nul byte
     */    
    stream.next_in = metadata;   /* Set file metadata as the first addition */
                                 /* to the compression stream input. */
    /*
     * Write `blob ` to the `metadata` array, followed by the size of the 
     * file being added to the cache.  Set the number of bytes available as
     * input for the next compression equal to the number of characters 
     * written + 1 (for the terminating null character).
     */
    stream.avail_in = 1 + sprintf(metadata, "blob %lu", 
                                  (unsigned long) st->st_size);
    /* Specify `out` as the location to write the next compressed output. */
    stream.next_out = out;
    /* Number of bytes available for storing the next compressed output. */
    stream.avail_out = max_out_bytes;

    /* Compress the data, which so far is just the file metadata. */
    while (deflate(&stream, 0) == Z_OK)
        /* Linus Torvalds: nothing */;

    /* Add the file content to the compression stream input. */
    stream.next_in = in;
    stream.avail_in = st->st_size;

    /* Compress the file content. */
    while (deflate(&stream, Z_FINISH) == Z_OK)
        /* Linus Torvalds: nothing */;

    /* Free data structures that were used for compression. */
    deflateEnd(&stream);
    /* Initialize the `c` SHA context structure. */
    SHA1_Init(&c);
    /*
     * Calculate the hash of the compressed output, which has total size 
     * `stream.total_out`. 
     */
    SHA1_Update(&c, out, stream.total_out); 
    /*
     * Store the SHA1 hash of the compressed output in the cache entry's 
     * `sha1` member. 
     */
    SHA1_Final(ce->sha1, &c);

    /*
     * Write the blob object to the object store and return with the return
     * value of the write_sha1_buffer function. 
     */
    return write_sha1_buffer(ce->sha1, out, stream.total_out); 
}

/*
 * Function: `add_file_to_cache`
 * Parameters:
 *      -path: The path of the file to add to the object store and index.
 * Purpose: Get information about the file to add to the cache, store the file 
 *          metadata in a cache_entry structure, then call the `index_fd()` 
 *          function to construct a blob object and write it to the object 
 *          store, and the `add_cache_entry()` function to insert the cache 
 *          entry into the `active_cache` array lexicographically.
 */
static int add_file_to_cache(char *path)
{
    int size, namelen;
    /* Used to reference a cache entry. */
    struct cache_entry *ce; 
    /*
     * Used to store file information obtained through an `fstat()` function 
     * call. 
     */
    struct stat st;
    /* File descriptor for the file to add to the cache. */
    int fd;

    /* Open the file to add to the cache and return a file descriptor. */
    fd = open(path, O_RDONLY); 

    /*
     * If the `open()` command fails, return -1. Remove the corresponding 
     * cache entry from the active_cache array if the file does not exist in 
     * the working directory.
     */
    if (fd < 0) {
        if (errno == ENOENT)
            return remove_file_from_cache(path);
        return -1;
    }

    /*
     * Get file information and store it in the `st` stat structure. If 
     * fstat() fails, release the file descriptor and return -1.
     */
    if (fstat(fd, &st) < 0) {
        close(fd);
        return -1;
    }

    /* Get the length of the file path string. */
    namelen = strlen(path); 
    /* Calculate the size to allocate to the cache entry in bytes. */
    size = cache_entry_size(namelen); 
    /* Allocate `size` bytes to the cache entry. */
    ce = malloc(size); 
    /* Initialize the cache entry to contain null characters. */
    memset(ce, 0, size); 
    /* Copy `path` into the cache entry's `name` member. */
    memcpy(ce->name, path, namelen); 

    /*
     * Copy the file metadata obtained through the fstat() call to the cache
     * entry structure members. 
     */
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
     * Call the index_fd() function to construct a blob object, compress it, 
     * calculate the SHA1 hash of the compressed blob object, then write the 
     * blob object to the object database.
     */
    if (index_fd(path, namelen, ce, fd, &st) < 0)
        return -1;

    /*
     * Insert the cache entry into the active_cache array lexicographically
     * and then return using the return value of `add_cache_entry`.
     */
    return add_cache_entry(ce);
}

/*
 * Function: `write_cache`
 * Parameters:
 *      -newfd: File descriptor associated with the index lock file.
 *      -cache: The array of pointers to cache entry structures to write to 
 *              the index lock file.
 *      -entries: The number of cache entries in the `active_cache` array.
 * Purpose: Construct the cache header, calculate the SHA1 hash of the cache 
 *          header and the cache entries in the `active_cache` array, and 
 *          then write them to the `.dircache/index.lock` file.
 */
static int write_cache(int newfd, struct cache_entry **cache, int entries)
{
    SHA_CTX c;                 /* Declare an SHA context structure. */
    struct cache_header hdr;   /* Declare a cache_header structure. */
    int i;                     /* For loop iterator. */

    /* Set this to the signature defined in "cache.h". */
    hdr.signature = CACHE_SIGNATURE; 
    /* The version is always set to 1 in this release. */
    hdr.version = 1; 
    /*
     * Store the number of cache entries in the `active_cache` array in the 
     * cache header. 
     */
    hdr.entries = entries; 

    /* Initialize the `c` SHA context structure. */
    SHA1_Init(&c); 
    /* Update the running SHA1 hash calculation with the cache header. */
    SHA1_Update(&c, &hdr, offsetof(struct cache_header, sha1));
    /* Update the running SHA1 hash calculation with each cache entry. */
    for (i = 0; i < entries; i++) {
        struct cache_entry *ce = cache[i];
        int size = ce_size(ce);
        SHA1_Update(&c, ce, size);
    }
    /* Store the final SHA1 hash in the header. */
    SHA1_Final(hdr.sha1, &c);

    /* Write the cache header to the index lock file. */
    if (write(newfd, &hdr, sizeof(hdr)) != sizeof(hdr))
        return -1;

    /* Write each of the cache entries to the index lock file. */
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
 * dot or dot-dot anywhere, and in fact, we don't even want any other 
 * dot-files (.dircache or anything else). They are hidden, for chist sake.
 *
 * Linus Torvalds: Also, we don't want double slashes or slashes at the end 
 * that can make pathnames ambiguous. 
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
 *      -argc: The number of command-line arguments supplied, inluding the 
 *             command itself.
 *      -argv: An array of the command line arguments, including the command 
 *             itself.
 * Purpose: Standard `main` function definition. Runs when the executable 
 *          `update-cache` is run from the command line.
 */
int main(int argc, char **argv)
{
    int i;         /* Iterator for `for` loop below. */
    int newfd;     /* File descriptor to reference the index lock file. */
    int entries;   /* The number of entries in the cache, as returned by */
                   /* read_cache(). */

    /* The name of the cache file. */
    char cache_file[]      = ".dircache/index";
    /* The name of the cache lock file. */
    char cache_lock_file[] = ".dircache/index.lock"; 

    /*
     * Read in the contents of the `.dircache/index` file into the 
     * `active_cache` array and return the number of cache entries. Display an
     * error message if the number of entries is < 0, indicating
     * an error in reading the cache, then return -1.
     */
    entries = read_cache();
    if (entries < 0) {
        perror("cache corrupted");
        return -1;
    }

    /*
     * Create and open a new cache lock file called `.dircache/index.lock` and 
     * return a file descriptor to reference it. Display an error message if 
     * the open() command returns a value < 0, indicating failure, then return 
     * -1.
     */
    newfd = OPEN_FILE(cache_lock_file, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (newfd < 0) {
        perror("unable to create new cachefile");
        return -1;
    }

    /*
     * Loop over the files to add to the cache, whose paths or filenames were 
     * passed in as command line arguments:
     *
     * ./update-cache path1 path2...
     */
    for (i = 1 ; i < argc; i++) {
        /* Store the ith path that was passed as a command line argument. */
        char *path = argv[i];

        /*
         * Verify the path. If the path is not valid, continue to the next 
         * file. 
         */
        if (!verify_path(path)) {
            fprintf(stderr, "Ignoring path %s\n", argv[i]);
            continue;
        }

        /*
         * This calls `add_file_to_cache()`, which does a few things:
         *      1) Opens the file at `path`.
         *      2) Gets information about the file and stores the file
         *         metadata in a cache_entry structure.
         *      3) Calls the index_fd() function to construct a corresponding
         *         blob object and write it to the object database.
         *      4) Calls the add_cache_entry() function to insert the cache
         *         entry into the active_cache array lexicographically.
         *
         * If any of these steps leads to a nonzero return code (i.e. fails), 
         * jump to the `out` label below.
         */
        if (add_file_to_cache(path)) {
            fprintf(stderr, "Unable to add %s to database\n", path);
            goto out;
        }
    }

    /*
     * This does a few things as well:
     *      1) Calls `write_cache()` to set up a cache header, calculate the
     *         SHA1 hash of the header and the cache entries, and then write 
     *         the entire cache to the index lock file.
     *      2) Renames the `.dircache/index.lock` file to `.dircache/index`.
     */
    if (!write_cache(newfd, active_cache, active_nr)) {
        close(newfd);
        if (RENAME(cache_lock_file, cache_file) != RENAME_FAIL) {
            return 0;
        }
    }

/* Unlink the `.dircache/index.lock` file. */
out:
    close(newfd);
    #ifndef BGIT_WINDOWS
    unlink(cache_lock_file);
    #else
    _unlink(cache_lock_file);
    #endif
}

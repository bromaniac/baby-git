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
 *  The purpose of this file is to be compiled is to define helper functions
 *  that the other files in the codebase use to interact with the object store
 *  and the index. This includes helper functions for reading and writing from
 *  the object store and index and validating existing cache_entries.
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
                
   -MAP_PRIVATE: Flag for the mmap() function's `prot` parameter describing
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
                                  scale of speed versuss compression on a 
                                  scale from 0-9. Sourced from <zlib.h>.

   -deflateBound(z_stream, sourceLen): Returns an upper bound on the 
                                       compressed size after deflation of 
                                       `sourceLen` bytes.

   -inflateInit(z_stream): Initializes the internal `z_stream` state for
                           decompression. Sourced from <zlib.h>.

   -Z_BEST_COMPRESSION: Translates to compression level 9, which, as an input 
                        to `deflateInit()`, indicates optimizing compression
                        size as opposed to compression speed. Sourced from
                        <zlib.h>.

   -deflate(z_stream, flush): Compresses as much data as possible and stops
                              when the input buffer becomes empty or the
                              output buffer becomes full. Sourced from 
                              <zlib.h>.

   -inflate(z_stream, flush): Decompresses as much data as possible and stops
                              when the input buffer becomes empty or the
                              output buffer becomes full. Sourced from 
                              <zlib.h>.

   -Z_OK: Successful zlib return code. It is equal to 0. Sourced from 
          <zlib.h>.

   -Z_FINISH: Flush value for zlib that specifies processing of remaining 
              input. It is equal to 4. Sourced from <zlib.h>.

   -deflateEnd(z_stream): All dynamically allocated data structures for
                          `z_stream` are freed. Sourced from <zlib.h>.

   -SHA1_Init(): Initializes a SHA_CTX structure. Sourced from 
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

   -O_CREAT: Flag for the open() function. If the file exists, this flag has 
             no effect except as noted under O_EXCL below. Otherwise, the file 
             shall be created. Sourced from <fcntl.h>.

   -O_EXCL: Flag for the open() function. If O_CREAT and O_EXCL are set, 
            open() shall fail if the file exists.

   -fprintf(stream, message, ...): Write `message` to the output `stream`.
                                   Sourced from <stdio.h>.
    
   -sscanf(string, format, args): Read input from `string` into `args` using 
                                  input format `format`. Sourced from 
                                  <stdio.h>.

   ****************************************************************

   The following variables are external variables defined in this source file:

   -sha1_file_directory: The path to the object store.

   -active_cache: An array of pointers to cache entries representing the 
                  current set of content that will be cached to file or that
                  has been retrieved from the cache file. 

   -active_nr: The number of cache entries in the active_cache array.

   -active_alloc: The maximum number of elements the active_cache array can 
                  hold. 

   ****************************************************************

   The following variables and functions are defined in this source file:

   -usage(): Print an error message and exit.

   -hexval(): Convert a hexadecimal symbol to its decimal equivalent.

   -get_sha1_hex(): Convert a 40-character hexadecimal representation of an 
                    SHA1 hash value to the equivalent 20-byte representation.

   -sha1_to_hex(): Convert a 20-byte representation of an SHA1 hash value to 
                   the equivalent 40-character hexadecimal representation.

   -sha1_file_name(): Build the path of an object in the object database
                      using the object's SHA1 hash value.

   -read_sha1_file(): Locate an object in the object database, read and 
                      inflate it, then return the inflated object data 
                      (without the prepended metadata).

   -write_sha1_file(): Deflate an object, calculate the hash value, then call
                       the write_sha1_buffer function to write the deflated
                       object to the object database.

   -write_sha1_buffer(): Write an object to the object database, using the
                         object's SHA1 hash value as index.

   -error(): Print an error message to standard error stream and return -1.

   -verify_hdr(): Validate a cache header.

   -read_cache(): Reads the cache entries in the `.dircache/index` file into 
                  the `active_cache` array.
*/

/* Used to store the path to the object store. */
const char *sha1_file_directory = NULL; 
/*
 * An array of pointers to cache entries representing the current set of 
 * content that will be cached to file or that has been retrieved from the 
 * cache file. 
 */
struct cache_entry **active_cache = NULL; 
/* The number of cache entries in the active_cache array. */
unsigned int active_nr = 0; 
/* The maximum number of elements the active_cache array can hold. */
unsigned int active_alloc = 0; 

/*
 * Function: `usage`
 * Parameters:
 *      -err: Error message to display.
 * Purpose: Display an error message and exit.
 */
void usage(const char *err)
{
    fprintf(stderr, "read-tree: %s\n", err);
    exit(1);
}

/* Function: `hexval`
 * Parameters:
 *      -c: Hexadecimal character to convert to decimal.
 * Purpose: Convert a hexadecimal symbol to its decimal equivalent.
 */
static unsigned hexval(char c)
{
    /* If the character is 0-9, return it converted to an integer 0-9. */
    if (c >= '0' && c <= '9')
        return c - '0';

    /* If the character is a-f, return it converted to an integer 10-15. */
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    /* If the character is A-F, return it converted to an integer 10-15. */
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    /*
     * If `c` is not a valid hexadecimal symbol, return the one's complement 
     * of 0. 
     */
    return ~0;
}

/*
 * Function: `get_sha1_hex`
 * Parameters:
 *      -hex: String containing the hexadecimal representation of an SHA1 hash 
              value.
 *      -sha1: Array for storing the 20-byte representation of the SHA1 hash
 *             value.
 * Purpose: Convert a 40-character hexadecimal representation of an SHA1 hash 
 *          value to the equivalent 20-byte representation.
 */
int get_sha1_hex(char *hex, unsigned char *sha1)
{
    int i;
    /*
     * Convert each two-digit hexadecimal number (ranging from 00 to ff) to a 
     * decimal number (ranging from 0 to 255). 
     */
    for (i = 0; i < 20; i++) {
        /*
         * The decimal equivalent of the first hexadecimal digit will form 
         * the 4 high bits of val; that of the second hex digit will form the 
         * 4 low bits.
         */
        unsigned int val = (hexval(hex[0]) << 4) | hexval(hex[1]);
        /* Return -1 if val is larger than 255. */
        if (val & ~0xff)
            return -1;
        *sha1++ = val;   /* Store val in sha1 array. */
        hex += 2;        /* Get next two-digit hexadecimal number. */
    }
    return 0;
}

/*
 * Function: `sha1_to_hex`
 * Parameters:
 *      -sha1: Array containing 20-byte representation of an SHA1 hash value.
 * Purpose: Convert a 20-byte representation of an SHA1 hash value to the
 *          equivalent 40-character hexadecimal representation.
 */
char *sha1_to_hex(unsigned char *sha1)
{
    /* String for storing the 40-character hexadecimal representation. */
    static char buffer[50];
    /*
     * Lookup array for getting the hexadecimal representation of a number
     * from 0 to 15. 
     */
    static const char hex[] = "0123456789abcdef";
    /* Pointer used for filling up the buffer string. */
    char *buf = buffer;
    int i;

    /*
     * Get the two-digit hexadecimal representation (ranging from 00 to ff) of
     * a number (ranging from 0 to 255).
     */
    for (i = 0; i < 20; i++) {
        unsigned int val = *sha1++;   /* Get the current number. */
        *buf++ = hex[val >> 4];       /* Convert the 4 high bits to hex. */
        *buf++ = hex[val & 0xf];      /* Convert the 4 low bits to hex. */
    }
    return buffer;   /* Return the hexadecimal representation. */
}

/*
 * Linus Torvalds: NOTE! This returns a statically allocated buffer, so you 
 * have to be careful about using it. Do a "strdup()" if you need to save the
 * filename.
 */

/*
 * Function: `sha1_file_name`
 * Parameters:
 *      -sha1: The SHA1 hash value used to identify the object in the object 
 *             store.
 * Purpose: Build the path of an object in the object database using the 
 *          object's SHA1 hash value.
 */
char *sha1_file_name(unsigned char *sha1)
{
    int i;
    /* `base` is a character array for storing the path to an object in the
     * object database. `name` is a pointer to the byte in `base` that is
     * after the object database path plus `/`.
     */
    static char *name, *base;

    /* If base has not been set. */
    if (!base) {
        /* Get the path to the object database. */
        char *sha1_file_directory 
                 = getenv(DB_ENVIRONMENT) ? : DEFAULT_DB_ENVIRONMENT;
        /* The length of the path. */
        int len = strlen(sha1_file_directory);
        /* Allocate space for the base string. */
        base = malloc(len + 60);
        /* Copy the object database path to the base string. */
        memcpy(base, sha1_file_directory, len);
        /* Initialize the rest of the base string to contain null bytes. */
        memset(base+len, 0, 60);
        /* Write a slash after the sha1_file_directory path. */
        base[len] = '/';
        /*
         * Write a slash to separate the two-character object directory from
         * the object filename.
         */
        base[len+3] = '/';
        /* Set name to point to the byte after the first slash above. */
        name = base + len + 1;
    }
    /*
     * Fill in the rest of the object path (object directory and filename)
     * using the object's SHA1 hash value.
     *
     * Convert each number in the sha1 array (ranging from 0 to 255) to a 
     * two-digit hexadecimal number (ranging from 00 to ff). The first 
     * two-digit hexadecimal number will form part of the object directory. 
     * The rest of the two-digit hexadecimal numbers will comprise the object 
     * filename. 
     */
    for (i = 0; i < 20; i++) {
        /*
         * Lookup array for getting the hexadecimal representation of a 
         * number from 0 to 15. 
         */
        static char hex[] = "0123456789abcdef";
        /* Get the current number from sha1. */
        unsigned int val = sha1[i];
        /*
         * Set the index of the base array. This will be name + 0, name + 3,
         * name + 5, name + 7,..., name + 39.
         */
        char *pos = name + i*2 + (i > 0);
        *pos++ = hex[val >> 4];   /* Convert the 4 high bits to hex. */
        *pos = hex[val & 0xf];    /* Convert the 4 low bits to hex. */
    }
    return base;   /* Return the path to the object. */
}

/*
 * Function: `read_sha1_file`
 * Parameters:
 *      -sha1: SHA1 hash value of an object.
 *      -type: The type of object that was read (blob, tree, or commit).
 *      -size: The size in bytes of the object data.
 * Purpose: Locate an object in the object database, read and inflate it, then 
 *          return the inflated object data (without the prepended metadata).
 */
void *read_sha1_file(unsigned char *sha1, char *type, unsigned long *size)
{
    z_stream stream;     /* Declare a zlib z_stream structure. */
    char buffer[8192];   /* Buffer for zlib inflated output. */
    struct stat st;      /* `stat` structure for storing file information. */
    int i;               /* Not used. Even almighty Linux makes mistakes. */
    int fd;              /* File descriptor to be associated with the */
                         /* object to be read. */
    int ret;             /* Return value of inflate command. */
    int bytes;           /* Used to track sizes of buffer content. */
    /*
     * `map` is a pointer to an object's mapped contents. `buf` is a pointer
     * to inflated object data.
     */
    void *map, *buf;
    /*
     * Build the path of an object in the object database using the object's 
     * SHA1 hash value.
     */
    char *filename = sha1_file_name(sha1); 

    /*
     * Open the object in the object store and associate `fd` with it. If the 
     * returned value is < 0, there was an error reading the file.
     */
    #ifndef BGIT_WINDOWS
    fd = open(filename, O_RDONLY );
    #else
    fd = open(filename, O_RDONLY | O_BINARY );
    #endif
    if (fd < 0) {
        perror(filename);
        return NULL;
    }

    /*
     * Get the file information and store it in the `st` structure. Release 
     * the file descriptor if there was an error.
     */
    if (fstat(fd, &st) < 0) {
        close(fd);
        return NULL;
    }

    /* Map contents of the object to memory. */
    #ifndef BGIT_WINDOWS
    map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (-1 == (int)(long)map)   /* Return NULL if mmap failed. */
        return NULL;
    #else
    void *fhandle = CreateFileMapping( (HANDLE) _get_osfhandle(fd), NULL, 
                                       PAGE_READONLY, 0, 0, NULL );
    if (!fhandle)
        return NULL;
    map = MapViewOfFile( fhandle, FILE_MAP_READ, 0, 0, st.st_size );
    CloseHandle( fhandle );
    if (map == (void *) NULL)
        return NULL;
    #endif
    close(fd);   /* Release the file descriptor. */

    /* Initialize the zlib stream to contain null characters. */
    memset(&stream, 0, sizeof(stream));
    /* Set map as location of the next input to the inflation stream. */
    stream.next_in = map; 
    /* Number of bytes available as input for next inflation. */
    stream.avail_in = st.st_size; 
    /* Set `buffer` as the location to write the next inflated output. */
    stream.next_out = buffer; 
    /* Number of bytes available for storing the next inflated output. */
    stream.avail_out = sizeof(buffer); 

    /* Initialize the stream for decompression. */
    inflateInit(&stream); 
    /* Decompress the object contents and store return code in `ret`. */
    ret = inflate(&stream, 0); 

    /*
     * Read the object type and size of the object data from the buffer and
     * store them in variables type and size, respectively.  Return NULL if 
     * the two conversions were not successful.
     */
    if (sscanf(buffer, "%10s %lu", type, size) != 2)
        return NULL;

    /*
     * The size of the buffer up to the first null character, i.e., the size
     * of the prepended metadata plus the terminating null character.
     */
    bytes = strlen(buffer) + 1; 
    /* Allocate space to `buf` that's equal to the object data size. */
    buf = malloc(*size); 
    /* Error if space could not be allocated. */
    if (!buf)
        return NULL;

    /*
     * Copy the inflated object data from buffer to buf, i.e, without the 
     * prepended metadata (the object type and expected object data size).
     */
    memcpy(buf, buffer + bytes, stream.total_out - bytes);
    /* The size of the inflated data without the prepended metadata. */
    bytes = stream.total_out - bytes;
    /* Continue inflation if not all data has been inflated. */
    if (bytes < *size && ret == Z_OK) {
        stream.next_out = buf + bytes;
        stream.avail_out = *size - bytes;
        while (inflate(&stream, Z_FINISH) == Z_OK)
            /* Linus Torvalds: nothing */;
    }
    /* Free memory structures that were used for the inflation. */
    inflateEnd(&stream);
    return buf;   /* Return the inflated object data. */
}

/*
 * Function: `write_sha1_file`
 * Parameters:
 *      -buf: The content to be deflated and written to the object store.
 *      -len: The length in bytes of the content pre-compression.
 * Purpose: Deflate an object, calculate the hash value, then call the
 *          write_sha1_buffer function to write the deflated object to the 
 *          object database.
 */
int write_sha1_file(char *buf, unsigned len)
{
    int size;                 /* Total size of compressed output. */
    char *compressed;         /* Used to store compressed output. */
    z_stream stream;          /* Declare zlib z_stream structure. */
    unsigned char sha1[20];   /* Array to store SHA1 hash. */
    SHA_CTX c;                /* Declare an SHA context structure. */

    /* Initialize the zlib stream to contain null characters. */
    memset(&stream, 0, sizeof(stream));

    /*
     * Initialize compression stream for optimized compression 
     * (as opposed to speed). 
     */
    deflateInit(&stream, Z_BEST_COMPRESSION);
    /* Determine upper bound on compressed size. */
    size = deflateBound(&stream, len); 
    /* Allocate `size` bytes of space to store the next compressed output. */
    compressed = malloc(size); 

    /* Specify buf as location of the next input to the compression stream. */
    stream.next_in = buf; 
    /* Number of bytes available as input for next compression. */
    stream.avail_in = len; 
    /* Specify compressed as location to write the next compressed output. */
    stream.next_out = compressed; 
    /* Number of bytes available for storing the next compressed output. */
    stream.avail_out = size; 

    /* Compress the content of buf, i.e., compress the object. */
    while (deflate(&stream, Z_FINISH) == Z_OK) 
    /* Linus Torvalds: nothing */;

    /*
     * Free memory structures that were dynamically allocated for the
     * compression. 
     */
    deflateEnd(&stream); 
    /* Get size of total compressed output. */
    size = stream.total_out; 

    /* Initialize the SHA context structure. */
    SHA1_Init(&c); 
    /* Calculate hash of the compressed output. */
    SHA1_Update(&c, compressed, size); 
    /* Store the SHA1 hash of the compressed output in `sha1`. */
    SHA1_Final(sha1, &c); 

    /* Write the compressed object to the object store. */
    if (write_sha1_buffer(sha1, compressed, size) < 0)
        return -1;
    /*
     * Display the 40-character hexadecimal representation of the object's 
     * SHA1 hash value.
     */
    printf("%s\n", sha1_to_hex(sha1));
    return 0;
}

/*
 * Function:`write_sha1_buffer`
 * Parameters:
 *      -sha1: The SHA1 hash of the deflated object to be written into the 
 *             object store.
 *      -buf:  The content to be written to the object store.
 *      -size: The size of the content to be written into the object store.
 * Purpose: Write an object to the object database, using the object's SHA1 
 *          hash value as index.
 */
int write_sha1_buffer(unsigned char *sha1, void *buf, unsigned int size)
{
    /*
     * Build the path of the object in the object database using the object's 
     * SHA1 hash.
     */
    char *filename = sha1_file_name(sha1);
    int i;    /* Unused variable. Even Linus Torvalds makes mistakes. */
    int fd;   /* File descriptor for the file to be written. */

    /* Open a new file in the object store and associate it with `fd`. */
    fd = OPEN_FILE(filename, O_WRONLY | O_CREAT | O_EXCL, 0666);

    /* Error if failure occurs when opening the file. */
    if (fd < 0)
        return (errno == EEXIST) ? 0 : -1;

    write(fd, buf, size);   /* Write the object to the object store. */
    close(fd);              /* Release the file descriptor. */
    return 0;
}

/*
 * Function: `error`
 * Paramters:
 *      -string: The error message to print.
 * Purpose: Print an error message to the standard error stream.
 */
static int error(const char * string)
{
    fprintf(stderr, "error: %s\n", string);
    return -1;
}

/*
 * Function: `verify_header`
 * Parameters:
 *      -hdr: A pointer to the cache header structure to validate.
 *      -size: The size in bytes of the cache file.
 * Purpose: Validate a cache_header.
 */
static int verify_hdr(struct cache_header *hdr, unsigned long size)
{
    SHA_CTX c;                /* Declare a SHA context. */
    unsigned char sha1[20];   /* Array to store SHA1 hash. */

    /*
     * Ensure the cache_header's signature matches the value defined in 
     * "cache.h". 
     */
    if (hdr->signature != CACHE_SIGNATURE)
        return error("bad signature");

    /* Ensure the cache_header was created with the correct version of Git. */
    if (hdr->version != 1)
        return error("bad version");

    /* Initialize the SHA context `c`. */
    SHA1_Init(&c); 

    /* Calculate the hash of the cache header and cache entries. */ 
    SHA1_Update(&c, hdr, offsetof(struct cache_header, sha1));
    SHA1_Update(&c, hdr+1, size - sizeof(*hdr));
    SHA1_Final(sha1, &c);

    /*
     * Compare the SHA1 hash calculated above to the SHA1 hash stored in the 
     * cache header. If they match, then the cache is valid.
     */
    if (memcmp(sha1, hdr->sha1, 20))
        return error("bad header sha1");
    return 0;
}

/*
 * Function: `read_cache`
 * Parameters: none
 * Purpose: Reads the cache entries in the `.dircache/index` file into the 
  8         `active_cache` array.
 */
int read_cache(void)
{
    int fd;           /* File descriptor. */
    int  i;           /* For loop iteration variable. */
    struct stat st;   /* `stat` structure for storing file information. */
    unsigned long size, offset;
    /*
     * Used to store the memory address in which to map the contents of the 
     * `.dircache/index` cache file.  
     */
    void *map; 
    /* Declare a pointer to a cache header, as defined in "cache.h". */
    struct cache_header *hdr; 

    /* Check if active_cache array is already populated. */
    errno = EBUSY;
    if (active_cache) 
        return error("more than one cachefile");

    /*
     * Get the path to the object store by first checking if anything is 
     * stored in the `DB_ENVIRONMENT` environment variable. If not, use the 
     * default path specified in `DEFAULT_DB_ENVIRONMENT`, which is 
     * `.dircache/objects`. Then check if the directory can be accessed.
     */
    errno = ENOENT;
    sha1_file_directory = getenv(DB_ENVIRONMENT);
    if (!sha1_file_directory)
        sha1_file_directory = DEFAULT_DB_ENVIRONMENT;
    if (access(sha1_file_directory, X_OK) < 0)
        return error("no access to SHA1 file directory");

    /*
     * Open the `.dircache/index` cache file and associate it with the `fd` 
     * file descriptor (just an integer).
     */
    #ifndef BGIT_WINDOWS
    fd = open(".dircache/index", O_RDONLY );
    #else
    fd = open(".dircache/index", O_RDONLY | O_BINARY );
    #endif
    /*
     * Return if file does not exist or if there was an error opening the
     * file.
     */
    if (fd < 0)
        return (errno == ENOENT) ? 0 : error("open failed");

    /*
     * I had to look up what this `(void *)-1` means. Apparently it is a
     * reference to an always invalid memory location that would not be
     * able to be returned in the event of successful operation.
     */
    #ifndef BGIT_WINDOWS
    map = (void *)-1;
    #else
    map = (void *) NULL;
    #endif

    /*
     * Get the cache file information and store it in the `st` stat structure.
     * Execute the `if` block if the `fstat()` command returns 0, i.e., is 
     * successful.
     */
    if (!fstat(fd, &st)) {
        map = NULL;

        /*
         * Set `size` equal to the `st_size` member of the `st` structure, 
         * which is the size of the `.dircache/index` file in bytes.
         */
        size = st.st_size;

        /*
         * Preset the error code to be returned to invalid argument if an 
         * error occurs. 
         */
        errno = EINVAL; 

        /*
         * Check to make sure the size of the returned index file is greater
         * than the size of the `cache_header` structure. This must be true 
         * for a valid cache since it must be made up of a cache header and 
         * at least one cache entry.
         */
        if (size > sizeof(struct cache_header)) {
            /*
             * Map the contents of the `.dircache/index` cache file to memory 
             * and return a pointer to that space. For more details on the
             * `mmap()` function, see:
             *
             * http://pubs.opengroup.org/onlinepubs/009695399/functions/
             * mmap.html
             * https://stackoverflow.com/questions/258091/
             * when-should-i-use-mmap-for-file-access
             */
            #ifndef BGIT_WINDOWS
            map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
            #else
            void *fhandle 
                = CreateFileMapping( (HANDLE) _get_osfhandle(fd), 
                                     NULL, PAGE_READONLY, 0, 0, NULL );
            if (!fhandle)
                return error("CreateFileMapping failed");
            map = MapViewOfFile( fhandle, FILE_MAP_READ, 0, 0, size );
            CloseHandle( fhandle );
            #endif
        }
    }

    /*
     * Deallocate the file descriptor `fd` so that it is available for future
     * calls to `open()`. For example, if fd = 1, the file descriptor 1 will 
     * be deallocated and be made avaliable to refer to files. 
     */
    close(fd);

    /*
     * Display an error message and return -1 if `mmap()` failed to map the 
     * file to memory. 
     */
    #ifndef BGIT_WINDOWS
    if (-1 == (int)(long)map)
        return error("mmap failed");
    #else
    if (map == (void *) NULL)
        return error("MapViewOfFile failed");
    #endif

    /*
     * Set the `hdr` cache header pointer to point to the memory address where 
     * the `.dircache/index` file's contents were mapped. Then call 
     * `verify_hdr()` to validate that the cache header. If the header is not 
     * valid, the code jumps to the `unmap` label at the end of this file.
     */
    hdr = map;
    if (verify_hdr(hdr, size) < 0)
        goto unmap;

    /* The number of cache entries in the cache. */
    active_nr = hdr->entries; 
    /* The maximum number of elements the active_cache array can hold. */
    active_alloc = alloc_nr(active_nr); 

    /*
     * Allocate memory for the `active_cache` array. Memory is allocated for
     * an array of `active_alloc` number of elements, each one having the size 
     * of a `cache_entry` structure.
     */
    active_cache = calloc(active_alloc, sizeof(struct cache_entry *));

    /*
     * `offset` is an index to the next byte of `map` to read. In this case,
     * set it to the beginning of the first cache entry after the header. 
     */
    offset = sizeof(*hdr); 

    /*
     * Add each cache entry into the `active_cache` array and increase the 
     * `offset` index by the size of the current cache entry..
     */
    for (i = 0; i < hdr->entries; i++) {
        struct cache_entry *ce = map + offset;
        offset = offset + ce_size(ce);
        active_cache[i] = ce;
    }
    
    /* Return the number of cache entries in the cache. */
    return active_nr;

/*
 * The lines of code after the 'unmap' label are only executed if the cache 
 * header is invalid. In that case, the mapping between the cache file and 
 * memory is removed to prevent memory leaks. Then display an error message 
 * and return -1.
 */
unmap:
    #ifndef BGIT_WINDOWS
    munmap(map, size);
    #else
    UnmapViewOfFile( map );
    #endif
    errno = EINVAL;
    return error("verify header failed");
}


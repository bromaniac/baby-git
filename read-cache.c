/*
 *  GIT - The information manager from hell
 *
 *  Copyright (C) Linus Torvalds, 2005
 *
 *  The purpose of this file is to be compiled is to define helper functions
 *  that the other files in the codebase use to interact with the object store
 *  and the index. This includes helper functions for reading and writing from
 *  the object store and index and validating existing cache_entries.
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

    -deflateBound(z_stream, sourceLen): Returns an upper bound on the compressed
                                        size after deflation of `sourceLen` bytes.

	-inflateInit(z_stream): Initializes the internal `z_stream` state
                            for decompression. Sourced from <zlib.h>.

    -Z_BEST_COMPRESSION: Translates to integer `9` which as an input to
                         `deflateInit()` indicates to optimize compressed
                         size as opposed to compression speed. Sourced from
                         <zlib.h>.

    -deflate(z_stream, flush): Compresses as much data as possible, and stops
                               when the input buffer becomes empty or the
                               output buffer becomes full. Sourced from <zlib.h>.

	-inflate(z_stream, flush): Decompresses as much data as possible, and stops
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

    -O_CREAT: Flag for the open() function indicating if the file exists,
              this flag has no effect except as noted under O_EXCL below.
              Otherwise, the file shall be created. Sourced from <fcntl.h>.

    -O_EXCL: Flag for the open() function indicating if O_CREAT and O_EXCL
             are set, open() shall fail if the file exists.

    -fprintf(stream, message): Place `message` on the named output
                               `stream`. Sourced from <stdio.h>.
	
	-sscanf(s, format): Read from the string `s`.

    ****************************************************************

    The following variables and functions are defined locally.

    -sha1_file_directory: The path to the object store.

    -active_cache: An array of cache_entries representing the current set of content being cached/retrieved from cache.

    -active_nr: The # of cache_entries in the active_cache.

    -active_alloc: The converted # of elements in the active_cache.

    -usage(): Print a usage message in the event of incorrect usage.

    -hexval(): Convert a hexadecimal character to it's decimal representation.

    -get_sha1_hex(): TODO

    -sha1_to_hex(): TODO

    -sha1_file_name(): TODO

    -read_sha1_file(): TODO

    -write_sha1_file(buf, len): Compress file content stored in `buf`, hash compressed output, and write to object store.

    -write_sha1_buffer(): Write compressed content to location in object store identified
                          by the content's SHA1 hash.

    -error(): Print an error message to the standard error stream.

    -verify_hdr(): Validate a cache_header.

    -read_cache(): Reads the contents of the `.dircache/index` file into the `active_cache` array.

*/

const char *sha1_file_directory = NULL; /* Used to store the path to the object store. */
struct cache_entry **active_cache = NULL; /* An array of cache_entries representing the current set of content being cached/retrieved from cache. */
unsigned int active_nr = 0; /* The # of cache_entries in the active_cache. */
unsigned int active_alloc = 0; /* The converted # of elements in the active_cache. */

/*
 * Function: `usage`
 * Parameters:
 *      -err: Error message to display.
 * Purpose: Display an error message when reading the cache fails.
 */
void usage(const char *err)
{
	fprintf(stderr, "read-tree: %s\n", err);
	exit(1);
}

/* Function: `hexval`
 * Parameters:
 *      -c: Hexadecimal character to convert to decimal.
 * Purpose: Convert a hexadecimal character to it's decimal representation.
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

    /* Return bitwise 'not' of 0, which is 1, this indicates that `c` was not hex, i.e. failure. */
	return ~0;
}

/*
 * Function: `get_sha1_hex`
 * Parameters:
 *      -hex: Hexadecimal string to convert to SHA1 representation.
 *      -sha1: String placeholder to store converted SHA1 value.
 * Purpose: Convert hexadecimal string to SHA1 representation.
 */
int get_sha1_hex(char *hex, unsigned char *sha1)
{
	int i;
	for (i = 0; i < 20; i++) {
		unsigned int val = (hexval(hex[0]) << 4) | hexval(hex[1]);
		if (val & ~0xff)
			return -1;
		*sha1++ = val;
		hex += 2;
	}
	return 0;
}

/*
 * Function: `sha1_to_hex`
 * Parameters:
 *      -sha1: SHA1 to convert to hexadecimal representation.
 * Purpose: Convert SHA1 string to hexadecimal representation.
 */
char * sha1_to_hex(unsigned char *sha1)
{
	static char buffer[50];
	static const char hex[] = "0123456789abcdef";
	char *buf = buffer;
	int i;

	for (i = 0; i < 20; i++) {
		unsigned int val = *sha1++;
		*buf++ = hex[val >> 4];
		*buf++ = hex[val & 0xf];
	}
	return buffer;
}

/*
 * NOTE! This returns a statically allocated buffer, so you have to be
 * careful about using it. Do a "strdup()" if you need to save the
 * filename.
 *
 * Function: `sha1_file_name`
 * Parameters:
 *      -sha1: The SHA1 used to identify the file in the object store.
 * Purpose: Return the file name (path and filename) of a file in the object store.
 */
char *sha1_file_name(unsigned char *sha1)
{
	int i;
	static char *name, *base;

	if (!base) {
		char *sha1_file_directory = getenv(DB_ENVIRONMENT) ? : DEFAULT_DB_ENVIRONMENT;
		int len = strlen(sha1_file_directory);
		base = malloc(len + 60);
		memcpy(base, sha1_file_directory, len);
		memset(base+len, 0, 60);
		base[len] = '/';
		base[len+3] = '/';
		name = base + len + 1;
	}
	for (i = 0; i < 20; i++) {
		static char hex[] = "0123456789abcdef";
		unsigned int val = sha1[i];
		char *pos = name + i*2 + (i > 0);
		*pos++ = hex[val >> 4];
		*pos = hex[val & 0xf];
	}
	return base;
}

/*
 * Function: `read_sha1_file`
 * Parameters:
 *      -sha1: SHA1 that identifies the file to read from the object store.
 *      -type: The type of the file to be read in.
 *      -size: The size in bytes of the file to be read in.
 * Purpose: Read in a file from the object store identified by it's SHA1.
 */
void * read_sha1_file(unsigned char *sha1, char *type, unsigned long *size)
{
	z_stream stream; /* Declare zlib compression stream. */
	char buffer[8192]; /* Character array to hold file content. */
	struct stat st; /* The `stat` object to hold file info of the file to be read. */
	int i; /* Not used. Even almighty Linux makes mistakes. */
    int fd; /* File descriptor: Integer to be associated with the file to be read. */
    int ret;
    int bytes;
	void *map, *buf;
	char *filename = sha1_file_name(sha1); /* Get the name of the file to be read in, identified by the passed-in SHA1. */

    /*
     * Associate a file descriptor with the file in the object store.
     * If the returned value is < 0, there was an error reading the file.
     */
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		perror(filename);
		return NULL;
	}

    /*
     * Get the file's info and release the file descriptor if there was an error.
     */
	if (fstat(fd, &st) < 0) {
		close(fd);
		return NULL;
	}

    /* Set up memory location to store the contents of the file to be added to cache. */
    map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd); /* Release the file descriptor for future use. */
	if (-1 == (int)(long)map) /* Return NULL (failure) if mmap failed. */
		return NULL;

	/*  
     * Allocate `sizeof(stream)`'s worth of unsigned char space to the compression stream.
    */
    memset(&stream, 0, sizeof(stream));
	stream.next_in = map; /* Set mmap'ed location as the first addition to the compression stream. */
	stream.avail_in = st.st_size; /* Allow size of object store file to be added to the stream. */
	stream.next_out = buffer; /* Set the `buffer` as the output destination for the uncompressed file content. */
	stream.avail_out = sizeof(buffer); /* Set the output size of the stream to the size of the buffer. */

	inflateInit(&stream); /* Initialize the stream for decompression. */
	ret = inflate(&stream, 0); /* Decompress the file contents and store return code in `ret` */

	/* Add the file type and size to the buffer. Fail if the 2 inputs are not successfully added. */
	if (sscanf(buffer, "%10s %lu", type, size) != 2)
		return NULL;

	bytes = strlen(buffer) + 1; /* The length in bytes of the buffer. */
	buf = malloc(*size); /* Allocate `size` bytes for `buf`. */
	if (!buf) /* Error if space could not be allocated. */
		return NULL;

	memcpy(buf, buffer + bytes, stream.total_out - bytes);
	bytes = stream.total_out - bytes;
	if (bytes < *size && ret == Z_OK) {
		stream.next_out = buf + bytes;
		stream.avail_out = *size - bytes;
		while (inflate(&stream, Z_FINISH) == Z_OK)
			/* nothing */;
	}
	inflateEnd(&stream);
	return buf;
}

/*
 * Function: `write_sha1_file`
 * Parameters:
 *      -buf: The content to be written to the object store pre-compression.
 *      -len: The length in bytes of the content pre-compression.
 * Purpose: Compress file content stored in `buf`, hash compressed output, and write to object store.
 */
int write_sha1_file(char *buf, unsigned len)
{
	int size;
	char *compressed;
	z_stream stream; /* Declare zlib compression stream. */
	unsigned char sha1[20]; /* Used to store SHA1 hash. */
	SHA_CTX c; /* Declare SHA context. */

	/*
     * Allocate `sizeof(stream)`'s worth of unsigned char space to the compression stream.
     */
	memset(&stream, 0, sizeof(stream));

    /* Initialize compression stream optimized for compression (as opposed to speed). */
	deflateInit(&stream, Z_BEST_COMPRESSION);
	size = deflateBound(&stream, len); /* Determine upper bound for content after compression. */
	compressed = malloc(size); /* Allocate `size` bytes for the compressed content. */

	stream.next_in = buf; /* Read file content into the compression stream. */
	stream.avail_in = len; /* Allow `len` bytes to be compressed. */
	stream.next_out = compressed; /* Output the compressed content to `compressed`. */
	stream.avail_out = size; /* Use the upper bound calculated above as max compression output. */

	while (deflate(&stream, Z_FINISH) == Z_OK) /* Compress the file content. */
		/* nothing */;
	deflateEnd(&stream); /* Free memory structures that were dynamically allocated for compression. */
	size = stream.total_out; /* Get size of total output of compression stream. */

	SHA1_Init(&c); /* Initialize the SHA context. */
	SHA1_Update(&c, compressed, size); /* Hash the compressed file content. */
	SHA1_Final(sha1, &c); /* Set `sha1` to the SHA1 hash of the compressed file content. */

	/* Write the compressed content to the object store. */
	if (write_sha1_buffer(sha1, compressed, size) < 0)
		return -1;
	printf("%s\n", sha1_to_hex(sha1));
	return 0;
}

/*
 * Function:`write_sha1_buffer`
 * Parameters:
 *      -sha1: The SHA1 hash of the content to be written into the object store.
 *      -buf:  The actual content to write into the file in the object store.
 *      -size: The size of the content to be written into the object store.
 *
 * Purpose: Write compressed content to location in object store identified
 *          by the content's SHA1 hash.
 */
int write_sha1_buffer(unsigned char *sha1, void *buf, unsigned int size)
{
    /*
     * Convert the `sha1` to a filename to name the file in the object store.
     */
	char *filename = sha1_file_name(sha1);
	int i; /* Unused variable. Even Linus Torvalds makes mistakes. */
    int fd; /* Will be used to store association to the new object store file. */

    /* Associate `fd` with a newly created file in the object store to hold the content. */
	fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0666);

    /* Error if failure occurs during file association. */
	if (fd < 0)
		return (errno == EEXIST) ? 0 : -1;

	write(fd, buf, size); /* Write the content to the new file in the object store. */
	close(fd); /* Release the file descriptor for future file associations. */
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
 *      -hdr: A pointer to the cache_header to validate.
 *      -size: The size in bytes of the cache_header.
 * Purpose: Validate a cache_header.
 */
static int verify_hdr(struct cache_header *hdr, unsigned long size)
{
	SHA_CTX c; /* Declare a SHA context. */
	unsigned char sha1[20]; /* Declare placeholder for SHA1. */

    /* Ensure the cache_header's signature matches the value defined in <cache.h>. */
	if (hdr->signature != CACHE_SIGNATURE)
		return error("bad signature");

    /* Ensure the cache_header was created with the correct version of git. */
	if (hdr->version != 1)
		return error("bad version");
	SHA1_Init(&c); /* Initialize the SHA context `c`. */

    /*
     *  Hash the cache_header and cache_entries to get the corresponding SHA1 of the content.
     */
	SHA1_Update(&c, hdr, offsetof(struct cache_header, sha1));
	SHA1_Update(&c, hdr+1, size - sizeof(*hdr));
	SHA1_Final(sha1, &c);

    /*
     * Compare the newly generated SHA1 hash to the value stored in the passed-in cache_header.
     * If it matches, then the cache is valid.
     */
	if (memcmp(sha1, hdr->sha1, 20))
		return error("bad header sha1");
	return 0;
}

/*
 * Type: function
 * Returns: integer representing # of active entries (or files) in the `.dircache/index` file.
 * Parameters: none
 * Purpose: Reads the contents of the `.dircache/index` file into the `active_cache` array.
 */
int read_cache(void)
{
	int fd; /* Use this to store reference to file descriptor. */
    int  i; /* For loop iteration variable. */
	struct stat st; /* Store output of `stat` command (file info). */
	unsigned long size, offset;
	void *map; /* Used to store the memory address at which to map the contents of the `.dircache/index` cache file.  */
	struct cache_header *hdr; /* Declare a point to a cache header as defined in <cache.h>. */

	/*
	 * `active_cache` refers to the variable defined at top of this file, which is
	 * always `NULL` when this file runs, so I don't think this error will ever
	 * be triggered.
	 */
	errno = EBUSY;
    if (active_cache) 
		return error("more than one cachefile");

	/*
	 * Get the path to the object store by first checking if anything is stored
	 * in the `DB_ENVIRONMENT` environment variable, and if not just using the
	 * default `DEFAULT_DB_ENVIRONMENT` which is `.dircache/objects`. Then check
	 * to make sure we have filesystem access to the directory.
	 */
	errno = ENOENT;
	sha1_file_directory = getenv(DB_ENVIRONMENT);
	if (!sha1_file_directory)
		sha1_file_directory = DEFAULT_DB_ENVIRONMENT;
	if (access(sha1_file_directory, X_OK) < 0)
		return error("no access to SHA1 file directory");

    /*
     * Associate the file descriptor `fd` (just an integer) with the
     * `.dircache/index` file, i.e. the current cache.
     */
	fd = open(".dircache/index", O_RDONLY);
	if (fd < 0)
		return (errno == ENOENT) ? 0 : error("open failed");

    /*
     * I had to look up what this `(void *)-1` means. Apparently it is a
     * reference to an always invalid memory location that would not be
     * able to be returned in the event of successful operation.
     */
	map = (void *)-1;

    /*
     * Populate the `st` variable with the output of `fstat()` operating on
     * the file reference `fd` defined above. Run the if statement if the
     * `fstat()` returns 0 (i.e. is successful).
     */
	if (!fstat(fd, &st)) {
		map = NULL;

        /*
         * Set `size` equal the `st_size` member of `st`, which is equal to
         * the size of the `.dircache/index` file in bytes.
         */
		size = st.st_size;

		errno = EINVAL; /* Preset the error code to be returned if an error occurs. */

        /*
         * Check to make sure the size of the returned index file is greater
         * than the size of the `cache_header` struct. This must be true for
         * a valid cache since it must be made up of a `cache_header` and at
         * least 1 `cache_entry`.
         */
		if (size > sizeof(struct cache_header))
            /*
             * Read the `.dircache/index` file's content into memory locaiton
             * referenced by `map`. Think of this as just a way to read in the
             * content stored in the cache into memory, so that it can be
             * accessed by this program. for more details on `mmap()` see:
             *
             * -http://pubs.opengroup.org/onlinepubs/009695399/functions/mmap.html
             * -https://stackoverflow.com/questions/258091/when-should-i-use-mmap-for-file-access
             */
			map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
	}

    /*
     * Deallocate the integer used by file descriptor `fd` so that it is available for future
     * calls to `open()`. For example if `fd` = 1, the number 1 will be freed up to be mapped
     * future files.
     */
	close(fd);

    /* Return an error in the event of `mmap()` failure to map the file to memory. */
	if (-1 == (int)(long)map)
		return error("mmap failed");

    /*
     * Point the `hdr` cache header pointer to the memory address where the `.dircache/index`
     * file's contents were mapped. Then run `verify_hdr()` to validate that the cache file
     * contains a valid header.  See `verify_hdr()` function above in this file to see how this
     * works. If the header is not valid, the code jumps to the `unmap` code at the end of this file.
     */
	hdr = map;
	if (verify_hdr(hdr, size) < 0)
		goto unmap;

	active_nr = hdr->entries; /* The # of cache entries (or files) that exist in the cache. */
	active_alloc = alloc_nr(active_nr); /* The # of elements to allocate for the `active_cache`. */

    /*
     * Allocate memory for the `active_cache`. The memory is allocated for an array of `active_alloc`
     * elements, each one having the size of a `cache_entry`.
     */
	active_cache = calloc(active_alloc, sizeof(struct cache_entry *));

    /*
     * This `offset` is used to skip past the header portion of the file content and iterate
     * over the cache_entries.
     */
	offset = sizeof(*hdr); 

    /*
     * Iterate over the cache_entries, each time increasing the offset by the size of the current
     * cache_entry. Add each cache_entry into the `active_cache` array, so we end up with an
     * array of populated cache_entries.
     */
	for (i = 0; i < hdr->entries; i++) {
		struct cache_entry *ce = map + offset;
		offset = offset + ce_size(ce);
		active_cache[i] = ce;
	}
    
    /* Return the number of cache_entries in the active_cache. */
	return active_nr;

/*
 * This 'unmap' code only runs if the cache header is invalid. In that case, we want to unmap the
 * memory location that is being used to store the file contents, so it doesn't hog up that space. 
 */
unmap:
	munmap(map, size);
	errno = EINVAL;
	return error("verify header failed");
}


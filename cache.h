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
 *  The purpose of this file is to define and include the required
 *	libraries, function signatures, and defaults for the Git `.c`
 *  programs to function. This file <cache.h> is included in all
 *  the `.c` files in this same (root) directory including:
 *
 *  cat-file.c, commit-tree.c, init-db.c, read-cache.c, read-tree.c,
 *  show-diff.c, update-cache.c, write-tree.c
 *
 */

/*
 * Only run this code in this file if `CACHE_H` has not been
 * defined yet. This is is to prevent multiple compilations of the
 * same code since it's included in multiple `.c` files.
 */
#ifndef CACHE_H 
#define CACHE_H /* Define the token `CACHE_H`. */

#include <stdio.h> /* Standard C library defining input/output tools. */
#include <string.h>
#include <unistd.h>
#include <sys/stat.h> /* Standard C library defining `stat` tools. */
#include <fcntl.h> /* Standard C library for working with files. */
#include <stddef.h> /* Standard C library for type definitions. */
#include <stdlib.h> /* Standard C library for library definitions. */
#include <stdarg.h> /* Standard C library for variable argument lists. */
#include <errno.h> /* Standard C library for system error numbers. */

#ifndef BGIT_WINDOWS
    #include <sys/mman.h> /* Standard C library for memory management declarations. */
#else
    #define SECURITY_WIN32
    #include <windows.h>
    #include <winsock2.h>
    #include <lmcons.h>
    #include <direct.h>
    #include <secext.h>
    #include <sys/mman.h>
#endif

#include <openssl/sha.h> /* Include SHA hash tools from openssl library. */
#include <zlib.h> /* Include compression tools from zlib library. */

/*
 * Linus Torvalds: Basic data structures for the directory cache.
 *
 * Linus Torvalds: NOTE NOTE NOTE! This is all in the native CPU byte format. It's
 * not even trying to be portable. It's trying to be efficient. It's
 * just a cache, after all.
 */

/* This `CACHE_SIGNATURE` is hardcoded to be loaded into all cache_headers. */
#define CACHE_SIGNATURE 0x44495243	/* Linus Torvalds: "DIRC" */

#ifdef BGIT_UNIX
    #define STAT_TIME_SEC( st, st_xtim ) ( (st)->st_xtim ## e )
    #define STAT_TIME_NSEC( st, st_xtim ) ( (st)->st_xtim.tv_nsec )

#elif defined BGIT_DARWIN
    #define STAT_TIME_SEC( st, st_xtim ) ( (st)->st_xtim ## espec.tv_sec )
    #define STAT_TIME_NSEC( st, st_xtim ) ( (st)->st_xtim ## espec.tv_nsec )

#elif defined BGIT_WINDOWS
    #define STAT_TIME_SEC( st, st_xtim ) ( (st)->st_xtim ## e )
    #define STAT_TIME_NSEC( st, st_xtim ) 0
#endif

/* Represents a header structure to identify a set of cache_entries. */
struct cache_header {
	unsigned int signature; /* Constant across all headers, to validate authenticity. */
	unsigned int version; /* Stores a reference to the version of Git that created the cache. */
	unsigned int entries; /* The number of cache_entries in the cache. */
	unsigned char sha1[20]; /* The SHA1 hash that identifies the cached content. */
};

/*
 * `cache_time` represents a time associated with a particular action take on a cache_entry.
 * For example, the time the entry was modified. For some info on file times, see:
 * https://www.quora.com/What-is-the-difference-between-mtime-atime-and-ctime
 */
struct cache_time {
	unsigned int sec;
	unsigned int nsec;
};

/*
 * A `cache_entry` represents hashed file contents of a file.
 * Hashing occurs post-compression.
 */
struct cache_entry {

	struct cache_time ctime; /* The change time of the entry. */
	struct cache_time mtime; /* The modification time of the entry. */
	unsigned int st_dev; /* Identifies the device containing the file. */

    /* 
     * The file serial number, which distinguishes this file from all
     * other files on the same device.
     */
	unsigned int st_ino;

    /*
     * Specifies the mode of the file. This includes file type information
     * and the file permission bits.
     */
	unsigned int st_mode;
	unsigned int st_uid; /* The user ID of the file’s owner. */
	unsigned int st_gid; /* The group ID of the file. */
	unsigned int st_size; /* This specifies the size of a regular file in bytes. */
	unsigned char sha1[20]; /* The SHA1 hash of the entry's file content. */
	unsigned short namelen; /* The length of the cached file's name. */
	unsigned char name[0]; /* The cached file's name. */
};

const char *sha1_file_directory; /* The path to the object store. */
struct cache_entry **active_cache; /* An array of cache entries which we call the `active_cache`. */
unsigned int active_nr; /* The number of entries in the `active_cache` array. */
unsigned int active_alloc; /* The result of calling `alloc_nr()` macro with `active_nr` as an argument. */

/*
 * If desired, you can use an environment variable name to set a custom path to
 * the object store.
 */
#define DB_ENVIRONMENT "SHA1_FILE_DIRECTORY"

/*
 * The default path to the object store.
 */
#define DEFAULT_DB_ENVIRONMENT ".dircache/objects"

/* These macros are used to calculate the size of a cache_entry. */
#define cache_entry_size(len) ((offsetof(struct cache_entry,name) + (len) + 8) & ~7)
#define ce_size(ce) cache_entry_size((ce)->namelen)

/*
 * See this link for details on this macro:
 * https://stackoverflow.com/questions/22090101/why-is-define-alloc-nrx-x163-2-macro-used-in-many-cache-h-files
 */
#define alloc_nr(x) (((x)+16)*3/2)

/* Reads the contents of the `.dircache/index` file into the `active_cache` array. */
extern int read_cache(void);

/* Linus Torvalds: Return a statically allocated filename matching the SHA1 signature */
extern char *sha1_file_name(unsigned char *sha1);

/* Linus Torvalds: Write a memory buffer out to the SHA1 file. */
extern int write_sha1_buffer(unsigned char *sha1, void *buf, unsigned int size);

/* Linus Torvalds: Read and unpack a SHA1 file into memory, write memory to a SHA1 file. */
extern void * read_sha1_file(unsigned char *sha1, char *type, unsigned long *size);
extern int write_sha1_file(char *buf, unsigned len);

/* Linus Torvalds: Convert to/from hex/sha1 representation. */
extern int get_sha1_hex(char *hex, unsigned char *sha1);
extern char *sha1_to_hex(unsigned char *sha1);	/* Linus Torvalds: static buffer! */

/* Print usage message to the command line. */
extern void usage(const char *err);

#endif /* Linus Torvalds: CACHE_H */
